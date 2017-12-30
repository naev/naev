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
#include "nxml.h"
#include "ndata.h"


#define XML_START_ID    "Start"  /**< XML document tag of module start file. */


/**
 * @brief The start data structure.
 */
typedef struct ndata_start_s {
   char *name; /**< Name of ndata. */
   char *ship; /**< Default starting ship model. */
   char *shipname; /**< Default starting ship name. */
   unsigned int credits; /**< Starting credits. */
   ntime_t date; /**< Starting date. */
   char *system; /**< Starting system. */
   double x; /**< Starting X position. */
   double y; /**< Starting Y position. */
   char *mission; /**< Starting mission. */
   char *event; /**< Starting event. */

   /* Tutorial stuff. */
   char *tutmisn; /**< Tutorial mission. */
   char *tutevt; /**< Tutorial event. */
   char *tutsys; /**< Tutorial system. */
   double tutx; /**< Tutorial x position. */
   double tuty; /**< Tutorial y position. */
} ndata_start_t;
static ndata_start_t start_data; /**< The actual starting data. */


/**
 * @brief Loads the module start data.
 *
 *    @return 0 on success.
 */
int start_load (void)
{
   size_t bufsize;
   char *buf;
   xmlNodePtr node, cur, tmp;
   xmlDocPtr doc;
   int scu, stp, stu;

   /* Defaults. */
   scu = -1;
   stp = -1;
   stu = -1;

   /* Try to read the file. */
   buf = ndata_read( START_DATA_PATH, &bufsize );
   if (buf == NULL)
      return -1;

   /* Load the XML file. */
   doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_START_ID)) {
      ERR( _("Malformed '%s' file: missing root element '%s'"), START_DATA_PATH, XML_START_ID );
      return -1;
   }

   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR( _("Malformed '%s' file: does not contain elements"), START_DATA_PATH );
      return -1;
   }
   do {
      xml_onlyNodes(node);

      xmlr_strd( node, "name", start_data.name );

      if (xml_isNode(node, "player")) { /* we are interested in the player */
         cur = node->children;
         do {
            xml_onlyNodes(cur);

            xmlr_uint( cur, "credits", start_data.credits );
            xmlr_strd( cur, "mission", start_data.mission );
            xmlr_strd( cur, "event",   start_data.event );

            if (xml_isNode(cur,"ship")) {
               xmlr_attr( cur, "name",    start_data.shipname);
               xmlr_strd( cur, "ship",    start_data.ship );
            }
            else if (xml_isNode(cur,"system")) {
               tmp = cur->children;
               do {
                  xml_onlyNodes(tmp);
                  /** system name, @todo percent chance */
                  xmlr_strd( tmp, "name", start_data.system );
                  /* position */
                  xmlr_float( tmp, "x", start_data.x );
                  xmlr_float( tmp, "y", start_data.y );
                  WARN(_("'%s' has unknown system node '%s'."), START_DATA_PATH, tmp->name);
               } while (xml_nextNode(tmp));
               continue;
            }
            WARN(_("'%s' has unknown player node '%s'."), START_DATA_PATH, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }

      if (xml_isNode(node,"date")) {
         cur = node->children;
         do {
            xml_onlyNodes(cur);

            xmlr_int( cur, "scu", scu );
            xmlr_int( cur, "stp", stp );
            xmlr_int( cur, "stu", stu );
            WARN(_("'%s' has unknown date node '%s'."), START_DATA_PATH, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }

      if (xml_isNode(node,"tutorial")) {
         cur = node->children;
         do {
            xml_onlyNodes(cur);

            xmlr_strd( cur, "mission", start_data.tutmisn );
            xmlr_strd( cur, "event", start_data.tutevt );

            if (xml_isNode(cur,"system")) {
               tmp = cur->children;
               do {
                  xml_onlyNodes(tmp);
                  /** system name, @todo percent chance */
                  xmlr_strd( tmp, "name", start_data.tutsys );
                  /* position */
                  xmlr_float( tmp, "x", start_data.tutx );
                  xmlr_float( tmp, "y", start_data.tuty );
                  WARN(_("'%s' has unknown system node '%s'."), START_DATA_PATH, tmp->name);
               } while (xml_nextNode(tmp));
               continue;
            }

            WARN(_("'%s' has unknown tutorial node '%s'."), START_DATA_PATH, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }

      WARN(_("'%s' has unknown node '%s'."), START_DATA_PATH, node->name);
   } while (xml_nextNode(node));

   /* Clean up. */
   xmlFreeDoc(doc);
   free(buf);

   /* Sanity checking. */
#define MELEMENT(o,s) \
   if (o) WARN(_("Module start data missing/invalid '%s' element"), s) /**< Define to help check for data errors. */
   MELEMENT( start_data.name==NULL, "name" );
   MELEMENT( start_data.credits==0, "credits" );
   MELEMENT( start_data.ship==NULL, "ship" );
   MELEMENT( start_data.system==NULL, "player system" );
   MELEMENT( start_data.tutsys==NULL, "tutorial system" );
   MELEMENT( scu<0, "scu" );
   MELEMENT( stp<0, "stp" );
   MELEMENT( stu<0, "stu" );
#undef MELEMENT

   /* Post process. */
   start_data.date = ntime_create( scu, stp, stu );

   return 0;
}


/**
 * @brief Cleans up after the module start data.
 */
void start_cleanup (void)
{
   free( start_data.name );
   free( start_data.shipname );
   free( start_data.ship );
   free( start_data.system );
   free( start_data.mission );
   free( start_data.event );
   free( start_data.tutsys );
   free( start_data.tutmisn );
   free( start_data.tutevt );
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
 * @brief Gets the module's starting ship's name.
 *    @return The default name of the starting ship.
 */
const char* start_shipname (void)
{
   return start_data.shipname;
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
ntime_t start_date (void)
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


/**
 * @brief Gets the starting event of the player.
 *    @return The starting event of the player (or NULL if inapplicable).
 */
const char* start_event (void)
{
   return start_data.event;
}


/**
 * @brief Gets the starting tutorial mission of the player.
 *    @return The starting tutorial mission of the player (or NULL if inapplicable).
 */
const char* start_tutMission (void)
{
   return start_data.tutmisn;
}


/**
 * @brief Gets the starting tutorial event of the player.
 *    @return The starting tutorial event of the player (or NULL if inapplicable).
 */
const char* start_tutEvent (void)
{
   return start_data.tutevt;
}


/**
 * @brief Gets the tutorial system.
 */
const char* start_tutSystem (void)
{
   return start_data.tutsys;
}


/**
 * @brief Gets the starting position of the tutorial.
 */
void start_tutPosition( double *x, double *y )
{
   *x = start_data.tutx;
   *y = start_data.tuty;
}

