/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "commodity.h"
#include "vec2.h"

typedef int64_t GatherableRef;
#define GATHERABLE_NULL ( ( 1ll << 32ll ) + ( 1ll << 32ll ) - 1ll )

/*
 * Gatherable objects
 */
GatherableRef gatherable_init( CommodityRef com, const vec2 *pos,
                               const vec2 *vel, double lifeleng, int qtt,
                               unsigned int player_only );
void          gatherable_render( void );
GatherableRef gatherable_getClosest( const vec2 *pos, double rad );
int           gatherable_getPos( vec2 *pos, vec2 *vel, GatherableRef id );
void          gatherable_free( void );
void          gatherable_update( double dt );
