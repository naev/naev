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


#ifndef OPENGL_TEX_H
#  define OPENGL_TEX_H


#include <stdint.h>

#include "SDL.h"
#include "SDL_opengl.h"

#include "physics.h"
#include "colour.h"

#include "ncompat.h"


/* Recommended for compatibility and such */
#if HAS_BIGENDIAN
#  define RMASK   0xff000000 /**< Red bit mask. */
#  define GMASK   0x00ff0000 /**< Green bit mask. */
#  define BMASK   0x0000ff00 /**< Blue bit mask. */
#  define AMASK   0x000000ff /**< Alpha bit mask. */
#else
#  define RMASK   0x000000ff /**< Red bit mask. */
#  define GMASK   0x0000ff00 /**< Green bit mask. */
#  define BMASK   0x00ff0000 /**< Blue bit mask. */
#  define AMASK   0xff000000 /**< Alpha bit mask. */
#endif
#define RGBAMASK  RMASK,GMASK,BMASK,AMASK


/*
 * Texture flags.
 */
#define OPENGL_TEX_MAPTRANS   (1<<0) /**< Create a transparency map. */
#define OPENGL_TEX_MIPMAPS    (1<<1) /**< Creates mipmaps. */

/**
 * @brief Abstraction for rendering sprite sheets.
 *
 * The basic unit all the graphic rendering works with.
 */
typedef struct glTexture_ {
   char *name; /**< name of the graphic */

   /* dimensions */
   double w; /**< Real width of the image. */
   double h; /**< Real height of the image. */
   double rw; /**< Padded POT width of the image. */
   double rh; /**< Padded POT height of the image. */

   /* sprites */
   double sx; /**< Number of sprites on the x axis. */
   double sy; /**< Number of sprites on the y axis. */
   double sw; /**< Width of a sprite. */
   double sh; /**< Height of a sprite. */
   double srw; /**< Sprite render width - equivalent to sw/rw. */
   double srh; /**< Sprite render height - equivalent to sh/rh. */

   /* data */
   GLuint texture; /**< the opengl texture itself */
   uint8_t* trans; /**< maps the transparency */

   /* properties */
   uint8_t flags; /**< flags used for texture properties */
} glTexture;


/*
 * Init/exit.
 */
int gl_initTextures (void);
void gl_exitTextures (void);


/*
 * Preparing.
 */
int gl_pot( int n );
SDL_Surface* gl_prepareSurface( SDL_Surface* surface ); /* Only preps it */

/*
 * Creating.
 */
glTexture* gl_loadImagePad( const char *name, SDL_Surface* surface,
      unsigned int flags, int w, int h, int sx, int sy, int freesur );
glTexture* gl_loadImagePadTrans( const char *name, SDL_Surface* surface, SDL_RWops *rw,
      unsigned int flags, int w, int h, int sx, int sy, int freesur );
glTexture* gl_loadImage( SDL_Surface* surface, const unsigned int flags ); /* Frees the surface. */
glTexture* gl_newImage( const char* path, const unsigned int flags );
glTexture* gl_newSprite( const char* path, const int sx, const int sy,
      const unsigned int flags );
glTexture* gl_dupTexture( glTexture *texture );

/*
 * Clean up.
 */
void gl_freeTexture( glTexture* texture );

/*
 * Info.
 */
int gl_texHasMipmaps (void);
int gl_texHasCompress (void);

/*
 * Misc.
 */
int gl_isTrans( const glTexture* t, const int x, const int y );
void gl_getSpriteFromDir( int* x, int* y, const glTexture* t, const double dir );
int gl_needPOT (void);


#endif /* OPENGL_TEX_H */

