/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nstd.c
 *
 * @brief Portable replacements for some functions.
 */


#include "nstd.h"


/**
 * @brief Checks to see if a key is alpha.
 *
 *    @param k Key to check.
 *    @return 1 if is alpha.
 */
int nstd_isalpha( SDLKey k )
{  
   int ret;
   
   ret = 0;
   
   /* Alpha. */
   if ((k >= SDLK_a) && (k <= SDLK_z))
      ret = 1;
   
   return ret;
}


/**
 * @brief like isalnum but for keysyms.
 *
 *    @param k Key to check.
 *    @return 1 if is alnum.
 */
int nstd_isalnum( SDLKey k )
{  
   int ret;

   ret = 0;

   /* Alpha. */
   if ((k >= SDLK_a) && (k <= SDLK_z))
      ret = 1;

   /* Number. */
   if ((k >= SDLK_0) && (k <= SDLK_9))
      ret = 1;

   return ret;
}


