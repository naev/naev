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


#ifndef OPENGL_MATRIX_H
#  define OPENGL_MATRIX_H


#include "opengl.h"


/*
 * Init/cleanup.
 */
int gl_initMatrix (void);
void gl_exitMatrix (void);


/*
 * Matrix mode.
 */
void gl_matrixMode( GLenum mode );


/*
 * Push/pop matrix.
 */
void gl_matrixPush (void);
void gl_matrixPop (void);


/*
 * Matrix manipulation.
 */
void gl_matrixIdentity (void);
void gl_matrixOrtho( double left, double right,
      double bottom, double top, double nearVal, double farVal );
void gl_matrixTranslate( double x, double y );
void gl_matrixScale( double x, double y );
void gl_matrixRotate( double a );


#endif /* OPENGL_MATRIX_H */

