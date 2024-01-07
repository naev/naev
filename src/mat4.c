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

#include <stdio.h>

#include "mat4.h"

void mat4_print( const mat4 *m )
{
   for (int i=0; i<4; i++) {
      for (int j=0; j<4; j++)
         printf("%6.1f ", m->m[j][i]);
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
            v += m1->m[i][k] * m2->m[k][j];
         out->m[i][j] = v;
      }
   }
}

/**
 * @brief Applies a transformation to another, storing the result in the left hand side.
 *
 *    @param[in, out] lhs Left hand side matrix.
 *    @param[in] rhs Right hand side matrix.
 */
void mat4_apply( mat4 *lhs, const mat4 *rhs )
{
   /* Process by rows. */
   for (int i=0; i<4; i++) {
      float l0 = lhs->m[i][0];
      float l1 = lhs->m[i][1];
      float l2 = lhs->m[i][2];

      float r0 = l0 * rhs->m[0][0] + l1 * rhs->m[1][0] + l2 * rhs->m[2][0];
      float r1 = l0 * rhs->m[0][1] + l1 * rhs->m[1][1] + l2 * rhs->m[2][1];
      float r2 = l0 * rhs->m[0][2] + l1 * rhs->m[1][2] + l2 * rhs->m[2][2];

      lhs->m[i][0] = r0;
      lhs->m[i][1] = r1;
      lhs->m[i][2] = r2;
   }
   lhs->m[3][0] += rhs->m[3][0];
   lhs->m[3][1] += rhs->m[3][1];
   lhs->m[3][2] += rhs->m[3][2];
}

/**
 * @brief Scales a homogeneous transformation matrix.
 *
 *    @param[in, out] m Matrix to apply scaling to.
 *    @param x Scaling on X axis.
 *    @param y Scaling on Y axis.
 *    @param z Scaling on Z axis.
 */
void mat4_scale( mat4 *m, double x, double y, double z )
{
   for (int i=0; i<4; i++) {
      m->m[0][i] *= x;
      m->m[1][i] *= y;
      m->m[2][i] *= z;
   }
}
void mat4_scale_xy( mat4 *m, double x, double y )
{
   for (int i=0; i<4; i++) {
      m->m[0][i] *= x;
      m->m[1][i] *= y;
   }
}

/**
 * @brief Translates a homogenous transformation matrix.
 *
 *    @param[in, out] m Matrix to apply scaling to.
 *    @param x Translation on X axis.
 *    @param y Translation on Y axis.
 *    @param z Translation on Z axis.
 */
void mat4_translate( mat4 *m, double x, double y, double z )
{
   for (int i=0; i<4; i++)
      m->m[3][i] += m->m[0][i] * x + m->m[1][i] * y + m->m[2][i] * z;
}
void mat4_translate_x( mat4 *m, double x )
{
   for (int i=0; i<4; i++)
      m->m[3][i] += m->m[0][i] * x;
}
void mat4_translate_xy( mat4 *m, double x, double y )
{
   for (int i=0; i<4; i++)
      m->m[3][i] += m->m[0][i] * x + m->m[1][i] * y;
}

/**
 * @brief Rotates an angle, in radians, around the z axis.
 *
 *    @param[in, out] m Matrix to multiply with.
 *    @param angle Angle in radians.
 */
void mat4_rotate2d( mat4 *m, double angle )
{
   double c, s, x, y;

   c = cos(angle);
   s = sin(angle);
   x = m->m[0][0];
   y = m->m[1][0];
   m->m[0][0] =  c*x + s*y;
   m->m[1][0] = -s*x + c*y;

   x = m->m[0][1];
   y = m->m[1][1];
   m->m[0][1] =  c*x + s*y;
   m->m[1][1] = -s*x + c*y;
}

/**
 * @brief Rotates the +x axis to the given vector.
 *
 *    @param[in, out] m Matrix to multiply with.
 *    @param c Angle cosine (or x coordinate of the vector).
 *    @param s Angle sine (or y coordinate of the vector).
 */
void mat4_rotate2dv( mat4 *m, double c, double s )
{
   double x, y;

   x = m->m[0][0];
   y = m->m[1][0];
   m->m[0][0] =  c*x + s*y;
   m->m[1][0] = -s*x + c*y;

   x = m->m[0][1];
   y = m->m[1][1];
   m->m[0][1] =  c*x + s*y;
   m->m[1][1] = -s*x + c*y;
}

