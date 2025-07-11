/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file opengl_tex.c
 *
 * @brief This file handles the opengl texture wrapper routines.
 */
/** @cond */
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "array.h"
#include "distance_field.h"
#include "log.h"
#include "md5.h"
#include "nfile.h"
#include "opengl.h"

/**
 * @brief Abstraction for rendering sprite sheets.
 *
 * The basic unit all the graphic rendering works with.
 */
typedef struct glTexture {
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
 * graphic list
 */
/**
 * @brief Represents a node in the texture list.
 */
typedef struct glTexList_ {
   glTexture  *tex;  /**< associated texture */
   const char *path; /**< Path pointer, stored in tex. */
   int         used; /**< counts how many times texture is being used */
   /* TODO We currently treat images with different number of sprites as
    * different images, i.e., they get reloaded and use more memory. However,
    * it should be possible to do something fancier and share the texture to
    * avoid this increase of memory (without sharing other parameters). */
   int          sx;    /**< X sprites */
   int          sy;    /**< Y sprites */
   unsigned int flags; /**< Flags being used. */
} glTexList;
static SDL_Mutex   *gl_lock = NULL; /**< Lock for OpenGL functions. */
static SDL_ThreadID tex_mainthread;
static SDL_Mutex   *tex_lock = NULL; /**< Lock for texture list manipulation. */

int gl_initTextures( void )
{
   gl_lock        = SDL_CreateMutex();
   tex_lock       = SDL_CreateMutex();
   tex_mainthread = SDL_GetCurrentThreadID();
   return 0;
}

/**
 * @brief Cleans up the opengl texture subsystem.
 */
void gl_exitTextures( void )
{
   SDL_DestroyMutex( tex_lock );
   SDL_DestroyMutex( gl_lock );
}

static void tex_ctxSet( void )
{
   if ( SDL_GetCurrentThreadID() != tex_mainthread )
      SDL_GL_MakeCurrent( gl_screen.window, gl_screen.context );
}

static void tex_ctxUnset( void )
{
   if ( SDL_GetCurrentThreadID() != tex_mainthread )
      SDL_GL_MakeCurrent( gl_screen.window, NULL );
}

void gl_contextSet( void )
{
   SDL_LockMutex( gl_lock );
   tex_ctxSet();
}

void gl_contextUnset( void )
{
   tex_ctxUnset();
   SDL_UnlockMutex( gl_lock );
}

/**
 * @brief Creates a framebuffer and its associated texture.
 *
 *    @param[out] fbo Framebuffer object id.
 *    @param[out] tex Texture id.
 *    @param width Width to use.
 *    @param height Height to use.
 *    @return 0 on success.
 */
int gl_fboCreate( GLuint *fbo, GLuint *tex, GLsizei width, GLsizei height )
{
   GLenum status;

   SDL_LockMutex( gl_lock );
   // tex_ctxSet();

   /* Create the render buffer. */
   glGenTextures( 1, tex );
   glBindTexture( GL_TEXTURE_2D, *tex );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );

   /* Create the frame buffer. */
   glGenFramebuffers( 1, fbo );
   glBindFramebuffer( GL_FRAMEBUFFER, *fbo );

   /* Attach the colour buffer. */
   glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           *tex, 0 );

   /* Check status. */
   status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
   if ( status != GL_FRAMEBUFFER_COMPLETE )
      WARN( _( "Error setting up framebuffer!" ) );

   /* Restore state. */
   glBindTexture( GL_TEXTURE_2D, 0 );
   glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.current_fbo );

   // tex_ctxUnset();
   SDL_UnlockMutex( gl_lock );

   gl_checkErr();

   return ( status == GL_FRAMEBUFFER_COMPLETE );
}

/**
 * @brief Adds a depth attachment to an FBO.
 */
int gl_fboAddDepth( GLuint fbo, GLuint *tex, GLsizei width, GLsizei height )
{
   GLenum status;

   SDL_LockMutex( gl_lock );
   // tex_ctxSet();

   /* Create the render buffer. */
   glGenTextures( 1, tex );
   glBindTexture( GL_TEXTURE_2D, *tex );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );

   /* Attach the depth. */
   glBindFramebuffer( GL_FRAMEBUFFER, fbo );
   glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           *tex, 0 );

   /* Check status. */
   status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
   if ( status != GL_FRAMEBUFFER_COMPLETE )
      WARN( _( "Error attaching depth to framebuffer!" ) );

   /* Restore state. */
   glBindTexture( GL_TEXTURE_2D, 0 );
   glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.current_fbo );

   // tex_ctxUnset();
   SDL_UnlockMutex( gl_lock );

   gl_checkErr();

   return ( status == GL_FRAMEBUFFER_COMPLETE );
}

