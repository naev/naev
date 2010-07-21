/*
 * See Licensing and Copyright notice in naev.h
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
void gl_matrixTranslate( double x, double y );
void gl_matrixScale( double x, double y );
void gl_matrixRotate( double a );


#endif /* OPENGL_MATRIX_H */

