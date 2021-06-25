/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file opengl_matrix.c
 *
 * @brief Handles OpenGL matrix stuff.
 */


/** @cond */
#include "naev.h"
/** @endcond */

#include "log.h"
#include "opengl.h"


/**
 * @brief Initializes the OpenGL matrix subsystem.
 *
 *    @return 0 on success.
 */
int gl_initMatrix (void)
{
   return 0;
}


/**
 * @brief Exits the OpenGL matrix subsystem.
 */
void gl_exitMatrix (void)
{
}


void gl_Matrix4_Print( gl_Matrix4 m ) {
   int i, j;

   for (i = 0; i < 4; i++) {
      for (j = 0; j < 4; j++) {
         printf("%6.1f ", m.m[j][i]);
      }
      printf("\n");
   }
}

gl_Matrix4 gl_Matrix4_Mult( gl_Matrix4 m1, gl_Matrix4 m2 ) {
   int i, j, k;
   gl_Matrix4 m = {{{0}}};

   for (i = 0; i < 4; i++) {
      for (j = 0; j < 4; j++) {
         for (k = 0; k < 4; k++) {
            m.m[j][i] += m1.m[k][i] * m2.m[j][k];
         }
      }
   }

   return m;
}

gl_Matrix4 gl_Matrix4_Identity( void ) {
   gl_Matrix4 m = {{{0}}};
   m.m[0][0] = 1;
   m.m[1][1] = 1;
   m.m[2][2] = 1;
   m.m[3][3] = 1;
   return m;
}

gl_Matrix4 gl_Matrix4_Ortho( double left, double right,
      double bottom, double top, double nearVal, double farVal )
{
   gl_Matrix4 mat = {{{0}}};
   double tx, ty, tz;

   /* https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml */
   tx = -(right + left) / (right - left);
   ty = -(top + bottom) / (top - bottom);
   tz = -(farVal + nearVal) / (farVal - nearVal);

   mat.m[0][0] = 2 / (right - left);
   mat.m[1][1] = 2 / (top - bottom);
   mat.m[2][2] = -2 / (farVal - nearVal);
   mat.m[3][3] = 1;
   mat.m[3][0] = tx;
   mat.m[3][1] = ty;
   mat.m[3][2] = tz;

   return mat;
}

gl_Matrix4 gl_Matrix4_Scale( gl_Matrix4 m, double x, double y, double z ) {
   int i;

   for (i = 0; i < 4; i++) {
      m.m[0][i] *= x;
      m.m[1][i] *= y;
      m.m[2][i] *= z;
   }

   return m;
}

gl_Matrix4 gl_Matrix4_Translate( gl_Matrix4 m, double x, double y, double z ) {
   int i;

   for (i = 0; i < 4; i++) {
      m.m[3][i] += m.m[0][i] * x + m.m[1][i] * y + m.m[2][i] * z;
   }

   return m;
}

/**
 * @brief Rotates an angle, in radians, around the z axis.
 *
 *    @param m Matrix to multiply with.
 *    @param angle Angle in radians.
 *    @return New projection matrix.
 */
gl_Matrix4 gl_Matrix4_Rotate2d( gl_Matrix4 m, double angle ) {
   double c, s, x, y;

   c = cos(angle);
   s = sin(angle);
   x = m.m[0][0];
   y = m.m[1][0];
   m.m[0][0] =  c*x + s*y;
   m.m[1][0] = -s*x + c*y;

   x = m.m[0][1];
   y = m.m[1][1];
   m.m[0][1] =  c*x + s*y;
   m.m[1][1] = -s*x + c*y;

   return m;
}

/**
 * @brief Rotates the +x axis to the given vector.
 *
 *    @param m Matrix to multiply with.
 *    @param c Angle cosine (or x coordinate of the vector).
 *    @param s Angle sine (or y coordinate of the vector).
 *    @return New projection matrix.
 */
gl_Matrix4 gl_Matrix4_Rotate2dv( gl_Matrix4 m, double c, double s ) {
   double x, y;

   x = m.m[0][0];
   y = m.m[1][0];
   m.m[0][0] =  c*x + s*y;
   m.m[1][0] = -s*x + c*y;

   x = m.m[0][1];
   y = m.m[1][1];
   m.m[0][1] =  c*x + s*y;
   m.m[1][1] = -s*x + c*y;

   return m;
}

GLfloat *gl_Matrix4_Ptr( gl_Matrix4 *m ) {
   return (GLfloat*)m->m;
}

void gl_Matrix4_Uniform( GLint location, gl_Matrix4 m ) {
   glUniformMatrix4fv(location, 1, GL_FALSE, gl_Matrix4_Ptr(&m));
}
