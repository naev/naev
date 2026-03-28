/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "space.h"

/*
 * Economy stuff.
 */
int  economy_init( void );
void economy_addQueuedUpdate( void );
int  economy_execQueued( void );
int  economy_update( unsigned int dt );
int  economy_refresh( void );
void economy_destroy( void );
void economy_clearKnown( void );
void economy_clearSingleSpob( Spob *p );

/*
 * Price stuff.
 */
int  economy_getAverageSpobPrice( CommodityRef com, const Spob *p,
                                  credits_t *mean, double *std );
void economy_averageSeenPrices( const Spob *p );
void economy_averageSeenPricesAtTime( const Spob *p, const ntime_t tupdate );
credits_t economy_getPrice( CommodityRef com, const StarSystem *sys,
                            const Spob *p );
credits_t economy_getPriceAtTime( CommodityRef com, const StarSystem *sys,
                                  const Spob *p, ntime_t t );

/*
 * Calculating the sinusoidal economy values
 */
void economy_initialiseCommodityPrices( void );
int  economy_getAveragePrice( CommodityRef com, credits_t *mean, double *std );
void economy_initialiseSingleSystem( StarSystem *sys, Spob *spob );
