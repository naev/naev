/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef NSTRING_H
#  define NSTRING_H


#include <stdlib.h>
#include <string.h>

#include <ncompat.h>


const char *nstrnstr( const char *haystack, const char *needle, size_t size );
#if HAS_POSIX && defined(_GNU_SOURCE)
#define nstrcasestr     strcasestr
#define nsnprintf       snprintf
#else /* HAVE_POSIX && defined(_GNU_SOURCE) */
const char *nstrcasestr( const char *haystack, const char *needle );
int nsnprintf( char *text, size_t maxlen, const char *fmt, ... );
#endif /* HAVE_POSIX */


#endif /* NSTRING_H */

