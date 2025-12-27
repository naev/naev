
/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "glad.h"

#include "mat4.h"

typedef struct mat3_ {
   union {
      /* Column-major; m[x][y] */
      GLfloat m[3][3];
      GLfloat ptr[9];
   };
} mat3;

/* Basic operations. */
__attribute__( ( const ) ) mat3 mat3_identity( void );
void                            mat3_print( const mat3 *m );
void                            mat3_from_mat4( mat3 *out, const mat4 *in );
void                            mat4_from_mat3( mat4 *out, const mat3 *in );
void mat3_mul_vec( vec3 *out, const mat3 *M, const vec3 *v );

/* Useful functions. */
double mat3_det( const mat3 *m );
void   mat3_invert( mat3 *m );
void   mat3_transpose( mat3 *m );
