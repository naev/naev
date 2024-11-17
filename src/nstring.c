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
 * @brief A bounded version of strstr. Conforms to BSD semantics.
 *
 *    @param haystack The string to search in
 *    @param size The size of haystack
 *    @param needle The string to search for
 *    @return A pointer to the first occurrence of needle in haystack, or NULL
 */
#if !HAVE_STRNSTR
char *strnstr( const char *haystack, const char *needle, size_t size )
{
   size_t      needlesize;
   const char *end, *giveup;

   needlesize = strlen( needle );
   /* We can give up if needle is empty, or haystack can never contain it */
   if ( needlesize == 0 || needlesize > size )
      return NULL;
   /* The pointer value that marks the end of haystack */
   end = haystack + size;
   /* The maximum value of i, because beyond this haystack cannot contain needle
    */
   giveup = end - needlesize + 1;

   /* i is used to iterate over haystack */
   for ( const char *i = haystack; i != giveup; i++ ) {
      const char *j, *k;
      /* j is used to iterate over part of haystack during comparison */
      /* k is used to iterate over needle during comparison */
      for ( j = i, k = needle; j != end && *k != '\0'; j++, k++ ) {
         /* Bail on the first character that doesn't match */
         if ( *j != *k )
            break;
      }
      /* If we've reached the end of needle, we've found a match */
      /* i contains the start of our match */
      if ( *k == '\0' )
         return (char *)i;
   }
   /* Fell through the loops, nothing found */
   return NULL;
}
#endif /* !HAVE_STRNSTR */

/**
 * @brief Return a pointer to a new string, which is a duplicate of the string
 * \p s (or, if necessary, which contains the first \p nn bytes of \p s plus a
 * terminating null).
 *
 * Taken from glibc. Conforms to POSIX.1-2008.
 */
#if !HAVE_STRNDUP
char *strndup( const char *s, size_t n )
{
   size_t len = MIN( strlen( s ), n );
   char *new  = (char *)malloc( len + 1 );
   if ( new == NULL )
      return NULL;
   new[len] = '\0';
   return (char *)memcpy( new, s, len );
}
#endif /* !HAVE_STRNDUP */

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
