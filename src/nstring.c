/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nstring.c
 *
 * @brief Some string routines for naev.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "nstring.h"

#include "log.h"

/**
 * @brief Sort function for sorting strings with qsort().
 */
int strsort( const void *p1, const void *p2 )
{
   return strcmp( *(const char **)p1, *(const char **)p2 );
}

/**
 * @brief Order-reversed version of strsort().
 */
int strsort_reverse( const void *p1, const void *p2 )
{
   return strsort( p2, p1 );
}

/**
 * @brief Like snprintf(), but returns the number of characters \em ACTUALLY
 * "printed" into the buffer. This makes it possible to chain these calls to
 * concatenate into a buffer without introducing a potential bug every time.
 *        This call was first added to the Linux kernel by Juergen Quade.
 */
int scnprintf( char *text, size_t maxlen, const char *fmt, ... )
{
   int     n;
   va_list ap;

   if ( !maxlen )
      return 0;

   va_start( ap, fmt );
   n = vsnprintf( text, maxlen, fmt, ap );
   va_end( ap );
   return MIN( maxlen - 1, (size_t)n );
}

/**
 * @brief Converts a numeric value to a string.
 *
 *    @param[out] dest String to write to.
 *    @param n Number to write.
 *    @param decimals Number of decimals to write.
 */
int num2str( char dest[NUM2STRLEN], double n, int decimals )
{
   /* Don't use decimals if not necessary. */
   if ( fabs( fmod( n, 1. ) ) < 1e-3 )
      decimals = 0;

   if ( n >= 1e15 )
      return snprintf( dest, NUM2STRLEN, "%.*f", decimals, n );
   else if ( n >= 1e12 )
      return snprintf(
         dest, NUM2STRLEN, _( "%.0f,%03.0f,%03.0f,%03.0f,%03.*f" ),
         floor( n / 1e12 ), floor( fmod( floor( fabs( n / 1e9 ) ), 1e3 ) ),
         floor( fmod( floor( fabs( n / 1e6 ) ), 1e3 ) ),
         floor( fmod( floor( fabs( n / 1e3 ) ), 1e3 ) ), decimals,
         fmod( floor( fabs( n ) ), 1e3 ) );
   else if ( n >= 1e9 )
      return snprintf( dest, NUM2STRLEN, _( "%.0f,%03.0f,%03.0f,%03.*f" ),
                       floor( n / 1e9 ),
                       floor( fmod( floor( fabs( n / 1e6 ) ), 1e3 ) ),
                       floor( fmod( floor( fabs( n / 1e3 ) ), 1e3 ) ), decimals,
                       fmod( floor( fabs( n ) ), 1e3 ) );
   else if ( n >= 1e6 )
      return snprintf( dest, NUM2STRLEN, _( "%.0f,%03.0f,%03.*f" ),
                       floor( n / 1e6 ),
                       floor( fmod( floor( fabs( n / 1e3 ) ), 1e3 ) ), decimals,
                       fmod( floor( fabs( n ) ), 1e3 ) );
   else if ( n >= 1e3 )
      return snprintf( dest, NUM2STRLEN, _( "%.0f,%03.*f" ), floor( n / 1e3 ),
                       decimals, fmod( floor( fabs( n ) ), 1e3 ) );
   return snprintf( dest, NUM2STRLEN, "%.*f", decimals, n );
}

/**
 * @brief Unsafe version of num2str that uses an internal buffer. Every call
 * overwrites the return value.
 *
 *    @param n Number to write.
 *    @param decimals Number of decimals to write.
 *    @return Fancy string number.
 */
const char *num2strU( double n, int decimals )
{
   static char num2strU_buf[NUM2STRLEN];
   num2str( num2strU_buf, n, decimals );
   return num2strU_buf;
}

/**
 * @brief Prints to stderr with line numbers.
 *
 *    @param str String to print.
 */
void print_with_line_numbers( const char *str )
{
   int counter = 0;
   logprintf( stderr, 0, "%03d: ", ++counter );
   for ( int i = 0; str[i] != '\0'; i++ ) {
      if ( str[i] == '\n' )
         logprintf( stderr, 0, "\n%03d: ", ++counter );
      else // if (str[i]!='\n')
         logprintf( stderr, 0, "%c", str[i] );
   }
   logprintf( stderr, 0, "\n" );
}
