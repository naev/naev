/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nxml.c
 *
 * Handles some complex xml parsing.
 */


/** @cond */
#include "naev.h"
/** @endcond */

#include "nxml.h"

#include "ndata.h"
#include "nstring.h"


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
glTexture* xml_parseTexture( xmlNodePtr node,
      const char *path, int defsx, int defsy,
      const unsigned int flags )
{
   int sx, sy;
   char *buf, filename[PATH_MAX];
   glTexture *tex;

   xmlr_attr_int_def(node, "sx", sx, defsx );
   xmlr_attr_int_def(node, "sy", sy, defsy );

   /* Get graphic to load. */
   buf = xml_get( node );
   if (buf == NULL)
      return NULL;

   /* Convert name. */
   snprintf( filename, sizeof(filename), (path != NULL) ? path : "%s", buf );

   /* Load the graphic. */
   if ((sx == 1) && (sy == 1))
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
   xmlTextWriterSetIndentString(writer, (const xmlChar*)" ");
   xmlTextWriterSetIndent(writer, 1);
}


/**
 * @brief Analogous to xmlParseMemory/xmlParseFile.
 * @param filename PhysFS file name.
 * @return doc (must xmlFreeDoc) on success, NULL on failure (will warn user).
 */
xmlDocPtr xml_parsePhysFS( const char* filename )
{
   char *buf;
   size_t bufsize;
   xmlDocPtr doc;

   /* @TODO: Don't slurp?
    * Can we directly create an InputStream backed by PHYSFS_*, or use SAX? */
   buf = ndata_read( filename, &bufsize );
   if (buf == NULL) {
      WARN( _("Unable to read data from '%s'"), filename );
      return NULL;
   }
   doc = xmlParseMemory( buf, bufsize );
   if (doc == NULL)
      WARN( _("Unable to parse document '%s'"), filename );
   free( buf );
   return doc;
}

int xmlw_saveTime( xmlTextWriterPtr writer, const char *name, struct tm *tm_time )
{
   time_t t;

   /* Save current time if NULL. */
   if (tm_time==NULL) {
      t = time(NULL);
      tm_time = localtime(&t);
   }

   xmlw_startElem(writer,name);

   xmlw_attr( writer, "year", "%d", tm_time->tm_year+1900 );
   xmlw_attr( writer, "month", "%d", tm_time->tm_mon );
   xmlw_attr( writer, "day", "%d", tm_time->tm_mday );
   xmlw_attr( writer, "hour", "%d", tm_time->tm_hour );
   xmlw_attr( writer, "minute", "%d", tm_time->tm_min );
   xmlw_attr( writer, "second", "%d", tm_time->tm_sec );

   xmlw_endElem(writer); /* name */

   return 0;
}


int xml_parseTime( xmlNodePtr node, struct tm *tm_time )
{
   time_t t = time(NULL);
   struct tm *local = localtime(&t);

   xmlr_attr_int_def( node, "year", tm_time->tm_year, local->tm_year );
   xmlr_attr_int_def( node, "month", tm_time->tm_mon, local->tm_mon );
   xmlr_attr_int_def( node, "day", tm_time->tm_mday, local->tm_mday );
   xmlr_attr_int_def( node, "hour", tm_time->tm_hour, local->tm_hour );
   xmlr_attr_int_def( node, "minute", tm_time->tm_min, local->tm_min );
   xmlr_attr_int_def( node, "second", tm_time->tm_sec, local->tm_sec );
   tm_time->tm_year -= 1900; /**< Correct it. */
   return 0;
}
