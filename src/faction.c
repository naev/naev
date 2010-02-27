/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file faction.c
 *
 * @brief Handles the NAEV factions.
 */


#include "faction.h"

#include "naev.h"

#include <stdlib.h>
#include <string.h>

#include "nxml.h"

#include "opengl.h"
#include "log.h"
#include "ndata.h"
#include "rng.h"
#include "colour.h"


#define XML_FACTION_ID     "Factions"   /**< XML section identifier */
#define XML_FACTION_TAG    "faction" /**< XML tag identifier. */

#define FACTION_DATA       "dat/faction.xml" /**< Faction xml file. */
#define FACTION_LOGO_PATH  "gfx/logo/" /**< Path to logo gfx. */


#define PLAYER_ALLY        70. /**< Above this player is considered ally. */
#define PLAYER_ENEMY       0. /**< Below this the player is considered an enemy. */


#define CHUNK_SIZE         32 /**< Size of chunk for allocation. */


#define FACTION_STATIC        (1<<0) /**< Faction doesn't change standing with player. */
#define FACTION_INVISIBLE     (1<<1) /**< Faction isn't exposed to the player. */

#define faction_setFlag(fa,f) ((fa)->flags |= (f))
#define faction_isFlag(fa,f)  ((fa)->flags & (f))


/**
 * @struct Faction
 *
 * @brief Represents a faction.
 */
typedef struct Faction_ {
   char *name; /**< Normal Name. */
   char *longname; /**< Long Name. */

   /* Graphics. */
   glTexture *logo_small; /**< Small logo. */
   glTexture *logo_tiny; /**< Tiny logo. */
   glColour *colour; /**< Faction specific colour. */

   /* Enemies */
   int *enemies; /**< Enemies by ID of the faction. */
   int nenemies; /**< Number of enemies. */

   /* Allies */
   int *allies; /**< Allies by ID of the faction. */
   int nallies; /**< Number of allies. */

   double player_def; /**< Default player standing. */
   double player; /**< Standing with player - from -100 to 100 */

   unsigned int flags; /**< Flags affecting the faction. */
} Faction;


static Faction* faction_stack = NULL; /**< Faction stack. */
int faction_nstack = 0; /**< Number of factions in the faction stack. */


/*
 * Prototypes
 */
/* static */
static void faction_sanitizePlayer( Faction* faction );
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
   for (i=0; i<faction_nstack; i++) 
      if (strcmp(faction_stack[i].name, name)==0)
         break;

   if (i != faction_nstack)
      return i;
   WARN("Faction '%s' not found in stack.", name);
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
   for (i=0; i<faction_nstack; i++) {
      if (!faction_isFlag( &faction_stack[i], FACTION_INVISIBLE ))
         f[m++] = i;
   }

   *n = m;
   return f;
}


/**
 * @brief Get's a factions short name.
 *
 *    @param f Faction to get the name of.
 *    @return Name of the faction.
 */
