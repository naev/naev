/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "commodity.h"

/*
 * Helper functions.
 */
void commodity_exchange_open( unsigned int wid );
void commodity_exchange_cleanup( void );
void commodity_update( unsigned int wid, const char *str );
void commodity_buy( unsigned int wid, const char *str );
void commodity_sell( unsigned int wid, const char *str );
int  commodity_canBuy( const Commodity *com );
int  commodity_canSell( const Commodity *com );
int  commodity_getMod( void );
void commodity_renderMod( double bx, double by, double w, double h,
                          void *data );
