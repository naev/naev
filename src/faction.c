/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file faction.c
 *
 * @brief Handles the Naev factions.
 */


/** @cond */
#include <assert.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "faction.h"

#include "array.h"
#include "colour.h"
#include "hook.h"
#include "log.h"
#include "ndata.h"
#include "nlua.h"
#include "nluadef.h"
#include "nxml.h"
#include "nstring.h"
#include "opengl.h"
#include "rng.h"
#include "space.h"


#define XML_FACTION_ID     "Factions"   /**< XML section identifier */
#define XML_FACTION_TAG    "faction" /**< XML tag identifier. */


#define FACTION_STATIC        (1<<0) /**< Faction doesn't change standing with player. */
#define FACTION_INVISIBLE     (1<<1) /**< Faction isn't exposed to the player. */
#define FACTION_KNOWN         (1<<2) /**< Faction is known to the player. */
#define FACTION_DYNAMIC       (1<<3) /**< Faction was created dynamically. */

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
   char *ai; /**< Name of the faction's default pilot AI. */

   /* Graphics. */
   glTexture *logo_small; /**< Small logo. */
   glTexture *logo_tiny; /**< Tiny logo. */
   const glColour *colour; /**< Faction specific colour. */

   /* Enemies */
   int *enemies; /**< Enemies by ID of the faction. */

   /* Allies */
   int *allies; /**< Allies by ID of the faction. */

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
   unsigned int oflags; /**< Original flags (for when new game is started). */
} Faction;

static Faction* faction_stack = NULL; /**< Faction stack. */


/*
 * Prototypes
 */
/* static */
static int faction_getRaw( const char *name );
static void faction_freeOne( Faction *f );
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
static int faction_getRaw( const char* name )
{
   int i;
   /* Escorts are part of the "player" faction. */
   if (strcmp(name, "Escort") == 0)
      return FACTION_PLAYER;

   if (name != NULL) {
      for (i=0; i<array_size(faction_stack); i++)
         if (strcmp(faction_stack[i].name, name)==0)
            break;

      if (i != array_size(faction_stack))
         return i;
   }
   return -1;
}


/**
 * @brief Checks to see if a faction exists by name.
 *
 *    @param name Name of the faction to seek.
 *    @return ID of the faction.
 */
int faction_exists( const char* name )
{
   return faction_getRaw(name)!=-1;
}


/**
 * @brief Gets a faction ID by name.
 *
 *    @param name Name of the faction to seek.
 *    @return ID of the faction.
 */
int faction_get( const char* name )
{
   int id = faction_getRaw(name);
   if (id<0)
      WARN(_("Faction '%s' not found in stack."), name);
   return id;
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
   f  = malloc( sizeof(int) * array_size(faction_stack) );

   /* Get IDs. */
   m = 0;
   for (i=0; i<array_size(faction_stack); i++)
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
   f  = malloc( sizeof(int) * array_size(faction_stack) );

   /* Get IDs. */
   m = 0;
   for (i=0; i<array_size(faction_stack); i++)
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

   for ( i=0; i<array_size(faction_stack); i++)
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
 * @brief Is faction dynamic.
 */
int faction_isDynamic( int id )
{
   return faction_isFlag( &faction_stack[id], FACTION_DYNAMIC );
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
 * @brief Gets a factions "real" (internal) name.
 *
 *    @param f Faction to get the name of.
 *    @return Name of the faction (internal/English).
 */
const char* faction_name( int f )
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
 * @brief Gets a factions short name (human-readable).
 *
 *    @param f Faction to get the name of.
 *    @return Name of the faction (in player's native language).
 */
const char* faction_shortname( int f )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }
   /* Don't want player to see their escorts as "Player" faction. */
   if (f == FACTION_PLAYER)
      return _("Escort");

   /* Possibly get display name. */
   if (faction_stack[f].displayname != NULL)
      return _(faction_stack[f].displayname);

   return _(faction_stack[f].name);
}


/**
 * @brief Gets the faction's long name (formal, human-readable).
 *
 *    @param f Faction to get the name of.
 *    @return The faction's long name (in player's native language).
 */
const char* faction_longname( int f )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }
   if (faction_stack[f].longname != NULL)
      return _(faction_stack[f].longname);
   return _(faction_stack[f].name);
}


/**
 * @brief Gets the name of the default AI profile for the faction's pilots.
 *
 *    @param f Faction ID.
 *    @return The faction's AI profile name.
 */
const char* faction_default_ai( int f )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."), f);
      return NULL;
   }
   return faction_stack[f].ai;
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
 *    @param[out] n Number of allies.
 *    @return The enemies of the faction.
 */
