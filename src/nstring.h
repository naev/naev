/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NSTRING_H
#  define NSTRING_H


/** @cond */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/** @endcond */

#include "attributes.h"
#include "ncompat.h"

#ifdef __MINGW_PRINTF_FORMAT
   #define BUILTIN_PRINTF_FORMAT __MINGW_PRINTF_FORMAT
#else
   #define BUILTIN_PRINTF_FORMAT printf
#endif

/**
 * @brief A function which passes on a format string to vsnprintf or the like should have PRINTF_FORMAT( i, j )
 *        in front of its prototype, where i is the 1-based index of the format-string argument and j is the
 *        1-based index where the formatted items (if any) start.
 * @todo We should declare the fmt argument nonnull, but we have to rip out some null checks first. */
#define PRINTF_FORMAT( i, j ) FORMAT( BUILTIN_PRINTF_FORMAT, i, j) /*NONNULL( i )*/

const char *nstrnstr( const char *haystack, const char *needle, size_t size );
#if HAS_POSIX && defined(_GNU_SOURCE)
#define nstrcasestr     strcasestr
#define nstrndup        strndup
#else /* HAS_POSIX && defined(_GNU_SOURCE) */
const char *nstrcasestr( const char *haystack, const char *needle );
char* nstrndup( const char *s, size_t n );
#endif /* HAS_POSIX && defined(_GNU_SOURCE) */

#if HAS_SNPRINTF
#define nsnprintf       snprintf
#else /* HAS_SNPRINTF */
PRINTF_FORMAT( 3, 4 ) int nsnprintf( char *text, size_t maxlen, const char *fmt, ... );
#endif /* HAS_SNPRINTF */

int strsort( const void *p1, const void *p2 );


#endif /* NSTRING_H */

