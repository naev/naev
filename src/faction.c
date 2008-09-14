/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file faction.c
 *
 * @brief Handles the NAEV factions.
 */


#include "faction.h"

#include <malloc.h>
#include <string.h>

#include "xml.h"

#include "opengl.h"
#include "naev.h"
#include "log.h"
#include "pack.h"
#include "rng.h"
#include "colour.h"


#define XML_FACTION_ID     "Factions"   /* XML section identifier */
#define XML_FACTION_TAG    "faction"

#define FACTION_DATA       "dat/faction.xml" /**< Faction xml file. */
#define FACTION_LOGO_PATH  "gfx/logo/" /**< Path to logo gfx. */


#define PLAYER_ALLY        70 /**< above this player is considered ally */


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

   /* Enemies */
   int *enemies; /**< Enemies by ID of the faction. */
   int nenemies; /**< Number of enemies. */

   /* Allies */
   int *allies; /**< Allies by ID of the faction. */
   int nallies; /**< Number of allies. */

   int player_def; /**< Default player standing. */
   int player; /**< Standing with player - from -100 to 100 */
} Faction;


static Faction* faction_stack = NULL; /**< Faction stack. */
static int faction_nstack = 0; /**< Number of factions in the faction stack. */


/*
 * Prototypes
 */
/* static */
static int faction_isFaction( int f );
static void faction_sanitizePlayer( Faction* faction );
static Faction* faction_parse( xmlNodePtr parent );
static void faction_parseSocial( xmlNodePtr parent );
/* externed */
int pfaction_save( xmlTextWriterPtr writer );
int pfaction_load( xmlNodePtr parent );


/**
 * @fn int faction_get( const char* name )
 *
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
 * @fn char* faction_name( int f )
 *
 * @brief Get's a factions short name.
 *
 *    @param f Faction to get the name of.
 *    @return Name of the faction.
 */
char* faction_name( int f )
{
   if ((f < 0) || (f >= faction_nstack)) {
      WARN("Faction id '%d' is invalid.",f);
      return NULL;
   }
   return faction_stack[f].name;
}


/**
 * @fn char* faction_longname( int f )
 *
 * @brief Gets the faction's long name (formal).
 *
 *    @param f Faction to get the name of.
 *    @return The faction's long name.
 */
char* faction_longname( int f )
{
   if ((f < 0) || (f >= faction_nstack)) {
      WARN("Faction id '%d' is invalid.",f);
      return NULL;
   }
   if (faction_stack[f].longname != NULL)
      return faction_stack[f].longname;
   return faction_stack[f].name;
}


/**
 * @fn glTexture* faction_logoSmall( int f )
 *
 * @brief Gets the faction's small logo.
 *
 *    @param f Faction to get the logo of.
 *    @return The faction's small logo image.
 */
glTexture* faction_logoSmall( int f )
{
   if ((f < 0) || (f >= faction_nstack)) {
      WARN("Faction id '%d' is invalid.",f);
      return NULL;
   }
   return faction_stack[f].logo_small;
}


/**
 * @fn static void faction_sanitizePlayer( Faction* faction )
 *
 * @brief Sanitizes player faction standing.
 *
 *    @param faction Faction to sanitize.
 */
static void faction_sanitizePlayer( Faction* faction )
{
   if (faction->player > 100)
      faction->player = 100;
   else if (faction->player < -100)
      faction->player = -100;
}


/**
 * @fn void faction_modPlayer( int f, int mod )
 *
 * @brief Modifies the player's standing with a faction.
 *
 * Affects enemies and allies too.
 *
 *    @param f Faction to modify player's standing.
 *    @param mod Modifier to modify by.
 *
 * @sa faction_modPlayerRaw
 */
