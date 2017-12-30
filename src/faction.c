/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file faction.c
 *
 * @brief Handles the Naev factions.
 */


#include "faction.h"

#include "naev.h"

#include <stdlib.h>
#include "nstring.h"

#include "nxml.h"

#include "nlua.h"
#include "nluadef.h"
#include "opengl.h"
#include "log.h"
#include "ndata.h"
#include "rng.h"
#include "colour.h"
#include "hook.h"
#include "space.h"


#define XML_FACTION_ID     "Factions"   /**< XML section identifier */
#define XML_FACTION_TAG    "faction" /**< XML tag identifier. */


#define CHUNK_SIZE         32 /**< Size of chunk for allocation. */

#define FACTION_STATIC        (1<<0) /**< Faction doesn't change standing with player. */
#define FACTION_INVISIBLE     (1<<1) /**< Faction isn't exposed to the player. */
#define FACTION_KNOWN         (1<<2) /**< Faction is known to the player. */

#define faction_setFlag(fa,f) ((fa)->flags |= (f))
#define faction_rmFlag(fa,f)  ((fa)->flags &= ~(f))
#define faction_isFlag(fa,f)  ((fa)->flags & (f))
#define faction_isKnown_(fa)   ((fa)->flags & (FACTION_KNOWN))

/**
 * @struct Faction
 *
 * @brief Represents a faction.
 */
typedef struct Faction_ {
   char *name; /**< Normal Name. */
   char *longname; /**< Long Name. */
   char *displayname; /**< Display name. */

   /* Graphics. */
   glTexture *logo_small; /**< Small logo. */
   glTexture *logo_tiny; /**< Tiny logo. */
   const glColour *colour; /**< Faction specific colour. */

   /* Enemies */
   int *enemies; /**< Enemies by ID of the faction. */
   int nenemies; /**< Number of enemies. */

   /* Allies */
   int *allies; /**< Allies by ID of the faction. */
   int nallies; /**< Number of allies. */

   /* Player information. */
   double player_def; /**< Default player standing. */
   double player; /**< Standing with player - from -100 to 100 */

   /* Scheduler. */
   nlua_env sched_env; /**< Lua scheduler script. */

   /* Behaviour. */
   nlua_env env; /**< Faction specific environment. */

   /* Equipping. */
   nlua_env equip_env; /**< Faction equipper enviornment. */

   /* Flags. */
   unsigned int flags; /**< Flags affecting the faction. */
} Faction;

static Faction* faction_stack = NULL; /**< Faction stack. */
int faction_nstack = 0; /**< Number of factions in the faction stack. */


/*
 * Prototypes
 */
/* static */
static void faction_sanitizePlayer( Faction* faction );
static void faction_modPlayerLua( int f, double mod, const char *source, int secondary );
static int faction_parse( Faction* temp, xmlNodePtr parent );
static void faction_parseSocial( xmlNodePtr parent );
/* externed */
int pfaction_save( xmlTextWriterPtr writer );
int pfaction_load( xmlNodePtr parent );


/**
 * @brief Gets a faction ID by name.
 *
 *    @param name Name of the faction to seek.
 *    @return ID of the faction.
 */
int faction_get( const char* name )
{
   int i;

   /* Escorts are part of the "player" faction. */
   if (strcmp(name, "Escort") == 0)
      return FACTION_PLAYER;

   if (name != NULL) {
      for (i=0; i<faction_nstack; i++)
         if (strcmp(faction_stack[i].name, name)==0)
            break;

      if (i != faction_nstack)
         return i;
   }

   WARN(_("Faction '%s' not found in stack."), name);
   return -1;
}


/**
 * @brief Gets all the factions.
 */
int* faction_getAll( int *n )
{
   int i;
   int *f;
   int m;

   /* Set up. */
   f  = malloc( sizeof(int) * faction_nstack );

   /* Get IDs. */
   m = 0;
   for (i=0; i<faction_nstack; i++)
      if (!faction_isFlag( &faction_stack[i], FACTION_INVISIBLE ))
         f[m++] = i;

   *n = m;
   return f;
}

/**
 * @brief Gets all the known factions.
 */
int* faction_getKnown( int *n )
{
   int i;
   int *f;
   int m;

   /* Set up. */
   f  = malloc( sizeof(int) * faction_nstack );

   /* Get IDs. */
   m = 0;
   for (i=0; i<faction_nstack; i++)
      if (!faction_isFlag( &faction_stack[i], FACTION_INVISIBLE ) && faction_isKnown_( &faction_stack[i] ))
         f[m++] = i;

   *n = m;
   return f;
}

/**
 * @brief Clears the known factions.
 */
void faction_clearKnown()
{
   int i;

   for ( i=0; i<faction_nstack; i++)
      if ( faction_isKnown_( &faction_stack[i] ))
         faction_rmFlag( &faction_stack[i], FACTION_KNOWN );
}

/**
 * @brief Is the faction invisible?
 */
int faction_isInvisible( int id )
{
   return faction_isFlag( &faction_stack[id], FACTION_INVISIBLE );
}

/**
 * @brief Sets the faction's invisible state
 */
int faction_setInvisible( int id, int state )
{
   if (!faction_isFaction(id)) {
      WARN(_("Faction id '%d' is invalid."),id);
      return -1;
   }
   if (state)
      faction_setFlag( &faction_stack[id], FACTION_INVISIBLE );
   else
      faction_rmFlag( &faction_stack[id], FACTION_INVISIBLE );

   return 0;
}

/**
 * @brief Is the faction known?
 */
int faction_isKnown( int id )
{
   return faction_isKnown_( &faction_stack[id] );
}

/**
 * @brief Sets the factions known state
 */
