/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "attributes.h"

int diff_loadAvailable( void );
NONNULL( 1 ) int diff_apply( const char *name );
NONNULL( 1 ) void diff_remove( const char *name );
void diff_clear( void );
void diff_free( void );
NONNULL( 1 ) int diff_isApplied( const char *name );
void unidiff_universeDefer( int enable );
