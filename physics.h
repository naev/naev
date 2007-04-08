

#ifndef PHYSICS_H
#  define PHYSICS_H

#include "all.h"

/*
 * base of all 2d Vector work
 */
typedef struct {
	double x, y; /* basic 2d vector components */
} Vector2d;

/*
 * used to describe any Solid in 2d space
 */
struct Solid {
	double mass, force, dir, dir_vel; /* properties */
	Vector2d vel, pos; /* position/velocity vectors */
	void (*update)( struct Solid*, const double ); /* update method */
};
typedef struct Solid Solid;


void solid_init( Solid* dest, const double mass, const Vector2d* vel, const Vector2d* pos );
Solid* solid_create( const double mass, const Vector2d* vel, const Vector2d* pos );
void solid_free( Solid* src );

#endif /* PHYSICS_H */