int faction_setKnown( int id, int state )
{
   if (state)
      faction_setFlag( &faction_stack[id], FACTION_KNOWN );
   else
      faction_rmFlag( &faction_stack[id], FACTION_KNOWN );

   return 0;
}

/**
 * @brief Gets a factions "real" name.
 *
 *    @param f Faction to get the name of.
 *    @return Name of the faction.
 */
char* faction_name( int f )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }
   /* Don't want player to see their escorts as "Player" faction. */
   if (f == FACTION_PLAYER)
      return "Escort";

   return faction_stack[f].name;
}


/**
 * @brief Gets a factions short name.
 *
 *    @param f Faction to get the name of.
 *    @return Name of the faction.
 */
char* faction_shortname( int f )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }
   /* Don't want player to see their escorts as "Player" faction. */
   if (f == FACTION_PLAYER)
      return "Escort";

   /* Possibly get display name. */
   if (faction_stack[f].displayname != NULL)
      return faction_stack[f].displayname;

   return faction_stack[f].name;
}


/**
 * @brief Gets the faction's long name (formal).
 *
 *    @param f Faction to get the name of.
 *    @return The faction's long name.
 */
char* faction_longname( int f )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }
   if (faction_stack[f].longname != NULL)
      return faction_stack[f].longname;
   return faction_stack[f].name;
}


/**
 * @brief Gets the faction's small logo (64x64 or smaller).
 *
 *    @param f Faction to get the logo of.
 *    @return The faction's small logo image.
 */
glTexture* faction_logoSmall( int f )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }

   return faction_stack[f].logo_small;
}


/**
 * @brief Gets the faction's tiny logo (24x24 or smaller).
 *
 *    @param f Faction to get the logo of.
 *    @return The faction's tiny logo image.
 */
glTexture* faction_logoTiny( int f )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }

   return faction_stack[f].logo_tiny;
}


/**
 * @brief Gets the colour of the faction
 *
 *    @param f Faction to get the colour of.
 *    @return The faction's colour
 */
const glColour* faction_colour( int f )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }

   return faction_stack[f].colour;
}


/**
 * @brief Gets the list of enemies of a faction.
 *
 *    @param f Faction to get enemies of.
 *    @param[out] Number of enemies.
 *    @return The enemies of the faction.
 */
int* faction_getEnemies( int f, int *n )
{
   int i, nenemies;
   int *enemies;

   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }

   /* Player's faction ratings can change, so regenerate each call. */
   if (f == FACTION_PLAYER) {
      nenemies = 0;
      enemies = malloc(sizeof(int)*faction_nstack);

      for (i=0; i<faction_nstack; i++)
         if (faction_isPlayerEnemy(i))
            enemies[nenemies++] = i;

      enemies = realloc(enemies, sizeof(int)*nenemies);

      free(faction_stack[f].enemies);
      faction_stack[f].enemies = enemies;
      faction_stack[f].nenemies = nenemies;
   }

   *n = faction_stack[f].nenemies;
   return faction_stack[f].enemies;
}


/**
 * @brief Gets the list of allies of a faction.
 *
 *    @param f Faction to get allies of.
 *    @param[out] Number of allies.
 *    @return The allies of the faction.
 */
int* faction_getAllies( int f, int *n )
{
   int i, nallies;
   int *allies;

   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }

   /* Player's faction ratings can change, so regenerate each call. */
   if (f == FACTION_PLAYER) {
      nallies = 0;
      allies = malloc(sizeof(int)*faction_nstack);

      for (i=0; i<faction_nstack; i++)
         if (faction_isPlayerFriend(i))
            allies[nallies++] = i;

      allies = realloc(allies, sizeof(int)*nallies);

      free(faction_stack[f].allies);
      faction_stack[f].allies = allies;
      faction_stack[f].nallies = nallies;
   }

   *n = faction_stack[f].nallies;
   return faction_stack[f].allies;
}


/**
 * @brief Adds an enemy to the faction's enemies list.
 *
 *    @param f The faction to add an enemy to.
 *    @param o The other faction to make an enemy.
 */
void faction_addEnemy( int f, int o)
{
   Faction *ff;
   int i;

   if (f==o) return;

   if (faction_isFaction(f))
      ff = &faction_stack[f];
   else { /* f is invalid */
      WARN(_("addEnemy: %d is an invalid faction"), f);
      return;
   }

   if (!faction_isFaction(o)) { /* o is invalid */
      WARN(_("addEnemy: %d is an invalid faction"), o);
      return;
   }

   /* player cannot be made an enemy this way */
   if (f==FACTION_PLAYER) {
      WARN(_("addEnemy: %d is the player faction"), f);
      return;
   }
   if (o==FACTION_PLAYER) {
      WARN(_("addEnemy: %d is the player faction"), o);
      return;
   }

   for (i=0;i<ff->nenemies;i++) {
      if (ff->enemies[i] == o)
         return;
   }

   ff->nenemies++;
   ff->enemies = realloc(ff->enemies, sizeof(int)*ff->nenemies);
   ff->enemies[ff->nenemies-1] = o;
}


/**
 * @brief Removes an enemy from the faction's enemies list.
 *
 *    @param f The faction to remove an enemy from.
 *    @param o The other faction to remove as an enemy.
 */
void faction_rmEnemy( int f, int o)
{
   Faction *ff;
   int i;

   if (f==o) return;

   if (faction_isFaction(f))
      ff = &faction_stack[f];
   else { /* f is invalid */
      WARN(_("rmEnemy: %d is an invalid faction"), f);
      return;
   }

   for (i=0;i<ff->nenemies;i++) {
      if (ff->enemies[i] == o) {
         ff->enemies[i] = ff->enemies[ff->nenemies-1];
         ff->nenemies--;
         ff->enemies = realloc(ff->enemies, sizeof(int)*ff->nenemies);
         return;
      }
   }
}


