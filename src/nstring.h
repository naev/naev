/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NSTRING_H
#  define NSTRING_H


#include <stdlib.h>


const char *nstrnstr( const char *haystack, const char *needle, size_t size );
const char *nstrcasestr( const char *haystack, const char *needle );


#endif /* NSTRING_H */

