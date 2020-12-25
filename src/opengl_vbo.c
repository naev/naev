/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file opengl_vbo.c
 *
 * @brief Handles OpenGL vbos.
 */


/** @cond */
#include "naev.h"
/** @endcond */

#include "log.h"
#include "opengl.h"


#define BUFFER_OFFSET(i) ((char *)(sizeof(char) * (i))) /**< Taken from OpengL spec. */


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
   return 0;
}


/**
 * @brief Exits the OpenGL VBO subsystem.
 */
void gl_exitVBO (void)
{
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

   /* Create the buffer. */
   glGenBuffers( 1, &vbo->id );

   /* Upload the data. */
   glBindBuffer( target, vbo->id );
   glBufferData( target, size, data, usage );
   glBindBuffer( target, 0 );

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
   glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
   glBufferData( GL_ARRAY_BUFFER, size, data, usage );

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
   glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
   glBufferSubData( GL_ARRAY_BUFFER, offset, size, data );

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
   glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
   return glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
}


/**
 * @brief Unmaps a buffer.
 *
 *    @param vbo VBO to unmap.
 */
void gl_vboUnmap( gl_vbo *vbo )
{
   (void) vbo;
   glUnmapBuffer( GL_ARRAY_BUFFER );

   /* Check for errors. */
   gl_checkErr();
}


/**
 * @brief Activates a VBO's offset.
 *
 *    @param vbo VBO to activate.
 *    @param index Index of generic vertex attribute.
 *    @param offset Offset (in bytes).
 *    @param size Specifies components per point.
 *    @param type Type of data (usually GL_FLOAT).
 *    @param stride Offset between consecutive points.
 */
void gl_vboActivateAttribOffset( gl_vbo *vbo, GLuint index, GLuint offset,
      GLint size, GLenum type, GLsizei stride )
{
   const GLvoid *pointer;

   /* Set up. */
   glBindBuffer( GL_ARRAY_BUFFER, vbo->id );
   pointer = BUFFER_OFFSET(offset);

   glVertexAttribPointer( index, size, type, GL_FALSE, stride, pointer );

   /* Check for errors. */
   gl_checkErr();
}


/**
 * @brief Destroys a VBO.
 *
 *    @param vbo VBO to destroy. (If NULL, function does nothing.)
 */
void gl_vboDestroy( gl_vbo *vbo )
{
   if (vbo == NULL)
      return;

   glDeleteBuffers( 1, &vbo->id );
   gl_checkErr();
   free(vbo);
}