/**
 * @brief Adds an ally to the faction's allies list.
 *
 *    @param f The faction to add an ally to.
 *    @param o The other faction to make an ally.
 */
void faction_addAlly( int f, int o)
{
   Faction *ff;
   int i;

   if (f==o) return;

   if (faction_isFaction(f))
      ff = &faction_stack[f];
   else { /* f is invalid */
      WARN(_("addAlly: %d is an invalid faction"), f);
      return;
   }

   if (!faction_isFaction(o)) { /* o is invalid */
      WARN(_("addAlly: %d is an invalid faction"), o);
      return;
   }

   /* player cannot be made an ally this way */
   if (f==FACTION_PLAYER) {
      WARN(_("addAlly: %d is the player faction"), f);
      return;
   }
   if (o==FACTION_PLAYER) {
      WARN(_("addAlly: %d is the player faction"), o);
      return;
   }

   for (i=0;i<ff->nallies;i++) {
      if (ff->allies[i] == o)
         return;
   }

   ff->nallies++;
   ff->allies = realloc(ff->allies, sizeof(int)*ff->nallies);
   ff->allies[ff->nallies-1] = o;
}


/**
 * @brief Removes an ally from the faction's allies list.
 *
 *    @param f The faction to remove an ally from.
 *    @param o The other faction to remove as an ally.
 */
void faction_rmAlly( int f, int o)
{
   Faction *ff;
   int i;

   if (f==o) return;

   if (faction_isFaction(f))
      ff = &faction_stack[f];
   else { /* f is invalid */
      WARN(_("rmAlly: %d is an invalid faction"), f);
      return;
   }

   for (i=0;i<ff->nallies;i++) {
      if (ff->allies[i] == o) {
         ff->allies[i] = ff->allies[ff->nallies-1];
         ff->nallies--;
         ff->allies = realloc(ff->allies, sizeof(int)*ff->nallies);
         return;
      }
   }
}


/**
 * @brief Gets the state associated to the faction scheduler.
 */
nlua_env faction_getScheduler( int f )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return LUA_NOREF;
   }

   return faction_stack[f].sched_env;
}


/**
 * @brief Gets the equipper state associated to the faction scheduler.
 */
nlua_env faction_getEquipper( int f )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return LUA_NOREF;
   }

   return faction_stack[f].equip_env;
}


/**
 * @brief Sanitizes player faction standing.
 *
 *    @param faction Faction to sanitize.
 */
static void faction_sanitizePlayer( Faction* faction )
{
   if (faction->player > 100.)
      faction->player = 100.;
   else if (faction->player < -100.)
      faction->player = -100.;
}


/**
 * @brief Mods player using the power of Lua.
 */
static void faction_modPlayerLua( int f, double mod, const char *source, int secondary )
{
   Faction *faction;
   double old, delta;
   HookParam hparam[3];

   faction = &faction_stack[f];

   /* Make sure it's not static. */
   if (faction_isFlag(faction, FACTION_STATIC))
      return;

   old   = faction->player;

   if (faction->env == LUA_NOREF)
      faction->player += mod;
   else {

      /* Set up the function:
       * faction_hit( current, amount, source, secondary ) */
      nlua_getenv( faction->env, "faction_hit" );
      lua_pushnumber(  naevL, faction->player );
      lua_pushnumber(  naevL, mod );
      lua_pushstring(  naevL, source );
      lua_pushboolean( naevL, secondary );

      /* Call function. */
      if (nlua_pcall( faction->env, 4, 1 )) { /* An error occurred. */
         WARN(_("Faction '%s': %s"), faction->name, lua_tostring(naevL,-1));
         lua_pop( naevL, 1 );
         return;
      }

      /* Parse return. */
      if (!lua_isnumber( naevL, -1 ))
         WARN( _("Lua script for faction '%s' did not return a number from 'faction_hit(...)'."), faction->name );
      else
         faction->player = lua_tonumber( naevL, -1 );
      lua_pop( naevL, 1 );
   }

   /* Sanitize just in case. */
   faction_sanitizePlayer( faction );

   /* Run hook if necessary. */
   delta = faction->player - old;
   if (fabs(delta) > 1e-10) {
      hparam[0].type    = HOOK_PARAM_FACTION;
      hparam[0].u.lf    = f;
      hparam[1].type    = HOOK_PARAM_NUMBER;
      hparam[1].u.num   = delta;
      hparam[2].type    = HOOK_PARAM_SENTINEL;
      hooks_runParam( "standing", hparam );

      /* Tell space the faction changed. */
      space_factionChange();
   }
}


/**
 * @brief Modifies the player's standing with a faction.
 *
 * Affects enemies and allies too.
 *
 *    @param f Faction to modify player's standing.
 *    @param mod Modifier to modify by.
 */
void faction_modPlayer( int f, double mod, const char *source )
{
   int i;
   Faction *faction;

   if (!faction_isFaction(f)) {
      WARN(_("%d is an invalid faction"), f);
      return;
   }
   faction = &faction_stack[f];

   /* Modify faction standing with parent faction. */
   faction_modPlayerLua( f, mod, source, 0 );

   /* Now mod allies to a lesser degree */
   for (i=0; i<faction->nallies; i++)
      /* Modify faction standing */
      faction_modPlayerLua( faction->allies[i], mod, source, 1 );

   /* Now mod enemies */
   for (i=0; i<faction->nenemies; i++)
      /* Modify faction standing. */
      faction_modPlayerLua( faction->enemies[i], -mod, source, 1 );
}

