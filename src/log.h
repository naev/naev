/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#define LOGMAX 1024

/** @cond */
#include <signal.h>
#include <stdio.h>

#include "gettext.h" // IWYU pragma: keep
/** @endcond */

#include "nstring.h"

#define LOG( str, ... )                                                        \
   do {                                                                        \
      char _LOGBUF[LOGMAX];                                                    \
      snprintf( _LOGBUF, sizeof( _LOGBUF ), str, ##__VA_ARGS__ );              \
      info_rust( _LOGBUF );                                                    \
   } while ( 0 )
#define LOGERR LOG
#define WARN( str, ... )                                                       \
   do {                                                                        \
      char _LOGBUF[LOGMAX];                                                    \
      snprintf( _LOGBUF, sizeof( _LOGBUF ), str, ##__VA_ARGS__ );              \
      warn_rust( _LOGBUF );                                                    \
   } while ( 0 )
#define ERR( str, ... )                                                        \
   do {                                                                        \
      char _LOGBUF[LOGMAX];                                                    \
      snprintf( _LOGBUF, sizeof( _LOGBUF ), str, ##__VA_ARGS__ );              \
      warn_rust( _LOGBUF );                                                    \
      abort();                                                                 \
   } while ( 0 )
#ifdef DEBUG
#undef DEBUG
#define DEBUG( str, ... )                                                      \
   do {                                                                        \
      char _LOGBUF[LOGMAX];                                                    \
      snprintf( _LOGBUF, sizeof( _LOGBUF ), str, ##__VA_ARGS__ );              \
      debug_rust( _LOGBUF );                                                   \
   } while ( 0 )
#ifndef DEBUGGING
#define DEBUGGING 1
#endif /* DEBUGGING */
#else  /* DEBUG */
#define DEBUG( str, ... )                                                      \
   do {                                                                        \
   } while ( 0 )
#endif /* DEBUG */
#define DEBUG_BLANK() DEBUG( "%s", "" )

PRINTF_FORMAT( 3, 4 )
NONNULL( 3 ) int logprintf( FILE *stream, int newline, const char *fmt, ... );
void log_init( void );
void log_redirect( void );
void log_clean( void );
int  log_warn( const char *file, size_t line, const char *func, const char *fmt,
               ... );

// From Rust
void debug_rust( const char *msg );
void info_rust( const char *msg );
void warn_rust( const char *msg );
