/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file fleet.c
 *
 * @brief Handles the fleet stuff.
 */


#include "fleet.h"

#include "naev.h"

#include "nstring.h"
#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "nxml.h"

#include "log.h"
#include "pilot.h"
#include "ndata.h"
#include "rng.h"



#define CHUNK_SIZE      32 /**< Size to allocate memory by. */


/* stack of fleets */
static Fleet* fleet_stack = NULL; /**< Fleet stack. */
static int nfleets = 0; /**< Number of fleets. */


/*
 * Prototypes.
 */
static int fleet_parse( Fleet *temp, const xmlNodePtr parent );


/**
 * @brief Grabs a fleet out of the stack.
 *
 *    @param name Name of the fleet to match.
 *    @return The fleet matching name or NULL if not found.
 */
Fleet* fleet_get( const char* name )
{
   int i;

   for (i=0; i<nfleets; i++)
      if (strcmp(fleet_stack[i].name, name)==0)
         return &fleet_stack[i];

   return NULL;
}


/**
 * @brief Creates a pilot belonging to a fleet.
 *
 *    @param flt Fleet to which pilot belongs to.
 *    @param plt Pilot to create.
 *    @param dir Direction to face.
 *    @param pos Position to create at.
 *    @param vel Initial velocity.
 *    @param ai AI to use (NULL is default).
 *    @param flags Flags to create with.
 *    @return The ID of the pilot created.
 *
 * @sa pilot_create
 */
unsigned int fleet_createPilot( Fleet *flt, FleetPilot *plt, double dir,
      Vector2d *pos, Vector2d *vel, const char* ai, PilotFlags flags )
{
   unsigned int p;
   p = pilot_create( plt->ship,
         plt->name,
         flt->faction,
         (ai != NULL) ? ai :
               (plt->ai != NULL) ? plt->ai :
                     flt->ai,
         dir,
         pos,
         vel,
         flags );
   return p;
}



/**
 * @brief Parses the fleet node.
 *
 *    @param temp Fleet to load.
 *    @param parent Parent xml node of the fleet in question.
 *    @return A newly allocated fleet loaded with data in parent node.
 */
static int fleet_parse( Fleet *temp, const xmlNodePtr parent )
{
   xmlNodePtr cur, node;
   FleetPilot* pilot;
   char* c;
   int mem;
   node = parent->xmlChildrenNode;

   /* Sane defaults and clean up. */
   memset( temp, 0, sizeof(Fleet) );
   temp->faction = -1;

   temp->name = (char*)xmlGetProp(parent,(xmlChar*)"name"); /* already mallocs */
   if (temp->name == NULL)
      WARN( _("Fleet in %s has invalid or no name"), FLEET_DATA_PATH );

   do { /* load all the data */
      xml_onlyNodes(node);

      /* Set faction. */
      if (xml_isNode(node,"faction")) {
         temp->faction = faction_get(xml_get(node));
         continue;
      }

      /* Set AI. */
      xmlr_strd(node,"ai",temp->ai);
      if (ai_getProfile ( temp->ai ) == NULL)
         WARN(_("Fleet '%s' has invalid AI '%s'."), temp->name, temp->ai );

      /* Set flags. */
      if (xml_isNode(node,"flags")) {
         cur = node->children;
         do {
            xml_onlyNodes(cur);
            WARN(_("Fleet '%s' has unknown flag node '%s'."), temp->name, cur->name);
         } while (xml_nextNode(cur));
         continue;
      }

      /* Load pilots. */
      else if (xml_isNode(node,"pilots")) {
         cur = node->children;
         mem = 0;
         do {
            xml_onlyNodes(cur);

            if (xml_isNode(cur,"pilot")) {

               /* See if must grow. */
               temp->npilots++;
               if (temp->npilots > mem) {
                  mem += CHUNK_SIZE;
                  temp->pilots = realloc(temp->pilots, mem * sizeof(FleetPilot));
               }
               pilot = &temp->pilots[temp->npilots-1];

               /* Clear memory. */
               memset( pilot, 0, sizeof(FleetPilot) );

               /* Check for name override */
               xmlr_attr(cur,"name",c);
               pilot->name = c; /* No need to free since it will have to later */

               /* Check for AI override */
               xmlr_attr(cur,"ai",pilot->ai);

               /* Load pilot's ship */
               xmlr_attr(cur,"ship",c);
               if (c==NULL)
                  WARN(_("Pilot %s in Fleet %s has null ship"), pilot->name, temp->name);
               pilot->ship = ship_get(c);
               if (pilot->ship == NULL)
                  WARN(_("Pilot %s in Fleet %s has invalid ship"), pilot->name, temp->name);
               if (c!=NULL)
                  free(c);
               continue;
            }

            WARN(_("Fleet '%s' has unknown pilot node '%s'."), temp->name, cur->name);
         } while (xml_nextNode(cur));

         /* Resize to minimum. */
         temp->pilots = realloc(temp->pilots, sizeof(FleetPilot)*temp->npilots);
         continue;
      }

      DEBUG(_("Unknown node '%s' in fleet '%s'"),node->name,temp->name);
   } while (xml_nextNode(node));

#define MELEMENT(o,s) \
if (o) WARN( _("Fleet '%s' missing '%s' element"), s,temp->name)
/**< Hack to check for missing fields. */
   MELEMENT(temp->ai==NULL,"ai");
   MELEMENT(temp->faction==-1,"faction");
   MELEMENT(temp->pilots==NULL,"pilots");
#undef MELEMENT

   return 0;
}