int* faction_getEnemies( int f, int *n )
{
   int i;
   int *enemies;
   int *tmp;

   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }

   /* Player's faction ratings can change, so regenerate each call. */
   if (f == FACTION_PLAYER) {
      enemies = array_create( int );

      for (i=0; i<array_size(faction_stack); i++)
         if (faction_isPlayerEnemy(i)) {
            tmp = &array_grow( &enemies );
            *tmp = i;
         }

      array_free( faction_stack[f].enemies );
      faction_stack[f].enemies = enemies;
   }

   *n = array_size( faction_stack[f].enemies );
   return faction_stack[f].enemies;
}


/**
 * @brief Gets the list of allies of a faction.
 *
 *    @param f Faction to get allies of.
 *    @param[out] n Number of allies.
 *    @return The allies of the faction.
 */
int* faction_getAllies( int f, int *n )
{
   int i;
   int *allies;
   int *tmp;

   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."),f);
      return NULL;
   }

   /* Player's faction ratings can change, so regenerate each call. */
   if (f == FACTION_PLAYER) {
      allies = array_create( int );

      for (i=0; i<array_size(faction_stack); i++)
         if (faction_isPlayerFriend(i)) {
            tmp = &array_grow( &allies );
            *tmp = i;
         }

      array_free( faction_stack[ f ].allies );
      faction_stack[f].allies = allies;
   }

   *n = array_size(faction_stack[f].allies);
   return faction_stack[f].allies;
}


/**
 * @brief Adds an enemy to the faction's enemies list.
 *
 *    @param f The faction to add an enemy to.
 *    @param o The other faction to make an enemy.
 */
void faction_addEnemy( int f, int o )
{
   Faction *ff;
   int i, *tmp;

   if (f==o) return;

   if (faction_isFaction(f))
      ff = &faction_stack[f];
   else { /* f is invalid */
      WARN(_("Faction id '%d' is invalid."), f);
      return;
   }

   if (!faction_isFaction(o)) { /* o is invalid */
      WARN(_("Faction id '%d' is invalid."), o);
      return;
   }

   /* player cannot be made an enemy this way */
   if (f==FACTION_PLAYER) {
      WARN(_("%d is the player faction"), f);
      return;
   }
   if (o==FACTION_PLAYER) {
      WARN(_("%d is the player faction"), o);
      return;
   }

   for (i=0;i<array_size(ff->enemies);i++) {
      if (ff->enemies[i] == o)
         return;
   }

   tmp = &array_grow( &ff->enemies );
   *tmp = o;
}


/**
 * @brief Removes an enemy from the faction's enemies list.
 *
 *    @param f The faction to remove an enemy from.
 *    @param o The other faction to remove as an enemy.
 */
