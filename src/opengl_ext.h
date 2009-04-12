/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef OPENGL_EXT_H
#  define OPENGL_EXT_H


#include "SDL_opengl.h"


/* GL_ARB_multitexture */
void (APIENTRY *nglActiveTexture)(GLenum texture);
void (APIENTRY *nglMultiTexCoord2d)(GLenum target,GLdouble s,GLdouble t);

/* GL_ARB_vertex_buffer_object */
void (APIENTRY *nglGenBuffers)(GLsizei n, GLuint* ids);
void (APIENTRY *nglBindBuffer)(GLenum target, GLuint id);
void (APIENTRY *nglBufferData)(GLenum target, GLsizei size, const void* data, GLenum usage);
void (APIENTRY *nglBufferSubData)(GLenum target, GLint offset, GLsizei size, void* data);
void* (APIENTRY *nglMapBuffer)(GLenum target, GLenum access);
void (APIENTRY *nglUnmapBuffer)(GLenum target);
void (APIENTRY *nglDeleteBuffers)(GLsizei n, const GLuint* ids);


/*
 * Initializes the extensions.
 */
int gl_initExtensions (void);
void gl_exitExtensions (void);


#endif /* OPENGL_EXT_H */
   
