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
#include "nstring.h"

void mat4_tostr( const mat4 *m, char *buf, size_t len )
{
   int l = 0;
   for ( int i = 0; i < 4; i++ ) {
      for ( int j = 0; j < 4; j++ )
         l += scnprintf( &buf[l], len - l, "%7.g ", m->m[j][i] );
      l += scnprintf( &buf[l], len - l, "\n" );
   }
}

void mat4_print( const mat4 *m )
{
   for ( int i = 0; i < 4; i++ ) {
      for ( int j = 0; j < 4; j++ )
         printf( "%7.g ", m->m[j][i] );
      printf( "\n" );
   }
}

/**
 * @brief Multiplies two matrices (out = m1 * m2).
 *
 * Note that out should not be neither m1 nor m2.
 *
 *    @param[out] out Output matrix.
 *    @param m1 First matrix to multiply.
 *    @param m2 Second matrix to multiply.
 */
void mat4_mul( mat4 *out, const mat4 *m1, const mat4 *m2 )
{
   for ( int i = 0; i < 4; i++ ) {
      for ( int j = 0; j < 4; j++ ) {
         GLfloat v = 0.;
         for ( int k = 0; k < 4; k++ )
            v += m1->m[i][k] * m2->m[k][j];
         out->m[i][j] = v;
      }
   }
}

/**
 * @brief Multiplies a matrix with a vector (out = m * v);
 *
 * Note that out should not be v.
 *
 *    @param[out] out Output vector.
 *    @param m Matrix to multiply.
 *    @param v Vector to multiply.
 */
void mat4_mul_vec( vec3 *out, const mat4 *m, const vec3 *v )
{
   for ( int i = 0; i < 3; i++ ) {
      GLfloat a = m->m[3][i];
      for ( int j = 0; j < 3; j++ )
         a += m->m[j][i] * v->v[j];
      out->v[i] = a;
   }
}

/**
 * @brief Applies a transformation to another, storing the result in the left
 * hand side.
 *
 *    @param[in, out] lhs Left hand side matrix.
 *    @param[in] rhs Right hand side matrix.
 */
