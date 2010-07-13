/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef SHIPYARD_H
#  define SHIPYARD_H


/*
 * Window stuff.
 */
void shipyard_open( unsigned int wid );
void shipyard_update( unsigned int wid, char* str );
void shipyard_buy( unsigned int wid, char* str );
void shipyard_trade( unsigned int wid, char* str );
void shipyard_rmouse( unsigned int wid, char* widget_name );


/*
 * Helpe functions.
 */
int shipyard_canBuy( char *shipname );
int shipyard_canTrade( char *shipname );


#endif /* SHIPYARD_H */
