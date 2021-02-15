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
 *        1-based index where the formatted items (if any) start. */
#define PRINTF_FORMAT( i, j ) FORMAT( BUILTIN_PRINTF_FORMAT, i, j)

const char *nstrnstr( const char *haystack, const char *needle, size_t size );
/*const*/ char *strcasestr( const char *haystack, const char *needle );
#if HAS_POSIX && defined(_GNU_SOURCE)
#define nstrndup        strndup
#else /* HAS_POSIX && defined(_GNU_SOURCE) */
char* nstrndup( const char *s, size_t n );
#endif /* HAS_POSIX && defined(_GNU_SOURCE) */

int strsort( const void *p1, const void *p2 );


#endif /* NSTRING_H */

