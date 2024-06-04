
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
void mat3_print( const mat3 *m );
void mat3_from_mat4( mat3 *out, const mat4 *in );

/* Affine transformations. */
void mat3_invert( mat3 *m );
void mat3_transpose( mat3 *m );
