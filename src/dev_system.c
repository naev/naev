/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev_system.c
 *
 * @brief Handles development of star system stuff.
 */

#include "dev_system.h"

#include "naev.h"

#include <stdlib.h> /* qsort */

#include "nxml.h"
#include "space.h"
#include "physics.h"
#include "nstring.h"


/*
 * Prototypes.
 */
static int dsys_compPlanet( const void *planet1, const void *planet2 );
static int dsys_compJump( const void *jmp1, const void *jmp2 );


/**
 * @brief Compare function for planet qsort.
 *
 *    @param planet1 Planet 1 to sort.
 *    @param planet2 Planet 2 to sort.
 *    @return Order to sort.
 */
static int dsys_compPlanet( const void *planet1, const void *planet2 )
{
   const Planet *p1, *p2;

   p1 = * (const Planet**) planet1;
   p2 = * (const Planet**) planet2;

   return strcmp( p1->name, p2->name );
}


/**
 * @brief Function for qsorting jumppoints.
 *
 *    @param jmp1 Jump Point 1 to sort.
 *    @param jmp2 Jump Point 2 to sort.
 *    @return Order to sort.
 */
static int dsys_compJump( const void *jmp1, const void *jmp2 )
{
   const JumpPoint *jp1, *jp2;

   jp1 = * (const JumpPoint**) jmp1;
   jp2 = * (const JumpPoint**) jmp2;

   return strcmp( jp1->target->name, jp2->target->name );
}



/**
 * @brief Saves a star system.
 *
 *    @param writer Write to use for saving the star system.
 *    @param sys Star system to save.
 *    @return 0 on success.
 */
int dsys_saveSystem( StarSystem *sys )
{
   int i, pos;
   xmlDocPtr doc;
   xmlTextWriterPtr writer;
   const Planet **sorted_planets;
   const JumpPoint **sorted_jumps, *jp;
   char *file, *cleanName;


   /* Reconstruct jumps so jump pos are updated. */
   system_reconstructJumps(sys);

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
   xmlw_startElem( writer, "ssys" );

   /* Attributes. */
   xmlw_attr( writer, "name", "%s", sys->name );

   /* General. */
   xmlw_startElem( writer, "general" );
   if (sys->background != NULL)
      xmlw_elem( writer, "background", "%s", sys->background );
   xmlw_elem( writer, "radius", "%f", sys->radius );
   xmlw_elem( writer, "stars", "%d", sys->stars );
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
   sorted_planets = malloc( sizeof(Planet*) * sys->nplanets);
   memcpy( sorted_planets, sys->planets, sizeof(Planet*) * sys->nplanets );
   qsort( sorted_planets, sys->nplanets, sizeof(Planet*), dsys_compPlanet );
   xmlw_startElem( writer, "assets" );
   for (i=0; i<sys->nplanets; i++)
      xmlw_elem( writer, "asset", "%s", sorted_planets[i]->name );
   xmlw_endElem( writer ); /* "assets" */
   free(sorted_planets);

   /* Jumps. */
   sorted_jumps = malloc( sizeof(JumpPoint*) * sys->njumps );
   for (i=0; i<sys->njumps; i++)
      sorted_jumps[i] = &sys->jumps[i];
   qsort( sorted_jumps, sys->njumps, sizeof(JumpPoint*), dsys_compJump );
   xmlw_startElem( writer, "jumps" );
   for (i=0; i<sys->njumps; i++) {
      jp = sorted_jumps[i];
      xmlw_startElem( writer, "jump" );
      xmlw_attr( writer, "target", "%s", jp->target->name );
      /* Position. */
      if (!jp_isFlag( jp, JP_AUTOPOS )) {
         xmlw_startElem( writer, "pos" );
         xmlw_attr( writer, "x", "%f", jp->pos.x );
         xmlw_attr( writer, "y", "%f", jp->pos.y );
         xmlw_endElem( writer ); /* "pos" */
      }
      else
         xmlw_elemEmpty( writer, "autopos" );
      /* Radius and misc properties. */
      if (jp->radius != 200.)
         xmlw_elem( writer, "radius", "%f", jp->radius );
      /* More flags. */
      if (jp_isFlag( jp, JP_HIDDEN ))
         xmlw_elemEmpty( writer, "hidden" );
      if (jp_isFlag( jp, JP_EXITONLY ))
         xmlw_elemEmpty( writer, "exitonly" );
      xmlw_elem( writer, "hide", "%f", sqrt(jp->hide) );
      xmlw_endElem( writer ); /* "jump" */
   }
   xmlw_endElem( writer ); /* "jumps" */
   free(sorted_jumps);

   xmlw_endElem( writer ); /** "ssys" */
   xmlw_done(writer);

   /* No need for writer anymore. */
   xmlFreeTextWriter(writer);

   /* Write data. */
   cleanName = calloc( 1, (strlen(sys->name)+1)*sizeof(char) );
   pos = 0;
   for (i=0; i<(int)strlen(sys->name); i++) {
      if (!ispunct(sys->name[i])) {
         if (sys->name[i] == ' ')
            cleanName[pos] = '_';
         else
            cleanName[pos] = tolower(sys->name[i]);
         pos++;
      }
   }
   file = malloc((pos+20)*sizeof(char));
   nsnprintf(file,(pos+20)*sizeof(char),"dat/ssys/%s.xml",cleanName);
   xmlSaveFileEnc( file, doc, "UTF-8" );

   /* Clean up. */
   xmlFreeDoc(doc);
   free(cleanName);

   return 0;
}


/**
 * @brief Saves all the star systems.
 *
 *    @return 0 on success.
 */
int dsys_saveAll (void)
{
   int i;
   int nsys;
   StarSystem *sys;

   sys = system_getAll( &nsys );

   /* Write systems. */
   for (i=0; i<nsys; i++)
      dsys_saveSystem( &sys[i] );

   return 0;
}