/**
 * @brief Modifies the player's standing without affecting others.
 *
 * Does not affect allies nor enemies.
 *
 *    @param f Faction whose standing to modify.
 *    @param mod Amount to modify standing by.
 *
 * @sa faction_modPlayer
 */
void faction_modPlayerSingle( int f, double mod, const char *source )
{
   if (!faction_isFaction(f)) {
      WARN(_("%d is an invalid faction"), f);
      return;
   }

   faction_modPlayerLua( f, mod, source, 0 );
}


/**
 * @brief Modifies the player's standing without affecting others.
 *
 * Does not affect allies nor enemies and does not run through the Lua script.
 *
 *    @param f Faction whose standing to modify.
 *    @param mod Amount to modify standing by.
 *
 * @sa faction_modPlayer
 */
void faction_modPlayerRaw( int f, double mod )
{
   Faction *faction;
   HookParam hparam[3];

   if (!faction_isFaction(f)) {
      WARN(_("%d is an invalid faction"), f);
      return;
   }

   faction = &faction_stack[f];
   faction->player += mod;
   /* Run hook if necessary. */
   hparam[0].type    = HOOK_PARAM_FACTION;
   hparam[0].u.lf    = f;
   hparam[1].type    = HOOK_PARAM_NUMBER;
   hparam[1].u.num   = mod;
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "standing", hparam );

   /* Sanitize just in case. */
   faction_sanitizePlayer( faction );

   /* Tell space the faction changed. */
   space_factionChange();
}


/**
 * @brief Sets the player's standing with a faction.
 *
 *    @param f Faction to set the player's standing for.
 *    @param value Value to set the player's standing to.
 */
void faction_setPlayer( int f, double value )
{
   Faction *faction;
   HookParam hparam[3];
   double mod;

   if (!faction_isFaction(f)) {
      WARN(_("%d is an invalid faction"), f);
      return;
   }

   faction = &faction_stack[f];
   mod = value - faction->player;
   faction->player = value;
   /* Run hook if necessary. */
   hparam[0].type    = HOOK_PARAM_FACTION;
   hparam[0].u.lf    = f;
   hparam[1].type    = HOOK_PARAM_NUMBER;
   hparam[1].u.num   = mod;
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "standing", hparam );

   /* Sanitize just in case. */
   faction_sanitizePlayer( faction );

   /* Tell space the faction changed. */
   space_factionChange();
}


/**
 * @brief Gets the player's standing with a faction.
 *
 *    @param f Faction to get player's standing from.
 *    @return The standing the player has with the faction.
 */
double faction_getPlayer( int f )
{
   if (faction_isFaction(f))
      return faction_stack[f].player;
   else {
      WARN(_("%d is an invalid faction"), f);
      return -1000;
   }
}


/**
 * @brief Gets the player's default standing with a faction.
 *
 *    @param f Faction to get player's default standing from.
 *    @return The default standing the player has with the faction.
 */
double faction_getPlayerDef( int f )
{
   if (faction_isFaction(f))
      return faction_stack[f].player_def;
   else {
      WARN(_("%d is an invalid faction"), f);
      return -1000;
   }
}


/**
 * @brief Gets whether or not the player is a friend of the faction.
 * 
 *    @param f Faction to check friendliness of.
 *    @return 1 if the player is a friend, 0 otherwise.
 */
int faction_isPlayerFriend( int f )
{
   Faction *faction;
   int r;

   faction = &faction_stack[f];

   if ( faction->env == LUA_NOREF )
      return 0;
   else
   {

      /* Set up the function:
       * faction_player_friend( standing ) */
      nlua_getenv( faction->env, "faction_player_friend" );
      lua_pushnumber( naevL, faction->player );

      /* Call function. */
      if ( nlua_pcall( faction->env, 1, 1 ) )
      {
         /* An error occurred. */
         WARN( _("Faction '%s': %s"), faction->name, lua_tostring( naevL, -1 ) );
         lua_pop( naevL, 1 );
         return 0;
      }

      /* Parse return. */
      if ( !lua_isboolean( naevL, -1 ) )
      {
         WARN( _("Lua script for faction '%s' did not return a boolean from 'faction_player_friend(...)'."), faction->name );
         r = 0;
      }
      else
         r = lua_toboolean( naevL, -1 );
      lua_pop( naevL, 1 );

      return r;
   }
}


/**
 * @brief Gets whether or not the player is an enemy of the faction.
 * 
 *    @param f Faction to check hostility of.
 *    @return 1 if the player is an enemy, 0 otherwise.
 */
int faction_isPlayerEnemy( int f )
{
   Faction *faction;
   int r;

   faction = &faction_stack[f];

   if ( faction->env == LUA_NOREF )
      return 0;
   else
   {

      /* Set up the function:
       * faction_player_enemy( standing ) */
      nlua_getenv( faction->env, "faction_player_enemy" );
      lua_pushnumber( naevL, faction->player );

      /* Call function. */
      if ( nlua_pcall( faction->env, 1, 1 ) )
      {
         /* An error occurred. */
         WARN( _("Faction '%s': %s"), faction->name, lua_tostring( naevL, -1 ) );
         lua_pop( naevL, 1 );
         return 0;
      }

      /* Parse return. */
      if ( !lua_isboolean( naevL, -1 ) )
      {
         WARN( _("Lua script for faction '%s' did not return a boolean from 'faction_player_enemy(...)'."), faction->name );
         r = 0;
      }
      else
         r = lua_toboolean( naevL, -1 );
      lua_pop( naevL, 1 );

      return r;
   }
}


