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
#include "space.h"
#include "physics.h"


/*
 * Prototypes.
 */
static int dpl_compPlanet( const void *planet1, const void *planet2 );
static int dpl_savePlanet( xmlTextWriterPtr writer, const Planet *p );


/**
 * @brief Compare function for planet qsort.
 */
static int dpl_compPlanet( const void *planet1, const void *planet2 )
{
   const Planet *p1, *p2;

   p1 = * (const Planet**) planet1;
   p2 = * (const Planet**) planet2;

   return strcmp( p1->name, p2->name );
}


/**
 * @brief Saves a planet.
 *
 *    @param write Write to use for saving the star planet.
 *    @param p Planet to save.
 *    @return 0 on success.
 */
static int dpl_savePlanet( xmlTextWriterPtr writer, const Planet *p )
{
   int i;

   xmlw_startElem( writer, "asset" );

   /* Attributes. */
   xmlw_attr( writer, "name", "%s", p->name );

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
   xmlw_startElem( writer, "presence" );
   if (p->faction >= 0)
      xmlw_elem( writer, "faction", "%s", faction_name( p->faction ) );
   xmlw_elem( writer, "value", "%f", p->presenceAmount );
   xmlw_elem( writer, "range", "%d", p->presenceRange );
   xmlw_endElem( writer );

   /* General. */
   if (p->real == ASSET_REAL) {
      xmlw_startElem( writer, "general" );
      xmlw_elem( writer, "class", "%c", planet_getClass( p ) );
      xmlw_elem( writer, "population", "%"PRIu64, p->population );
      xmlw_startElem( writer, "services" );
      if (planet_hasService( p, PLANET_SERVICE_LAND ))
         xmlw_elemEmpty( writer, "land" );
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
      xmlw_startElem( writer, "commodities" );
      for (i=0; i<p->ncommodities; i++)
         xmlw_elem( writer, "commodity", "%s", p->commodities[i]->name );
      xmlw_endElem( writer ); /* "commodities" */
      xmlw_elem( writer, "description", "%s", p->description );
      xmlw_elem( writer, "bar", "%s", p->bar_description );
      xmlw_endElem( writer ); /* "general" */
   }

   /* Tech. */
   tech_groupWrite( writer, p->tech );

   xmlw_endElem( writer ); /** "planet" */

   return 0;
}


/**
 * @saves All the star planets.
 *
 *    @return 0 on success.
 */
int dpl_saveAll (void)
{
   int i;
   /*char file[PATH_MAX];*/
   xmlDocPtr doc;
   xmlTextWriterPtr writer;
   int np;
   const Planet *p;
   const Planet **sorted_p;

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
   xmlw_startElem( writer, "Assets" );

   /* Sort planets. */
   p        = planet_getAll( &np );
   sorted_p = malloc( sizeof(Planet*) * np );
   for (i=0; i<np; i++)
      sorted_p[i]  = &p[i];
   qsort( sorted_p, np, sizeof(Planet*), dpl_compPlanet );

   /* Write planets. */
   for (i=0; i<np; i++)
      dpl_savePlanet( writer, sorted_p[i] );

   /* Clean up sorted planet.s */
   free(sorted_p);

   /* End writer. */
   xmlw_endElem( writer ); /* "Assets" */
   xmlw_done( writer );

   /* No need for writer anymore. */
   xmlFreeTextWriter( writer );

   /* Write data. */
   xmlSaveFileEnc( "asset.xml", doc, "UTF-8" );

   /* Clean up. */
   xmlFreeDoc(doc);

   return 0;
}


