/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef OPENGL_VBO_H
#  define OPENGL_VBO_H


#include "opengl.h"


/*
 * Init/cleanup.
 */
int gl_initVBO (void);
void gl_exitVBO (void);


/*
 * Create.
 */
GLuint gl_vboCreate( GLenum target, GLsizei size, const void* data, GLenum usage );


/*
 * Destroy.
 */
void gl_vboDestroy( GLuint vbo );


#endif /* OPENGL_VBO_H */
   
