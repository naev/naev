
/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file mat3.c
 *
 * @brief Handles OpenGL matrix stuff for 3 dimension matrices.
 */
#include <stdio.h>

#include "mat3.h"

mat3 mat3_identity( void )
{
   const mat3 m = { .m = { { 1., 0., 0. }, { 0., 1., 0. }, { 0., 0., 1. } } };
   return m;
}

void mat3_print( const mat3 *m )
{
   for ( int i = 0; i < 3; i++ ) {
      for ( int j = 0; j < 3; j++ )
         printf( "%7.g ", m->m[j][i] );
      printf( "\n" );
   }
}

void mat3_from_mat4( mat3 *out, const mat4 *in )
{
   for ( int i = 0; i < 3; i++ )
      for ( int j = 0; j < 3; j++ )
         out->m[i][j] = in->m[i][j];
}

void mat4_from_mat3( mat4 *out, const mat3 *in )
{
   out->m[0][0] = in->m[0][0];
   out->m[0][1] = in->m[0][1];
   out->m[0][2] = 0.0;
   out->m[0][3] = in->m[0][2];

   out->m[1][0] = in->m[1][0];
   out->m[1][1] = in->m[1][1];
   out->m[1][2] = 0.0;
   out->m[1][3] = in->m[1][2];

   out->m[2][0] = 0.0;
   out->m[2][1] = 0.0;
   out->m[2][2] = 1.0;
   out->m[2][3] = 0.0;

   out->m[3][0] = in->m[2][0];
   out->m[3][1] = in->m[2][1];
   out->m[3][2] = 0.0;
   out->m[3][3] = in->m[2][2];
}

void mat3_mul_vec( vec3 *out, const mat3 *M, const vec3 *v )
{
   for ( int i = 0; i < 3; i++ ) {
      GLfloat a = 0.;
      for ( int j = 0; j < 3; j++ )
         a += M->m[j][i] * v->v[j];
      out->v[i] = a;
   }
}

double mat3_det( const mat3 *m )
{
   return m->m[0][0] * ( m->m[1][1] * m->m[2][2] - m->m[2][1] * m->m[1][2] ) -
          m->m[0][1] * ( m->m[1][0] * m->m[2][2] - m->m[1][2] * m->m[2][0] ) +
          m->m[0][2] * ( m->m[1][0] * m->m[2][1] - m->m[1][1] * m->m[2][0] );
}

void mat3_invert( mat3 *m )
{
   const double invdet = 1. / mat3_det( m );
   const mat3   o      = *m;
   m->m[0][0] = ( o.m[1][1] * o.m[2][2] - o.m[2][1] * o.m[1][2] ) * invdet;
   m->m[0][1] = ( o.m[0][2] * o.m[2][1] - o.m[0][1] * o.m[2][2] ) * invdet;
   m->m[0][2] = ( o.m[0][1] * o.m[1][2] - o.m[0][2] * o.m[1][1] ) * invdet;
   m->m[1][0] = ( o.m[1][2] * o.m[2][0] - o.m[1][0] * o.m[2][2] ) * invdet;
   m->m[1][1] = ( o.m[0][0] * o.m[2][2] - o.m[0][2] * o.m[2][0] ) * invdet;
   m->m[1][2] = ( o.m[1][0] * o.m[0][2] - o.m[0][0] * o.m[1][2] ) * invdet;
   m->m[2][0] = ( o.m[1][0] * o.m[2][1] - o.m[2][0] * o.m[1][1] ) * invdet;
   m->m[2][1] = ( o.m[2][0] * o.m[0][1] - o.m[0][0] * o.m[2][1] ) * invdet;
   m->m[2][2] = ( o.m[0][0] * o.m[1][1] - o.m[1][0] * o.m[0][1] ) * invdet;
}

void mat3_transpose( mat3 *m )
{
   /* Not optimal due to caching, but simplest. */
   for ( int i = 0; i < 3 - 1; i++ ) {
      for ( int j = i + 1; j < 3; j++ ) {
         double t   = m->m[i][j];
         m->m[i][j] = m->m[j][i];
         m->m[j][i] = t;
      }
   }
}