/**
 * @brief Gets the colour of the faction based on it's standing with the player.
 *
 * Used to unify the colour checks all over.
 *
 *    @param f Faction to get the colour of based on player's standing.
 *    @return Pointer to the colour.
 */
const glColour* faction_getColour( int f )
{
   if (f<0) return &cInert;
   else if (areAllies(FACTION_PLAYER,f)) return &cFriend;
   else if (areEnemies(FACTION_PLAYER,f)) return &cHostile;
   else return &cNeutral;
}


/**
 * @brief Gets the faction character associated to it's standing with the player.
 *
 * Use this to do something like "\a%c", faction_getColourChar( some_faction ) in the
 *  font print routines.
 *
 *    @param f Faction to get the colour of based on player's standing.
 *    @return The character associated to the faction.
 */
char faction_getColourChar( int f )
{
   if (f<0) return 'I';
   else if (areEnemies(FACTION_PLAYER,f)) return 'H';
   else if (areAllies(FACTION_PLAYER,f)) return 'F';
   else return 'N';
}


/**
 * @brief Gets the player's standing in human readable form.
 *
 *    @param f Faction to get standing of.
 *    @return Human readable player's standing.
 */
const char *faction_getStandingText( int f )
{
   Faction *faction;
   const char *r;

   faction = &faction_stack[f];

   if ( faction->env == LUA_NOREF )
      return "???";
   else
   {

      /* Set up the function:
       * faction_standing_text( standing ) */
      nlua_getenv( faction->env, "faction_standing_text" );
      lua_pushnumber( naevL, faction->player );

      /* Call function. */
      if ( nlua_pcall( faction->env, 1, 1 ) )
      {
         /* An error occurred. */
         WARN( _("Faction '%s': %s"), faction->name, lua_tostring( naevL, -1 ) );
         lua_pop( naevL, 1 );
         return "???";
      }

      /* Parse return. */
      if ( !lua_isstring( naevL, -1 ) )
      {
         WARN( _("Lua script for faction '%s' did not return a string from 'faction_standing_text(...)'."), faction->name );
         r = "???";
      }
      else
         r = lua_tostring( naevL, -1 );
      lua_pop( naevL, 1 );

      return r;
   }
}


/**
 * @brief Gets the broad faction standing.
 *
 *    @param f Faction to get broad standing of.
 *    @param bribed Whether or not the respective pilot is bribed.
 *    @param override If positive sets to ally, if negative sets to hostile.
 *    @return Human readable broad player's standing.
 */
const char *faction_getStandingBroad( int f, int bribed, int override )
{
   Faction *faction;
   const char *r;

   faction = &faction_stack[f];

   if ( faction->env == LUA_NOREF )
      return "???";
   else
   {
      /* Set up the function:
       * faction_standing_broad( standing, bribed, override ) */
      nlua_getenv( faction->env, "faction_standing_broad" );
      lua_pushnumber( naevL, faction->player );
      lua_pushboolean( naevL, bribed );
      lua_pushnumber( naevL, override );

      /* Call function. */
      if ( nlua_pcall( faction->env, 3, 1 ) )
      {
         /* An error occurred. */
         WARN( _("Faction '%s': %s"), faction->name, lua_tostring( naevL, -1 ) );
         lua_pop( naevL, 1 );
         return "???";
      }

      /* Parse return. */
      if ( !lua_isstring( naevL, -1 ) )
      {
         WARN( _("Lua script for faction '%s' did not return a string from 'faction_standing_broad(...)'."), faction->name );
         r = "???";
      }
      else
         r = lua_tostring( naevL, -1 );
      lua_pop( naevL, 1 );

      return r;
   }
}


/**
 * @brief Checks whether two factions are enemies.
 *
 *    @param a Faction A.
 *    @param b Faction B.
 *    @return 1 if A and B are enemies, 0 otherwise.
 */
int areEnemies( int a, int b)
{
   Faction *fa, *fb;
   int i;

   if (a==b) return 0; /* luckily our factions aren't masochistic */

   /* handle a */
   if (faction_isFaction(a))
      fa = &faction_stack[a];
   else { /* a is invalid */
      WARN(_("areEnemies: %d is an invalid faction"), a);
      return 0;
   }

   /* handle b */
   if (faction_isFaction(b))
      fb = &faction_stack[b];
   else { /* b is invalid */
      WARN(_("areEnemies: %d is an invalid faction"), b);
      return 0;
   }

   /* player handled separately */
   if (a==FACTION_PLAYER) {
      return faction_isPlayerEnemy(b);
   }
   else if (b==FACTION_PLAYER) {
      return faction_isPlayerEnemy(a);
   }

   for (i=0;i<fa->nenemies;i++)
      if (fa->enemies[i] == b)
         return 1;
   for (i=0;i<fb->nenemies;i++)
      if(fb->enemies[i] == a)
         return 1;

   return 0;
}


/**
 * @brief Checks whether two factions are allies or not.
 *
 *    @param a Faction A.
 *    @param b Faction B.
 *    @return 1 if A and B are allies, 0 otherwise.
 */
int areAllies( int a, int b )
{
   Faction *fa, *fb;
   int i;

   /* If they are the same they must be allies. */
   if (a==b) return 1;

   /* handle a */
   if (faction_isFaction(a))
      fa = &faction_stack[a];
   else { /* a is invalid */
      WARN(_("%d is an invalid faction"), a);
      return 0;
   }

   /* handle b */
   if (faction_isFaction(b))
      fb = &faction_stack[b];
   else { /* b is invalid */
      WARN(_("%d is an invalid faction"), b);
      return 0;
   }

   /* we assume player becomes allies with high rating */
   if (a==FACTION_PLAYER) {
      return faction_isPlayerFriend(b);
   }
   else if (b==FACTION_PLAYER) {
      return faction_isPlayerFriend(a);
   }

   for (i=0;i<fa->nallies;i++)
      if (fa->allies[i] == b)
         return 1;
   for (i=0;i<fb->nallies;i++)
      if(fb->allies[i] == a)
         return 1;

   return 0;
}


