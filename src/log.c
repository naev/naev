/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file log.c
 *
 * @brief Home of loprintf.
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

