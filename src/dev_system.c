/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file dev_system.c
 *
 * @brief Handles development of star system stuff.
 */
/** @cond */
#include <stdlib.h> /* qsort */

#include "naev.h"
/** @endcond */

#include "dev_system.h"

#include "array.h"
#include "conf.h"
#include "dev_uniedit.h"
#include "nstring.h"
#include "nxml.h"
#include "physics.h"
#include "space.h"
#include "nebula.h"

/*
 * Prototypes.
 */
static int dsys_compSpob( const void *spob1, const void *spob2 );
static int dsys_compJump( const void *jmp1, const void *jmp2 );

/**
 * @brief Compare function for spob qsort.
 *
 *    @param spob1 Spob 1 to sort.
 *    @param spob2 Spob 2 to sort.
 *    @return Order to sort.
 */
static int dsys_compSpob( const void *spob1, const void *spob2 )
{
   const Spob *p1, *p2;
   p1 = * (const Spob**) spob1;
   p2 = * (const Spob**) spob2;
   return strcmp( p1->name, p2->name );
}

/**
 * @brief Compare function for spob qsort.
 *
 *    @param spob1 Virtual spob 1 to sort.
 *    @param spob2 Virtual spob 2 to sort.
 *    @return Order to sort.
 */
static int dsys_compVirtualSpob( const void *spob1, const void *spob2 )
{
   const VirtualSpob *va1, *va2;
   va1 = *(const VirtualSpob**) spob1;
   va2 = *(const VirtualSpob**) spob2;
   return strcmp( va1->name, va2->name );
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
 *    @param sys Star system to save.
 *    @return 0 on success.
 */
int dsys_saveSystem( StarSystem *sys )
{
   xmlDocPtr doc;
   xmlTextWriterPtr writer;
   const Spob **sorted_spobs;
   const VirtualSpob **sorted_virtualspobs;
   const JumpPoint **sorted_jumps;
   char *file, *cleanName;

   /* Reconstruct jumps so jump pos are updated. */
   system_reconstructJumps(sys);

   /* Create the writer. */
   writer = xmlNewTextWriterDoc(&doc, 0);
   if (writer == NULL) {
      WARN(_("testXmlwriterDoc: Error creating the xml writer"));
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
   if (sys->map_shader != NULL)
      xmlw_elem( writer, "map_shader", "%s", sys->map_shader );
   if (sys->features != NULL)
      xmlw_elem( writer, "features", "%s", sys->features );
   xmlw_elem( writer, "radius", "%f", sys->radius );
   xmlw_elem( writer, "spacedust", "%d", sys->spacedust );
   xmlw_elem( writer, "interference", "%f", sys->interference );
   if (sys->nebu_density > 0.) {
      xmlw_startElem( writer, "nebula" );
      xmlw_attr( writer, "volatility", "%f", sys->nebu_volatility );
      if (fabs(sys->nebu_hue*360.0 - NEBULA_DEFAULT_HUE) > 1e-5)
         xmlw_attr( writer, "hue", "%f", sys->nebu_hue*360.0 );
      xmlw_str( writer, "%f", sys->nebu_density );
      xmlw_endElem( writer ); /* "nebula" */
   }
   xmlw_endElem( writer ); /* "general" */

   /* Position. */
   xmlw_startElem( writer, "pos" );
   xmlw_attr( writer, "x", "%f", sys->pos.x );
   xmlw_attr( writer, "y", "%f", sys->pos.y );
   xmlw_endElem( writer ); /* "pos" */

   /* Sort spobs. */
   sorted_spobs = malloc( sizeof(Spob*) * array_size(sys->spobs) );
   memcpy( sorted_spobs, sys->spobs, sizeof(Spob*) * array_size(sys->spobs) );
   qsort( sorted_spobs, array_size(sys->spobs), sizeof(Spob*), dsys_compSpob );

   /* Sort virtual spobs. */
   sorted_virtualspobs = malloc( sizeof(VirtualSpob*) * array_size(sys->spobs_virtual) );
   memcpy( sorted_virtualspobs, sys->spobs_virtual, sizeof(VirtualSpob*) * array_size(sys->spobs_virtual) );
   qsort( sorted_virtualspobs, array_size(sys->spobs_virtual), sizeof(VirtualSpob*), dsys_compVirtualSpob );

   /* Write spobs and clean up. */
   xmlw_startElem( writer, "spobs" );
   for (int i=0; i<array_size(sys->spobs); i++)
      xmlw_elem( writer, "spob", "%s", sorted_spobs[i]->name );
   for (int i=0; i<array_size(sys->spobs_virtual); i++)
      xmlw_elem( writer, "spob_virtual", "%s", sorted_virtualspobs[i]->name );
   xmlw_endElem( writer ); /* "spobs" */
   free(sorted_spobs);
   free(sorted_virtualspobs);

   /* Jumps. */
   sorted_jumps = malloc( sizeof(JumpPoint*) * array_size(sys->jumps) );
   for (int i=0; i<array_size(sys->jumps); i++)
      sorted_jumps[i] = &sys->jumps[i];
   qsort( sorted_jumps, array_size(sys->jumps), sizeof(JumpPoint*), dsys_compJump );
   xmlw_startElem( writer, "jumps" );
   for (int i=0; i<array_size(sys->jumps); i++) {
      const JumpPoint *jp = sorted_jumps[i];
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
      xmlw_elem( writer, "hide", "%f", jp->hide );
      xmlw_endElem( writer ); /* "jump" */
   }
   xmlw_endElem( writer ); /* "jumps" */
   free(sorted_jumps);

   /* Asteroids. */
   if (array_size(sys->asteroids) > 0 || array_size(sys->astexclude) > 0) {
      xmlw_startElem( writer, "asteroids" );
      for (int i=0; i<array_size(sys->asteroids); i++) {
         const AsteroidAnchor *ast = &sys->asteroids[i];
         xmlw_startElem( writer, "asteroid" );

         /* Type Groups */
         for (int j=0; j<array_size(ast->groups); j++)
            xmlw_elem( writer, "group", "%s", ast->groups[j]->name );

         /* Radius */
         xmlw_elem( writer, "radius", "%f", ast->radius );

         /* Position */
         xmlw_startElem( writer, "pos" );
         xmlw_attr( writer, "x", "%f", ast->pos.x );
         xmlw_attr( writer, "y", "%f", ast->pos.y );
         xmlw_endElem( writer ); /* "pos" */

         /* Misc. properties. */
         if (ast->density != ASTEROID_DEFAULT_DENSITY)
            xmlw_elem( writer, "density", "%f", ast->density );
         if (ast->maxspeed != ASTEROID_DEFAULT_MAXSPEED)
            xmlw_elem( writer, "maxspeed", "%f", ast->maxspeed );
         if (ast->thrust != ASTEROID_DEFAULT_THRUST)
            xmlw_elem( writer, "thrust", "%f", ast->thrust );
         xmlw_endElem( writer ); /* "asteroid" */
      }
      for (int i=0; i<array_size(sys->astexclude); i++) {
         const AsteroidExclusion *aexcl = &sys->astexclude[i];
         xmlw_startElem( writer, "exclusion" );

         /* Radius */
         xmlw_elem( writer, "radius", "%f", aexcl->radius );

         /* Position */
         xmlw_startElem( writer, "pos" );
         xmlw_attr( writer, "x", "%f", aexcl->pos.x );
         xmlw_attr( writer, "y", "%f", aexcl->pos.y );
         xmlw_endElem( writer ); /* "pos" */
         xmlw_endElem( writer ); /* "exclusion" */
      }
      xmlw_endElem( writer ); /* "asteroids" */
   }

   if (sys->stats != NULL) {
      xmlw_startElem( writer, "stats" );
      ss_listToXML( writer, sys->stats );
      xmlw_endElem( writer ); /* "stats" */
   }

   if (array_size(sys->tags)>0) {
      xmlw_startElem( writer, "tags" );
      for (int i=0; i<array_size(sys->tags); i++)
         xmlw_elem( writer, "tag", "%s", sys->tags[i] );
      xmlw_endElem( writer ); /* "tags" */
   }

   xmlw_endElem( writer ); /** "ssys" */
   xmlw_done(writer);

   /* No need for writer anymore. */
   xmlFreeTextWriter(writer);

   /* Write data. */
   cleanName = uniedit_nameFilter( sys->name );
   asprintf( &file, "%s/%s.xml", conf.dev_save_sys, cleanName );
   if (xmlSaveFileEnc( file, doc, "UTF-8" ) < 0)
      WARN("Failed writing '%s'!", file);

   /* Clean up. */
   xmlFreeDoc(doc);
   free(cleanName);
   free(file);

   return 0;
}

/**
 * @brief Saves all the star systems.
 *
 *    @return 0 on success.
 */
int dsys_saveAll (void)
{
   StarSystem *sys = system_getAll();

   /* Write systems. */
   for (int i=0; i<array_size(sys); i++)
      dsys_saveSystem( &sys[i] );

   return 0;
}
