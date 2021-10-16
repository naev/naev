/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "land.h"

/*
 * Window stuff.
 */
void shipyard_open( unsigned int wid );
void shipyard_update( unsigned int wid, const char* str );
void shipyard_cleanup (void);

/*
 * Helper functions.
 */
int shipyard_canBuy( const char *shipname, Planet *planet );
int shipyard_canTrade( const char *shipname );
