/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file nxml.c
 *
 * Handles some complex xml parsing.
 */


#include "nxml.h"

#include "naev.h"

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

   /*
    * Sane defaults.
    */
   sx = defsx;
   sy = defsy;

   /* Read x sprites. */
   xmlr_attr(node, "sx", buf );
   if (buf != NULL) {
      sx = atoi(buf);
      free(buf);
   }

   /* Read y sprites. */
   xmlr_attr(node, "sy", buf );
   if (buf != NULL) {
      sy = atoi(buf);
      free(buf);
   }

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


