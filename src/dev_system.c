/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev_system.c
 *
 * @brief Handles developement of star system stuff.
 */

#include "dev_system.h"

#include "naev.h"

#include "nxml.h"
#include "space.h"


/*
 * Prototypes.
 */
static int dsys_saveSystem( xmlTextWriterPtr writer, const StarSystem *sys );


/**
 * @brief Saves a star system.
 *
 *    @param write Write to use for saving the star system.
 *    @param sys Star system to save.
 *    @return 0 on success.
 */
static int dsys_saveSystem( xmlTextWriterPtr writer, const StarSystem *sys )
{
   int i;

   xmlw_startElem( writer, "ssys" );

   /* Attributes. */
   xmlw_attr( writer, "name", "%s", sys->name );

   /* General. */
   xmlw_startElem( writer, "general" );
   xmlw_elem( writer, "stars", "%d", sys->stars );
   xmlw_elem( writer, "asteroids", "%d", sys->asteroids );
   xmlw_elem( writer, "interference", "%f", sys->interference );
   xmlw_startElem( writer, "nebula" );
   xmlw_attr( writer, "volatility", "%f", sys->nebu_volatility );
   xmlw_str( writer, "%f", sys->nebu_density );
   xmlw_endElem( writer ); /* "nebula" */
   xmlw_endElem( writer ); /* "general" */

   /* Position. */
   xmlw_startElem( writer, "pos" );
   xmlw_elem( writer, "x", "%f", sys->pos.x );
   xmlw_elem( writer, "y", "%f", sys->pos.y );
   xmlw_endElem( writer ); /* "pos" */

   /* Planets. */
   xmlw_startElem( writer, "planets" );
   for (i=0; i<sys->nplanets; i++)
      xmlw_elem( writer, "planet", "%s", sys->planets[i]->name );
   xmlw_endElem( writer ); /* "planets" */

   /* Fleets. */
   xmlw_startElem( writer, "fleets" );
   for (i=0; i<sys->nfltdat; i++)
      xmlw_elem( writer, "fleet", "%s", sys->fltdat[i] );
   xmlw_endElem( writer );

   /* Jumps. */
   xmlw_startElem( writer, "jumps" );
   for (i=0; i<sys->njumps; i++)
      xmlw_elem( writer, "jump", "%s", system_getIndex( sys->jumps[i] )->name );
   xmlw_endElem( writer ); /* "jumps" */

   xmlw_endElem( writer ); /** "ssys" */

   return 0;
}


/**
 * @brief Function for qsorting sysetms.
 */
static int dsys_compSys( const void *sys1, const void *sys2 )
{
   const StarSystem *s1, *s2;

   s1 = * (const StarSystem**) sys1;
   s2 = * (const StarSystem**) sys2;

   return strcmp( s1->name, s2->name );
}


/**
 * @saves All the star systems.
 *
 *    @return 0 on success.
 */
int dsys_saveAll (void)
{
   int i;
   /*char file[PATH_MAX];*/
   xmlDocPtr doc;
   xmlTextWriterPtr writer;
   int nsys;
   const StarSystem *sys;
   const StarSystem **sorted_sys;

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
   xmlw_startElem( writer, "Systems" );

   /* Sort systems. */
   sys = system_getAll( &nsys );
   sorted_sys = malloc( sizeof(StarSystem*) * nsys );
   for (i=0; i<nsys; i++)
      sorted_sys[i] = &sys[i];
   qsort( sorted_sys, nsys, sizeof(StarSystem*), dsys_compSys );

   /* Write systems. */
   for (i=0; i<nsys; i++)
      dsys_saveSystem( writer, sorted_sys[i] );

   /* Clean up sorted system.s */
   free(sorted_sys);

   /* End writer. */
   xmlw_endElem( writer ); /* "Systems" */
   xmlw_done(writer);

   /* No need for writer anymore. */
   xmlFreeTextWriter(writer);

   /* Write data. */
   xmlSaveFileEnc( "ssys.xml", doc, "UTF-8" );

   /* Clean up. */
   xmlFreeDoc(doc);

   return 0;
}


