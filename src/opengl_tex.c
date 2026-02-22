/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file opengl_tex.c
 *
 * @brief This file handles the openGL texture wrapper routines.
 */
/** @cond */
#include <SDL3/SDL_mutex.h>
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "array.h"
#include "log.h"
#include "opengl.h"

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
 * @brief Cleans up the openGL texture subsystem.
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
   glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

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
 * @brief Sets x and y to be the appropriate sprite for `glTexture` using `dir`.
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
