

#ifndef PHYSICS_H
#  define PHYSICS_H

#include "all.h"

/*
 * base of all 2d Vector work
 */
typedef struct {
	FP x, y;
} Vector2d;

/*
 * used to describe any Solid in 2d space
 */
struct Solid {
	FP mass, force, dir;
	Vector2d vel, pos;
	void (*update)( struct Solid*, const FP );
};
typedef struct Solid Solid;


void solid_init( Solid* dest, const FP mass, const Vector2d* vel, const Vector2d* pos );
Solid* solid_create( const FP mass, const Vector2d* vel, const Vector2d* pos );


#endif /* PHYSICS_H */