/**
 * @brief Checks whether or not a faction is valid.
 *
 *    @param f Faction to check for validity.
 *    @return 1 if faction is valid, 0 otherwise.
 */
int faction_isFaction( int f )
{
   if ((f<0) || (f>=faction_nstack))
      return 0;
   return 1;
}


/**
 * @brief Parses a single faction, but doesn't set the allies/enemies bit.
 *
 *    @param temp Faction to load data into.
 *    @param parent Parent node to extract faction from.
 *    @return Faction created from parent node.
 */
static int faction_parse( Faction* temp, xmlNodePtr parent )
{
   xmlNodePtr node;
   int player;
   char buf[PATH_MAX], *dat, *ctmp;
   glColour *col;
   size_t ndat;

   /* Clear memory. */
   memset( temp, 0, sizeof(Faction) );
   temp->equip_env = LUA_NOREF;
   temp->env = LUA_NOREF;
   temp->sched_env = LUA_NOREF;

   player = 0;
   node = parent->xmlChildrenNode; 
   do {
      /* Only care about nodes. */
      xml_onlyNodes(node);

      /* Can be 0 or negative, so we have to take that into account. */
      if (xml_isNode(node,"player")) {
         temp->player_def = xml_getFloat(node);
         player = 1;
         continue;
      }

      xmlr_strd(node,"name",temp->name);
      xmlr_strd(node,"longname",temp->longname);
      xmlr_strd(node,"display",temp->displayname);
      if (xml_isNode(node, "colour")) {
         ctmp = xml_get(node);
         if (ctmp != NULL)
            temp->colour = col_fromName(xml_raw(node));
         /* If no named colour is present, RGB attributes are used. */
         else {
            /* Initialize in case a colour channel is absent. */
            col = calloc( 1, sizeof(glColour) );

            xmlr_attr(node,"r",ctmp);
            if (ctmp != NULL) {
               col->r = atof(ctmp);
               free(ctmp);
            }

            xmlr_attr(node,"g",ctmp);
            if (ctmp != NULL) {
               col->g = atof(ctmp);
               free(ctmp);
            }

            xmlr_attr(node,"b",ctmp);
            if (ctmp != NULL) {
               col->b = atof(ctmp);
               free(ctmp);
            }

            col->a = 1.;
            temp->colour = col;
         }
         continue;
      }

      if (xml_isNode(node, "spawn")) {
         if (temp->sched_env != LUA_NOREF)
            WARN(_("Faction '%s' has duplicate 'spawn' tag."), temp->name);
         nsnprintf( buf, sizeof(buf), "dat/factions/spawn/%s.lua", xml_raw(node) );
         temp->sched_env = nlua_newEnv(1);
         nlua_loadStandard( temp->sched_env);
         dat = ndata_read( buf, &ndat );
         if (nlua_dobufenv(temp->sched_env, dat, ndat, buf) != 0) {
            WARN(_("Failed to run spawn script: %s\n"
                  "%s\n"
                  "Most likely Lua file has improper syntax, please check"),
                  buf, lua_tostring(naevL,-1));
            nlua_freeEnv( temp->sched_env );
            temp->sched_env = LUA_NOREF;
         }
         free(dat);
         continue;
      }

      if (xml_isNode(node, "standing")) {
         if (temp->env != LUA_NOREF)
            WARN(_("Faction '%s' has duplicate 'standing' tag."), temp->name);
         nsnprintf( buf, sizeof(buf), "dat/factions/standing/%s.lua", xml_raw(node) );
         temp->env = nlua_newEnv(1);
         nlua_loadStandard( temp->env );
         dat = ndata_read( buf, &ndat );
         if (nlua_dobufenv(temp->env, dat, ndat, buf) != 0) {
            WARN(_("Failed to run standing script: %s\n"
                  "%s\n"
                  "Most likely Lua file has improper syntax, please check"),
                  buf, lua_tostring(naevL,-1));
            nlua_freeEnv( temp->env );
            temp->env = LUA_NOREF;
         }
         free(dat);
         continue;
      }

      if (xml_isNode(node, "known")) {
         faction_setFlag(temp, FACTION_KNOWN);
         continue;
      }

      if (xml_isNode(node, "equip")) {
         if (temp->equip_env != LUA_NOREF)
            WARN(_("Faction '%s' has duplicate 'equip' tag."), temp->name);
         nsnprintf( buf, sizeof(buf), "dat/factions/equip/%s.lua", xml_raw(node) );
         temp->equip_env = nlua_newEnv(1);
         nlua_loadStandard( temp->equip_env );
         dat = ndata_read( buf, &ndat );
         if (nlua_dobufenv(temp->equip_env, dat, ndat, buf) != 0) {
            WARN(_("Failed to run equip script: %s\n"
                  "%s\n"
                  "Most likely Lua file has improper syntax, please check"),
                  buf, lua_tostring(naevL, -1));
            nlua_freeEnv( temp->equip_env );
            temp->equip_env = LUA_NOREF;
         }
         free(dat);
         continue;
      }

      if (xml_isNode(node,"logo")) {
         if (temp->logo_small != NULL)
            WARN(_("Faction '%s' has duplicate 'logo' tag."), temp->name);
         nsnprintf( buf, PATH_MAX, FACTION_LOGO_PATH"%s_small.png", xml_get(node));
         temp->logo_small = gl_newImage(buf, 0);
         nsnprintf( buf, PATH_MAX, FACTION_LOGO_PATH"%s_tiny.png", xml_get(node));
         temp->logo_tiny = gl_newImage(buf, 0);
         continue;
      }

      if (xml_isNode(node,"static")) {
         faction_setFlag(temp, FACTION_STATIC);
         continue;
      }

      if (xml_isNode(node,"invisible")) {
         faction_setFlag(temp, FACTION_INVISIBLE);
         continue;
      }

      /* Avoid warnings. */
      if (xml_isNode(node,"allies") || xml_isNode(node,"enemies"))
         continue;

      DEBUG(_("Unknown node '%s' in faction '%s'"),node->name,temp->name);
   } while (xml_nextNode(node));

   if (temp->name == NULL)
      WARN(_("Faction from %s has invalid or no name"), FACTION_DATA_PATH);
   if (player==0)
      DEBUG(_("Faction '%s' missing player tag."), temp->name);
   if ((temp->env==LUA_NOREF) && !faction_isFlag( temp, FACTION_STATIC ))
      WARN(_("Faction '%s' has no Lua and isn't static!"), temp->name);

   return 0;
}


