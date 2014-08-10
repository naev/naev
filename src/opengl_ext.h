/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef OPENGL_EXT_H
#  define OPENGL_EXT_H


#include "SDL_opengl.h"

/* GL_SGIS_generate_mipmap */
void (APIENTRY *nglGenerateMipmap)(GLenum target);

/* GL_ARB_multitexture */
void (APIENTRY *nglActiveTexture)(GLenum texture);
void (APIENTRY *nglClientActiveTexture)(GLenum texture);
void (APIENTRY *nglMultiTexCoord2d)(GLenum target,GLdouble s,GLdouble t);

/* GL_ARB_vertex_buffer_object */
void (APIENTRY *nglGenBuffers)(GLsizei n, GLuint* ids);
void (APIENTRY *nglBindBuffer)(GLenum target, GLuint id);
void (APIENTRY *nglBufferData)(GLenum target, GLsizei size, const void* data, GLenum usage);
void (APIENTRY *nglBufferSubData)(GLenum target, GLint offset, GLsizei size, void* data);
void* (APIENTRY *nglMapBuffer)(GLenum target, GLenum access);
void (APIENTRY *nglUnmapBuffer)(GLenum target);
void (APIENTRY *nglDeleteBuffers)(GLsizei n, const GLuint* ids);

/* GL_ARB_texture_compression */
void (APIENTRY *nglCompressedTexImage2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);


/*
 * Initializes the extensions.
 */
int gl_initExtensions (void);
void gl_exitExtensions (void);


#endif /* OPENGL_EXT_H */

