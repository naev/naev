

#ifndef PHYSICS_H
#  define PHYSICS_H

#include "all.h"

/*
 * base of all 2d Vector work
 */
typedef struct {
	FP x, y; /* basic 2d vector components */
} Vector2d;

/*
 * used to describe any Solid in 2d space
 */
struct Solid {
	FP mass, force, dir; /* properties */
	Vector2d vel, pos; /* position/velocity vectors */
	void (*update)( struct Solid*, const FP ); /* update method */
};
typedef struct Solid Solid;


void solid_init( Solid* dest, const FP mass, const Vector2d* vel, const Vector2d* pos );
Solid* solid_create( const FP mass, const Vector2d* vel, const Vector2d* pos );
void solid_free( Solid* src );

#endif /* PHYSICS_H */
