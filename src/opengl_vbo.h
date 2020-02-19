/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef OPENGL_VBO_H
#  define OPENGL_VBO_H


#include "opengl.h"


struct gl_vbo_s;
typedef struct gl_vbo_s gl_vbo;


/*
 * Init/cleanup.
 */
int gl_initVBO (void);
void gl_exitVBO (void);


/*
 * Create.
 */
gl_vbo* gl_vboCreateStream( GLsizei size, void* data );
gl_vbo* gl_vboCreateDynamic( GLsizei size, void* data );
gl_vbo* gl_vboCreateStatic( GLsizei size, void* data );


/*
 * Modify.
 */
void gl_vboData( gl_vbo *vbo, GLsizei size, void* data );
void gl_vboSubData( gl_vbo *vbo, GLint offset, GLsizei size, void* data );
void* gl_vboMap( gl_vbo *vbo );
void gl_vboUnmap( gl_vbo *vbo );
void gl_vboActivate( gl_vbo *vbo, GLuint class, GLint size, GLenum type, GLsizei stride );
void gl_vboActivateOffset( gl_vbo *vbo, GLuint class, GLuint offset,
      GLint size, GLenum type, GLsizei stride );
void gl_vboActivateAttribOffset( gl_vbo *vbo, GLuint index, GLuint offset,
      GLint size, GLenum type, GLsizei stride );


/*
 * Destroy.
 */
void gl_vboDestroy( gl_vbo* vbo );


#endif /* OPENGL_VBO_H */

