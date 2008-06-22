/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef PHYSICS_H
#  define PHYSICS_H


#include <math.h>


#define VX(v)     ((v).x)
#define VY(v)     ((v).y)
#define VMOD(v)   ((v).mod)
#define VANGLE(v) ((v).angle)

#define MOD(x,y)  (sqrt((x)*(x)+(y)*(y)))
#define ANGLE(x,y) (((x)==0.) ? 0. : (((x)<0.) ? atan((y)/(x))+M_PI : atan((y)/(x))))

#define vect_dist(v,u)  MOD((v)->x-(u)->x,(v)->y-(u)->y)
#define vect_odist(v)   MOD((v)->x,(v)->y)


/*
 * base of all 2d Vector work
 */
typedef struct Vector2d_ {                                          
   double x, y; /* cartesian values */                              
   double mod, angle; /* polar values */
} Vector2d;


/*
 * misc
 */
double angle_diff( const double ref, double a );
void limit_speed( Vector2d* vel, const double speed, const double dt );


/*
 * vector manipulation
 */
void vect_cset( Vector2d* v, const double x, const double y );
void vect_csetmin( Vector2d* v, const double x, const double y ); /* does not set mod nor angle */
void vect_pset( Vector2d* v, const double mod, const double angle );
void vectcpy( Vector2d* dest, const Vector2d* src );
void vectnull( Vector2d* v );
double vect_angle( const Vector2d* ref, const Vector2d* v );
void vect_cadd( Vector2d* v, const double x, const double y );
void vect_reflect( Vector2d* r, Vector2d* v, Vector2d* n );


/*
 * used to describe any Solid in 2d space
 */
typedef struct Solid_ {
   double mass, dir, dir_vel; /* properties */
   Vector2d vel, pos, force; /* position/velocity vectors */
   void (*update)( struct Solid_*, const double ); /* update method */
} Solid;


/*
 * solid manipulation
 */
void solid_init( Solid* dest, const double mass, const double dir,
      const Vector2d* pos, const Vector2d* vel );
Solid* solid_create( const double mass, const double dir,
      const Vector2d* pos, const Vector2d* vel );
void solid_free( Solid* src );


#endif /* PHYSICS_H */


