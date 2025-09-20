/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include <stddef.h>

#include "glad.h"

#include "vec3.h"

typedef struct mat4_ {
   union {
      /* Column-major; m[x][y] */
      GLfloat m[4][4];
      GLfloat ptr[16];
   };
} mat4;

typedef struct quat {
   GLfloat q[4]; /* x, y, z, w */
} quat;

/* Basic operations. */
void mat4_tostr( const mat4 *m, char *buf, size_t len );
void mat4_print( const mat4 *m );
void mat4_mul( mat4 *out, const mat4 *m1, const mat4 *m2 );
void mat4_mul_vec( vec3 *out, const mat4 *m, const vec3 *v );

/* Affine transformations. */
void mat4_apply( mat4 *lhs, const mat4 *rhs );
void mat4_scale( mat4 *m, double x, double y, double z );
void mat4_scale_xy( mat4 *m, double x, double y );
void mat4_translate( mat4 *m, double x, double y, double z );
void mat4_translate_x( mat4 *m, double x );
void mat4_translate_xy( mat4 *m, double x, double y );
void mat4_translate_scale_xy( mat4 *m, double x, double y, double w, double h );
void mat4_rotate( mat4 *m, double angle, double x, double y, double z );
void mat4_rotate_quaternion( mat4 *m, const quat *q );
void mat4_rotate2d( mat4 *m, double angle );
void mat4_rotate2dv( mat4 *m, double x, double y );
void mat4_trs( mat4 *m, const vec3 *t, const quat *r, const vec3 *s );

/* Creation functions. */
__attribute__( ( const ) ) mat4 mat4_identity( void );
__attribute__( ( const ) ) mat4 mat4_ortho( double left, double right,
                                            double bottom, double top,
                                            double nearVal, double farVal );
mat4 mat4_lookat( const vec3 *eye, const vec3 *centre, const vec3 *up );
mat4 mat4_perspective( double fov, double aspect, double near, double far );

/* Quaternion stuff. */
void quat_normalize( quat *q );
void quat_slerp( GLfloat qm[4], const GLfloat qa[4], const GLfloat qb[4],
                 GLfloat t );
