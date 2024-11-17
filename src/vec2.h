/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <math.h>
/** @endcond */

#define VX( v ) ( ( v ).x )         /**< Gets the X component of a vector. */
#define VY( v ) ( ( v ).y )         /**< Gets the Y component of a vector. */
#define VMOD( v ) ( ( v ).mod )     /**< Gets the modulus of a vector. */
#define VANGLE( v ) ( ( v ).angle ) /**< Gets the angle of a vector. */

#define MOD( x, y )                                                            \
   ( hypot(                                                                    \
      ( x ),                                                                   \
      ( y ) ) ) /**< Gets the modulus of a vector by cartesian coordinates. */
#define ANGLE( x, y )                                                          \
   ( atan2( y, x ) ) /**< Gets the angle of two cartesian coordinates. */

#define vec2_dist( v, u )                                                      \
   MOD( ( v )->x - ( u )->x,                                                   \
        ( v )->y - ( u )->y ) /**< Gets the distance between two vectors. */
#define vec2_dist2( v, u )                                                     \
   ( ( ( v )->x - ( u )->x ) * ( ( v )->x - ( u )->x ) +                       \
     ( ( v )->y - ( u )->y ) * ( ( v )->y - ( u )->y ) )
#define vec2_odist( v )                                                        \
   MOD( ( v )->x,                                                              \
        ( v )->y ) /**< Gets the distance of a vector from the origin. */
#define vec2_odist2( v )                                                       \
   ( ( v )->x * ( v )->x +                                                     \
     ( v )->y * ( v )->y ) /**< Gets the squared distance of a vector from the \
                              origin. */

/*
 * Update options.
 */
#define SOLID_UPDATE_RK4 0   /**< Default Runge-Kutta 3-4 update. */
#define SOLID_UPDATE_EULER 1 /**< Simple Euler update. */

/**
 * @brief Represents a 2d vector.
 */
typedef struct vec2_ {
   double x;     /**< X cartesian position of the vector. */
   double y;     /**< Y cartesian position of the vector. */
   double mod;   /**< Modulus of the vector. */
   double angle; /**< Angle of the vector. */
} vec2;          /**< 2 dimensional vector. */

/*
 * vector manipulation
 */
void   vec2_cset( vec2 *v, double x, double y );
void   vec2_csetmin( vec2 *v, double x,
                     double y ); /* does not set mod nor angle */
void   vec2_pset( vec2 *v, double mod, double angle );
void   vectnull( vec2 *v );
double vec2_angle( const vec2 *ref, const vec2 *v );
void   vec2_cadd( vec2 *v, double x, double y );
void   vec2_padd( vec2 *v, double m, double a );
void   vec2_reflect( vec2 *r, const vec2 *v, const vec2 *n );
double vec2_dot( const vec2 *a, const vec2 *b );
void vec2_uv( double *u, double *v, const vec2 *source, const vec2 *reference );
void vec2_uv_decomp( vec2 *u, vec2 *v, const vec2 *reference );
