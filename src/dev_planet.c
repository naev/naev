/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev_planet.c
 *
 * @brief Handles the planet stuff.
 */

#include "dev_planet.h"

#include "naev.h"

#include <stdlib.h> /* qsort */

#include "nxml.h"
#include "physics.h"
#include "nfile.h"
#include "nstring.h"


/**
 * @brief Saves a planet.
 *
 *    @param writer Write to use for saving the star planet.
 *    @param p Planet to save.
 *    @return 0 on success.
 */
int dpl_savePlanet( const Planet *p )
{
   xmlDocPtr doc;
   xmlTextWriterPtr writer;
   char *file, *cleanName;
   int i, pos;

   /* Create the writer. */
   writer = xmlNewTextWriterDoc(&doc, 0);
   if (writer == NULL) {
      WARN("testXmlwriterDoc: Error creating the xml writer");
      return -1;
   }

   /* Set the writer parameters. */
   xmlw_setParams( writer );

   /* Start writer. */
   xmlw_start(writer);
   xmlw_startElem( writer, "asset" );

   /* Attributes. */
   xmlw_attr( writer, "name", "%s", p->name );

   /* Explicit virtualness. */
   if (p->real == ASSET_VIRTUAL)
      xmlw_elemEmpty( writer, "virtual" );

   /* Position. */
   if (p->real == ASSET_REAL) {
      xmlw_startElem( writer, "pos" );
      xmlw_elem( writer, "x", "%f", p->pos.x );
      xmlw_elem( writer, "y", "%f", p->pos.y );
      xmlw_endElem( writer ); /* "pos" */
   }

   /* GFX. */
   if (p->real == ASSET_REAL) {
      xmlw_startElem( writer, "GFX" );
      xmlw_elem( writer, "space", "%s", p->gfx_spacePath );
      xmlw_elem( writer, "exterior", "%s", p->gfx_exteriorPath );
      xmlw_endElem( writer ); /* "GFX" */
   }

   /* Presence. */
   if (p->faction >= 0) {
      xmlw_startElem( writer, "presence" );
      xmlw_elem( writer, "faction", "%s", faction_name( p->faction ) );
      xmlw_elem( writer, "value", "%f", p->presenceAmount );
      xmlw_elem( writer, "range", "%d", p->presenceRange );
      xmlw_endElem( writer );
   }

   /* General. */
   if (p->real == ASSET_REAL) {
      xmlw_startElem( writer, "general" );
      xmlw_elem( writer, "class", "%c", planet_getClass( p ) );
      xmlw_elem( writer, "population", "%"PRIu64, p->population );
      xmlw_elem( writer, "hide", "%f", sqrt(p->hide) );
      xmlw_startElem( writer, "services" );
      if (planet_hasService( p, PLANET_SERVICE_LAND )) {
         if (p->land_func == NULL)
            xmlw_elemEmpty( writer, "land" );
         else
            xmlw_elem( writer, "land", "%s", p->land_func );
      }
      if (planet_hasService( p, PLANET_SERVICE_REFUEL ))
         xmlw_elemEmpty( writer, "refuel" );
      if (planet_hasService( p, PLANET_SERVICE_BAR ))
         xmlw_elemEmpty( writer, "bar" );
      if (planet_hasService( p, PLANET_SERVICE_MISSIONS ))
         xmlw_elemEmpty( writer, "missions" );
      if (planet_hasService( p, PLANET_SERVICE_COMMODITY ))
         xmlw_elemEmpty( writer, "commodity" );
      if (planet_hasService( p, PLANET_SERVICE_OUTFITS ))
         xmlw_elemEmpty( writer, "outfits" );
      if (planet_hasService( p, PLANET_SERVICE_SHIPYARD ))
         xmlw_elemEmpty( writer, "shipyard" );
      xmlw_endElem( writer ); /* "services" */
      if (planet_hasService( p, PLANET_SERVICE_LAND )) {
         xmlw_startElem( writer, "commodities" );
         for (i=0; i<p->ncommodities; i++)
            xmlw_elem( writer, "commodity", "%s", p->commodities[i]->name );
         xmlw_endElem( writer ); /* "commodities" */
         xmlw_elem( writer, "description", "%s", p->description );
         if (planet_hasService( p, PLANET_SERVICE_BAR ))
            xmlw_elem( writer, "bar", "%s", p->bar_description );
      }
      xmlw_endElem( writer ); /* "general" */
   }

   /* Tech. */
   if (planet_hasService( p, PLANET_SERVICE_LAND ))
      tech_groupWrite( writer, p->tech );

   xmlw_endElem( writer ); /** "planet" */
   xmlw_done( writer );

   /* No need for writer anymore. */
   xmlFreeTextWriter( writer );

   /* Write data. */
   cleanName = calloc( 1, (strlen(p->name)+1) * sizeof(char) );
   pos = 0;
   for (i=0; i<(int)strlen(p->name); i++) {
      if (!ispunct(p->name[i])) {
         if (p->name[i] == ' ')
            cleanName[pos] = '_';
         else
            cleanName[pos] = tolower(p->name[i]);
         pos++;
      }
   }
   file = malloc((strlen(cleanName)+20)*sizeof(char));
   nsnprintf(file,(strlen(cleanName)+20)*sizeof(char),"dat/assets/%s.xml",cleanName);
   xmlSaveFileEnc( file, doc, "UTF-8" );

   /* Clean up. */
   xmlFreeDoc(doc);
   free(cleanName);

   return 0;
}


/**
 * @brief Saves all the star planets.
 *
 *    @return 0 on success.
 */
int dpl_saveAll (void)
{
   int i;
   int np;
   const Planet *p;

   p = planet_getAll( &np );

   /* Write planets. */
   for (i=0; i<np; i++)
      dpl_savePlanet( &p[i] );

   return 0;
}


