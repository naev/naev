/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "pilot.h"

/*
 * Hooks.
 */
void pilot_addHook( Pilot *pilot, int type, unsigned int hook );
int  pilot_runHook( Pilot *p, int hook_type );
void pilots_rmHook( unsigned int hook );
void pilot_clearHooks( Pilot *p );

/*
 * Global hooks.
 */
void pilots_addGlobalHook( int type, unsigned int hook );
void pilots_rmGlobalHook( unsigned int hook );
void pilots_clearGlobalHooks( void );
void pilot_freeGlobalHooks( void );
