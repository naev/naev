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

/**
 * @file log.c
 *
 * @brief Home of logprintf.
 */


#include "log.h"

#include "naev.h"

#include <stdio.h>
#include <stdarg.h>

#include "console.h"


#ifndef NOLOGPRINTFCONSOLE
/**
 * @brief Like fprintf but also prints to the naev console.
 */
int logprintf( FILE *stream, const char *fmt, ... )
{
   va_list ap;
   char buf[2048];

   if (fmt == NULL)
      return 0;
   else { /* get the message */
      va_start( ap, fmt );
      vsnprintf( &buf[2], sizeof(buf)-2, fmt, ap );
      va_end( ap );
   }

   /* Add to console. */
   if (stream == stderr) {
      buf[0] = '\e';
      buf[1] = 'r';
      cli_addMessage( buf );
   }
   else
      cli_addMessage( &buf[2] );

   /* Also print to the stream. */
   return fprintf( stream, "%s", &buf[2] );
}
#endif /* NOLOGPRINTFCONSOLE */