/**
 * @brief Sets x and y to be the appropriate sprite for glTexture using dir.
 *
 * Very slow, try to cache if possible like the pilots do instead of using
 *  in O(n^2) or worse functions.
 *
 *    @param[out] x X sprite to use.
 *    @param[out] y Y sprite to use.
 *    @param sx Number of sprites in X direction.
 *    @param sy Number of sprites in Y direction.
 *    @param dir Direction to get sprite from.
 */
void gl_getSpriteFromDir( int *x, int *y, int sx, int sy, double dir )
{
   int    s;
   double shard, rdir;

#ifdef DEBUGGING
   if ( ( dir > 2. * M_PI ) || ( dir < 0. ) ) {
      WARN( _( "Angle not between 0 and 2.*M_PI [%f]." ), dir );
      *x = *y = 0;
      return;
   }
#endif /* DEBUGGING */

   /* what each image represents in angle */
   shard = 2. * M_PI / ( sy * sx );

   /* real dir is slightly moved downwards */
   rdir = dir + shard / 2.;

   /* now calculate the sprite we need */
   s = (int)( rdir / shard );

   /* makes sure the sprite is "in range" */
   if ( s > ( sy * sx - 1 ) )
      s = s % ( sy * sx );

   ( *x ) = s % sx;
   ( *y ) = s / sx;
}

/**
 * @brief Copy a texture array.
 */
glTexture **gl_copyTexArray( const glTexture **tex )
{
   glTexture **t;
   int         n = array_size( tex );

   if ( n <= 0 )
      return NULL;

   t = array_create_size( glTexture *, n );
   for ( int i = 0; i < array_size( tex ); i++ )
      array_push_back( &t, gl_dupTexture( tex[i] ) );
   return t;
}

/**
 * @brief Adds an element to a texture array.
 */
glTexture **gl_addTexArray( glTexture **tex, glTexture *t )
{
   if ( tex == NULL )
      tex = array_create_size( glTexture *, 1 );
   array_push_back( &tex, t );
   return tex;
}

// TODO have to port the distance field stuff to rust
#if 0
/**
 * @brief Loads a surface into an opengl texture.
 *
 *    @param surface Surface to load into a texture.
 *    @param flags Flags to use.
 *    @param freesur Whether or not to free the surface.
 *    @param[out] vmax The maximum value in the case of an SDF texture.
 *    @return The opengl texture id.
 */
static GLuint gl_loadSurface( SDL_Surface *surface, unsigned int flags,
                              int freesur, double *vmax )
{
   const SDL_PixelFormatEnum fmt = SDL_PIXELFORMAT_ABGR8888;
   GLuint                    texture;
   SDL_Surface              *rgba;
   int                       has_alpha = surface->format->Amask;

   gl_contextSet();

   /* Get texture. */
   texture = gl_texParameters( flags );

   /* Now load the texture data up
    * It doesn't work with indexed ones, so I guess converting is best bet. */
   if ( surface->format->format != fmt )
      rgba = SDL_ConvertSurfaceFormat( surface, fmt, 0 );
   else
      rgba = surface;

   SDL_LockSurface( rgba );
   if ( flags & OPENGL_TEX_SDF ) {
      const float border[] = { 0., 0., 0., 0. };
      uint8_t    *trans    = SDL_MapAlpha( rgba, 0 );
      GLfloat    *dataf = make_distance_mapbf( trans, rgba->w, rgba->h, vmax );
      free( trans );
      glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
      glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, rgba->w, rgba->h, 0, GL_RED,
                    GL_FLOAT, dataf );
      glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
      free( dataf );
   } else {
      GLint internalformat;
      if ( flags & OPENGL_TEX_NOTSRGB )
         internalformat = has_alpha ? GL_RGBA : GL_RGB;
      else
         internalformat = has_alpha ? GL_SRGB_ALPHA : GL_SRGB;

      *vmax = 1.;
      glPixelStorei( GL_UNPACK_ALIGNMENT,
                     MIN( rgba->pitch & -rgba->pitch, 8 ) );
      glTexImage2D( GL_TEXTURE_2D, 0, internalformat, rgba->w, rgba->h, 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels );
      glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
   }
   SDL_UnlockSurface( rgba );
   if ( rgba != surface )
      SDL_DestroySurface( rgba );

   /* Create mipmaps. */
   if ( flags & OPENGL_TEX_MIPMAPS ) {
      /* Do fancy stuff. */
      if ( GLAD_GL_ARB_texture_filter_anisotropic ) {
         GLfloat param;
         glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY, &param );
         glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, param );
      }

      /* Now generate the mipmaps. */
      glGenerateMipmap( GL_TEXTURE_2D );
   }

   /* Unbind the texture. */
   glBindTexture( GL_TEXTURE_2D, 0 );

   /* cleanup */
   if ( freesur )
      SDL_DestroySurface( surface );
   gl_checkErr();

   gl_contextUnset();

   return texture;
}
#endif
