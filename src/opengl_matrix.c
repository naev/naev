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

void gl_Matrix4_Print( gl_Matrix4 m )
{
   for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
         printf("%6.1f ", m.m[j][i]);
      }
      printf("\n");
   }
}

gl_Matrix4 gl_Matrix4_Mult( gl_Matrix4 m1, gl_Matrix4 m2 )
{
   gl_Matrix4 m = {{{0}}};

   for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
         for (int k = 0; k < 4; k++) {
            m.m[j][i] += m1.m[k][i] * m2.m[j][k];
         }
      }
   }

   return m;
}

gl_Matrix4 gl_Matrix4_Identity( void )
{
   gl_Matrix4 m = {{{0}}};
   m.m[0][0] = 1.;
   m.m[1][1] = 1.;
   m.m[2][2] = 1.;
   m.m[3][3] = 1.;
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

   mat.m[0][0] = 2. / (right - left);
   mat.m[1][1] = 2. / (top - bottom);
   mat.m[2][2] = -2. / (farVal - nearVal);
   mat.m[3][3] = 1.;
   mat.m[3][0] = tx;
   mat.m[3][1] = ty;
   mat.m[3][2] = tz;

   return mat;
}

gl_Matrix4 gl_Matrix4_Scale( gl_Matrix4 m, double x, double y, double z )
{
   for (int i = 0; i < 4; i++) {
      m.m[0][i] *= x;
      m.m[1][i] *= y;
      m.m[2][i] *= z;
   }
   return m;
}

gl_Matrix4 gl_Matrix4_Translate( gl_Matrix4 m, double x, double y, double z )
{
   for (int i = 0; i < 4; i++)
      m.m[3][i] += m.m[0][i] * x + m.m[1][i] * y + m.m[2][i] * z;
   return m;
}

/**
 * @brief Rotates an angle, in radians, around the z axis.
 *
 *    @param m Matrix to multiply with.
 *    @param angle Angle in radians.
 *    @return New projection matrix.
 */
gl_Matrix4 gl_Matrix4_Rotate2d( gl_Matrix4 m, double angle )
{
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
gl_Matrix4 gl_Matrix4_Rotate2dv( gl_Matrix4 m, double c, double s )
{
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

/**
 * @brief Multiplies the given matrix by a rotation. (Follows the right-hand rule.)
 *
 *    @param m Matrix to multiply with.
 *    @param angle Angle in radians.
 *    @param x X component of the axis of rotation.
 *    @param y Y component of the axis of rotation.
 *    @param z Z component of the axis of rotation.
 *    @return New projection matrix.
 */
gl_Matrix4 gl_Matrix4_Rotate( gl_Matrix4 m, double angle, double x, double y, double z )
{
   double norm, c, s;
   gl_Matrix4 rot;

   norm = sqrt( pow2(x) + pow2(y) + pow2(z) );
   c = cos(angle);
   s = sin(angle);
   x /= norm;
   y /= norm;
   z /= norm;
   rot.m[0][0] = x*x*(1.-c) + c;
   rot.m[0][1] = y*x*(1.-c) + z*s;
   rot.m[0][2] = x*z*(1.-c) - y*s;
   rot.m[0][3] = 0.;
   rot.m[1][0] = x*y*(1.-c) - z*s;
   rot.m[1][1] = y*y*(1.-c) + c;
   rot.m[1][2] = y*z*(1.-c) + x*s;
   rot.m[1][3] = 0.;
   rot.m[2][0] = x*z*(1.-c) + y*s;
   rot.m[2][1] = y*z*(1.-c) - x*s;
   rot.m[2][2] = z*z*(1.-c) + c;
   rot.m[2][3] = 0.;
   rot.m[3][0] = 0.;
   rot.m[3][1] = 0.;
   rot.m[3][2] = 0.;
   rot.m[3][3] = 1.;

   return gl_Matrix4_Mult( m, rot );
}

GLfloat *gl_Matrix4_Ptr( gl_Matrix4 *m )
{
   return (GLfloat*)m->m;
}

void gl_Matrix4_Uniform( GLint location, gl_Matrix4 m )
{
   glUniformMatrix4fv(location, 1, GL_FALSE, gl_Matrix4_Ptr(&m));
}
