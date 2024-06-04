
/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file mat3.c
 *
 * @brief Handles OpenGL matrix stuff for 3 dimension matrices.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include <stdio.h>

#include "mat3.h"

void mat3_print( const mat3 *m )
{
   for ( int i = 0; i < 3; i++ ) {
      for ( int j = 0; j < 3; j++ )
         printf( "%6.1f ", m->m[j][i] );
      printf( "\n" );
   }
}

void mat3_from_mat4( mat3 *out, const mat4 *in )
{
   for ( int i = 0; i < 3; i++ )
      for ( int j = 0; j < 3; j++ )
         out->m[i][j] = in->m[i][j];
}

double mat3_det( const mat3 *m )
{
   return m->m[0][0] * ( m->m[1][1] * m->m[2][2] - m->m[2][1] * m->m[1][2] ) -
          m->m[0][1] * ( m->m[1][0] * m->m[2][2] - m->m[1][2] * m->m[2][0] ) +
          m->m[0][2] * ( m->m[1][0] * m->m[2][1] - m->m[1][1] * m->m[2][0] );
}

void mat3_invert( mat3 *m )
{
   double     invdet = mat3_det( m );
   const mat3 o      = *m;
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
