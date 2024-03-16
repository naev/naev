/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <signal.h>
#include <stdio.h>

#include "gettext.h"
/** @endcond */

#include "nstring.h"

#define LOG( str, ... ) ( logprintf( stdout, 1, str, ##__VA_ARGS__ ) )
#define LOGERR( str, ... ) ( logprintf( stderr, 1, str, ##__VA_ARGS__ ) )
#ifdef DEBUG_PARANOID /* Will cause WARNs to blow up */
#define WARN( str, ... )                                                       \
   ( logprintf( stderr, 0, _( "WARNING %s:%d [%s]: " ), __FILE__, __LINE__,    \
                __func__ ),                                                    \
     logprintf( stderr, 1, str, ##__VA_ARGS__ ), raise( SIGINT ) )
#else /* DEBUG_PARANOID */
#define WARN( str, ... )                                                       \
   ( logprintf( stderr, 0, _( "Warning: [%s] " ), __func__ ),                  \
     logprintf( stderr, 1, str, ##__VA_ARGS__ ) )
#endif /* DEBUG_PARANOID */
#define ERR( str, ... )                                                        \
   ( logprintf( stderr, 0, _( "ERROR %s:%d [%s]: " ), __FILE__, __LINE__,      \
                __func__ ),                                                    \
     logprintf( stderr, 1, str, ##__VA_ARGS__ ), abort() )
#ifdef DEBUG
#undef DEBUG
#define DEBUG( str, ... ) LOG( str, ##__VA_ARGS__ )
#ifndef DEBUGGING
#define DEBUGGING 1
#endif /* DEBUGGING */
#else  /* DEBUG */
#define DEBUG( str, ... )                                                      \
   do {                                                                        \
      ;                                                                        \
   } while ( 0 )
#endif /* DEBUG */
#define DEBUG_BLANK() DEBUG( "%s", "" )

PRINTF_FORMAT( 3, 4 )
NONNULL( 3 ) int logprintf( FILE *stream, int newline, const char *fmt, ... );
void log_init( void );
void log_redirect( void );
void log_clean( void );
