/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "commodity.h"
#include "ntime.h"

/*
 * Getting data.
 */
const char *start_name( void );
const char *start_ship( void );
const char *start_shipname( void );
const char *start_acquired( void );
const char *start_gui( void );
credits_t   start_credits( void );
ntime_t     start_date( void );
const char *start_system( void );
void        start_position( double *x, double *y );
const char *start_mission( void );
const char *start_event( void );
const char *start_chapter( void );
const char *start_spob_lua_default( void );
const char *start_dtype_default( void );
const char *start_local_map_default( void );
