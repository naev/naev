/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nmath.c
 *
 * @brief Some math routines for naev.
 */

#include "nstring.h"

#include "naev.h"

#include "log.h"


/**
 * @brief A bounded version of strstr
 *
 *    @param haystack The string to search in
 *    @param size The size of haystack
 *    @param needle The string to search for
 *    @return A pointer to the first occurrence of needle in haystack, or NULL
 */
const char *nstrnstr( const char *haystack, const char *needle, size_t size )
{
   size_t needlesize;
   const char *i, *j, *k, *end, *giveup;

   needlesize = strlen(needle);
   /* We can give up if needle is empty, or haystack can never contain it */
   if (needlesize == 0 || needlesize > size)
      return NULL;
   /* The pointer value that marks the end of haystack */
   end = haystack + size;
   /* The maximum value of i, because beyond this haystack cannot contain needle */
   giveup = end - needlesize + 1;

   /* i is used to iterate over haystack */
   for (i = haystack; i != giveup; i++) {
      /* j is used to iterate over part of haystack during comparison */
      /* k is used to iterate over needle during comparison */
      for (j = i, k = needle; j != end && *k != '\0'; j++, k++) {
         /* Bail on the first character that doesn't match */
         if (*j != *k)
            break;
      }
      /* If we've reached the end of needle, we've found a match */
      /* i contains the start of our match */
      if (*k == '\0')
         return i;
   }
   /* Fell through the loops, nothing found */
   return NULL;
}


/**
 * @brief Finds a string inside another string case insensitively.
 *
 *    @param haystack String to look into.
 *    @param needle String to find.
 *    @return Pointer in haystack where needle was found or NULL if not found.
 */
#if !(HAS_POSIX && defined(_GNU_SOURCE))
const char *nstrcasestr( const char *haystack, const char *needle )
{
   size_t hay_len, needle_len;

   /* Get lengths. */
   hay_len     = strlen(haystack);
   needle_len  = strlen(needle);

   /* Slow search. */
   while (hay_len >= needle_len) {
      if (strncasecmp(haystack, needle, needle_len) == 0)
         return haystack;

      haystack++;
      hay_len--;
   }

   return NULL;
}
#endif /* !(HAS_POSIX && defined(_GNU_SOURCE)) */


/**
 * @brief nsnprintf wrapper.
 */
#if !(HAS_POSIX && defined(_GNU_SOURCE))
int nsnprintf( char *text, size_t maxlen, const char *fmt, ... )
{
   va_list ap;
   int retval;

   va_start(ap, fmt);
   retval = vsnprintf(text, maxlen, fmt, ap);
   va_end(ap);

   /* mingw64 doesn't seem to want to null terminate stuff... */
   text[ maxlen-1 ] = '\0';

   return retval;
}
#endif /* !(HAS_POSIX && defined(_GNU_SOURCE)) */


