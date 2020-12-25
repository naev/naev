/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef LAND_SHIPYARD_H
#  define LAND_SHIPYARD_H


#include "land.h"


/*
 * Window stuff.
 */
void shipyard_open( unsigned int wid );
void shipyard_update( unsigned int wid, char* str );
void shipyard_cleanup (void);


/*
 * Helper functions.
 */
int shipyard_canBuy( const char *shipname, Planet *planet );
int shipyard_canTrade( const char *shipname );


#endif /* LAND_SHIPYARD_H */
