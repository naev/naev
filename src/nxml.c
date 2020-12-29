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

   xmlr_attr_atoi_neg1(node, "sx", sx );
   xmlr_attr_atoi_neg1(node, "sy", sy );

   /*
    * Safe defaults.
    */
   if (sx == -1) sx = defsx;
   if (sy == -1) sy = defsy;

   /* Get graphic to load. */
   buf = xml_get( node );
   if (buf == NULL)
      return NULL;

   /* Convert name. */
   nsnprintf( filename, PATH_MAX, (path != NULL) ? path : "%s", buf );

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
