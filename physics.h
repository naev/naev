

#ifndef PHYSICS_H
#  define PHYSICS_H

#include "all.h"

#define VX(v)		((v).mod*cos((v).angle))
#define VY(v)		((v).mod*sin((v).angle))
#define VMOD(v)	((v).mod)
#define VANGLE(v)	((v).angle)

#define MOD(x,y)	(sqrt(x*x+y*y))
#define ANGLE(x,y) ((x==0.) ? 0. : ((x<0.)?atan(y/x)+M_PI:atan(y/x)))


/*
 * base of all 2d Vector work
 */
typedef struct {
	double mod, angle;
} Vector2d;

/*
 * vector manipulation
 */
void vect_cinit( Vector2d* v, double x, double y );
void vect_pinit( Vector2d* v, double mod, double angle );


/*
 * used to describe any Solid in 2d space
 */
struct Solid {
	double mass, dir, dir_vel; /* properties */
	Vector2d vel, pos, force; /* position/velocity vectors */
	void (*update)( struct Solid*, const double ); /* update method */
};
typedef struct Solid Solid;


/*
 * solid manipulation
 */
void solid_init( Solid* dest, const double mass, const Vector2d* vel, const Vector2d* pos );
Solid* solid_create( const double mass, const Vector2d* vel, const Vector2d* pos );
void solid_free( Solid* src );

#endif /* PHYSICS_H */
