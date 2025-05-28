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
