/*
 * See Licensing and Copyright notice in naev.h
 */
#ifndef LAND_TRADE_H
#  define LAND_TRADE_H

#include "land.h"
#include "land_takeoff.h"

/*
 * Helper functions.
 */
void commodity_exchange_open( unsigned int wid );
void commodity_update( unsigned int wid, const char *str );
void commodity_buy( unsigned int wid, const char *str );
void commodity_sell( unsigned int wid, const char *str );
int  commodity_canBuy( const Commodity* com );
int  commodity_canSell( const Commodity* com );
int commodity_getMod (void);
void commodity_renderMod( double bx, double by, double w, double h, void *data );

#endif /* LAND_TRADE_H */
