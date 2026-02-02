/**
 * See Licensing and Copyright notice in "naev.h"
 */
/**
 * @file nxml.c
 *
 * Handles some complex xml parsing.
 */
#include "nxml.h"
#include <inttypes.h>

#include <ctype.h>
#include <inttypes.h>

#include "ndata.h"

/**
 * @brief Parses a texture handling the `sx` and `sy` elements.
 *
 *    @param node Node to parse.
 *    @param path Path to get file from, should be in the format of
 *           `"PREFIX%sSUFFIX"`.
 *    @param defsx Default X sprites.
 *    @param defsy Default Y sprites.
 *    @param flags Image parameter control flags.
 *    @return The texture from the node or `NULL` if an error occurred.
 */
glTexture *xml_parseTexture( xmlNodePtr node, const char *path, int defsx,
                             int defsy, const unsigned int flags )
{
   int        sx, sy;
   char      *buf, filename[PATH_MAX];
   glTexture *tex;

   xmlr_attr_int_def( node, "sx", sx, defsx );
   xmlr_attr_int_def( node, "sy", sy, defsy );

   /* Get graphic to load. */
   buf = xml_get( node );
   if ( buf == NULL )
      return NULL;

   /* Check for absolute pathe. */
   if ( ( buf[0] == '/' ) || ( path == NULL ) )
      snprintf( filename, sizeof( filename ), "%s", buf );
   else
      snprintf( filename, sizeof( filename ), path, buf );

   /* Load the graphic. */
   if ( ( sx == 1 ) && ( sy == 1 ) )
      tex = gl_newImage( filename, flags );
   else
      tex = gl_newSprite( filename, sx, sy, flags );

   /* Return result. */
   return tex;
}

/**
 * @brief Sets up the standard xml write parameters.
 */
void xmlw_setParams( xmlTextWriterPtr writer )
{
   xmlTextWriterSetIndentString( writer, (const xmlChar *)" " );
   xmlTextWriterSetIndent( writer, 1 );
}

/**
 * @brief Analogous to `xmlParseMemory` / `xmlParseFile`.
 * @param filename `PhysFS` file name.
 * @return xml document (must `xmlFreeDoc`) on success, `NULL` on failure (will
 * warn user).
 */
xmlDocPtr xml_parsePhysFS( const char *filename )
{
   char     *buf;
   size_t    bufsize;
   xmlDocPtr doc;

   /* @TODO: Don't slurp?
    * Can we directly create an InputStream backed by PHYSFS_*, or use SAX? */
   buf = ndata_read( filename, &bufsize );
   if ( buf == NULL ) {
      WARN( _( "Unable to read data from '%s'" ), filename );
      return NULL;
   }
   /* Empty file, we ignore these. */
   if ( bufsize == 0 ) {
      free( buf );
      return NULL;
   }
   doc = xmlParseMemory( buf, bufsize );
   if ( doc == NULL )
      WARN( _( "Unable to parse document '%s'" ), filename );
   free( buf );
   return doc;
}

int xmlw_saveTime( xmlTextWriterPtr writer, const char *name, time_t t )
{
   xmlw_elem( writer, name, "%lld", (long long)t );
   return 0;
}

int xmlw_saveNTime( xmlTextWriterPtr writer, const char *name, ntime_t t )
{
   int maj, min, inc;
   ntime_split( t, &maj, &min, &inc );
   xmlw_startElem( writer, name );
   xmlw_attr( writer, "maj", "%d", maj );
   xmlw_attr( writer, "min", "%d", min );
   xmlw_attr( writer, "inc", "%d", inc );
   xmlw_endElem( writer );
   return 0;
}

int xml_parseTime( xmlNodePtr node, time_t *t )
{
   *t = xml_getULong( node );
   return 0;
}

int xml_parseNTime( xmlNodePtr node, ntime_t *t )
{
   /* Old format (until 0.13.0) */
   if ( xml_get( node ) != NULL ) {
      // Handles old format
      uint64_t n = xml_getULong( node );
      *t = ntime_create( n / ( 5000l * 10000l * 1000l ), n / ( 10000 * 1000 ),
                         n / 1000 );
   } else {
      int maj, min, inc;
      xmlr_attr_int_def( node, "maj", maj, 0 );
      xmlr_attr_int_def( node, "min", min, 0 );
      xmlr_attr_int_def( node, "inc", inc, 0 );
      *t = ntime_create( maj, min, inc );
   }
   return 0;
}

int nxml_init( void )
{
   LIBXML_TEST_VERSION
   xmlInitParser();
   return 0;
}
