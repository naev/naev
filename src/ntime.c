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

#include "naev.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hook.h"
#include "economy.h"


/**
 * @brief Used for storing time increments to not trigger hooks during Lua
 *        calls and such.
 */
typedef struct NTimeUpdate_s {
   struct NTimeUpdate_s *next; /**< Next in the linked list. */
   unsigned int inc; /**< Time increment assosciated. */
} NTimeUpdate_t;
static NTimeUpdate_t *ntime_inclist = NULL; /**< Time increment list. */


static unsigned int naev_time = 0; /**< Contains the current time in mSTU. */


/**
 * @brief Gets the current time.
 *
 *    @return The current time in mSTU.
 */
unsigned int ntime_get (void)
{
   return naev_time;
}


/**
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
 * @brief Sets the time absolutely, does NOT generate an event, used at init.
 *
 *    @param t Absolute time to set to in STU.
 */
void ntime_set( unsigned int t )
{
   naev_time = t;
}


/**
 * @brief Sets the time relatively.
 *
 *    @param t Time modifier in STU.
 */
void ntime_inc( unsigned int t )
{
   naev_time += t;
   hooks_run("time");
   economy_update( t );
}


/**
 * @brief Sets the time relatively.
 *
 * This does NOT call hooks and such, they must be run with ntime_refresh
 *  manually later.
 *
 *    @param t Time modifier in STU.
 */
void ntime_incLagged( unsigned int t )
{
   NTimeUpdate_t *ntu, *iter;

   /* Create the time increment. */
   ntu = malloc(sizeof(NTimeUpdate_t));
   ntu->next = NULL;
   ntu->inc = t;

   /* Only member. */
   if (ntime_inclist == NULL)
      ntime_inclist = ntu;

   else {
      /* Find end of list. */
      for (iter = ntime_inclist; iter->next != NULL; iter = iter->next);
      /* Append to end. */
      iter->next = ntu;
   }
}


/**
 * @brief Checks to see if ntime has any hooks pending to run.
 */
void ntime_refresh (void)
{
   NTimeUpdate_t *ntu;

   /* We have to run all the increments one by one to ensure all hooks get
    * run and that no collisions occur. */
   while (ntime_inclist != NULL) {
      ntu = ntime_inclist;

      /* Run hook stuff and actually update time. */
      naev_time += ntu->inc;
      hooks_run("time");
      economy_update( ntu->inc );

      /* Remove the increment. */
      ntime_inclist = ntu->next;

      /* Free the increment. */
      free(ntu);
   }
}

