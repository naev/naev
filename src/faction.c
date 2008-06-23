/*
 * See Licensing and Copyright notice in naev.h
 */



#include "faction.h"

#include <malloc.h>
#include <string.h>

#include "xml.h"

#include "naev.h"
#include "log.h"
#include "pack.h"
#include "rng.h"


#define XML_FACTION_ID     "Factions"   /* XML section identifier */
#define XML_FACTION_TAG    "faction"

#define FACTION_DATA       "dat/faction.xml"


#define PLAYER_ALLY        70 /* above this player is considered ally */


typedef struct Faction_ {
   char* name; /* Normal Name */
   char* longname; /* Long Name */

   /* Enemies */
   int* enemies;
   int nenemies;

   /* Allies */
   int* allies;
   int nallies;

   int player; /* standing with player - from -100 to 100 */
} Faction;


static Faction* faction_stack = NULL;
static int faction_nstack = 0;


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


/*
 * returns the faction of name name
 */
int faction_get( const char* name )
{
   int i;
   for (i=0; i<faction_nstack; i++) 
      if (strcmp(faction_stack[i].name, name)==0)
         break;

   if (i != faction_nstack)
      return i;
   DEBUG("Faction '%s' not found in stack.", name);
   return -1;
}


/*
 * returns the faction's name
 */
char* faction_name( int f )
{
   return faction_stack[f].name;
}


/*
 * returns the faction's long name (formal)
 */
char* faction_longname( int f )
{
   if (faction_stack[f].longname != NULL)
      return faction_stack[f].longname;
   return faction_stack[f].name;
}


/*
 * Sanitizes player faction standing.
 */
static void faction_sanitizePlayer( Faction* faction )
{
   if (faction->player > 100)
      faction->player = 100;
   else if (faction->player < -100)
      faction->player = -100;
}


/*
 * Modifies the player's standing with a faction.
 */
void faction_modPlayer( int f, int mod )
{
   int i;
   Faction *faction, *ally, *enemy;

   if (faction_isFaction(f)) {
      faction = &faction_stack[f];

      /* Faction in question gets direct increment. */
      faction->player += mod;
      faction_sanitizePlayer(faction);

      /* Now mod allies to a lesser degree */
      for (i=0; i<faction->nallies; i++) {
         ally = &faction_stack[faction->allies[i]];

         ally->player += RNG(0,mod/2);
         faction_sanitizePlayer(ally);
      }

      /* Now mod enemies */
      for (i=0; i<faction->nenemies; i++) {
         enemy = &faction_stack[faction->enemies[i]];

         enemy->player -= RNG(0,mod/2);
         faction_sanitizePlayer(enemy);
      }
   }
   else {
      DEBUG("%d is an invalid faction", f);
      return;
   }
}


/*
 * Gets the player's standing with a faction.
 */
int faction_getPlayer( int f )
{
   if (faction_isFaction(f)) {
      return faction_stack[f].player;
   }    
   else {
      DEBUG("%d is an invalid faction", f);
      return -1000;
   }
}


/*
 * Returns the player's standing
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


/*
 * returns 1 if Faction a and b are enemies
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
         DEBUG("areEnemies: %d is an invalid faction", b);
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
         DEBUG("areEnemies: %d is an invalid faction", a);
         return 0;
      }
   }


   /* handle a */
   if (faction_isFaction(a)) fa = &faction_stack[a];
   else { /* a isn't valid */
      DEBUG("areEnemies: %d is an invalid faction", a);
      return 0;
   }

   /* handle b */
   if (faction_isFaction(b)) fb = &faction_stack[b];
   else { /* b is invalid */
      DEBUG("areEnemies: %d is an invalid faction", b);
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


/*
 * returns 1 if Faction a and b are allies
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
         DEBUG("%d is an invalid faction", b);
         return 0;
      }
   }
   if (b==FACTION_PLAYER) {
      if (faction_isFaction(a)) {
         if (faction_stack[a].player > PLAYER_ALLY) return 1;
         else return 0;
      }    
      else {
         DEBUG("%d is an invalid faction", a);
         return 0;
      }
   }


   if ((a==FACTION_PLAYER) || (b==FACTION_PLAYER)) /* player has no allies */
      return 0;

   /* handle a */
   if (faction_isFaction(a)) fa = &faction_stack[a];
   else { /* a isn't valid */
      DEBUG("%d is an invalid faction", a);
      return 0;
   }

   /* handle b */
   if (faction_isFaction(b)) fb = &faction_stack[b];
   else { /* b is invalid */
      DEBUG("%d is an invalid faction", b);
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


/*
 * returns true if f is a faction
 */
static int faction_isFaction( int f )
{
   if ((f<0) || (f>=faction_nstack))
      return 0;
   return 1;
}


/*
 * parses a single faction, but doesn't set the allies/enemies bit
 */
static Faction* faction_parse( xmlNodePtr parent )
{
   xmlNodePtr node;
   int player;
   Faction* temp;
   
   temp = CALLOC_ONE(Faction);

   temp->name = xml_nodeProp(parent,"name");
   if (temp->name == NULL) WARN("Faction from "FACTION_DATA" has invalid or no name");

   player = 0;
   node = parent->xmlChildrenNode;
   do {
      if (xml_isNode(node,"player")) {
         temp->player = xml_getInt(node);
         player = 1;
         continue;
      }
      xmlr_strd(node,"longname",temp->longname);
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


/*
 * loads up all the factions
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
   faction_stack[0].name = strdup("Player");
   faction_stack[0].longname = NULL;
   faction_stack[0].nallies = 0;
   faction_stack[0].nenemies = 0;
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
   xmlCleanupParser();

   DEBUG("Loaded %d Faction%s", faction_nstack, (faction_nstack==1) ? "" : "s" );

   return 0;
}

void factions_free (void)
{
   int i;

   /* free factions */
   for (i=0; i<faction_nstack; i++) {
      free(faction_stack[i].name);
      if (faction_stack[i].longname != NULL) free(faction_stack[i].longname);
      if (faction_stack[i].nallies > 0) free(faction_stack[i].allies);
      if (faction_stack[i].nenemies > 0) free(faction_stack[i].enemies);
   }
   free(faction_stack);
   faction_stack = NULL;
   faction_nstack = 0;
}


/*
 * player faction saving/loading
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
int pfaction_load( xmlNodePtr parent )
{
   xmlNodePtr node, cur;
   char *str;

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"factions")) {
         cur = node->xmlChildrenNode;
         do {
            if (xml_isNode(cur,"faction")) {
               xmlr_attr(cur,"name",str); 
               faction_stack[faction_get(str)].player = xml_getInt(cur);
               free(str);
            }
         } while (xml_nextNode(cur));
      }
   } while (xml_nextNode(node));

   return 0;
}