void faction_rmEnemy( int f, int o )
{
   Faction *ff;
   int i;

   if (f==o) return;

   if (faction_isFaction(f))
      ff = &faction_stack[f];
   else { /* f is invalid */
      WARN(_("Faction id '%d' is invalid."), f);
      return;
   }

   for (i=0;i<array_size(ff->enemies);i++) {
      if (ff->enemies[i] == o) {
         array_erase( &ff->enemies, &ff->enemies[i], &ff->enemies[i+1] );
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
void faction_addAlly( int f, int o )
{
   Faction *ff;
   int i, *tmp;

   if (f==o) return;

   if (faction_isFaction(f))
      ff = &faction_stack[f];
   else { /* f is invalid */
      WARN(_("Faction id '%d' is invalid."), f);
      return;
   }

   if (!faction_isFaction(o)) { /* o is invalid */
      WARN(_("Faction id '%d' is invalid."), o);
      return;
   }

   /* player cannot be made an ally this way */
   if (f==FACTION_PLAYER) {
      WARN(_("%d is the player faction"), f);
      return;
   }
   if (o==FACTION_PLAYER) {
      WARN(_("%d is the player faction"), o);
      return;
   }

   for (i=0;i<array_size(ff->allies);i++) {
      if (ff->allies[i] == o)
         return;
   }

   tmp = &array_grow( &ff->allies );
   *tmp = o;
}


/**
 * @brief Removes an ally from the faction's allies list.
 *
 *    @param f The faction to remove an ally from.
 *    @param o The other faction to remove as an ally.
 */
void faction_rmAlly( int f, int o )
{
   Faction *ff;
   int i;

   if (f==o) return;

   if (faction_isFaction(f))
      ff = &faction_stack[f];
   else { /* f is invalid */
      WARN(_("Faction id '%d' is invalid."), f);
      return;
   }

   for (i=0;i<array_size(ff->allies);i++) {
      if (ff->allies[i] == o) {
         array_erase( &ff->allies, &ff->allies[i], &ff->allies[i+1] );
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
   if (FABS(delta) > 1e-10) {
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
 *    @param source Source of the faction modifier.
 *
 *   Possible sources:
 *    - "kill" : Pilot death.
 *    - "distress" : Pilot distress signal.
 *    - "script" : Either a mission or an event.
 *
 */
void faction_modPlayer( int f, double mod, const char *source )
{
   int i;
   Faction *faction;

   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."), f);
      return;
   }
   faction = &faction_stack[f];

   /* Modify faction standing with parent faction. */
   faction_modPlayerLua( f, mod, source, 0 );

   /* Now mod allies to a lesser degree */
   for (i=0; i<array_size(faction->allies); i++)
      /* Modify faction standing */
      faction_modPlayerLua( faction->allies[i], mod, source, 1 );

   /* Now mod enemies */
   for (i=0; i<array_size(faction->enemies); i++)
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
 *    @param source Source of the faction modifier.
 *
 *   Possible sources:
 *    - "kill" : Pilot death.
 *    - "distress" : Pilot distress signal.
 *    - "script" : Either a mission or an event.
 *
 * @sa faction_modPlayer
 */
void faction_modPlayerSingle( int f, double mod, const char *source )
{
   if (!faction_isFaction(f)) {
      WARN(_("Faction id '%d' is invalid."), f);
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
      WARN(_("Faction id '%d' is invalid."), f);
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
      WARN(_("Faction id '%d' is invalid."), f);
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
      WARN(_("Faction id '%d' is invalid."), f);
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
      WARN(_("Faction id '%d' is invalid."), f);
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
 * @brief Gets the faction character associated to its standing with the player.
 *
 * Use this to do something like "#%c", faction_getColourChar( some_faction ) in the
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
 *    @return Human readable player's standing (in player's native language).
 */
const char *faction_getStandingText( int f )
{
   Faction *faction;
   const char *r;

   /* Escorts always have the same standing. */
   if (f == FACTION_PLAYER)
      return _("Escort");

   faction = &faction_stack[f];

   if ( faction->env == LUA_NOREF )
      return _("???");
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
         return _("???");
      }

      /* Parse return. */
      if ( !lua_isstring( naevL, -1 ) )
      {
         WARN( _("Lua script for faction '%s' did not return a string from 'faction_standing_text(...)'."), faction->name );
         r = _("???");
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

   /* Escorts always have the same standing. */
   if (f == FACTION_PLAYER)
      return _("Escort");

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
      WARN(_("Faction id '%d' is invalid."), a);
      return 0;
   }

   /* handle b */
   if (faction_isFaction(b))
      fb = &faction_stack[b];
   else { /* b is invalid */
      WARN(_("Faction id '%d' is invalid."), b);
      return 0;
   }

   /* player handled separately */
   if (a==FACTION_PLAYER) {
      return faction_isPlayerEnemy(b);
   }
   else if (b==FACTION_PLAYER) {
      return faction_isPlayerEnemy(a);
   }

   for (i=0;i<array_size(fa->enemies);i++)
      if (fa->enemies[i] == b)
         return 1;
   for (i=0;i<array_size(fb->enemies);i++)
      if (fb->enemies[i] == a)
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
      WARN(_("Faction id '%d' is invalid."), a);
      return 0;
   }

   /* handle b */
   if (faction_isFaction(b))
      fb = &faction_stack[b];
   else { /* b is invalid */
      WARN(_("Faction id '%d' is invalid."), b);
      return 0;
   }

   /* we assume player becomes allies with high rating */
   if (a==FACTION_PLAYER) {
      return faction_isPlayerFriend(b);
   }
   else if (b==FACTION_PLAYER) {
      return faction_isPlayerFriend(a);
   }

   for (i=0;i<array_size(fa->allies);i++)
      if (fa->allies[i] == b)
         return 1;
   for (i=0;i<array_size(fb->allies);i++)
      if (fb->allies[i] == a)
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
   if ((f<0) || (f>=array_size(faction_stack)))
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
   node   = parent->xmlChildrenNode;
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
      xmlr_strd(node,"ai",temp->ai);
      if (xml_isNode(node, "colour")) {
         ctmp = xml_get(node);
         if (ctmp != NULL)
            temp->colour = col_fromName(xml_raw(node));
         /* If no named colour is present, RGB attributes are used. */
         else {
            /* Initialize in case a colour channel is absent. */
            col = calloc( 1, sizeof(glColour) );

            xmlr_attr_float(node,"r",col->r);
            xmlr_attr_float(node,"g",col->g);
            xmlr_attr_float(node,"b",col->b);

            col->a = 1.;
            temp->colour = col;
         }
         continue;
      }

      if (xml_isNode(node, "spawn")) {
         if (temp->sched_env != LUA_NOREF)
            WARN(_("Faction '%s' has duplicate 'spawn' tag."), temp->name);
         nsnprintf( buf, sizeof(buf), FACTIONS_PATH"spawn/%s.lua", xml_raw(node) );
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
         nsnprintf( buf, sizeof(buf), FACTIONS_PATH"standing/%s.lua", xml_raw(node) );
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
         nsnprintf( buf, sizeof(buf), FACTIONS_PATH"equip/%s.lua", xml_raw(node) );
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
      WARN(_("Unable to read data from '%s'"), FACTION_DATA_PATH);
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
   Faction *base;
   int *tmp;

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

   assert( base != NULL );

   /* Create arrays, not much memory so it doesn't really matter. */
   base->allies = array_create( int );
   base->enemies = array_create( int );

   /* Parse social stuff. */
   node = parent->xmlChildrenNode;
   do {

      /* Grab the allies */
      if (xml_isNode(node,"allies")) {
         cur = node->xmlChildrenNode;

         do {
            if (xml_isNode(cur,"ally")) {
               tmp = &array_grow( &base->allies );
               *tmp = faction_get(xml_get(cur));
            }
         } while (xml_nextNode(cur));
      }

      /* Grab the enemies */
      if (xml_isNode(node,"enemies")) {
         cur = node->xmlChildrenNode;

         do {
            if (xml_isNode(cur,"enemy")) {
               tmp = &array_grow( &base->enemies );
               *tmp = faction_get(xml_get(cur));
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));
}


/**
 * @brief Resets player standing and flags of factions to default.
 */
void factions_reset (void)
{
   int i;
   for (i=0; i<array_size(faction_stack); i++) {
      faction_stack[i].player = faction_stack[i].player_def;
      faction_stack[i].flags = faction_stack[i].oflags;
   }
}


/**
 * @brief Loads up all the factions from the data file.
 *
 *    @return 0 on success.
 */
int factions_load (void)
{
   xmlNodePtr factions, node;
   int i, j, k, r;
   Faction *f, *sf;


   /* Load the document. */
   xmlDocPtr doc = xml_parsePhysFS( FACTION_DATA_PATH );
   if (doc == NULL)
      return -1;

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
   faction_stack = array_create( Faction );
   f = &array_grow( &faction_stack );
   memset( f, 0, sizeof(Faction) );
   f->name        = strdup("Player");
   f->flags       = FACTION_STATIC | FACTION_INVISIBLE;
   f->equip_env   = LUA_NOREF;
   f->env         = LUA_NOREF;
   f->sched_env   = LUA_NOREF;
   f->allies      = array_create( int );
   f->enemies     = array_create( int );

   /* First pass - gets factions */
   node = factions;
   do {
      if (xml_isNode(node,XML_FACTION_TAG)) {
         f = &array_grow( &faction_stack );

         /* Load faction. */
         faction_parse( f, node );
         f->oflags = f->flags;
      }
   } while (xml_nextNode(node));

   /* Second pass - sets allies and enemies */
   node = factions;
   do {
      if (xml_isNode(node,XML_FACTION_TAG))
         faction_parseSocial(node);
   } while (xml_nextNode(node));

   /* Third pass, Make allies/enemies symmetric. */
   for (i=0; i<array_size(faction_stack); i++) {
      f = &faction_stack[i];

      /* First run over allies and make sure it's mutual. */
      for (j=0; j < array_size(f->allies); j++) {
         sf = &faction_stack[ f->allies[j] ];

         r = 0;
         for (k=0; k < array_size(sf->allies); k++)
            if (sf->allies[k] == i) {
               r = 1;
               break;
            }

         /* Add ally if necessary. */
         if (r == 0)
            faction_addAlly( f->allies[j], i );
      }

      /* Now run over enemies. */
      for (j=0; j < array_size(f->enemies); j++) {
         sf = &faction_stack[ f->enemies[j] ];

         r = 0;
         for (k=0; k < array_size(sf->enemies); k++)
            if (sf->enemies[k] == i) {
               r = 1;
               break;
            }

         if (r == 0)
            faction_addEnemy( f->enemies[j], i );
      }
   }

   xmlFreeDoc(doc);

   DEBUG( n_( "Loaded %d Faction", "Loaded %d Factions", array_size(faction_stack) ), array_size(faction_stack) );

   return 0;
}


/**
 * @brief Frees a single faction.
 */
static void faction_freeOne( Faction *f )
{
   free(f->name);
   free(f->longname);
   free(f->displayname);
   free(f->ai);
   gl_freeTexture(f->logo_small);
   gl_freeTexture(f->logo_tiny);
   array_free(f->allies);
   array_free(f->enemies);
   if (f->sched_env != LUA_NOREF)
      nlua_freeEnv( f->sched_env );
   if (f->env != LUA_NOREF)
      nlua_freeEnv( f->env );
   if (!faction_isFlag(f, FACTION_DYNAMIC) && (f->equip_env != LUA_NOREF))
      nlua_freeEnv( f->equip_env );
}


/**
 * @brief Frees the factions.
 */
void factions_free (void)
{
   int i;

   /* free factions */
   for (i=0; i<array_size(faction_stack); i++)
      faction_freeOne( &faction_stack[i] );
   array_free(faction_stack);
   faction_stack = NULL;
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

   for (i=1; i<array_size(faction_stack); i++) { /* player is faction 0 */
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
               xmlr_attr_strd(cur, "name", str);
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
         *n = array_size(faction_stack);
         group = malloc(sizeof(int) * *n);
         for (i = 0; i < array_size(faction_stack); i++)
            group[i] = i;
         break;

      case 1: /* 'friendly' */
         for (i = 0; i < array_size(faction_stack); i++)
            if (areAllies(FACTION_PLAYER, i)) {
               (*n)++;
               group = realloc(group, sizeof(int) * *n);
               group[*n - 1] = i;
            }
         break;

      case 2: /* 'neutral' */
         for (i = 0; i < array_size(faction_stack); i++)
            if (!areAllies(FACTION_PLAYER, i) && !areEnemies(FACTION_PLAYER, i)) {
               (*n)++;
               group = realloc(group, sizeof(int) * *n);
               group[*n - 1] = i;
            }
         break;

      case 3: /* 'hostile' */
         for (i = 0; i < array_size(faction_stack); i++)
            if (areEnemies(FACTION_PLAYER, i)) {
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


/**
 * @brief Clears dynamic factions.
 */
void factions_clearDynamic (void)
{
   int i;
   Faction *f;
   for (i=0; i<array_size(faction_stack); i++) {
      f = &faction_stack[i];
      if (faction_isFlag(f, FACTION_DYNAMIC)) {
         faction_freeOne( f );
         array_erase( &faction_stack, f, f+1 );
         i--;
      }
   }
}


/**
 * @brief Dynamically add a faction.
 *
 *    @param base Faction to base it off (negative for none).
 *    @param name Name of the faction to set.
 *    @param display Display name to use.
 *    @param ai Default pilot AI to use (if NULL, inherit from base).
 */
int faction_dynAdd( int base, const char* name, const char* display, const char* ai )
{
   Faction *f, *bf;
   int i, *tmp;

   f = &array_grow( &faction_stack );
   memset( f, 0, sizeof(Faction) );
   f->name        = strdup( name );
   f->displayname = display==NULL ? NULL : strdup( display );
   f->ai          = ai==NULL ? NULL : strdup( ai );
   f->allies      = array_create( int );
   f->enemies     = array_create( int );
   f->equip_env   = LUA_NOREF;
   f->env         = LUA_NOREF;
   f->sched_env   = LUA_NOREF;
   f->flags       = FACTION_STATIC | FACTION_INVISIBLE | FACTION_DYNAMIC | FACTION_KNOWN;
   if (base>=0) {
      bf = &faction_stack[base];

      if (bf->ai!=NULL && ai==NULL)
         f->ai = strdup( bf->ai );
      if (bf->logo_small!=NULL)
         f->logo_small = gl_dupTexture( bf->logo_small );
      if (bf->logo_tiny!=NULL)
         f->logo_tiny = gl_dupTexture( bf->logo_tiny );

      for (i=0; i<array_size(bf->allies); i++) {
         tmp = &array_grow( &f->allies );
         *tmp = i;
      }
      for (i=0; i<array_size(bf->enemies); i++) {
         tmp = &array_grow( &f->enemies );
         *tmp = i;
      }

      f->player_def = bf->player_def;
      f->player = bf->player;
      f->colour = bf->colour;

      /* Lua stuff. */
      f->equip_env = bf->equip_env;
   }

   return f-faction_stack;
}
