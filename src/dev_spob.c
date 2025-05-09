/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file dev_spob.c
 *
 * @brief Handles the spob stuff.
 */
/** @cond */
#include <stdlib.h>
/** @endcond */

#include <libgen.h>

#include "dev_spob.h"

#include "array.h"
#include "conf.h"
#include "nxml.h"
#include "start.h"

/**
 * @brief Saves a spob.
 *
 *    @param p Spob to save.
 *    @return 0 on success.
 */
int dpl_saveSpob( const Spob *p )
{
   xmlDocPtr        doc;
   xmlTextWriterPtr writer;
   char            *file;
   int              ret         = 0;
   const char      *lua_default = start_spob_lua_default();

   if ( conf.dev_data_dir == NULL ) {
      WARN( _( "%s is not set!" ), "conf.dev_data_dir" );
      return -1;
   }

   /* Create the writer. */
   writer = xmlNewTextWriterDoc( &doc, 0 );
   if ( writer == NULL ) {
      WARN( _( "testXmlwriterDoc: Error creating the xml writer" ) );
      return -1;
   }

   /* Set the writer parameters. */
   xmlw_setParams( writer );

   /* Start writer. */
   xmlw_start( writer );
   xmlw_startElem( writer, "spob" );

   /* Attributes. */
   xmlw_attr( writer, "name", "%s", p->name );

   /* Some global attributes. */
   if ( p->display != NULL )
      xmlw_elem( writer, "display", "%s", p->display );
   if ( p->feature != NULL )
      xmlw_elem( writer, "feature", "%s", p->feature );
   if ( ( p->lua_file != NULL ) &&
        ( ( lua_default == NULL ) || strcmp( lua_default, p->lua_file ) != 0 ) )
      xmlw_elem( writer, "lua", "%s", p->lua_file_raw );
   if ( spob_isFlag( p, SPOB_RADIUS ) )
      xmlw_elem( writer, "radius", "%f", p->radius );
   if ( p->marker != NULL )
      xmlw_elem( writer, "marker", "%s", p->marker->name );

   /* Position. */
   xmlw_startElem( writer, "pos" );
   xmlw_attr( writer, "x", "%f", p->pos.x );
   xmlw_attr( writer, "y", "%f", p->pos.y );
   xmlw_endElem( writer ); /* "pos" */

   /* GFX. */
   xmlw_startElem( writer, "GFX" );
   if ( p->gfx_space3dPath != NULL ) {
      xmlw_startElem( writer, "space3d" );
      xmlw_attr( writer, "size", "%f", p->gfx_space3d_size );
      xmlw_str( writer, "%s", p->gfx_space3dPath );
      xmlw_endElem( writer ); /* space3d */
   }
   if ( p->gfx_spacePath != NULL )
      xmlw_elem( writer, "space", "%s", p->gfx_spacePath );
   if ( p->gfx_exteriorPath != NULL )
      xmlw_elem( writer, "exterior", "%s", p->gfx_exteriorPath );
   if ( p->gfx_commPath != NULL )
      xmlw_elem( writer, "comm", "%s", p->gfx_commPath );
   xmlw_endElem( writer ); /* "GFX" */

   /* Presence. */
   if ( p->presence.faction >= 0 ) {
      xmlw_startElem( writer, "presence" );
      xmlw_elem( writer, "faction", "%s", faction_name( p->presence.faction ) );
      xmlw_elem( writer, "base", "%f", p->presence.base );
      xmlw_elem( writer, "bonus", "%f", p->presence.bonus );
      xmlw_elem( writer, "range", "%d", p->presence.range );
      xmlw_endElem( writer );
   }

   /* General. */
   xmlw_startElem( writer, "general" );
   xmlw_elem( writer, "class", "%s", p->class );
   xmlw_elem( writer, "population", "%g", (double)p->population );
   xmlw_elem( writer, "hide", "%f", p->hide );
   xmlw_startElem( writer, "services" );
   if ( spob_hasService( p, SPOB_SERVICE_LAND ) )
      xmlw_elemEmpty( writer, "land" );
   if ( spob_hasService( p, SPOB_SERVICE_REFUEL ) )
      xmlw_elemEmpty( writer, "refuel" );
   if ( spob_hasService( p, SPOB_SERVICE_BAR ) )
      xmlw_elemEmpty( writer, "bar" );
   if ( spob_hasService( p, SPOB_SERVICE_MISSIONS ) )
      xmlw_elemEmpty( writer, "missions" );
   if ( spob_hasService( p, SPOB_SERVICE_COMMODITY ) )
      xmlw_elemEmpty( writer, "commodity" );
   if ( spob_hasService( p, SPOB_SERVICE_OUTFITS ) )
      xmlw_elemEmpty( writer, "outfits" );
   if ( spob_hasService( p, SPOB_SERVICE_SHIPYARD ) )
      xmlw_elemEmpty( writer, "shipyard" );
   if ( spob_hasService( p, SPOB_SERVICE_BLACKMARKET ) )
      xmlw_elemEmpty( writer, "blackmarket" );
   if ( spob_isFlag( p, SPOB_NOMISNSPAWN ) )
      xmlw_elemEmpty( writer, "nomissionspawn" );
   if ( spob_isFlag( p, SPOB_UNINHABITED ) )
      xmlw_elemEmpty( writer, "uninhabited" );
   if ( spob_isFlag( p, SPOB_NOLANES ) )
      xmlw_elemEmpty( writer, "nolanes" );
   if ( spob_isFlag( p, SPOB_NOCOMMODITIES ) )
      xmlw_elemEmpty( writer, "nocommodities" );
   xmlw_endElem( writer ); /* "services" */
   if ( p->description != NULL )
      xmlw_elem( writer, "description", "%s", p->description );
   if ( p->bar_description != NULL )
      xmlw_elem( writer, "bar", "%s", p->bar_description );
   xmlw_endElem( writer ); /* "general" */

   /* Tech. */
   if ( spob_hasService( p, SPOB_SERVICE_LAND ) )
      tech_groupWrite( writer, p->tech );

   if ( array_size( p->tags ) > 0 ) {
      xmlw_startElem( writer, "tags" );
      for ( int i = 0; i < array_size( p->tags ); i++ )
         xmlw_elem( writer, "tag", "%s", p->tags[i] );
      xmlw_endElem( writer ); /* "tags" */
   }

   xmlw_endElem( writer ); /* "spob" */
   xmlw_done( writer );

   /* No need for writer anymore. */
   xmlFreeTextWriter( writer );

   /* Write data. */
   SDL_asprintf( &file, "%s/%s", conf.dev_data_dir, p->filename );
   if ( xmlSaveFileEnc( file, doc, "UTF-8" ) < 0 ) {
      WARN( "Failed to write '%s'!", file );
      ret = -1;
   }

   /* Clean up. */
   xmlFreeDoc( doc );
   free( file );

   return ret;
}

/**
 * @brief Saves all the star spobs.
 *
 *    @return 0 on success.
 */
int dpl_saveAll( void )
{
   const Spob *p   = spob_getAll();
   int         ret = 0;

   /* Write spobs. */
   for ( int i = 0; i < array_size( p ); i++ )
      ret |= dpl_saveSpob( &p[i] );

   return ret;
}
