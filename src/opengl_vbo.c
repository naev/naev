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
 *    @param size Size of the buffer (multiply by sizeof(type)).
 *    @param data The actual datat to use.
 *    @param usage Usage to use.
 *    @return ID of the vbo.
 */
GLuint gl_vboCreate( GLenum target, GLsizei size, const void* data, GLenum usage )
{
   GLuint vbo;

   /* Create the buffer. */
   nglGenBuffers( 1, &vbo );

   /* Upload the data. */
   nglBindBuffer( target, vbo );
   nglBufferData( target, size, data, usage );

   /* Check for errors. */
   gl_checkErr();

   return vbo;
}


/**
 * @brief Destroys a VBO.
 *
 *    @param vbo VBO to destroy.
 */
void gl_vboDestroy( GLuint vbo )
{
   /* Destroy VBO. */
   nglDeleteBuffers( 1, &vbo );

   /* Check for errors. */
   gl_checkErr();
}

