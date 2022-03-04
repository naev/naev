/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file mat4.c
 *
 * @brief Handles OpenGL matrix stuff.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "log.h"
#include "opengl.h"

void mat4_print( mat4 m )
{
   for (int i=0; i<4; i++) {
      for (int j=0; j<4; j++) {
         printf("%6.1f ", m.m[j][i]);
      }
      printf("\n");
   }
}

/**
 * @brief Multiplies two matrices (out = m1 * m2).
 *
 * Note that out should not be neither m1 nor m2.
 *
 *    @param[out] out Output matrix.
 *    @param m1 First matrix to mulitply.
 *    @param m2 Second matrix to multiply.
 */
void mat4_mul( mat4 *out, const mat4 *m1, const mat4 *m2 )
{
   for (int i=0; i<4; i++) {
      for (int j=0; j<4; j++) {
         GLfloat v = 0.;
         for (int k=0; k<4; k++)
            v += m1->m[k][i] * m2->m[j][k];
         out->m[j][i] = v;
      }
   }
}

mat4 mat4_identity (void)
{
   const mat4 m = { .m = {
      { 1., 0., 0., 0. },
      { 0., 1., 0., 0. },
      { 0., 0., 1., 0. },
      { 0., 0., 0., 1. }
   } };
   return m;
}

mat4 mat4_ortho( double left, double right,
      double bottom, double top, double nearVal, double farVal )
{
   mat4 mat = {{{0}}};
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

mat4 mat4_scale( mat4 m, double x, double y, double z )
{
   for (int i = 0; i < 4; i++) {
      m.m[0][i] *= x;
      m.m[1][i] *= y;
      m.m[2][i] *= z;
   }
   return m;
}

mat4 mat4_translate( mat4 m, double x, double y, double z )
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
mat4 mat4_rotate2d( mat4 m, double angle )
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
mat4 mat4_rotate2dv( mat4 m, double c, double s )
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
mat4 mat4_rotate( mat4 m, double angle, double x, double y, double z )
{
   double norm, c, s;
   mat4 rot, out;

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

   mat4_mul( &out, &m, &rot );
   return out;
}

GLfloat *mat4_ptr( mat4 *m )
{
   return (GLfloat*)m->m;
}

void mat4_uniform( GLint location, mat4 m )
{
   glUniformMatrix4fv(location, 1, GL_FALSE, mat4_ptr(&m));
}
