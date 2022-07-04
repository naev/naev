/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "commodity.h"
#include "pilot.h"

/**
 * @struct Gatherable
 *
 * @brief Represents stuff that can be gathered.
 */
typedef struct Gatherable_ {
   const Commodity *type;/**< Type of commodity. */
   vec2 pos;         /**< Position. */
   vec2 vel;         /**< Velocity. */
   double timer;     /**< Timer to de-spawn the gatherable. */
   double lifeleng;  /**< nb of seconds before de-spawn. */
   int quantity;     /**< Quantity of material. */
   int sx;           /**< X sprite to use. */
   int sy;           /**< Y sprite to use. */
} Gatherable;

/*
 * Init/cleanup.
 */
int gatherable_load (void);
void gatherable_cleanup (void);

/*
 * Gatherable objects
 */
int gatherable_init( const Commodity* com, vec2 pos, vec2 vel, double lifeleng, int qtt );
void gatherable_render( void );
int gatherable_getClosest( vec2 pos, double rad );
int gatherable_getPos( vec2* pos, vec2* vel, int id );
void gatherable_free( void );
void gatherable_update( double dt );
void gatherable_gather( int pilot );
