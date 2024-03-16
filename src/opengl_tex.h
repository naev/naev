/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include "SDL.h"
#include <stdint.h>
/** @endcond */

#include "attributes.h"
#include "colour.h"

/* Recommended for compatibility and such */
#define RMASK SDL_SwapLE32( 0x000000ff ) /**< Red bit mask. */
#define GMASK SDL_SwapLE32( 0x0000ff00 ) /**< Green bit mask. */
#define BMASK SDL_SwapLE32( 0x00ff0000 ) /**< Blue bit mask. */
#define AMASK SDL_SwapLE32( 0xff000000 ) /**< Alpha bit mask. */
#define RGBAMASK RMASK, GMASK, BMASK, AMASK

/*
 * Texture flags.
 */
#define OPENGL_TEX_MAPTRANS ( 1 << 0 ) /**< Create a transparency map. */
#define OPENGL_TEX_MIPMAPS ( 1 << 1 )  /**< Creates mipmaps. */
#define OPENGL_TEX_VFLIP                                                       \
   ( 1 << 2 ) /**< Assume loaded from an image (where positive y means down).  \
               */
#define OPENGL_TEX_SKIPCACHE                                                   \
   ( 1 << 3 ) /**< Skip caching checks and create new texture. */
#define OPENGL_TEX_SDF                                                         \
   ( 1 << 4 ) /**< Convert to an SDF. Only the alpha channel gets used. */
#define OPENGL_TEX_CLAMP_ALPHA                                                 \
   ( 1 << 5 ) /**< Clamp image border to transparency. */

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

   /* sprites */
   double sx;  /**< Number of sprites on the x axis. */
   double sy;  /**< Number of sprites on the y axis. */
   double sw;  /**< Width of a sprite. */
   double sh;  /**< Height of a sprite. */
   double srw; /**< Sprite render width - equivalent to sw/w. */
   double srh; /**< Sprite render height - equivalent to sh/h. */

   /* data */
   GLuint   texture; /**< the opengl texture itself */
   uint8_t *trans;   /**< maps the transparency */
   double   vmax;    /**< Maximum value for SDF textures. */

   /* properties */
   uint8_t flags; /**< flags used for texture properties */
} glTexture;

/*
 * Init/exit.
 */
int  gl_initTextures( void );
void gl_exitTextures( void );

/*
 * Creating.
 */
USE_RESULT glTexture *gl_loadImageData( float *data, int w, int h, int sx,
                                        int sy, const char *name );
USE_RESULT glTexture *gl_newImage( const char *path, const unsigned int flags );
USE_RESULT glTexture                       *
gl_newImageRWops( const char *path, SDL_RWops *rw,
                                        const unsigned int flags ); /* Does not close the RWops. */
USE_RESULT glTexture *gl_newSprite( const char *path, const int sx,
                                    const int sy, const unsigned int flags );
USE_RESULT glTexture *gl_newSpriteRWops( const char *path, SDL_RWops *rw,
                                         const int sx, const int sy,
                                         const unsigned int flags );
USE_RESULT glTexture *gl_dupTexture( const glTexture *texture );
USE_RESULT glTexture *gl_rawTexture( const char *name, GLuint tex, double w,
                                     double h );

/*
 * Clean up.
 */
void gl_freeTexture( glTexture *texture );

/*
 * FBO stuff.
 */
int gl_fboCreate( GLuint *fbo, GLuint *tex, GLsizei width, GLsizei height );
int gl_fboAddDepth( GLuint fbo, GLuint *tex, GLsizei width, GLsizei height );

/*
 * Misc.
 */
void        gl_contextSet( void );
void        gl_contextUnset( void );
int         gl_isTrans( const glTexture *t, const int x, const int y );
void        gl_getSpriteFromDir( int *x, int *y, int sx, int sy, double dir );
glTexture **gl_copyTexArray( glTexture **tex );
glTexture **gl_addTexArray( glTexture **tex, glTexture *t );
