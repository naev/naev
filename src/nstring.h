/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NSTRING_H
#  define NSTRING_H


#include <stdlib.h>
#include <string.h>

#include "attributes.h"
#include "ncompat.h"


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

