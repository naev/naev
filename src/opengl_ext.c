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
#include "conf.h"


/*
 * Prototypes
 */
static void* gl_extGetProc( const char *proc );
static int gl_extVBO (void);
static int gl_extMultitexture (void);
static int gl_extMipmaps (void);


/**
 * @brief Tries to load an opengl function pointer.
 *
 * This function will generate a warning if fails, if you want to test to see
 *  if it's available use SDL_GL_GetProcAddress directly.
 *
 *    @param proc Function to find.
 *    @return Function pointer to proc or NULL on error.
 */
static void* gl_extGetProc( const char *proc )
{
   void *procGL;

   procGL = SDL_GL_GetProcAddress( proc );
   if (procGL == NULL)
      WARN("OpenGL function pointer to '%s' not found.", proc);

   return procGL;
}


/**
 * @brief Tries to load the multitexture extensions.
 */
static int gl_extMultitexture (void)
{
   /* Multitexture. */
   if (gl_hasVersion( 1, 3 )) {
      nglActiveTexture        = gl_extGetProc("glActiveTexture");
      nglClientActiveTexture  = gl_extGetProc("glClientActiveTexture");
      nglMultiTexCoord2d      = gl_extGetProc("glMultiTexCoord2d");
   }
   else if (gl_hasExt("GL_ARB_multitexture")) {
      nglActiveTexture        = gl_extGetProc("glActiveTextureARB");
      nglClientActiveTexture  = gl_extGetProc("glClientActiveTextureARB");
      nglMultiTexCoord2d      = gl_extGetProc("glMultiTexCoord2dARB");
   }
   else {
      nglActiveTexture        = NULL;
      nglClientActiveTexture  = NULL;
      nglMultiTexCoord2d      = NULL;
      WARN("GL_ARB_multitexture not found!");
   }
   return 0;
}


/**
 * @brief Loads the VBO extensions.
 */
static int gl_extVBO (void)
{
   /* Vertex Buffers. */
   if (conf.vbo && gl_hasVersion( 1, 5 )) {
      nglGenBuffers     = gl_extGetProc("glGenBuffers");
      nglBindBuffer     = gl_extGetProc("glBindBuffer");
      nglBufferData     = gl_extGetProc("glBufferData");
      nglBufferSubData  = gl_extGetProc("glBufferSubData");
      nglMapBuffer      = gl_extGetProc("glMapBuffer");
      nglUnmapBuffer    = gl_extGetProc("glUnmapBuffer");
      nglDeleteBuffers  = gl_extGetProc("glDeleteBuffers");
   }
   else if (conf.vbo && gl_hasExt("GL_ARB_vertex_buffer_object")) {
      nglGenBuffers     = gl_extGetProc("glGenBuffersARB");
      nglBindBuffer     = gl_extGetProc("glBindBufferARB");
      nglBufferData     = gl_extGetProc("glBufferDataARB");
      nglBufferSubData  = gl_extGetProc("glBufferSubDataARB");
      nglMapBuffer      = gl_extGetProc("glMapBufferARB");
      nglUnmapBuffer    = gl_extGetProc("glUnmapBufferARB");
      nglDeleteBuffers  = gl_extGetProc("glDeleteBuffersARB");
   }
   else {
      nglGenBuffers     = NULL;
      nglBindBuffer     = NULL;
      nglBufferData     = NULL;
      nglBufferSubData  = NULL;
      nglMapBuffer      = NULL;
      nglUnmapBuffer    = NULL;
      nglDeleteBuffers  = NULL;
      if (!conf.vbo)
         DEBUG("VBOs disabled.");
      else
         WARN("GL_ARB_vertex_buffer_object not found!");
   }
   return 0;
}


/**
 * @brief Tries to initialize the mipmap extension.
 */
static int gl_extMipmaps (void)
{
   nglGenerateMipmap = SDL_GL_GetProcAddress("glGenerateMipmap");
   if (nglGenerateMipmap==NULL)
      nglGenerateMipmap = SDL_GL_GetProcAddress("glGenerateMipmapEXT");
   if (nglGenerateMipmap==NULL)
      WARN("glGenerateMipmap not found.");
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
   gl_extMipmaps();

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

