/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/** @endcond */

#include "attributes.h"

#ifdef __MINGW_PRINTF_FORMAT
#define BUILTIN_PRINTF_FORMAT __MINGW_PRINTF_FORMAT
#else
#define BUILTIN_PRINTF_FORMAT printf
#endif

/**
 * @brief A function which passes on a format string to `vsnprintf` or the like
 * should have PRINTF_FORMAT( i, j ) in front of its prototype, where i is the
 * 1-based index of the format-string argument and j is the 1-based index where
 * the formatted items (if any) start. */
#define PRINTF_FORMAT( i, j ) FORMAT( BUILTIN_PRINTF_FORMAT, i, j )

PRINTF_FORMAT( 3, 4 )
int scnprintf( char *text, size_t maxlen, const char *fmt, ... );

int strsort( const void *p1, const void *p2 );
int strsort_reverse( const void *p1, const void *p2 );

#define NUM2STRLEN 16
int         num2str( char dest[NUM2STRLEN], double n, int decimals );
const char *num2strU( double n, int decimals );

void print_with_line_numbers( const char *str );
