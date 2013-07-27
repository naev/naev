/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file dev_system.c
 *
 * @brief Handles development of star system stuff.
 */

#include "dev_system.h"
#include "dev_uniedit.h"

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

/* externs */
extern Commodity* commodity_stack;
extern int econ_nprices;


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
   int i, c, len;
   xmlDocPtr doc;
   xmlTextWriterPtr writer;
   const Planet **sorted_planets;
   const JumpPoint **sorted_jumps, *jp;
   char *file, *cleanName, buf[32];


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

      /* preserve prices */
   xmlw_startElem(writer,"prices");
   xmlw_attr(writer,"sys","%s",sys->name);
   for (c=0; c<econ_nprices; c++) {
      if (sys->is_priceset[c]){
         xmlw_startElem(writer,"commodity");
         xmlw_attr(writer, "name", "%s", commodity_stack[c].name);
         nsnprintf(buf, 32, "%.2f", sys->prices[c]);
         xmlw_str(writer, "%s", buf);
         xmlw_endElem(writer); /* "commodity" */
      }
   }
   xmlw_endElem(writer); /* prices */

   xmlw_endElem( writer ); /** "ssys" */
   xmlw_done(writer);

   /* No need for writer anymore. */
   xmlFreeTextWriter(writer);

   /* Write data. */
   cleanName = uniedit_nameFilter( sys->name );
   len       = (strlen(cleanName)+14);
   file      = malloc( len );
   nsnprintf( file, len, "dat/ssys/%s.xml", cleanName );
   xmlSaveFileEnc( file, doc, "UTF-8" );
   free( file );

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
/**
 * @brief Saves selected systems as a map outfit file.
 *
 *    @return 0 on success.
 */
int dsys_saveMap (StarSystem **uniedit_sys, int uniedit_nsys)
{
   int i, j, k, len;
   xmlDocPtr doc;
   xmlTextWriterPtr writer;
   StarSystem *s;
   char *file, *cleanName;

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
   xmlw_startElem( writer, "outfit" );

   /* Attributes. */
   xmlw_attr( writer, "name", "Editor-generated Map" );

   /* General. */
   xmlw_startElem( writer, "general" );
   xmlw_elem( writer, "mass", "%d", 0 );
   xmlw_elem( writer, "price", "%d", 1000 );
   xmlw_elem( writer, "description", "%s", "This map has been created by the universe editor." );
   xmlw_elem( writer, "gfx_store", "%s", "map" );
   xmlw_endElem( writer ); /* "general" */

   xmlw_startElem( writer, "specific" );
   xmlw_attr( writer, "type", "map" );

   /* Iterate over all selected systems. Save said systems and any NORMAL jumps they might share. */
   for (i = 0; i < uniedit_nsys; i++) {
      s = uniedit_sys[i];
      xmlw_startElem( writer, "sys" );
      xmlw_attr( writer, "name", s->name );

      /* Iterate jumps and see if they lead to any other systems in our array. */
      for (j = 0; j < s->njumps; j++) {
         /* Ignore hidden and exit-only jumps. */
         if (jp_isFlag(&s->jumps[j], JP_EXITONLY ))
            continue;
         if (jp_isFlag(&s->jumps[j], JP_HIDDEN))
            continue;
         /* This is a normal jump. */
         for (k = 0; k < uniedit_nsys; k++) {
            if (s->jumps[j].target == uniedit_sys[k]) {
               xmlw_elem( writer, "jump", "%s", uniedit_sys[k]->name );
               break;
            }
         }
      }

      /* Iterate assets and add them */
      for (j = 0; j < s->nplanets; j++) {
         if (s->planets[j]->real)
            xmlw_elem( writer, "asset", "%s", s->planets[j]->name );
      }

      xmlw_endElem( writer ); /* "sys" */
   }

   xmlw_endElem( writer ); /* "specific" */
   xmlw_endElem( writer ); /* "outfit" */
   xmlw_done(writer);

   /* No need for writer anymore. */
   xmlFreeTextWriter(writer);

   /* Write data. */
   cleanName = uniedit_nameFilter( "saved map" );
   len       = (strlen(cleanName)+22);
   file      = malloc( len );
   nsnprintf( file, len, "dat/outfits/maps/%s.xml", cleanName );
   xmlSaveFileEnc( file, doc, "UTF-8" );
   free( file );

   /* Clean up. */
   xmlFreeDoc(doc);
   free(cleanName);

   return 0;
}


