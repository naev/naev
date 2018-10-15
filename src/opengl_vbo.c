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


#define BUFFER_OFFSET(i) ((char *)NULL + (i)) /**< Taken from OpengL spec. */


/**
 * @brief VBO types.
 */
typedef enum gl_vboType_e {
   NGL_VBO_NULL, /**< No VBO type. */
   NGL_VBO_STREAM, /**< VBO streaming type. */
   NGL_VBO_DYNAMIC, /**< VBO dynamic type. */
   NGL_VBO_STATIC /**< VBO static type. */
} gl_vboType;


/**
 * @brief Contains the VBO.
 */
struct gl_vbo_s {
   GLuint id; /**< VBO ID. */
   gl_vboType type; /**< VBO type. */
   GLsizei size; /**< VBO size. */
   char* data; /**< VBO data. */
};


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
 *    @param size Size of the buffer (in bytes).
 *    @param data The actual datat to use.
 *    @param usage Usage to use.
 *    @return ID of the vbo.
 */
static gl_vbo* gl_vboCreate( GLenum target, GLsizei size, void* data, GLenum usage )
{
   gl_vbo *vbo;

   /* Allocate. */
   vbo = calloc( 1, sizeof(gl_vbo) );

   /* General stuff. */
   vbo->size = size;

   if (has_vbo) {
      /* Create the buffer. */
      nglGenBuffers( 1, &vbo->id );

      /* Upload the data. */
      nglBindBuffer( target, vbo->id );
      nglBufferData( target, size, data, usage );
      nglBindBuffer( target, 0 );
   }
   else {
      vbo->size = size;
      vbo->data = malloc(size);
      if (data == NULL)
         memset( vbo->data, 0, size );
      else
         memcpy( vbo->data, data, size );
   }

   /* Check for errors. */
   gl_checkErr();

   return vbo;
}


/**
 * @brief Reloads new data or grows the size of the vbo.
 *
 *    @param vbo VBO to set new data of.
 *    @param size Size of new data.
 *    @param data New data.
 */
void gl_vboData( gl_vbo *vbo, GLsizei size, void* data )
{
   GLenum usage;

   vbo->size = size;

   if (has_vbo) {
      /* Get usage. */
      if (vbo->type == NGL_VBO_STREAM)
         usage = GL_STREAM_DRAW;
      else if (vbo->type == NGL_VBO_DYNAMIC)
         usage = GL_DYNAMIC_DRAW;
      else if (vbo->type == NGL_VBO_STATIC)
         usage = GL_STATIC_DRAW;
      else
         usage = GL_STREAM_DRAW;

      /* Get new data. */
      nglBindBuffer( GL_ARRAY_BUFFER, vbo->id );
      nglBufferData( GL_ARRAY_BUFFER, size, data, usage );
   }
   else {
      /* Grow memory. */
      vbo->data = realloc( vbo->data, size );
      if (data == NULL)
         memset( vbo->data, 0, size );
      else
         memcpy( vbo->data, data, size );
   }

   /* Check for errors. */
   gl_checkErr();
}


/**
 * @brief Loads some data into the VBO.
 *
 *    @param vbo VBO to load data into.
 *    @param offset Offset location of the data (in bytes).
 *    @param size Size of the data (in bytes).
 *    @param data Pointer to the data.
 */
void gl_vboSubData( gl_vbo *vbo, GLint offset, GLsizei size, void* data )
{
   if (has_vbo) {
      nglBindBuffer( GL_ARRAY_BUFFER, vbo->id );
      nglBufferSubData( GL_ARRAY_BUFFER, offset, size, data );
   }
   else
      memcpy( &vbo->data[offset], data, size );

   /* Check for errors. */
   gl_checkErr();
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

   /* Check for errors. */
   gl_checkErr();

   return vbo;
}

/**
 * @brief Creates a dynamic vbo.
 *
 *    @param size Size of the dynamic vbo (multiply by sizeof(type)).
 *    @param data Data for the VBO.
 */
gl_vbo* gl_vboCreateDynamic( GLsizei size, void* data )
{
   gl_vbo *vbo;

   vbo = gl_vboCreate( GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW );
   vbo->type = NGL_VBO_DYNAMIC;

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
gl_vbo* gl_vboCreateStatic( GLsizei size, void* data )
{
   gl_vbo *vbo;

   vbo = gl_vboCreate( GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW );
   vbo->type = NGL_VBO_STATIC;

   /* Check for errors. */
   gl_checkErr();

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

   /* Check for errors. */
   gl_checkErr();
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
   gl_vboActivateOffset( vbo, class, 0, size, type, stride );
}


/**
 * @brief Activates a VBO's offset.
 *
 *    @param vbo VBO to activate.
 *    @param class Should be one of GL_COLOR_ARRAY, GL_VERTEX_ARRAY,
 *           GL_TEXTURE_COORD_ARRAY, GL_TEXTURE0 or GL_TEXTURE1.
 *    @param offset Offset (in bytes).
 *    @param size Specifies components per point.
 *    @param type Type of data (usually GL_FLOAT).
 *    @param stride Offset between consecutive points.
 */
void gl_vboActivateOffset( gl_vbo *vbo, GLuint class, GLuint offset,
      GLint size, GLenum type, GLsizei stride )
{
   const GLvoid *pointer;

   /* Set up. */
   if (has_vbo) {
      nglBindBuffer( GL_ARRAY_BUFFER, vbo->id );
      pointer = BUFFER_OFFSET(offset);
   }
   else
      pointer = &vbo->data[offset];

   /* Class specific. */
   switch (class) {
      case GL_COLOR_ARRAY:
         glEnableClientState(class);
         glColorPointer( size, type, stride, pointer );
         break;

      case GL_VERTEX_ARRAY:
         glEnableClientState(class);
         glVertexPointer( size, type, stride, pointer );
         break;

      case GL_TEXTURE_COORD_ARRAY:
         glEnableClientState(class);
         glTexCoordPointer( size, type, stride, pointer );
         break;

      case GL_TEXTURE0:
         nglClientActiveTexture( GL_TEXTURE0 );
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glTexCoordPointer( size, type, stride, pointer );
         break;

      case GL_TEXTURE1:
         nglClientActiveTexture( GL_TEXTURE1 );
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glTexCoordPointer( size, type, stride, pointer );
         break;

      default:
         WARN(_("Unknown VBO class."));
         break;
   }

   /* Check for errors. */
   gl_checkErr();
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
   if (nglClientActiveTexture != NULL) {
      nglClientActiveTexture( GL_TEXTURE1 );
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      nglClientActiveTexture( GL_TEXTURE0 );
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }

   /* Check for errors. */
   gl_checkErr();
}


/**
 * @brief Destroys a VBO.
 *
 *    @param vbo VBO to destroy.
 */
void gl_vboDestroy( gl_vbo *vbo )
{
   if (has_vbo)
      /* Destroy VBO. */
      nglDeleteBuffers( 1, &vbo->id );
   else
      free(vbo->data);

   /* Check for errors. */
   gl_checkErr();

   /* Free memory. */
   free(vbo);
}


/**
 * @brief Checks to see if the VBOs are supported by HW.
 *
 *    @return 1 if VBOs are enabled and supported by HW.
 */
int gl_vboIsHW (void)
{
   return has_vbo;
}