/**
 * @brief Loads all the fleets.
 *
 *    @return 0 on success.
 */
static int fleet_loadFleets (void)
{
   int mem;
   size_t bufsize;
   char *buf;
   xmlNodePtr node;
   xmlDocPtr doc;

   /* Load the data. */
   buf = ndata_read( FLEET_DATA_PATH, &bufsize);
   doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode; /* fleets node */
   if (strcmp((char*)node->name,"Fleets")) {
      ERR( _("Malformed %s file: missing root element 'Fleets'."), FLEET_DATA_PATH);
      return -1;
   }

   node = node->xmlChildrenNode; /* first fleet node */
   if (node == NULL) {
      ERR( _("Malformed %s file: does not contain elements."), FLEET_DATA_PATH );
      return -1;
   }

   mem = 0;
   do {
      if (xml_isNode(node,"fleet")) {
         /* See if memory must grow. */
         nfleets++;
         if (nfleets > mem) {
            mem += CHUNK_SIZE;
            fleet_stack = realloc(fleet_stack, sizeof(Fleet) * mem);
         }

         /* Load the fleet. */
         fleet_parse( &fleet_stack[nfleets-1], node );
      }
   } while (xml_nextNode(node));
   /* Shrink to minimum. */
   fleet_stack = realloc(fleet_stack, sizeof(Fleet) * nfleets);

   xmlFreeDoc(doc);
   free(buf);

   return 0;
}


/**
 * @brief Loads all the fleets
 *
 *    @return 0 on success.
 */
int fleet_load (void)
{
   if (fleet_loadFleets())
      return -1;

   DEBUG( ngettext( "Loaded %d Fleet", "Loaded %d Fleets", nfleets ), nfleets );

   return 0;
}


/**
 * @brief Cleans up by freeing all the fleet data.
 */
void fleet_free (void)
{
   int i,j;

   /* Free the fleet stack. */
   if (fleet_stack != NULL) {
      for (i=0; i<nfleets; i++) {
         for (j=0; j<fleet_stack[i].npilots; j++) {
            if (fleet_stack[i].pilots[j].name)
               free(fleet_stack[i].pilots[j].name);
            if (fleet_stack[i].pilots[j].ai)
               free(fleet_stack[i].pilots[j].ai);
         }
         free(fleet_stack[i].name);
         free(fleet_stack[i].pilots);
         free(fleet_stack[i].ai);
      }
      free(fleet_stack);
   }
   fleet_stack = NULL;
   nfleets = 0;
}

