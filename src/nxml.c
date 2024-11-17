/**
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nxml.c
 *
 * Handles some complex xml parsing.
 */
#include "nxml.h"

#include "ndata.h"

/**
 * @brief Parses a texture handling the sx and sy elements.
 *
 *    @param node Node to parse.
 *    @param path Path to get file from, should be in the format of
 *           "PREFIX%sSUFFIX".
 *    @param defsx Default X sprites.
 *    @param defsy Default Y sprites.
 *    @param flags Image parameter control flags.
 *    @return The texture from the node or NULL if an error occurred.
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
 * @brief Analogous to xmlParseMemory/xmlParseFile.
 * @param filename PhysFS file name.
 * @return doc (must xmlFreeDoc) on success, NULL on failure (will warn user).
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
   xmlw_elem( writer, name, "%lu", t );
   return 0;
}

int xml_parseTime( xmlNodePtr node, time_t *t )
{
   *t = xml_getULong( node );
   return 0;
}
