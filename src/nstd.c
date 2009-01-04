/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nstd.c
 *
 * @brief Portable replacements for some functions.
 */


#include "nstd.h"


#define nstd_checkascii(k)       ((k & 0xff) == k) /**< Checks to see if k is in ascii area. */


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
 * @brief Checks to see if a key is alphanumeric.
 *
 *    @param k Key to check.
 *    @return 1 if is alphanumeric.
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


/**
 * @brief Checks to see if a key is a control character.
 *
 *    @param k Key to check.
 *    @return 1 if is a control character.
 */
int nstd_iscntrl( SDLKey k )
{
   return nstd_checkascii(k) ? iscntrl(k) : 0;
}

