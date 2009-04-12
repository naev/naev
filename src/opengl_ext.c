/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file opengl.c
 *
 * @brief This file handles most of the more generic opengl functions.
 */


#include "opengl.h"

#include "naev.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_version.h"

#include "log.h"


/*
 * Prototypes
 */
static int gl_extVBO (void);
static int gl_extMultitexture (void);


static int gl_extMultitexture (void)
{
   /* Multitexture. */
   if (gl_hasExt("GL_ARB_multitexture")) {
      nglActiveTexture = SDL_GL_GetProcAddress("glActiveTexture");
      nglMultiTexCoord2d = SDL_GL_GetProcAddress("glMultiTexCoord2d");
   }
   else {
      nglActiveTexture = NULL;
      nglMultiTexCoord2d = NULL;
      WARN("GL_ARB_multitexture not found!");
   }
   return 0;
}

static int gl_extVBO (void)
{
   /* Vertex Buffers. */
   if (gl_hasVersion( 1, 5)) {
      nglGenBuffers = SDL_GL_GetProcAddress("glGenBuffers");
      nglBindBuffer = SDL_GL_GetProcAddress("glBindBuffer");
      nglBufferData = SDL_GL_GetProcAddress("glBufferData");
      nglBufferSubData = SDL_GL_GetProcAddress("glBufferSubData");
      nglMapBuffer = SDL_GL_GetProcAddress("glMapBuffer");
      nglUnmapBuffer = SDL_GL_GetProcAddress("glUnmapBuffer");
      nglDeleteBuffers = SDL_GL_GetProcAddress("glDeleteBuffers");
   }
   else if (gl_hasExt("GL_ARB_vertex_buffer_object")) {
      nglGenBuffers = SDL_GL_GetProcAddress("glGenBuffersARB");
      nglBindBuffer = SDL_GL_GetProcAddress("glBindBufferARB");
      nglBufferData = SDL_GL_GetProcAddress("glBufferDataARB");
      nglBufferSubData = SDL_GL_GetProcAddress("glBufferSubDataARB");
      nglMapBuffer = SDL_GL_GetProcAddress("glMapBufferARB");
      nglUnmapBuffer = SDL_GL_GetProcAddress("glUnmapBufferARB");
      nglDeleteBuffers = SDL_GL_GetProcAddress("glDeleteBuffersARB");
   }
   else {
      nglGenBuffers = NULL;
      nglBindBuffer = NULL;
      nglBufferData = NULL;
      nglBufferSubData = NULL;
      nglMapBuffer = NULL;
      nglUnmapBuffer = NULL;
      nglDeleteBuffers = NULL;
      WARN("GL_ARB_vertex_buffer_object not found!");
   }
   return 0;
}


/**
 * @brief Initializes opengl extensions.
 *
 *    @return 0 on success.
 */
int gl_initExtensions (void)
{
   gl_extMultitexture();
   gl_extVBO();

   return 0;
}


/**
 * @brief Cleans up the opengl extensions.
 *
 *    @return 0 on success.
 */
void gl_exitExtensions (void)
{
}