/**
 * @brief Parses the social tidbits of a faction: allies and enemies.
 *
 *    @param parent Node containing the faction.
 */
static void faction_parseSocial( xmlNodePtr parent )
{
   xmlNodePtr node, cur;
   char *buf;
   Faction *base;
   int mod;
   int mem;

   /* Get name. */
   base = NULL;
   node = parent->xmlChildrenNode;
   do {
      xml_onlyNodes(node);
      if (xml_isNode(node,"name")) {
         base = &faction_stack[ faction_get( xml_get(node) ) ];
         break;
      }
   } while (xml_nextNode(node));

   /* Parse social stuff. */
   node = parent->xmlChildrenNode;
   do {

      /* Grab the allies */
      if (xml_isNode(node,"allies")) {
         cur = node->xmlChildrenNode;

         mem = 0;
         do {
            if (xml_isNode(cur,"ally")) {
               mod = faction_get(xml_get(cur));
               base->nallies++;
               if (base->nallies > mem) {
                  mem += CHUNK_SIZE;
                  base->allies = realloc(base->allies, sizeof(int)*mem);
               }
               base->allies[base->nallies-1] = mod;
            }
         } while (xml_nextNode(cur));
         if (base->nallies > 0)
            base->allies = realloc(base->allies, sizeof(int)*base->nallies);
      }

      /* Grab the enemies */
      if (xml_isNode(node,"enemies")) {
         cur = node->xmlChildrenNode;

         mem = 0;
         do {
            if (xml_isNode(cur,"enemy")) {
               mod = faction_get(xml_get(cur));
               base->nenemies++;
               if (base->nenemies > mem) {
                  mem += CHUNK_SIZE;
                  base->enemies = realloc(base->enemies, sizeof(int)*mem);
               }
               base->enemies[base->nenemies-1] = mod;
            }
         } while (xml_nextNode(cur));
         if (base->nenemies > 0)
            base->enemies = realloc(base->enemies, sizeof(int)*base->nenemies);
      }
   } while (xml_nextNode(node));
}


/**
 * @brief Resets the player's standing with the factions to default.
 */
void factions_reset (void)
{
   int i;
   for (i=0; i<faction_nstack; i++)
      faction_stack[i].player = faction_stack[i].player_def;
}


/**
 * @brief Loads up all the factions from the data file.
 *
 *    @return 0 on success.
 */
int factions_load (void)
{
   int mem;
   size_t bufsize;
   char *buf = ndata_read( FACTION_DATA_PATH, &bufsize);

   xmlNodePtr factions, node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode; /* Factions node */
   if (!xml_isNode(node,XML_FACTION_ID)) {
      ERR(_("Malformed %s file: missing root element '%s'"), FACTION_DATA_PATH, XML_FACTION_ID);
      return -1;
   }

   factions = node->xmlChildrenNode; /* first faction node */
   if (factions == NULL) {
      ERR(_("Malformed %s file: does not contain elements"), FACTION_DATA_PATH);
      return -1;
   }

   /* player faction is hard-coded */
   faction_stack = calloc( 1, sizeof(Faction) );
   faction_stack[0].name = strdup("Player");
   faction_stack[0].flags = FACTION_STATIC | FACTION_INVISIBLE;
   faction_stack[0].equip_env = LUA_NOREF;
   faction_stack[0].env = LUA_NOREF;
   faction_stack[0].sched_env = LUA_NOREF;
   faction_nstack++;

   /* First pass - gets factions */
   node = factions;
   mem = 0;
   do {
      if (xml_isNode(node,XML_FACTION_TAG)) {
         /* See if must grow memory.  */
         faction_nstack++;
         if (faction_nstack > mem) {
            mem += CHUNK_SIZE;
            faction_stack = realloc(faction_stack, sizeof(Faction)*mem);
         }

         /* Load faction. */
         faction_parse(&faction_stack[faction_nstack-1], node);
      }
   } while (xml_nextNode(node));

   /* Shrink to minimum size. */
   faction_stack = realloc(faction_stack, sizeof(Faction)*faction_nstack);

   /* Second pass - sets allies and enemies */
   node = factions;
   do {
      if (xml_isNode(node,XML_FACTION_TAG))
         faction_parseSocial(node);
   } while (xml_nextNode(node));

#ifdef DEBUGGING
   int i, j, k, r;
   Faction *f, *sf;

   /* Third pass, makes sure allies/enemies are sane. */
   for (i=0; i<faction_nstack; i++) {
      f = &faction_stack[i];

      /* First run over allies and make sure it's mutual. */
      for (j=0; j < f->nallies; j++) {
         sf = &faction_stack[ f->allies[j] ];

         r = 0;
         for (k=0; k < sf->nallies; k++)
            if (sf->allies[k] == i)
               r = 1;

         if (r == 0)
            WARN(_("Faction: %s and %s aren't completely mutual allies!"),
                  f->name, sf->name );
      }

      /* Now run over enemies. */
      for (j=0; j < f->nenemies; j++) {
         sf = &faction_stack[ f->enemies[j] ];

         r = 0;
         for (k=0; k < sf->nenemies; k++)
            if (sf->enemies[k] == i)
               r = 1;

         if (r == 0)
            WARN(_("Faction: %s and %s aren't completely mutual enemies!"),
                  f->name, sf->name );
      }
   }
#endif /* DEBUGGING */

   xmlFreeDoc(doc);
   free(buf);

   DEBUG( ngettext( "Loaded %d Faction", "Loaded %d Factions", faction_nstack ), faction_nstack );

   return 0;
}


