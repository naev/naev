/*
 * Copyright 2006-2021 Naev DevTeam
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public Licence as published by
 * the Free Software Foundation, either version 3 of the Licence, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public Licence
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file naev.h
 *
 * @brief Header file with generic functions and Naev-specifics.
 */
#pragma once

/** @cond */
#include <math.h> // IWYU pragma: export

#include <SDL3/SDL_stdinc.h>
/** @endcond */

#define APPNAME "Naev" /**< Application name. */

#define ABS( x )                                                               \
   ( ( ( x ) < 0 ) ? -( x ) : ( x ) ) /**< Returns absolute value. */
#define FABS( x )                                                              \
   ( ( ( x ) < 0. ) ? -( x ) : ( x ) ) /**< Returns float absolute value. */

#define MAX( x, y )                                                            \
   ( ( ( x ) > ( y ) ) ? ( x ) : ( y ) ) /**< Returns maximum. */
#define MIN( x, y )                                                            \
   ( ( ( x ) > ( y ) ) ? ( y ) : ( x ) ) /**< Returns minimum. */
#define CLAMP( a, b, x )                                                       \
   ( ( x ) < ( a )                                                             \
        ? ( a )                                                                \
        : ( ( x ) > ( b )                                                      \
               ? ( b )                                                         \
               : ( x ) ) ) /**< Clamps x between a and b: a <= x <= b. */

#define SIGN( x )                                                              \
   ( ( ( x ) > 0 ) ? 1 : -1 ) /**< Returns the sign of a value. */
#define FSIGN( x )                                                             \
   ( ( ( x ) > 0. ) ? 1. : -1. ) /**< Returns the sign of a value. */

#define pow2( x ) ( ( x ) * ( x ) ) /**< ^2 */

/* maximum filename path */
#ifndef PATH_MAX
#define PATH_MAX 256 /**< If not already defined. */
#endif               /* PATH_MAX */

/* Default maximum string length */
#define STRMAX 4096
#define STRMAX_SHORT 1024

#define DOUBLE_TOL 1e-6

#include "nlua.h"

/*
 * Misc stuff.
 */
extern Uint32       SDL_LOOPDONE;
extern const double fps_min;
extern double       elapsed_time_mod;
void                fps_setPos( double x, double y );
void                fps_display( double dt );
double              fps_current( void );
void                naev_resize( void );
void                naev_toggleFullscreen( void );
void                update_routine( double dt, int dohooks );
const char         *naev_version( int long_version );
int                 naev_versionCompare( const char *version );
int    naev_versionCompareTarget( const char *version, const char *target );
int    naev_versionMatchReq( const char *version, const char *req );
void   naev_quit( void );
int    naev_isQuit( void );
double naev_getrealdt( void );
int    naev_event_resize( Uint32 type );

int       naev_shouldRenderLoadscreen( void );
void      naev_doRenderLoadscreen( void );
void      naev_renderLoadscreen( void );
void      print_SDLversion( void );
void      window_caption( void );
nlua_env *loadscreen_load( void );
void      loadscreen_unload( void );
void      loadscreen_update( double done, const char *msg );
void      load_all( void );
void      unload_all( void );
void      fps_init( void );
int       naev_main_events( void );
int       naev_main_cleanup( void );
int       naev_main_setup( void );
void      main_loop( int nested );
