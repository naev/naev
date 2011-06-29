/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef LAND_SHIPYARD_H
#  define LAND_SHIPYARD_H


/*
 * Window stuff.
 */
void shipyard_open( unsigned int wid );
void shipyard_update( unsigned int wid, char* str );


/*
 * Helper functions.
 */
int shipyard_canBuy( char *shipname );
int shipyard_canTrade( char *shipname );


#endif /* LAND_SHIPYARD_H */
