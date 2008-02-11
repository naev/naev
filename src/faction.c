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


#define XML_FACTION_ID     "Factions"   /* XML section identifier */
#define XML_FACTION_TAG    "faction"
#define XML_ALLIANCE_ID    "Alliances"
#define XML_ALLIANCE_TAG   "alliance"
#define XML_ENEMIES_ID     "Enemies"
#define XML_ENEMIES_TAG    "enemies"

#define FACTION_DATA  "dat/faction.xml"


#define ALLIANCE_OFFSET    27182 /* special offset for alliances */


typedef struct Faction_ {
   char* name;

   int* enemies;
   int nenemies;
   int* allies;
   int nallies;

} Faction;


static Faction* faction_stack = NULL;
static int nfactions = 0;


/*
 * used to save alliances
 */
typedef struct Alliance_ {
   char* name;
   int* factions;
   int nfactions;
} Alliance;


/*
 * alliance stack
 */
static Alliance* alliances = NULL;
static int nalliances = 0;


/*
 * Prototypes
 */
static Faction* faction_parse( xmlNodePtr parent );
static void alliance_parse( xmlNodePtr parent );
static void enemies_parse( xmlNodePtr parent );
static Alliance* alliance_get( char* name );


/*
 * returns the faction of name name
 */
int faction_get( const char* name )
{
   int i;
   for (i=0; i<nfactions; i++) 
      if (strcmp(faction_stack[i].name, name)==0)
         break;

   if (i != nfactions)
      return i;
   DEBUG("Faction '%s' not found in stack.", name);
   return -1;
}


/*
 * returns the id of an alliance
 */