/**
 * @brief Multiplies the given matrix by a rotation. (Follows the right-hand rule.)
 *
 *    @param[in, out] m Matrix to multiply with.
 *    @param angle Angle in radians.
 *    @param x X component of the axis of rotation.
 *    @param y Y component of the axis of rotation.
 *    @param z Z component of the axis of rotation.
 */
void mat4_rotate( mat4 *m, double angle, double x, double y, double z )
{
   double norm, c, s;
   mat4 R;

   norm = sqrt( pow2(x) + pow2(y) + pow2(z) );
   c = cos(angle);
   s = sin(angle);
   x /= norm;
   y /= norm;
   z /= norm;
   R.m[0][0] = x*x*(1.-c) + c;
   R.m[0][1] = y*x*(1.-c) + z*s;
   R.m[0][2] = x*z*(1.-c) - y*s;
   R.m[0][3] = 0.;
   R.m[1][0] = x*y*(1.-c) - z*s;
   R.m[1][1] = y*y*(1.-c) + c;
   R.m[1][2] = y*z*(1.-c) + x*s;
   R.m[1][3] = 0.;
   R.m[2][0] = x*z*(1.-c) + y*s;
   R.m[2][1] = y*z*(1.-c) - x*s;
   R.m[2][2] = z*z*(1.-c) + c;
   R.m[2][3] = 0.;
   R.m[3][0] = 0.;
   R.m[3][1] = 0.;
   R.m[3][2] = 0.;
   R.m[3][3] = 1.;

   mat4_apply( m, &R );
}

/**
 * @brief Creates an identity matrix.
 *
 *    @return A new identity matrix.
 */
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

/**
 * @brief Creates an orthographic projection matrix.
 */
mat4 mat4_ortho( double left, double right,
      double bottom, double top, double nearVal, double farVal )
{
   mat4 mat = {{{{0}}}};
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

/**
 * @brief Creates a matrix with a transformation to look at a center point from an eye with an up vector.
 *
 *    @param[in] eye Vector representing the eye position that is looking at something.
 *    @param[in] center Vector representing the position that is being looked at.
 *    @param[in] up Vector representing the "upward" direction. Has to be a unitary vector.
 *    @return The newly created matrix.
 */
mat4 mat4_lookat( const vec3 *eye, const vec3 *center, const vec3 *up )
{
   vec3 forward, side, upc;
   mat4 H;

   vec3_sub( &forward, center, eye );
   vec3_normalize( &forward ); /* Points towards the center from the eye. */

   /* side = forward x up */
   vec3_cross( &side, &forward, up );
   vec3_normalize( &side ); /* Points to the side. */

   /* upc = side x forward */
   vec3_cross( &upc, &side, &forward );
   /* No need to normalize since forward and side and unitary. */

   /* First column. */
   H.m[0][0] = side.v[0];
   H.m[0][1] = side.v[1];
   H.m[0][2] = side.v[2];
   H.m[0][3] = 0.;
   /* Second column. */
   H.m[1][0] = upc.v[0];
   H.m[1][1] = upc.v[1];
   H.m[1][2] = upc.v[2];
   H.m[1][3] = 0.;
   /* Third column. */
   H.m[2][0] = -forward.v[0];
   H.m[2][1] = -forward.v[1];
   H.m[2][2] = -forward.v[2];
   H.m[2][3] = 0.;
   /* Fourth column. */
   H.m[3][0] = -eye->v[0];
   H.m[3][1] = -eye->v[1];
   H.m[3][2] = -eye->v[2];
   H.m[3][3] = 1.;

   return H;
}

/**
 * @brief Creates a matrix with a perspective transformation.
 *
 *    @param fov Field of view.
 *    @param aspect Aspect ratio.
 *    @param near Near plane.
 *    @param far Far plane.
 *    @return The newly created matrix.
 */
mat4 mat4_perspective( double fov, double aspect, double near, double far )
{
   mat4 H;
   double c = 1. / tan( fov * 0.5 );
   double d = far - near;

   /* First column. */
   H.m[0][0] = c / aspect;
   H.m[0][1] = 0.;
   H.m[0][2] = 0.;
   H.m[0][3] = 0.;
   /* Second column. */
   H.m[1][0] = 0.;
   H.m[1][1] = c;
   H.m[1][2] = 0.;
   H.m[1][3] = 0.;
   /* Third column. */
   H.m[2][0] = 0.;
   H.m[2][1] = 0.;
   H.m[2][2] = -(far+near) / d;
   H.m[2][3] = -1.;
   /* Fourth column. */
   H.m[3][0] = 0.;
   H.m[3][1] = 0.;
   H.m[3][2] = -2.*far*near / d;
   H.m[3][3] = 0.;

   return H;
}