/**
 * @brief Frees the factions.
 */
void factions_free (void)
{
   int i;

   /* free factions */
   for (i=0; i<faction_nstack; i++) {
      free(faction_stack[i].name);
      if (faction_stack[i].longname != NULL)
         free(faction_stack[i].longname);
      if (faction_stack[i].displayname != NULL)
         free(faction_stack[i].displayname);
      if (faction_stack[i].logo_small != NULL)
         gl_freeTexture(faction_stack[i].logo_small);
      if (faction_stack[i].logo_tiny != NULL)
         gl_freeTexture(faction_stack[i].logo_tiny);
      if (faction_stack[i].nallies > 0)
         free(faction_stack[i].allies);
      if (faction_stack[i].nenemies > 0)
         free(faction_stack[i].enemies);
      if (faction_stack[i].sched_env != LUA_NOREF)
         nlua_freeEnv( faction_stack[i].sched_env );
      if (faction_stack[i].env != LUA_NOREF)
         nlua_freeEnv( faction_stack[i].env );
      if (faction_stack[i].equip_env != LUA_NOREF)
         nlua_freeEnv( faction_stack[i].equip_env );
   }
   free(faction_stack);
   faction_stack = NULL;
   faction_nstack = 0;
}


/**
 * @brief Saves player's standings with the factions.
 *
 *    @param writer The xml writer to use.
 *    @return 0 on success.
 */
int pfaction_save( xmlTextWriterPtr writer )
{
   int i;

   xmlw_startElem(writer,"factions");

   for (i=1; i<faction_nstack; i++) { /* player is faction 0 */
      /* Must not be static. */
      if (faction_isFlag( &faction_stack[i], FACTION_STATIC ))
         continue;

      xmlw_startElem(writer,"faction");

      xmlw_attr(writer,"name","%s",faction_stack[i].name);
      xmlw_elem(writer, "standing", "%f", faction_stack[i].player);

      if (faction_isKnown_(&faction_stack[i]))
         xmlw_elemEmpty(writer, "known");

      xmlw_endElem(writer); /* "faction" */
   }

   xmlw_endElem(writer); /* "factions" */

   return 0;
}


/**
 * @brief Loads the player's faction standings.
 *
 *    @param parent Parent xml node to read from.
 *    @return 0 on success.
 */
int pfaction_load( xmlNodePtr parent )
{
   xmlNodePtr node, cur, sub;
   char *str;
   int faction;

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"factions")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"faction")) {
               xmlr_attr(cur, "name", str);
               faction = faction_get(str);

               if (faction != -1) { /* Faction is valid. */

                  sub = cur->xmlChildrenNode;
                  do {
                     if (xml_isNode(sub,"standing")) {

                        /* Must not be static. */
                        if (!faction_isFlag( &faction_stack[faction], FACTION_STATIC ))
                           faction_stack[faction].player = xml_getFloat(sub);
                        continue;
                     }
                     if (xml_isNode(sub,"known")) {
                        faction_setFlag(&faction_stack[faction], FACTION_KNOWN);
                        continue;
                     }
                  } while (xml_nextNode(sub));
               }
               free(str);
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   return 0;
}


/**
 * @brief Returns an array of faction ids.
 *
 *    @param *n Writes the number of elements.
 *    @param which Which factions to get. (0,1,2,3 : all, friendly, neutral, hostile)
 *    @return A pointer to an array, or NULL.
 */
int *faction_getGroup( int *n, int which )
{
   int *group;
   int i;

   /* Set defaults. */
   group = NULL;
   *n = 0;

   switch(which) {
      case 0: /* 'all' */
         *n = faction_nstack;
         group = malloc(sizeof(int) * *n);
         for(i = 0; i < faction_nstack; i++)
            group[i] = i;
         break;

      case 1: /* 'friendly' */
         for(i = 0; i < faction_nstack; i++)
            if(areAllies(FACTION_PLAYER, i)) {
               (*n)++;
               group = realloc(group, sizeof(int) * *n);
               group[*n - 1] = i;
            }
         break;

      case 2: /* 'neutral' */
         for(i = 0; i < faction_nstack; i++)
            if(!areAllies(FACTION_PLAYER, i) && !areEnemies(FACTION_PLAYER, i)) {
               (*n)++;
               group = realloc(group, sizeof(int) * *n);
               group[*n - 1] = i;
            }
         break;

      case 3: /* 'hostile' */
         for(i = 0; i < faction_nstack; i++)
            if(areEnemies(FACTION_PLAYER, i)) {
               (*n)++;
               group = realloc(group, sizeof(int) * *n);
               group[*n - 1] = i;
            }
         break;

      default:
         /* Defaults have already been set. */
         break;
   }

   return group;
}
