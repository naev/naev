/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file opengl_ext.c
 *
 * @brief Handles opengl extensions.
 */


#include "opengl.h"

#include "naev.h"

#include "SDL.h"
#include "SDL_version.h"

#include "log.h"
#include "conf.h"


/*
 * Prototypes
 */
static void APIENTRY glGenerateMipmapNaev( GLenum target );
static void* gl_extGetProc( const char *proc );
static int gl_extVBO (void);
static int gl_extMultitexture (void);
static int gl_extMipmaps (void);
static int gl_extCompression (void);


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
      return -1;
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
      if (conf.vbo) {
         WARN("GL_ARB_vertex_buffer_object not found!");
         return -1;
      }
   }
   return 0;
}


/**
 * @brief Wrapper for glGenerateMipmap around GL_SGIS_generate_mipmap
 */
static void APIENTRY glGenerateMipmapNaev( GLenum target )
{
   glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
   glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
}


/**
 * @brief Tries to initialize the mipmap extension.
 */
static int gl_extMipmaps (void)
{
   if (!conf.mipmaps) {
      nglGenerateMipmap = NULL;
      return 0;
   }

   if (gl_hasVersion( 3, 0 ))
      nglGenerateMipmap = SDL_GL_GetProcAddress("glGenerateMipmap");
   else if (gl_hasExt("GL_EXT_framebuffer_object"))
      nglGenerateMipmap = SDL_GL_GetProcAddress("glGenerateMipmapEXT");
   else if (gl_hasExt("GL_SGIS_generate_mipmap"))
      nglGenerateMipmap = glGenerateMipmapNaev;
   else {
      nglGenerateMipmap = NULL;
      WARN("glGenerateMipmap not found.");
      return -1;
   }

   return 0;
}


/**
 * @brief Tries to initialize the texture compression.
 */
static int gl_extCompression (void)
{
   int i, found;
   GLint num, *ext;

   if (!conf.compress) {
      nglCompressedTexImage2D = NULL;
      return 0;
   }

   /* Find the extension. */
   if (gl_hasVersion( 1, 3 ))
      nglCompressedTexImage2D = gl_extGetProc("glCompressedTexImage2D");
   else if (gl_hasExt("GL_ARB_texture_compression"))
      nglCompressedTexImage2D = gl_extGetProc("glCompressedTexImage2DARB");
   else {
      nglCompressedTexImage2D = NULL;
      WARN("GL_ARB_texture_compression not found.");
      return -1;
   }

   /* See what is supported. */
   found = 0;
   glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &num);
   if (num > 0) {
      ext = malloc( sizeof(GLint) * num );
      glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, ext);
      for (i=0; i<num; i++)
         if (ext[i] == GL_COMPRESSED_RGBA) {
            found = 1;
            break;
         }

      free(ext);
   }

   /* Not supported. */
   if (found == 0) {
      nglCompressedTexImage2D = NULL;
      return -1;
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
   gl_extMipmaps();
   gl_extCompression();

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
