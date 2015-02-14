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
void gl_vboDeactivate (void);


/*
 * Destroy.
 */
void gl_vboDestroy( gl_vbo* vbo );


/*
 * Info.
 */
int gl_vboIsHW (void);


#endif /* OPENGL_VBO_H */

