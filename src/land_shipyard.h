/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "ship.h"
#include "space.h"

/*
 * Window stuff.
 */
void shipyard_open( unsigned int wid );
void shipyard_update( unsigned int wid, const char* str );
void shipyard_cleanup (void);

/*
 * Helper functions.
 */
int shipyard_canBuy( const Ship *ship, const Spob *spob );
int shipyard_canTrade( const Ship *ship, const Spob *spob );