void faction_modPlayer( int f, int mod )
{
   int i;
   Faction *faction, *ally, *enemy;

   if (!faction_isFaction(f)) {
      WARN("%d is an invalid faction", f);
      return;
   }

   faction = &faction_stack[f];

   /* Faction in question gets direct increment. */
   faction->player += mod;
   faction_sanitizePlayer(faction);

   /* Now mod allies to a lesser degree */
   for (i=0; i<faction->nallies; i++) {
      ally = &faction_stack[faction->allies[i]];

      ally->player += RNG(0,(mod*3)/4);
      faction_sanitizePlayer(ally);
   }

   /* Now mod enemies */
   for (i=0; i<faction->nenemies; i++) {
      enemy = &faction_stack[faction->enemies[i]];

      enemy->player -= MIN(1,RNG(0,(mod*3)/4));
      faction_sanitizePlayer(enemy);
   }
}


/**
 * @fn void faction_modPlayerRaw( int f, int mod )
 *
 * @brief Modifies the player's standing without affecting others.
 *
 * Does not affect allies nor enemies.
 *
 *    @param f Faction whose standing to modiy.
 *    @param mod Amount to modiy standing by.
 *
 * @sa faction_modPlayer
 */
void faction_modPlayerRaw( int f, int mod )
{
   Faction *faction;

   if (!faction_isFaction(f)) {
      WARN("%d is an invalid faction", f);
      return;
   }

   faction = &faction_stack[f];

   faction->player += mod;
   faction_sanitizePlayer(faction);
}


/**
 * @fn int faction_getPlayer( int f )
 *
 * @brief Gets the player's standing with a faction.
 *
 *    @param f Faction to get player's standing from.
 *    @return The standing the player has with the faction.
 */
int faction_getPlayer( int f )
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
 * @fn glColour* faction_getColour( int f )
 *
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
 * @fn char *faction_getStanding( int mod )
 *
 * @brief Get's the player's standing in human readable form.
 *
 *    @param mod Player's standing.
 *    @return Human readable player's standing.
 */
static char *player_standings[] = {
   "Hero", /* 0 */
   "Admired",
   "Great",
   "Good",
   "Decent",
   "Wanted", /* 5 */
   "Outlaw",
   "Criminal",
   "Enemy"
};
#define STANDING(m,i)  if (mod >= m) return player_standings[i];
char *faction_getStanding( int mod )
{
   STANDING(  90, 0 );
   STANDING(  70, 1 );
   STANDING(  50, 2 );
   STANDING(  30, 3 );
   STANDING(   0, 4 );
   STANDING( -15, 5 );
   STANDING( -30, 6 );
   STANDING( -50, 7 );
   return player_standings[8];
}
#undef STANDING


/**
 * @fn int areEnemies( int a, int b)
 *
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
         if (faction_stack[b].player < 0)
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
         if (faction_stack[a].player < 0)
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
 * @fn int areAllies( int a, int b )
 *
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

   if (a==b) return 1;

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
 * @fn static int faction_isFaction( int f )
 *
 * @brief Checks whether or not a faction is valid.
 *
 *    @param f Faction to check for validity.
 *    @return 1 if faction is valid, 0 otherwise.
 */
static int faction_isFaction( int f )
{
   if ((f<0) || (f>=faction_nstack))
      return 0;
   return 1;
}


/**
 * @fn static Faction* faction_parse( xmlNodePtr parent )
 *
 * @brief Parses a single faction, but doesn't set the allies/enemies bit.
 *
 *    @param parent Parent node to extract faction from.
 *    @return Faction created from parent node.
 */
static Faction* faction_parse( xmlNodePtr parent )
{
   xmlNodePtr node;
   int player;
   Faction* temp;
   char buf[PATH_MAX];
   
   temp = CALLOC_ONE(Faction);

   temp->name = xml_nodeProp(parent,"name");
   if (temp->name == NULL) WARN("Faction from "FACTION_DATA" has invalid or no name");

   player = 0;
   node = parent->xmlChildrenNode;
   do {
      /* Can be 0 or negative, so we have to take that into account. */
      if (xml_isNode(node,"player")) {
         temp->player_def = xml_getInt(node);
         player = 1;
         continue;
      }

      xmlr_strd(node,"longname",temp->longname);

      if (xml_isNode(node,"logo")) {
         snprintf( buf, PATH_MAX, FACTION_LOGO_PATH"%s_small.png", xml_get(node));
         temp->logo_small = gl_newImage(buf);
         continue;
      }
   } while (xml_nextNode(node));

   if (player==0) DEBUG("Faction '%s' missing player tag.", temp->name);

   return temp;
}


