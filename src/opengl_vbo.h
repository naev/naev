/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef OPENGL_VBO_H
#  define OPENGL_VBO_H


#include "opengl.h"


/**
 * @brief VBO types.
 */
typedef enum gl_vboType_e {
   NGL_VBO_NULL,
   NGL_VBO_STREAM,
   NGL_VBO_STATIC
} gl_vboType;


/**
 * @brief Contains the VBO.
 */
typedef struct gl_vbo_s {
   GLuint id;
   gl_vboType type;
   GLsizei size;
   void* data;
} gl_vbo;


/*
 * Init/cleanup.
 */
int gl_initVBO (void);
void gl_exitVBO (void);


/*
 * Create.
 */
gl_vbo* gl_vboCreateStream( GLsizei size, void* data );
gl_vbo* gl_vboCreateStatic( GLsizei size, void* data );


/*
 * Modify.
 */
void* gl_vboMap( gl_vbo *vbo );
void gl_vboUnmap( gl_vbo *vbo );
void gl_vboActivate( gl_vbo *vbo, GLuint class, GLint size, GLenum type, GLsizei stride );
void gl_vboDeactivate (void);


/*
 * Destroy.
 */
void gl_vboDestroy( gl_vbo* vbo );


#endif /* OPENGL_VBO_H */
   
