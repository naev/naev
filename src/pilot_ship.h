/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "pilot.h"

int pilot_shipLInit( Pilot *pilot );
int pilot_shipLCleanup( Pilot *pilot );
int pilot_shipLUpdate( Pilot *pilot, double dt );
int pilot_shipLExplodeInit( Pilot *pilot );
int pilot_shipLExplodeUpdate( Pilot *pilot, double dt );
