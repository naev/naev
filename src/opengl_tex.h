/*
 * See Licensing and Copyright notice in naev.h
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
#define OPENGL_TEX_MAPTRANS   (1<<0)  /**< Create a transparency map. */

/**
 * @brief Abstraction for rendering spriteshets.
 *
 * The basic unit all the graphic rendering works with.
 */
typedef struct glTexture_ {
   char *name; /**< name of the graphic */

   /* dimensions */
   double w; /**< Real width of the image. */
   double h; /**< Real heiht of the image. */
   double rw; /**< Padded POT width of the image. */
   double rh; /**< Padded POT height of the image. */

   /* sprites */
   double sx; /**< Number of sprites on the x axis. */
   double sy; /**< Number of sprites on the y axis. */
   double sw; /**< Width of a sprite. */
   double sh; /**< Height of a sprite. */

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
glTexture* gl_loadImage( SDL_Surface* surface ); /* Frees the surface. */
glTexture* gl_newImage( const char* path, const unsigned int flags );
glTexture* gl_newSprite( const char* path, const int sx, const int sy,
      const unsigned int flags );
glTexture* gl_dupTexture( glTexture *texture );

/*
 * Clean up.
 */
void gl_freeTexture( glTexture* texture );

/*
 * Misc.
 */
int gl_isTrans( const glTexture* t, const int x, const int y );
void gl_getSpriteFromDir( int* x, int* y, const glTexture* t, const double dir );


#endif /* OPENGL_TEX_H */
   
