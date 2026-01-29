/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <SDL3/SDL_endian.h>
#include <SDL3/SDL_iostream.h>
#include <stdint.h>
/** @endcond */

#include "attributes.h"
#include "colour.h"

/* Recommended for compatibility and such */
#define RMASK SDL_Swap32LE( 0x000000ff ) /**< Red bit mask. */
#define GMASK SDL_Swap32LE( 0x0000ff00 ) /**< Green bit mask. */
#define BMASK SDL_Swap32LE( 0x00ff0000 ) /**< Blue bit mask. */
#define AMASK SDL_Swap32LE( 0xff000000 ) /**< Alpha bit mask. */
#define RGBAMASK RMASK, GMASK, BMASK, AMASK

/*
 * Texture flags.
 */
#define OPENGL_TEX_MIPMAPS ( 1 << 1 ) /**< Creates mipmaps. */
#define OPENGL_TEX_SKIPCACHE                                                   \
   ( 1 << 2 ) /**< Skip caching checks and create new texture. */
#define OPENGL_TEX_SDF                                                         \
   ( 1 << 3 ) /**< Convert to an SDF. Only the alpha channel gets used. */
#define OPENGL_TEX_CLAMP_ALPHA                                                 \
   ( 1 << 4 ) /**< Clamp image border to transparency. */
#define OPENGL_TEX_NOTSRGB ( 1 << 5 ) /**< Texture is not in SRGB format. */

struct glTexture;
typedef struct glTexture glTexture;

/*
 * Init/exit.
 */
int  gl_initTextures( void );
void gl_exitTextures( void );

/*
 * Creating.
 */
int                   gl_texExistsPath( const char *path );
USE_RESULT glTexture *gl_texExistsOrCreate( const char  *path,
                                            unsigned int flags, int sx, int sy,
                                            int *created );
USE_RESULT glTexture *gl_loadImageData( float *data, int w, int h, int sx,
                                        int sy, const char *name,
                                        unsigned int flags );
USE_RESULT glTexture *gl_newImage( const char *path, const unsigned int flags );
USE_RESULT glTexture *gl_tryNewImage( const char        *path,
                                      const unsigned int flags );
USE_RESULT glTexture *gl_newSprite( const char *path, const int sx,
                                    const int sy, const unsigned int flags );
USE_RESULT glTexture *gl_newSpriteRWops( const char *path, SDL_IOStream *rw,
                                         const int sx, const int sy,
                                         const unsigned int flags );
USE_RESULT glTexture *gl_dupTexture( const glTexture *texture );
glTexture *gl_resizeTexture( const glTexture *texture, double scale );
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
glTexture **gl_copyTexArray( const glTexture **tex );
glTexture **gl_addTexArray( glTexture **tex, glTexture *t );

/* Transition getters. */
const char *tex_name( const glTexture *tex );
double      tex_w( const glTexture *tex );
double      tex_h( const glTexture *tex );
double      tex_sw( const glTexture *tex );
double      tex_sh( const glTexture *tex );
double      tex_sx( const glTexture *tex );
double      tex_sy( const glTexture *tex );
double      tex_srw( const glTexture *tex );
double      tex_srh( const glTexture *tex );
int         tex_isSDF( const glTexture *tex );
int         tex_hasTrans( const glTexture *tex );
GLuint      tex_tex( const glTexture *tex );
GLuint      tex_sampler( const glTexture *tex );
double      tex_vmax( const glTexture *tex );
void        tex_setTex( glTexture *tex, GLuint texture );
