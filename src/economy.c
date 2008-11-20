/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file economy.c
 *
 * @brief Handles economy stuff.
 *
 * @todo Use Nodal Analysis to create a real dynamic economy.
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


#define XML_COMMODITY_ID      "Commodities" /**< XML document identifier */
#define XML_COMMODITY_TAG     "commodity" /**< XML commodity identifier. */
#define COMMODITY_DATA        "dat/commodity.xml" /**< Comodity XML file. */


/* commodity stack */
static Commodity* commodity_stack = NULL; /**< Contains all the commodities. */
static int commodity_nstack = 0; /**< Number of commodities in the stack. */


/*
 * prototypes
 */
static void commodity_freeOne( Commodity* com );
static Commodity* commodity_parse( xmlNodePtr parent );


/**
 * @fn void credits2str( char *str, unsigned int credits, int decimals )
 *
 * @brief Converts credits to a usable string for displaying.
 *
 *    @param[out] str Output is stored here, must have at least a length of 10
 *                     char.
 *    @param credits Credits to display.
 *    @param decimals Decimals to use.
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


/**
 * @fn Commodity* commodity_get( const char* name )
 *
 * @brief Gets a commoditiy by name.
 *
 *    @param name Name to match.
 *    @return Commodity matching name.
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


/**
 * @fn static void commodity_freeOne( Commodity* com )
 *
 * @brief Frees a commodity.
 *
 *    @param com Commodity to free.
 */
static void commodity_freeOne( Commodity* com )
{
   if (com->name) free(com->name);
   if (com->description) free(com->description);
}


/**
 * @Fn static Commodity* commodity_parse( xmlNodePtr parent )
 *
 * @brief Loads a commodity.
 *
 *    @param parent XML node to load from.
 *    @return Commodity loaded from parent.
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


/**
 * @brief Throws cargo out in space graphically.
 *
 *    @param pilot ID of the pilot throwing the stuff out 
 *    @param com Commodity to throw out.
 *    @param quantity Quantity thrown out.
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


/**
 * @fn int commodity_load (void)
 *
 * @brief Loads all the commodity data.
 *
 *    @return 0 on success.
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

   DEBUG("Loaded %d Commodit%s", commodity_nstack, (commodity_nstack==1) ? "y" : "ies" );

   return 0;


}


/**
 * @fn void commodity_free (void)
 *
 * @brief Frees all the loaded commodities.
 */
void commodity_free (void)
{
   int i;
   for (i=0; i<commodity_nstack; i++)
      commodity_freeOne( &commodity_stack[i] );
   free( commodity_stack );
   commodity_stack = NULL;
   commodity_nstack = 0;
}



