/*
 * See Licensing and Copyright notice in naev.h
 */
#include "vec2.h"

/**
 * @brief Set the vector value using cartesian coordinates
 *
 *    @param v Vector to set.
 *    @param x X value for vector.
 *    @param y Y value for vector.
 */
void vec2_cset( vec2 *v, double x, double y )
{
   v->x     = x;
   v->y     = y;
   v->mod   = MOD( x, y );
   v->angle = ANGLE( x, y );
}

/**
 * @brief Creates a minimal vector only valid for blitting and not other
 * operations.
 *
 *    @param v Vector to set.
 *    @param x X value for vector.
 *    @param y Y value for vector.
 */
void vec2_csetmin( vec2 *v, double x, double y )
{
   v->x = x;
   v->y = y;
}

/**
 * @brief Set the vector value using polar coordinates.
 *
 *    @param v Vector to set.
 *    @param mod Modulus of the vector.
 *    @param angle Angle of the vector.
 */
void vec2_pset( vec2 *v, double mod, double angle )
{
   v->mod   = mod;
   v->angle = angle;
   v->x     = v->mod * cos( v->angle );
   v->y     = v->mod * sin( v->angle );
}

/**
 * @brief Sets a vector to NULL.
 *
 *    @param v Vector to set to NULL.
 */
void vectnull( vec2 *v )
{
   v->x     = 0.;
   v->y     = 0.;
   v->mod   = 0.;
   v->angle = 0.;
}

/**
 * @brief Get the direction pointed to by two vectors (from ref to v).
 *
 *    @param ref Reference vector.
 *    @param v Vector to get angle from reference vector.
 *    @return Angle between ref and v.
 */
double vec2_angle( const vec2 *ref, const vec2 *v )
{
   double x = v->x - ref->x;
   double y = v->y - ref->y;
   return ANGLE( x, y );
}

/**
 * @brief Adds x and y to the current vector
 *
 *    @param v Vector to add x and y to.
 *    @param x X value to add to vector.
 *    @param y Y value to add to vector.
 */
void vec2_cadd( vec2 *v, double x, double y )
{
   v->x += x;
   v->y += y;
   v->mod   = MOD( v->x, v->y );
   v->angle = ANGLE( v->x, v->y );
}

/**
 * @brief Adds a polar 2d vector to the current vector.
 *
 *    @param v Vector to add x and y to.
 *    @param m Module of vector to add.
 *    @param a Angle of vector to add.
 */
void vec2_padd( vec2 *v, double m, double a )
{
   v->x += m * cos( a );
   v->y += m * sin( a );
   v->mod   = MOD( v->x, v->y );
   v->angle = ANGLE( v->x, v->y );
}

/**
 * @brief Mirrors a vector off another, stores results in vector.
 *
 *    @param r Resulting vector of the reflection.
 *    @param v Vector to reflect.
 *    @param n Normal to reflect off of.
 */
void vec2_reflect( vec2 *r, const vec2 *v, const vec2 *n )
{
   double dot = vec2_dot( v, n );
   r->x       = v->x - ( ( 2. * dot ) * n->x );
   r->y       = v->y - ( ( 2. * dot ) * n->y );
   r->mod     = MOD( r->x, r->y );
   r->angle   = ANGLE( r->x, r->y );
}

/**
 * @brief Vector dot product.
 *
 *    @param a Vector 1 for dot product.
 *    @param b Vector 2 for dot product.
 *    @return Dot product of vectors.
 */
double vec2_dot( const vec2 *a, const vec2 *b )
{
   return a->x * b->x + a->y * b->y;
}

/**
 * @brief Determines the magnitude of the source vector components.
 *
 *    @param[out] u Parallel component to reference vector.
 *    @param[out] v Perpendicular component to reference vector.
 *    @param source Source vector.
 *    @param reference_vector Reference vector.
 */
void vec2_uv( double *u, double *v, const vec2 *source,
              const vec2 *reference_vector )
{
   vec2 unit_parallel, unit_perpendicular;

   vec2_uv_decomp( &unit_parallel, &unit_perpendicular, reference_vector );

   *u = vec2_dot( source, &unit_parallel );
   *v = vec2_dot( source, &unit_perpendicular );
}

/**
 * @brief Does UV decomposition of the reference vector.
 *
 *    @param[out] u Parallel component of the reference vector.
 *    @param[out] v Perpendicular component of the reference vector.
 *    @param reference_vector The reference vector to decompose.
 */
void vec2_uv_decomp( vec2 *u, vec2 *v, const vec2 *reference_vector )
{
   vec2_pset( u, 1, VANGLE( *reference_vector ) );
   vec2_pset( v, 1, VANGLE( *reference_vector ) + M_PI_2 );
}
