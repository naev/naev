/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef PHYSICS_H
#  define PHYSICS_H


#include <math.h>


#define VX(v)     ((v).x) /**< Gets the X component of a vector. */
#define VY(v)     ((v).y) /**< Gets the Y component of a vector. */
#define VMOD(v)   ((v).mod) /**< Gets the modulus of a vector. */
#define VANGLE(v) ((v).angle) /**< Gets the angle of a vector. */

#define MOD(x,y)  (sqrt((x)*(x)+(y)*(y))) /**< Gets the modulus of a vector by cartesian coordinates. */
#define ANGLE(x,y) (atan2(y,x)) /**< Gets the angle of two cartesian coordinates. */

#define vect_dist(v,u)  MOD((v)->x-(u)->x,(v)->y-(u)->y) /**< Gets the distance between two vectors. */
#define vect_dist2(v,u) (((v)->x-(u)->x)*((v)->x-(u)->x)+((v)->y-(u)->y)*((v)->y-(u)->y))
#define vect_odist(v)   MOD((v)->x,(v)->y) /**< Gets the distance of a vector from the origin. */
#define vect_odist2(v)  ((v)->x*(v)->x+(v)->y*(v)->y) /**< Gets the squared distance of a vector from the origin. */

/*
 * Update options.
 */
#define SOLID_UPDATE_RK4      0 /**< Default Runge-Kutta 3-4 update. */
#define SOLID_UPDATE_EULER    1 /**< Simple Euler update. */


/**
 * @brief Represents a 2d vector.
 */
typedef struct Vector2d_ {
   double x; /**< X cartesian position of the vector. */
   double y; /**< Y cartesian position of the vector. */
   double mod; /**< Modulus of the vector. */
   double angle; /**< Angle of the vector. */
} Vector2d; /**< 2 dimensional vector. */


/*
 * misc
 */
double angle_diff( const double ref, double a );


/*
 * vector manipulation
 */
void vect_cset( Vector2d* v, const double x, const double y );
void vect_csetmin( Vector2d* v, const double x, const double y ); /* does not set mod nor angle */
void vect_pset( Vector2d* v, const double mod, const double angle );
void vectnull( Vector2d* v );
double vect_angle( const Vector2d* ref, const Vector2d* v );
void vect_cadd( Vector2d* v, const double x, const double y );
void vect_padd( Vector2d* v, const double m, const double a );
void vect_reflect( Vector2d* r, Vector2d* v, Vector2d* n );
double vect_dot( Vector2d* a, Vector2d* b );
void vect_uv(double* u, double *v, Vector2d* source, Vector2d* reference);
void vect_uv_decomp(Vector2d* u, Vector2d* v, Vector2d* reference);


/**
 * @brief Represents a solid in the game.
 */
typedef struct Solid_ {
   double mass; /**< Solid's mass. */
   double dir; /**< Direction solid is facing in rad. */
   double dir_vel; /**< Velocity at which solid is rotating in rad/s. */
   Vector2d vel; /**< Velocity of the solid. */
   Vector2d pos; /**< Position of the solid. */
   double thrust; /**< Relative X force, basically simplified for our thrust model. */
   double speed_max; /**< Maximum speed. */
   void (*update)( struct Solid_*, const double ); /**< Update method. */
} Solid;


/*
 * solid manipulation
 */
double solid_maxspeed( Solid *s, double speed, double thrust );
void solid_init( Solid* dest, const double mass, const double dir,
      const Vector2d* pos, const Vector2d* vel, int update );
Solid* solid_create( const double mass, const double dir,
      const Vector2d* pos, const Vector2d* vel, int update );
void solid_free( Solid* src );


#endif /* PHYSICS_H */


