/*
 * See Licensing and Copyright notice in naev.h
 */



#include "economy.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "naev.h"
#include "xml.h"
#include "pack.h"
#include "log.h"
#include "spfx.h"
#include "pilot.h"
#include "rng.h"


#define XML_COMMODITY_ID      "Commodities"   /* XML section identifier */
#define XML_COMMODITY_TAG     "commodity"
#define COMMODITY_DATA        "dat/commodity.xml"


/* commodity stack */
static Commodity* commodity_stack = NULL;
static int commodity_nstack = 0;


/*
 * prototypes
 */
static void commodity_freeOne( Commodity* com );
static Commodity* commodity_parse( xmlNodePtr parent );


/*
 * converts credits to a usable string for displaying
 * str must have 10 characters alloced
 */
void credits2str( char *str, unsigned int credits, int decimals )
{
   if (decimals < 0)
      snprintf( str, 32, "%d", credits );
   else if (credits >= 1000000000)
      snprintf( str, 16, "%.*fB", decimals, (double)credits / 1000000000. );
   else if (credits >= 1000000)                
      snprintf( str, 16, "%.*fM", decimals, (double)credits / 1000000. );
   else if (credits >= 1000)              
      snprintf( str, 16, "%.*fK", decimals, (double)credits / 1000. );
   else snprintf (str, 16, "%d", credits );
}


/*
 * gets a commoditiy
 */
Commodity* commodity_get( const char* name )
{
   int i;
   for (i=0; i<commodity_nstack; i++)
      if (strcmp(commodity_stack[i].name,name)==0)
         return &commodity_stack[i];
   
   WARN("Commodity '%s' not found in stack", name);
   return NULL;
}


/*
 * frees a commodity
 */
static void commodity_freeOne( Commodity* com )
{
   if (com->name) free(com->name);
   if (com->description) free(com->description);
}


/*
 * loads a commodity
 */
static Commodity* commodity_parse( xmlNodePtr parent )
{
   xmlNodePtr node;
   Commodity* temp = CALLOC_ONE(Commodity);

   temp->name = (char*)xmlGetProp(parent,(xmlChar*)"name");
   if (temp->name == NULL) WARN("Commodity from "COMMODITY_DATA" has invalid or no name");

   node = parent->xmlChildrenNode;

   do {
      if (xml_isNode(node,"description"))
         temp->description = strdup( xml_get(node) );
      else if (xml_isNode(node,"high"))
         temp->high = xml_getInt(node);
      else if (xml_isNode(node,"medium"))
         temp->medium = xml_getInt(node);
      else if (xml_isNode(node,"low"))
         temp->low = xml_getInt(node);
   } while (xml_nextNode(node));

#if 0 /* shouldn't be needed atm */
#define MELEMENT(o,s)   if (o) WARN("Commodity '%s' missing '"s"' element", temp->name)
   MELEMENT(temp->description==NULL,"description");
   MELEMENT(temp->high==0,"high");
   MELEMENT(temp->medium==0,"medium");
   MELEMENT(temp->low==0,"low");
#undef MELEMENT
#endif

   return temp;
}


/*
 * Throws cargo out in space graphically.
 */
void commodity_Jettison( int pilot, Commodity* com, int quantity )
{
   (void)com;
   int i;
   Pilot* p;
   int n, effect;
   double px,py, bvx, bvy, r,a, vx,vy;

   p = pilot_get( pilot );

   n = MAX( 1, RNG(quantity/10, quantity/5) );
   px = p->solid->pos.x;
   py = p->solid->pos.y;
   bvx = p->solid->vel.x;
   bvy = p->solid->vel.y;
   for (i=0; i<n; i++) {
      effect = spfx_get("cargo");

      /* Radial distribution gives much nicer results */
      r = RNGF()*25 - 12.5;
      a = (double)RNG(0,359);
      vx = bvx + r*cos(a);
      vy = bvy + r*sin(a);
      
      /* Add the cargo effect */
      spfx_add( effect, px, py, vx, vy, SPFX_LAYER_BACK );
   }
}


/*
 * init/exit
 */
int commodity_load (void)
{
   uint32_t bufsize;
   char *buf = pack_readfile(DATA, COMMODITY_DATA, &bufsize);

   xmlNodePtr node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   Commodity* temp = NULL;

   node = doc->xmlChildrenNode; /* Commoditys node */
   if (strcmp((char*)node->name,XML_COMMODITY_ID)) {
      ERR("Malformed "COMMODITY_DATA" file: missing root element '"XML_COMMODITY_ID"'");
      return -1;
   }

   node = node->xmlChildrenNode; /* first faction node */
   if (node == NULL) {
      ERR("Malformed "COMMODITY_DATA" file: does not contain elements");
      return -1;
   }

   do {
      if (node->type==XML_NODE_START) {
         if (strcmp((char*)node->name,XML_COMMODITY_TAG)==0) {
            temp = commodity_parse(node);
            commodity_stack = realloc(commodity_stack,
                  sizeof(Commodity)*(++commodity_nstack));
            memcpy(commodity_stack+commodity_nstack-1, temp, sizeof(Commodity));
            free(temp);
         }
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);
   xmlCleanupParser();

   DEBUG("Loaded %d Commodit%s", commodity_nstack, (commodity_nstack==1) ? "y" : "ies" );

   return 0;


}
void commodity_free (void)
{
   int i;
   for (i=0; i<commodity_nstack; i++)
      commodity_freeOne( &commodity_stack[i] );
   free( commodity_stack );
   commodity_stack = NULL;
   commodity_nstack = 0;
}



