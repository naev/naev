/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

typedef struct vec3_ {
   double v[3];
} vec3;

void vec3_add( vec3 *out, const vec3 *a, const vec3 *b );
void vec3_sub( vec3 *out, const vec3 *a, const vec3 *b );
void vec3_wadd( vec3 *out, const vec3 *a, const vec3 *b, double wa, double wb );
void vec3_max( vec3 *out, const vec3 *a, const vec3 *b );
void vec3_min( vec3 *out, const vec3 *a, const vec3 *b );
double vec3_dot( const vec3 *a, const vec3 *b );
void   vec3_cross( vec3 *out, const vec3 *a, const vec3 *b );
void   vec3_normalize( vec3 *a );
double vec3_dist( const vec3 *a, const vec3 *b );
double vec3_length( const vec3 *a );

double vec3_distPointTriangle( const vec3 *point, const vec3 tri[3] );