/*
 * Parses the social tidbits: allies and enemies.
 */
static void faction_parseSocial( xmlNodePtr parent )
{
   xmlNodePtr node, cur;
   char *buf;
   Faction *base;
   int mod;

   buf = xml_nodeProp(parent,"name");
   base = &faction_stack[faction_get(buf)];
   free(buf);

   node = parent->xmlChildrenNode;
   do {

      /* Grab the allies */
      if (xml_isNode(node,"allies")) {
         cur = node->xmlChildrenNode;

         do {
            if (xml_isNode(cur,"ally")) {
               mod = faction_get(xml_get(cur));
               base->nallies++;
               base->allies = realloc(base->allies, sizeof(int)*base->nallies);
               base->allies[base->nallies-1] = mod;
            }
         } while (xml_nextNode(cur));
      }

      /* Grab the enemies */
      if (xml_isNode(node,"enemies")) {
         cur = node->xmlChildrenNode;

         do {                           
            if (xml_isNode(cur,"enemy")) {
               mod = faction_get(xml_get(cur));
               base->nenemies++;
               base->enemies = realloc(base->enemies, sizeof(int)*base->nenemies);
               base->enemies[base->nenemies-1] = mod;
            }
         } while (xml_nextNode(cur));

      }
   } while (xml_nextNode(node));
}


/**
 * @fn void factions_reset (void)
 *
 * @brief Resets the player's standing with the factions to default.
 */
void factions_reset (void)
{
   int i;
   for (i=0; i<faction_nstack; i++)
      faction_stack[i].player = faction_stack[i].player_def;
}


/**
 * @fn int factions_load (void)
 *
 * @brief Loads up all the factions from the data file.
 *
 *    @return 0 on success.
 */
int factions_load (void)
{
   uint32_t bufsize;
   char *buf = pack_readfile(DATA, FACTION_DATA, &bufsize);

   xmlNodePtr factions, node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   Faction* temp = NULL;

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
   faction_nstack++;

   /* First pass - gets factions */
   node = factions;
   do {
      if (xml_isNode(node,XML_FACTION_TAG)) {
         temp = faction_parse(node);
         faction_stack = realloc(faction_stack, sizeof(Faction)*(++faction_nstack));        
         memcpy(faction_stack+faction_nstack-1, temp, sizeof(Faction));                  
         free(temp);
      }
   } while (xml_nextNode(node));

   /* Second pass - sets allies and enemies */
   node = factions;
   do {
      if (xml_isNode(node,XML_FACTION_TAG))
         faction_parseSocial(node);
   } while (xml_nextNode(node));


   xmlFreeDoc(doc);
   free(buf);

   DEBUG("Loaded %d Faction%s", faction_nstack, (faction_nstack==1) ? "" : "s" );

   return 0;
}


/**
 * @fn void factions_free (void)
 *
 * @brief Frees the factions.
 */
void factions_free (void)
{
   int i;

   /* free factions */
   for (i=0; i<faction_nstack; i++) {
      free(faction_stack[i].name);
      if (faction_stack[i].longname != NULL) free(faction_stack[i].longname);
      if (faction_stack[i].logo_small != NULL) gl_freeTexture(faction_stack[i].logo_small);
      if (faction_stack[i].nallies > 0) free(faction_stack[i].allies);
      if (faction_stack[i].nenemies > 0) free(faction_stack[i].enemies);
   }
   free(faction_stack);
   faction_stack = NULL;
   faction_nstack = 0;
}


/**
 * @fn int pfaction_save( xmlTextWriterPtr writer )
 *
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
      xmlw_str(writer, "%d", faction_stack[i].player);

      xmlw_endElem(writer); /* "faction" */
   }

   xmlw_endElem(writer); /* "factions" */

   return 0;
}


/**
 * @fn int pfaction_load( xmlNodePtr parent )
 *
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
                  faction_stack[faction].player = xml_getInt(cur);
               free(str);
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   return 0;
}


