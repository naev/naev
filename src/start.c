/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file start.c
 *
 * @brief Contains information about the module scenario start.
 *
 * This information is important when creating a new game.
 */

#include "start.h"

#include "naev.h"

#include "log.h"
#include "md5.h"
#include "nxml.h"
#include "pack.h"
#include "ndata.h"


#define XML_START_ID    "Start"  /**< XML document tag of module start file. */
#define START_DATA      "dat/start.xml" /**< Path to module start file. */


/**
 * @brief The start data structure.
 */
typedef struct ndata_start_s {
   char *name; /**< Name of ndata. */
   char *ship; /**< Default starting ship name. */
   unsigned int credits; /**< Starting credits. */
   int date; /**< Starting date. */
   char *system; /**< Starting system. */
   double x; /**< Starting X position. */
   double y; /**< Starting Y position. */
   char *mission; /**< Starting mission. */
} ndata_start_t;
static ndata_start_t start_data; /**< The actual starting data. */


/**
 * @brief Loads the module start data.
 *
 *    @return 0 on success.
 */
int start_load (void)
{
   uint32_t bufsize;
   char *buf;
   xmlNodePtr node, cur, tmp;
   xmlDocPtr doc;

   /* Try to read teh file. */
   buf = ndata_read( START_DATA, &bufsize );
   if (buf == NULL)
      return -1;

   /* Load the XML file. */
   doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_START_ID)) {
      ERR("Malformed '"START_DATA"' file: missing root element '"XML_START_ID"'");
      return -1;
   }

   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed '"START_DATA"' file: does not contain elements");
      return -1;
   }
   do {
      xml_onlyNodes(node);

      xmlr_strd( node, "name", start_data.name );

      if (xml_isNode(node, "player")) { /* we are interested in the player */
         cur = node->children;
         do {
            xml_onlyNodes(cur);

            xmlr_strd( cur, "ship", start_data.ship );
            xmlr_uint( cur, "credits", start_data.credits );
            xmlr_int( cur, "date", start_data.date );
            xmlr_strd( cur, "mission", start_data.mission );
            
            if (xml_isNode(cur,"system")) {
               tmp = cur->children;
               do {
                  xml_onlyNodes(tmp);
                  /** system name, @todo percent chance */
                  xmlr_strd( tmp, "name", start_data.system );
                  /* position */
                  xmlr_float( tmp, "x", start_data.x );
                  xmlr_float( tmp, "y", start_data.y );
                  WARN("'"START_DATA"' has unknown system node '%s'.", tmp->name);
               } while (xml_nextNode(tmp));
               continue;
            }
            WARN("'"START_DATA"' has unknown player node '%s'.", cur->name);
         } while (xml_nextNode(cur));
         continue;
      }
      WARN("'"START_DATA"' has unknown node '%s'.", node->name);
   } while (xml_nextNode(node));

   /* Clean up. */
   xmlFreeDoc(doc);
   free(buf);
   xmlCleanupParser();

   /* Sanity checking. */
#define MELEMENT(o,s) \
   if (o) WARN("Module start data missing/invalid '"s"' element") /**< Define to help check for data errors. */
   MELEMENT( start_data.name==NULL, "name" );
   MELEMENT( start_data.credits==0, "credits" );
   MELEMENT( start_data.date==0, "date" );
   MELEMENT( start_data.ship==NULL, "ship" );
   MELEMENT( start_data.mission==NULL, "mission" );
   MELEMENT( start_data.system==NULL, "system" );
#undef MELEMENT


   return 0;
}


/**
 * @brief Cleans up after the module start data.
 */
void start_cleanup (void)
{
   if (start_data.name != NULL)
      free(start_data.name);
   if (start_data.ship != NULL)
      free(start_data.ship);
   if (start_data.system != NULL)
      free(start_data.system);
   if (start_data.mission != NULL)
      free(start_data.mission);
   memset( &start_data, 0, sizeof(start_data) );
}


/**
 * @brief Gets the module name.
 *    @return Name of the module.
 */
const char* start_name (void)
{
   return start_data.name;
}


/**
 * @brief Gets the module player starting ship.
 *    @return The starting ship of the player.
 */
const char* start_ship (void)
{
   return start_data.ship;
}


/**
 * @brief Gets the player's starting credits.
 *    @return The starting credits of the player.
 */
unsigned int start_credits (void)
{
   return start_data.credits;
}


/**
 * @brief Gets the starting date.
 *    @return The starting date of the player.
 */
int start_date (void)
{
   return start_data.date;
}


/**
 * @brief Gets the starting system name.
 *    @return The name of the starting system.
 */
const char* start_system (void)
{
   return start_data.system;
}


/**
 * @brief Gets the starting position of the player.
 *    @param[out] Starting X position.
 *    @param[out] Starting Y position.
 */
void start_position( double *x, double *y )
{
   *x = start_data.x;
   *y = start_data.y;
}


/**
 * @brief Gets the starting mission of the player.
 *    @return The starting mission of the player (or NULL if inapplicable).
 */
const char* start_mission (void)
{
   return start_data.mission;
}