char* faction_name( int f )
{
   if (!faction_isFaction(f)) {
      WARN("Faction id '%d' is invalid.",f);
      return NULL;
   }
   /* Don't want player to see his escorts as "Player" faction. */
   if (f == FACTION_PLAYER)
      return "Escort";
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
      WARN("Faction id '%d' is invalid.",f);
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
      WARN("Faction id '%d' is invalid.",f);
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
      WARN("Faction id '%d' is invalid.",f);
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
glColour* faction_colour( int f )
{
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
   if (!faction_isFaction(f)) {
      WARN("Faction id '%d' is invalid.",f);
      return NULL;
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
   if (!faction_isFaction(f)) {
      WARN("Faction id '%d' is invalid.",f);
      return NULL;
   }
   *n = faction_stack[f].nallies;
   return faction_stack[f].allies;
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
 * @brief Modifies the player's standing with a faction.
 *
 * Affects enemies and allies too.
 *
 *    @param f Faction to modify player's standing.
 *    @param mod Modifier to modify by.
 *
 * @sa faction_modPlayerRaw
 */
void faction_modPlayer( int f, double mod )
{
   int i;
   Faction *faction;
   double m;

   if (!faction_isFaction(f)) {
      WARN("%d is an invalid faction", f);
      return;
   }

   /* Modify faction standing with parent faction. */
   faction_modPlayerRaw( f, mod );

   /* Now mod allies to a lesser degree */
   faction = &faction_stack[f];
   for (i=0; i<faction->nallies; i++) {

      /* Enemies are made faster. */
      m = RNG_2SIGMA()/4. + 0.5;
      if (mod > 0.)
         m *= 0.75;

      /* Modify faction standing */
      faction_modPlayerRaw( faction->allies[i], m*mod );
   }

   /* Now mod enemies */
   for (i=0; i<faction->nenemies; i++) {

      /* Enemies are made faster. */
      m = RNG_2SIGMA()/4. + 0.5;
      if (mod < 0.)
         m *= 0.75;

      /* Modify faction standing. */
      faction_modPlayerRaw( faction->enemies[i], -m*mod );
   }
}


/**
 * @brief Modifies the player's standing without affecting others.
 *
 * Does not affect allies nor enemies.
 *
 *    @param f Faction whose standing to modiy.
 *    @param mod Amount to modiy standing by.
 *
 * @sa faction_modPlayer
 */
void faction_modPlayerRaw( int f, double mod )
{
   Faction *faction;

   if (!faction_isFaction(f)) {
      WARN("%d is an invalid faction", f);
      return;
   }

   faction = &faction_stack[f];

   /* Make sure it's not static. */
   if (faction_isFlag(faction, FACTION_STATIC))
      return;

   faction->player += mod;
   faction_sanitizePlayer(faction);
}


/**
 * @brief Gets the player's standing with a faction.
 *
 *    @param f Faction to get player's standing from.
 *    @return The standing the player has with the faction.
 */
double faction_getPlayer( int f )
{
   if (faction_isFaction(f)) {
      return faction_stack[f].player;
   }    
   else {
      WARN("%d is an invalid faction", f);
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
   if (faction_isFaction(f)) {
      return faction_stack[f].player_def;
   }    
   else {
      WARN("%d is an invalid faction", f);
      return -1000;
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
glColour* faction_getColour( int f )
{
   if (f<0) return &cInert;
   else if (areAllies(FACTION_PLAYER,f)) return &cFriend;
   else if (areEnemies(FACTION_PLAYER,f)) return &cHostile;
   else return &cNeutral;
}


/**
 * @brief Gets the faction character assosciated to it's standing with the player.
 *
 * Use this to do something like "\e%c", faction_getColourChar( some_faction ) in the
 *  font print routines.
 *
 *    @param f Faction to get the colour of based on player's standing.
 *    @return The character assosciated to the faction.
 */
char faction_getColourChar( int f )
{
   if (f<0) return 'I';
   else if (areEnemies(FACTION_PLAYER,f)) return 'H';
   else if (areAllies(FACTION_PLAYER,f)) return 'F';
   else return 'N';
}


#define STANDING(m,s)  if (mod >= m) return s /**< Hack to get standings easily. */
/**
 * @brief Get's the player's standing in human readable form.
 *
 *    @param mod Player's standing.
 *    @return Human readable player's standing.
 */
char *faction_getStanding( double mod )
{
   STANDING(  90., "Hero" );
   STANDING(  70., "Admired" );
   STANDING(  50., "Great" );
   STANDING(  30., "Good" );
   STANDING(   0., "Decent"  );
   STANDING( -15., "Wanted" );
   STANDING( -30., "Outlaw" );
   STANDING( -50., "Criminal" );
   return "Enemy";
}
#undef STANDING


/**
 * @brief Gets the broad faction standing.
 *
 *    @param mod Player's standing.
 *    @return Human readable broad player's standing.
 */
char *faction_getStandingBroad( double mod )
{
   if (mod > PLAYER_ALLY) return "Friendly";
   else if (mod > PLAYER_ENEMY) return "Neutral";
   return "Hostile";

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

   /* player handled seperately */
   if (a==FACTION_PLAYER) {
      if (faction_isFaction(b)) {
         if (faction_stack[b].player < PLAYER_ENEMY)
            return 1;
         else return 0;
      }
      else {
         WARN("areEnemies: %d is an invalid faction", b);
         return 0;
      }
   }
   if (b==FACTION_PLAYER) {
      if (faction_isFaction(a)) {
         if (faction_stack[a].player < PLAYER_ENEMY)
            return 1;
         else return 0;
      }
      else {
         WARN("areEnemies: %d is an invalid faction", a);
         return 0;
      }
   }

   /* handle a */
   if (faction_isFaction(a)) fa = &faction_stack[a];
   else { /* a isn't valid */
      WARN("areEnemies: %d is an invalid faction", a);
      return 0;
   }

   /* handle b */
   if (faction_isFaction(b)) fb = &faction_stack[b];
   else { /* b is invalid */
      WARN("areEnemies: %d is an invalid faction", b);
      return 0;
   }

   /* both are factions */
   if (fa && fb) {
      for (i=0;i<fa->nenemies;i++)
         if (fa->enemies[i] == b)
            return 1;
      for (i=0;i<fb->nenemies;i++)
         if(fb->enemies[i] == a)
            return 1;
   }

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

   /* we assume player becomes allies with high rating */
   if (a==FACTION_PLAYER) {
      if (faction_isFaction(b)) {
         if (faction_stack[b].player > PLAYER_ALLY) return 1;
         else return 0;
      }
      else {
         WARN("%d is an invalid faction", b);
         return 0;
      }
   }
   if (b==FACTION_PLAYER) {
      if (faction_isFaction(a)) {
         if (faction_stack[a].player > PLAYER_ALLY) return 1;
         else return 0;
      }    
      else {
         WARN("%d is an invalid faction", a);
         return 0;
      }
   }


   if ((a==FACTION_PLAYER) || (b==FACTION_PLAYER)) /* player has no allies */
      return 0;

   /* handle a */
   if (faction_isFaction(a)) fa = &faction_stack[a];
   else { /* a isn't valid */
      WARN("%d is an invalid faction", a);
      return 0;
   }

   /* handle b */
   if (faction_isFaction(b)) fb = &faction_stack[b];
   else { /* b is invalid */
      WARN("%d is an invalid faction", b);
      return 0;
   }

   /* both are factions */
   if (fa && fb) {
      for (i=0;i<fa->nallies;i++)
         if (fa->allies[i] == b)
            return 1;
      for (i=0;i<fb->nallies;i++)
         if(fb->allies[i] == a)
            return 1;
   }
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
   char buf[PATH_MAX];

   /* Clear memory. */
   memset( temp, 0, sizeof(Faction) );

   temp->name = xml_nodeProp(parent,"name");
   if (temp->name == NULL)
      WARN("Faction from "FACTION_DATA" has invalid or no name");

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

      xmlr_strd(node,"longname",temp->longname);
      if (xml_isNode(node, "colour")) {
         temp->colour = col_fromName(xml_raw(node));
         continue;
      }

      if (xml_isNode(node,"logo")) {
         snprintf( buf, PATH_MAX, FACTION_LOGO_PATH"%s_small.png", xml_get(node));
         temp->logo_small = gl_newImage(buf, 0);
         snprintf( buf, PATH_MAX, FACTION_LOGO_PATH"%s_tiny.png", xml_get(node));
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

      DEBUG("Unknown node '%s' in faction '%s'",node->name,temp->name);
   } while (xml_nextNode(node));

   if (player==0)
      DEBUG("Faction '%s' missing player tag.", temp->name);

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

   buf = xml_nodeProp(parent,"name");
   base = &faction_stack[faction_get(buf)];
   free(buf);

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
   uint32_t bufsize;
   char *buf = ndata_read( FACTION_DATA, &bufsize);

   xmlNodePtr factions, node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode; /* Factions node */
   if (!xml_isNode(node,XML_FACTION_ID)) {
      ERR("Malformed "FACTION_DATA" file: missing root element '"XML_FACTION_ID"'");
      return -1;
   }

   factions = node->xmlChildrenNode; /* first faction node */
   if (factions == NULL) {
      ERR("Malformed "FACTION_DATA" file: does not contain elements");
      return -1;
   }

   /* player faction is hardcoded */
   faction_stack = malloc( sizeof(Faction) );
   memset(faction_stack, 0, sizeof(Faction) );
   faction_stack[0].name = strdup("Player");
   faction_stack[0].flags = FACTION_STATIC | FACTION_INVISIBLE;
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
            WARN("Faction: %s and %s aren't completely mutual allies!",
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
            WARN("Faction: %s and %s aren't completely mutual enemies!",
                  f->name, sf->name );
      }
   }
#endif /* DEBUGGING */

   xmlFreeDoc(doc);
   free(buf);

   DEBUG("Loaded %d Faction%s", faction_nstack, (faction_nstack==1) ? "" : "s" );

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
      if (faction_stack[i].logo_small != NULL)
         gl_freeTexture(faction_stack[i].logo_small);
      if (faction_stack[i].logo_tiny != NULL)
         gl_freeTexture(faction_stack[i].logo_tiny);
      if (faction_stack[i].nallies > 0)
         free(faction_stack[i].allies);
      if (faction_stack[i].nenemies > 0)
         free(faction_stack[i].enemies);
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
      xmlw_startElem(writer,"faction");

      xmlw_attr(writer,"name","%s",faction_stack[i].name);
      xmlw_str(writer, "%f", faction_stack[i].player);

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
   xmlNodePtr node, cur;
   char *str;
   int faction;

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"factions")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"faction")) {
               xmlr_attr(cur,"name",str); 
               faction = faction_get(str);
               if (faction != -1) /* Faction is valid. */
                  faction_stack[faction].player = xml_getFloat(cur);
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
int* faction_getGroup( int *n, int which ) {
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