int faction_getAlliance( char* name )
{
   int i;
   for (i=0; i<nalliances; i++)
      if (strcmp(alliances[i].name, name)==0)
         break;
   
   if (i != nalliances)
      return ALLIANCE_OFFSET + i;
   DEBUG("Alliance '%s' not found in stack.", name);
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
 * returns the alliance of name name
 */
static Alliance* alliance_get( char* name )
{
   int i;
   for (i=0; i<nalliances; i++)
      if (strcmp(alliances[i].name, name)==0)
         break;
   
   if (i != nalliances)
      return alliances+i;
   return NULL;
}


/*
 * returns 1 if Faction a and b are enemies
 */
int areEnemies( int a, int b)
{
   Faction *fa, *fb;
   int i;

   if (a==b) return 0; /* luckily our factions aren't masochistic */

   /* handle a */
   if (faction_isFaction(a)) fa = &faction_stack[a];
   else { /* a isn't valid */
      DEBUG("areEnemies: %d is an invalid faction/alliance", a);
      return 0;
   }

   /* handle b */
   if (faction_isFaction(b)) fb = &faction_stack[b];
   else { /* b is invalid */
      DEBUG("areEnemies: %d is an invalid faction/alliance", b);
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

   /* handle a */
   if (faction_isFaction(a)) fa = &faction_stack[a];
   else { /* a isn't valid */
      DEBUG("areEnemies: %d is an invalid faction/alliance", a);
      return 0;
   }

   /* handle b */
   if (faction_isFaction(b)) fb = &faction_stack[b];
   else { /* b is invalid */
      DEBUG("areEnemies: %d is an invalid faction/alliance", b);
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
 * is faction f part of alliance a?
 */
int faction_ofAlliance( int f, int a )
{
   int i;
   Alliance* aa;

   /* validize faction and alliance */
   if (!faction_isFaction(f)) {
      DEBUG("faction_ofAlliance: invalid faction '%d'", f);
      return 0;
   }
   if (!faction_isAlliance(a)) {
      DEBUG("faction_ofAlliance: invalid alliance '%d'", a);
      return 0;
   }

   aa = &alliances[a-ALLIANCE_OFFSET];

   for (i=0; i<aa->nfactions; i++)
      if (aa->factions[i] == f)
         return 1;
   return 0;
}


/*
 * returns true if a is an alliance
 */
int faction_isAlliance( int a )
{
   if ((a<ALLIANCE_OFFSET) || (a>=ALLIANCE_OFFSET+nalliances))
      return 0;
   return 1;
}


/*
 * returns true if f is a faction
 */
int faction_isFaction( int f )
{
   if ((f<0) || (f>=nfactions))
      return 0;
   return 1;
}


/*
 * parses a single faction, but doesn't set the allies/enemies bit
 */

static Faction* faction_parse( xmlNodePtr parent )
{
   Faction* temp = CALLOC_ONE(Faction);

   temp->name = (char*)xmlGetProp(parent,(xmlChar*)"name");
   if (temp->name == NULL) WARN("Faction from "FACTION_DATA" has invalid or no name");

   return temp;
}


/*
 * sets the allies bit in the faction_stack
 */
static void alliance_parse( xmlNodePtr parent )
{
   Alliance* a;
   int *i,j,n,m;
   Faction* ft;
   xmlNodePtr node, cur;

   node = parent->xmlChildrenNode;

   do {                                
      if ((node->type==XML_NODE_START) &&
            (strcmp((char*)node->name,XML_ALLIANCE_TAG)==0)) {

         /* alloc a new alliance */
         alliances = realloc(alliances, sizeof(Alliance)*(++nalliances));
         alliances[nalliances-1].name = (char*)xmlGetProp(node,(xmlChar*)"name");
         alliances[nalliances-1].factions = NULL;
         alliances[nalliances-1].nfactions = 0;

         /* parse the current alliance's allies */
         cur = node->xmlChildrenNode;
         do {
            if (strcmp((char*)cur->name,"ally")==0) {

               /* add the faction (and pointers to make life easier) */
               a = alliances + nalliances-1;
               i = &a->nfactions;
               (*i)++;

               /* load the faction */
               a->factions = realloc( a->factions, (*i)*sizeof(int) );
               a->factions[(*i)-1] = faction_get((char*)cur->children->content);
                     
               if (a->factions[(*i)-1] == -1)
                  WARN("Faction '%s' in alliance '%s' doesn't exist in "FACTION_DATA,
                        (char*)cur->children->content, a->name );
            }
         } while (xml_nextNode(cur));


         /* set the stuff needed in faction_stack */
         for (j=0; j<(*i); j++) {
            ft = &faction_stack[ a->factions[j] ];
            ft->nallies += (*i)-1;
            ft->allies = realloc( ft->allies, (ft->nallies)*sizeof(int));
            for (n=0,m=0; n<(*i); n++,m++) { /* add as ally all factions except self */
               if (n==j) m--;
               else if (n!=j)
                  ft->allies[ ft->nallies-(*i)+1+m ] = a->factions[n];
            }
         }
      }
   } while (xml_nextNode(node));
}


/*
 * sets the enemies bit in the faction_stack
 */
static void enemies_parse( xmlNodePtr parent )
{
   xmlNodePtr node, cur;
   int** f;
   Faction* ft;
   Alliance* a;
   int i, *j, n, m, x, y, z, e;
   char* type;

   node = parent->xmlChildrenNode;

   do {
      if ((node->type==XML_NODE_START) &&
            (strcmp((char*)node->name,XML_ENEMIES_TAG)==0)) {

         i = 0;
         f = NULL;
         j = NULL;

         cur = node->xmlChildrenNode;
         do {
            if (strcmp((char*)cur->name,"enemy")==0) {

               type = (char*)xmlGetProp(cur,(xmlChar*)"type");
               
               i++;
               j = realloc(j, sizeof(int)*i);
               f = realloc(f, sizeof(int*)*i);

               /* enemy thing is an alliance */
               if (strcmp(type,"alliance")==0) {
                  a = alliance_get( (char*)cur->children->content );
                  if (a==NULL)
                     WARN("Alliance %s not found in stack", (char*)cur->children->content);
                  j[i-1] = a->nfactions;
                  f[i-1] = a->factions;
               }
               /* enemy thing is only a faction */
               else if (strcmp(type,"faction")==0) {
                  j[i-1] = 1;
                  f[i-1] = malloc(sizeof(int));
                  f[i-1][0] = faction_get( (char*)cur->children->content );
                  if (f[i-1][0] == -1)
                     WARN("Faction %s not found in stack", (char*)cur->children->content);
               }

               free(type);
            }
         } while (xml_nextNode(cur));

         /* now actually parse and load up the enemies */
         for (n=0; n<i; n++) { /* unsigned int** */
            for (m=0; m<j[n]; m++) { /* unsigned int* */

               /* add all the faction enemies to nenemies and alloc */
               for (e=0,x=0; x<i; x++)
                  if (x!=n) e += j[x]; /* stores the total enemies */
               /* now alloc the memory */
               ft = &faction_stack[ f[n][m] ];
               ft->nenemies += e;
               ft->enemies = realloc( ft->enemies, sizeof(int)*ft->nenemies );

               /* now add the actual enemies */
               for (x=0,z=0; x<i; x++)
                  if (x!=n) /* make sure isn't from the same group */
                     for (y=0; y<j[x]; y++,z++)
                        ft->enemies[ ft->nenemies-e + z ] = f[x][y];
            }
         }
         /* free all the temporary memory */
         for (x=0; x<i; x++)
            if (j[x]==1) free(f[x]); /* free the single malloced factions */
         free(f); /* free the rest */
         free(j);
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

   xmlNodePtr node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   Faction* temp = NULL;

   node = doc->xmlChildrenNode; /* Factions node */
   if (strcmp((char*)node->name,XML_FACTION_ID)) {
      ERR("Malformed "FACTION_DATA" file: missing root element '"XML_FACTION_ID"'");
      return -1;
   }

   node = node->xmlChildrenNode; /* first faction node */
   if (node == NULL) {
      ERR("Malformed "FACTION_DATA" file: does not contain elements");
      return -1;
   }

   do {
      if (node->type==XML_NODE_START) {
         if (strcmp((char*)node->name,XML_FACTION_TAG)==0) {
            temp = faction_parse(node);
            faction_stack = realloc(faction_stack, sizeof(Faction)*(++nfactions));        
            memcpy(faction_stack+nfactions-1, temp, sizeof(Faction));                  
            free(temp);
         }
         else if (strcmp((char*)node->name,XML_ALLIANCE_ID)==0)
            alliance_parse(node);
         else if (strcmp((char*)node->name,XML_ENEMIES_ID)==0)
            enemies_parse(node);
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);
   xmlCleanupParser();

   DEBUG("Loaded %d Faction%s", nfactions, (nfactions==1) ? "" : "s" );

   return 0;
}

void factions_free (void)
{
   int i;

   /* free aliances */
   for (i=0; i<nalliances; i++) {
      free(alliances[i].name);
      free(alliances[i].factions);
   }
   free(alliances);
   alliances = NULL;
   nalliances = 0;

   /* free factions */
   for (i=0; i<nfactions; i++) {
      free(faction_stack[i].name);
      if (faction_stack[i].nallies > 0) free(faction_stack[i].allies);
      if (faction_stack[i].nenemies > 0) free(faction_stack[i].enemies);
   }
   free(faction_stack);
   faction_stack = NULL;
   nfactions = 0;
}

