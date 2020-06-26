/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef ECONOMY_H
#  define ECONOMY_H


#include "commodity.h"
//#include "space.h"


/*
 * Economy stuff.
 */
int economy_init (void);
void economy_addQueuedUpdate (void);
int economy_execQueued (void);
int economy_update( unsigned int dt );
int economy_refresh (void);
void economy_destroy (void);


/*
 * Calculating the sinusoidal economy values
 */
void economy_initialiseCommodityPrices(void);
int economy_getAveragePrice( const Commodity *com, credits_t *mean, double *std );


#endif /* ECONOMY_H */
