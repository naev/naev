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


typedef struct gl_Matrix4_ {
   /* Column-major; m[x][y] */
   GLfloat m[4][4];
} gl_Matrix4;

void gl_Matrix4_Print( gl_Matrix4 m );
gl_Matrix4 gl_Matrix4_Mult( gl_Matrix4 m1, gl_Matrix4 m2 );
gl_Matrix4 gl_Matrix4_Identity( void );
gl_Matrix4 gl_Matrix4_Ortho( double left, double right,
      double bottom, double top, double nearVal, double farVal );
gl_Matrix4 gl_Matrix4_Scale( gl_Matrix4 m, double x, double y, double z );
gl_Matrix4 gl_Matrix4_Translate( gl_Matrix4 m, double x, double y, double z );
gl_Matrix4 gl_Matrix4_Rotate2d( gl_Matrix4 m, double angle );
GLfloat *gl_Matrix4_Ptr( gl_Matrix4 *m );
void gl_Matrix4_Uniform( GLint location, gl_Matrix4 m );

#endif /* OPENGL_MATRIX_H */