void mat4_apply( mat4 *lhs, const mat4 *rhs )
{
   /* Process by rows. */
   for ( int i = 0; i < 4; i++ ) {
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
   for ( int i = 0; i < 4; i++ ) {
      m->m[0][i] *= x;
      m->m[1][i] *= y;
      m->m[2][i] *= z;
   }
}
void mat4_scale_xy( mat4 *m, double x, double y )
{
   for ( int i = 0; i < 4; i++ ) {
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
   for ( int i = 0; i < 4; i++ )
      m->m[3][i] += m->m[0][i] * x + m->m[1][i] * y + m->m[2][i] * z;
}
void mat4_translate_x( mat4 *m, double x )
{
   for ( int i = 0; i < 4; i++ )
      m->m[3][i] += m->m[0][i] * x;
}
void mat4_translate_xy( mat4 *m, double x, double y )
{
   for ( int i = 0; i < 4; i++ )
      m->m[3][i] += m->m[0][i] * x + m->m[1][i] * y;
}
void mat4_translate_scale_xy( mat4 *m, double x, double y, double w, double h )
{
   for ( int i = 0; i < 4; i++ ) {
      m->m[3][i] += m->m[0][i] * x + m->m[1][i] * y;
      m->m[0][i] *= w;
      m->m[1][i] *= h;
   }
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

   c          = cos( angle );
   s          = sin( angle );
   x          = m->m[0][0];
   y          = m->m[1][0];
   m->m[0][0] = c * x + s * y;
   m->m[1][0] = -s * x + c * y;

   x          = m->m[0][1];
   y          = m->m[1][1];
   m->m[0][1] = c * x + s * y;
   m->m[1][1] = -s * x + c * y;
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

   x          = m->m[0][0];
   y          = m->m[1][0];
   m->m[0][0] = c * x + s * y;
   m->m[1][0] = -s * x + c * y;

   x          = m->m[0][1];
   y          = m->m[1][1];
   m->m[0][1] = c * x + s * y;
   m->m[1][1] = -s * x + c * y;
}

/**
 * @brief Multiplies the given matrix by a rotation. (Follows the right-hand
 * rule.)
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
   mat4   R;

   norm = sqrt( pow2( x ) + pow2( y ) + pow2( z ) );
   c    = cos( angle );
   s    = sin( angle );
   x /= norm;
   y /= norm;
   z /= norm;
   R.m[0][0] = x * x * ( 1. - c ) + c;
   R.m[0][1] = y * x * ( 1. - c ) + z * s;
   R.m[0][2] = x * z * ( 1. - c ) - y * s;
   R.m[0][3] = 0.;
   R.m[1][0] = x * y * ( 1. - c ) - z * s;
   R.m[1][1] = y * y * ( 1. - c ) + c;
   R.m[1][2] = y * z * ( 1. - c ) + x * s;
   R.m[1][3] = 0.;
   R.m[2][0] = x * z * ( 1. - c ) + y * s;
   R.m[2][1] = y * z * ( 1. - c ) - x * s;
   R.m[2][2] = z * z * ( 1. - c ) + c;
   R.m[2][3] = 0.;
   R.m[3][0] = 0.;
   R.m[3][1] = 0.;
   R.m[3][2] = 0.;
   R.m[3][3] = 1.;

   mat4_apply( m, &R );
}

/**
 * @brief Applies a quaternion transformation.
 *
 *    @param[in, out] m Matrix to multiply with.
 *    @param q Quaternion to use for rotation.
 */
void mat4_rotate_quaternion( mat4 *m, const quat *q )
{
   mat4 R;

   GLfloat qx, qy, qz, qw;
   qx = q->q[0];
   qy = q->q[1];
   qz = q->q[2];
   qw = q->q[3];

   R.ptr[0] = ( 1. - 2. * qy * qy - 2. * qz * qz );
   R.ptr[1] = ( 2. * qx * qy + 2. * qz * qw );
   R.ptr[2] = ( 2. * qx * qz - 2. * qy * qw );
   R.ptr[3] = 0.;

   R.ptr[4] = ( 2. * qx * qy - 2. * qz * qw );
   R.ptr[5] = ( 1. - 2. * qx * qx - 2. * qz * qz );
   R.ptr[6] = ( 2. * qy * qz + 2. * qx * qw );
   R.ptr[7] = 0.;

   R.ptr[8]  = ( 2. * qx * qz + 2. * qy * qw );
   R.ptr[9]  = ( 2. * qy * qz - 2. * qx * qw );
   R.ptr[10] = ( 1. - 2. * qx * qx - 2. * qy * qy );
   R.ptr[11] = 0.;

   R.ptr[12] = 0.;
   R.ptr[13] = 0.;
   R.ptr[14] = 0.;
   R.ptr[15] = 1.;

   mat4_apply( m, &R );
}

/**
 * @brief Creates a homogeneous transform matrix from a translation, rotation,
 * and scaling. Uses T*R*S order.
 */
void mat4_trs( mat4 *m, const vec3 *t, const quat *r, const vec3 *s )
{
   GLfloat tx, ty, tz;
   GLfloat qx, qy, qz, qw;
   GLfloat sx, sy, sz;

   tx = t->v[0];
   ty = t->v[1];
   tz = t->v[2];

   qx = r->q[0];
   qy = r->q[1];
   qz = r->q[2];
   qw = r->q[3];

   sx = s->v[0];
   sy = s->v[1];
   sz = s->v[2];

   m->ptr[0] = ( 1 - 2 * qy * qy - 2 * qz * qz ) * sx;
   m->ptr[1] = ( 2 * qx * qy + 2 * qz * qw ) * sx;
   m->ptr[2] = ( 2 * qx * qz - 2 * qy * qw ) * sx;
   m->ptr[3] = 0.f;

   m->ptr[4] = ( 2 * qx * qy - 2 * qz * qw ) * sy;
   m->ptr[5] = ( 1 - 2 * qx * qx - 2 * qz * qz ) * sy;
   m->ptr[6] = ( 2 * qy * qz + 2 * qx * qw ) * sy;
   m->ptr[7] = 0.f;

   m->ptr[8]  = ( 2 * qx * qz + 2 * qy * qw ) * sz;
   m->ptr[9]  = ( 2 * qy * qz - 2 * qx * qw ) * sz;
   m->ptr[10] = ( 1 - 2 * qx * qx - 2 * qy * qy ) * sz;
   m->ptr[11] = 0.f;

   m->ptr[12] = tx;
   m->ptr[13] = ty;
   m->ptr[14] = tz;
   m->ptr[15] = 1.f;
}

/**
 * @brief Creates an identity matrix.
 *
 *    @return A new identity matrix.
 */
mat4 mat4_identity( void )
{
   const mat4 m = { .m = { { 1., 0., 0., 0. },
                           { 0., 1., 0., 0. },
                           { 0., 0., 1., 0. },
                           { 0., 0., 0., 1. } } };
   return m;
}

/**
 * @brief Creates an orthographic projection matrix.
 */
mat4 mat4_ortho( double left, double right, double bottom, double top,
                 double nearVal, double farVal )
{
   mat4   mat = { { { { 0 } } } };
   double tx, ty, tz;

   /* https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml
    */
   tx = -( right + left ) / ( right - left );
   ty = -( top + bottom ) / ( top - bottom );
   tz = -( farVal + nearVal ) / ( farVal - nearVal );

   mat.m[0][0] = 2. / ( right - left );
   mat.m[1][1] = 2. / ( top - bottom );
   mat.m[2][2] = -2. / ( farVal - nearVal );
   mat.m[3][3] = 1.;
   mat.m[3][0] = tx;
   mat.m[3][1] = ty;
   mat.m[3][2] = tz;

   return mat;
}

/**
 * @brief Creates a matrix with a transformation to look at a centre point from
 * an eye with an up vector.
 *
 *    @param[in] eye Vector representing the eye position that is looking at
 * something.
 *    @param[in] centre Vector representing the position that is being looked
 * at.
 *    @param[in] up Vector representing the "upward" direction. Has to be a
 * unitary vector.
 *    @return The newly created matrix.
 */
mat4 mat4_lookat( const vec3 *eye, const vec3 *centre, const vec3 *up )
{
   vec3 forward, side, upc;
   mat4 H;

   vec3_sub( &forward, centre, eye );
   vec3_normalize( &forward ); /* Points towards the centre from the eye. */

   /* side = forward x up */
   vec3_cross( &side, &forward, up );
   vec3_normalize( &side ); /* Points to the side. */

   /* upc = side x forward */
   vec3_cross( &upc, &side, &forward );
   /* No need to normalize since forward and side and unitary. */

   /* First column. */
   H.m[0][0] = side.v[0];
   H.m[1][0] = side.v[1];
   H.m[2][0] = side.v[2];
   H.m[3][0] = 0.;
   /* Second column. */
   H.m[0][1] = upc.v[0];
   H.m[1][1] = upc.v[1];
   H.m[2][1] = upc.v[2];
   H.m[3][1] = 0.;
   /* Third column. */
   H.m[0][2] = -forward.v[0];
   H.m[1][2] = -forward.v[1];
   H.m[2][2] = -forward.v[2];
   H.m[3][2] = 0.;
   /* Fourth column. */
   H.m[0][3] = 0.; //-eye->v[0];
   H.m[1][3] = 0.; //-eye->v[1];
   H.m[2][3] = 0.; //-eye->v[2];
   H.m[3][3] = 1.;

   mat4_translate( &H, -eye->v[0], -eye->v[1], -eye->v[2] );

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
   mat4   H;
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
   H.m[2][2] = -( far + near ) / d;
   H.m[2][3] = -1.;
   /* Fourth column. */
   H.m[3][0] = 0.;
   H.m[3][1] = 0.;
   H.m[3][2] = -2. * far * near / d;
   H.m[3][3] = 0.;

   return H;
}

void quat_normalize( quat *q )
{
   GLfloat d = sqrtf( pow2( q->q[0] ) + pow2( q->q[1] ) + pow2( q->q[2] ) +
                      pow2( q->q[3] ) );
   for ( int i = 0; i < 4; i++ )
      q->q[i] /= d;
}

void quat_slerp( GLfloat qm[4], const GLfloat qa[4], const GLfloat qb[4],
                 GLfloat t )
{
   // Calculate angle between them.
   double cosHalfTheta =
      qa[3] * qb[3] + qa[0] * qb[0] + qa[1] * qb[1] + qa[2] * qb[2];
   // if qa=qb or qa=-qb then theta = 0 and we can return qa
   if ( fabs( cosHalfTheta ) >= 1.0 ) {
      qm[3] = qa[3];
      qm[0] = qa[0];
      qm[1] = qa[1];
      qm[2] = qa[2];
      return;
   }
   // Calculate temporary values.
   double halfTheta    = acos( cosHalfTheta );
   double sinHalfTheta = sqrt( 1.0 - cosHalfTheta * cosHalfTheta );
   // if theta = 180 degrees then result is not fully defined
   // we could rotate around any axis normal to qa or qb
   if ( fabs( sinHalfTheta ) < 0.001 ) { // fabs is floating point absolute
      qm[3] = ( qa[3] * 0.5 + qb[3] * 0.5 );
      qm[0] = ( qa[0] * 0.5 + qb[0] * 0.5 );
      qm[1] = ( qa[1] * 0.5 + qb[1] * 0.5 );
      qm[2] = ( qa[2] * 0.5 + qb[2] * 0.5 );
      return;
   }
   double ratioA = sin( ( 1 - t ) * halfTheta ) / sinHalfTheta;
   double ratioB = sin( t * halfTheta ) / sinHalfTheta;
   // calculate Quaternion.
   qm[3] = ( qa[3] * ratioA + qb[3] * ratioB );
   qm[0] = ( qa[0] * ratioA + qb[0] * ratioB );
   qm[1] = ( qa[1] * ratioA + qb[1] * ratioB );
   qm[2] = ( qa[2] * ratioA + qb[2] * ratioB );
}
