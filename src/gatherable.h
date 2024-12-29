/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "commodity.h"
#include "vec2.h"

/*
 * Init/cleanup.
 */
int  gatherable_load( void );
void gatherable_cleanup( void );

/*
 * Gatherable objects
 */
int  gatherable_init( const Commodity *com, const vec2 *pos, const vec2 *vel,
                      double lifeleng, int qtt, unsigned int player_only );
void gatherable_render( void );
int  gatherable_getClosest( const vec2 *pos, double rad );
int  gatherable_getPos( vec2 *pos, vec2 *vel, int id );
void gatherable_free( void );
void gatherable_update( double dt );
