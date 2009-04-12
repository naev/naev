/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file opengl_vbo.c
 *
 * @brief Handles OpenGL vbos.
 */


#include "opengl.h"

#include "naev.h"

#include "log.h"


static int has_vbo = 0; /**< Whether or not has VBO. */


/**
 * Prototypes.
 */
static gl_vbo* gl_vboCreate( GLenum target, GLsizei size, void* data, GLenum usage );


/**
 * @brief Initializes the OpenGL VBO subsystem.
 *
 *    @return 0 on success.
 */
int gl_initVBO (void)
{
   if (nglGenBuffers != NULL)
      has_vbo = 1;

   return 0;
}


/**
 * @brief Exits the OpenGL VBO subsystem.
 */
void gl_exitVBO (void)
{
   has_vbo = 0;
}


/**
 * @brief Creates a VBO.
 *
 *    @param target Target to create to (usually GL_ARRAY_BUFFER).
 *    @param size Size of the buffer (multiply by sizeof(type)).
 *    @param data The actual datat to use.
 *    @param usage Usage to use.
 *    @return ID of the vbo.
 */
static gl_vbo* gl_vboCreate( GLenum target, GLsizei size, void* data, GLenum usage )
{
   gl_vbo *vbo;

   /* Allocate. */
   vbo = malloc( sizeof(gl_vbo) );
   memset( vbo, 0, sizeof(gl_vbo) );

   /* General stuff. */
   vbo->size = size;

   if (has_vbo) {
      /* Create the buffer. */
      nglGenBuffers( 1, &vbo->id );

      /* Upload the data. */
      nglBindBuffer( target, vbo->id );
      vbo->data = data;
      nglBufferData( target, size, data, usage );
      nglBindBuffer( target, 0 );
   }
   else {
      vbo->size = size;
      vbo->data = malloc(size);
      memcpy( vbo->data, data, size );
   }

   /* Check for errors. */
   gl_checkErr();

   return vbo;
}


/**
 * @brief Creates a stream vbo.
 *
 *    @param size Size of the stream vbo (multiply by sizeof(type)).
 *    @param data Data for the VBO.
 */
gl_vbo* gl_vboCreateStream( GLsizei size, void* data )
{
   gl_vbo *vbo;
   
   vbo = gl_vboCreate( GL_ARRAY_BUFFER, size, data, GL_STREAM_DRAW );
   vbo->type = NGL_VBO_STREAM;

   return vbo;
}


/**
 * @brief Creates a stream vbo.
 *
 *    @param size Size of the stream vbo (multiply by sizeof(type)).
 *    @param data Data for the VBO.
 */
gl_vbo* gl_vboCreateStatic( GLsizei size, void* data )
{
   gl_vbo *vbo;
   
   vbo = gl_vboCreate( GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW );
   vbo->type = NGL_VBO_STATIC;

   return vbo;
}


/**
 * @brief Maps a buffer.
 *
 *    @param vbo VBO to map.
 *    @return The data contained in the vbo.
 */
void* gl_vboMap( gl_vbo *vbo )
{
   if (has_vbo) {
      nglBindBuffer( GL_ARRAY_BUFFER, vbo->id );
      return nglMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
   }
   else
      return vbo->data;
}


/**
 * @brief Unmaps a buffer.
 *
 *    @param vbo VBO to unmap.
 */
void gl_vboUnmap( gl_vbo *vbo )
{
   (void) vbo;
   if (has_vbo)
      nglUnmapBuffer( GL_ARRAY_BUFFER );
}


/**
 * @brief Activates a VBO.
 *
 *    @param vbo VBO to activate.
 *    @param Should be one of GL_COLOR_ARRAY, GL_VERTEX_ARRAY, or GL_TEXTURE_COORD_ARRAY.
 *    @param size Specifies components per point.
 *    @param type Type of data (usually GL_FLOAT).
 *    @param stride Offset between consecutive points.
 */
void gl_vboActivate( gl_vbo *vbo, GLuint class, GLint size, GLenum type, GLsizei stride )
{
   const GLvoid *pointer;

   /* Set up. */
   glEnableClientState(class);
   if (has_vbo) {
      nglBindBuffer( GL_ARRAY_BUFFER, vbo->id );
      pointer = 0;
   }
   else
      pointer = vbo->data;

   /* Class specific. */
   switch (class) {
      case GL_COLOR_ARRAY:
         glColorPointer( size, type, stride, pointer );
         break;

      case GL_VERTEX_ARRAY:
         glVertexPointer( size, type, stride, pointer );
         break;

      case GL_TEXTURE_COORD_ARRAY:
         glVertexPointer( size, type, stride, pointer );
         break;

      default:
         WARN("Unknown VBO class.");
         break;
   }
}


/**
 * @brief Deactivates the vbo stuff.
 */
void gl_vboDeactivate (void)
{
   if (has_vbo)
      nglBindBuffer(GL_ARRAY_BUFFER, 0);
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


/**
 * @brief Destroys a VBO.
 *
 *    @param vbo VBO to destroy.
 */
void gl_vboDestroy( gl_vbo *vbo )
{
   if (has_vbo) {
      /* Destroy VBO. */
      nglDeleteBuffers( 1, &vbo->id );
   }

   /* Check for errors. */
   gl_checkErr();

   /* Free memory. */
   free(vbo);
}

