/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file ntime.c
 *
 * @brief Handles the NAEV time.
 *
 * The basic unit of time is the STU.  There are 1000 STU in a MTU.  The time
 *  representation is generally UST which consists of MTU.STU.
 *
 * Acronyms:
 *    - MTU : Major Time Unit (1000 STU)
 *    - STU : Synchronized Time Unit
 *    - UST : Universal Synchronized Time
 */


#include "ntime.h"

#include <stdio.h>
#include <string.h>

#include "naev.h"
#include "hook.h"


static unsigned int naev_time = 0; /**< Contains the current time in mSTU. */



/**
 * @fn unsigned int ntime_get (void)
 *
 * @brief Gets the current time.
 */
unsigned int ntime_get (void)
{
   return naev_time;
}


/**
 * @fn char* ntime_pretty( unsigned int t )
 *
 * @brief Gets the time in a pretty human readable format.
 *
 *    @param t Time to print (in STU), if 0 it'll use the current time.
 *    @return The time in a human readable format (must free).
 */
char* ntime_pretty( unsigned int t )
{
   unsigned int nt;
   int mtu, stu;
   char str[128], *ret;

   if (t==0) nt = naev_time;
   else nt = t;

   /* UST (Universal Synchronized Time) - unit is STU (Synchronized Time Unit) */
   mtu = nt / (1000*NTIME_UNIT_LENGTH);
   stu = (nt / (NTIME_UNIT_LENGTH)) % 1000;
   if (mtu == 0) /* only STU */
      snprintf( str, 128, "%03d STU", stu );
   else /* UST format */
      snprintf( str, 128, "UST %d.%03d", mtu, stu );

   ret = strdup(str);

   return ret;
}


/**
 * @fn void ntime_set( unsigned int t )
 *
 * @brief Sets the time absolutely, does NOT generate an event, used at init.
 *
 *    @param t Absolute time to set to in STU.
 */
void ntime_set( unsigned int t )
{
   naev_time = t;
}


/**
 * @fn void ntime_inc( unsigned int t )
 *
 * @brief Sets the time relatively.
 *
 *    @param t Time modifier in STU.
 */
void ntime_inc( unsigned int t )
{
   naev_time += t;

   hooks_run("time");
}



