/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file ntime.c
 *
 * @brief Handles the Naev time.
 *
 * 1 cycle  = 5e3 periods = 50e6 seconds
 * 1 period = 10e3 seconds
 *
 * Generally displayed as:
 *  UST <cycles>:<periods>.<seconds>
 * The number of seconds digits can be variable, for example:
 *
 *  UST 630:3726.1
 *  UST 630:3726.12
 *  UST 630:3726.124
 *  UST 630:3726.1248
 *  UST 630:3726.12489
 *
 * Are all valid.
 *
 * Definitions / abbreviations:
 *    - UST: Universal Synchronized Time, the name of the time system.
 *    - seconds: Smallest named time unit. Equal to the Earth second.
 *    - periods: Most commonly used time unit. Periods are the new hours.
 *       1 period = 10,000 seconds (about 2.8 Earth hours).
 *    - cycles: Used for long-term time periods. 1 cycle = 5000 periods
 *       (about 579 Earth days).
 */


#include "ntime.h"

#include "naev.h"

#include <stdio.h>
#include "nstring.h"
#include <stdlib.h>

#include "hook.h"
#include "economy.h"


#define NT_SECONDS_DIV   (1000)      /* Divider for extracting seconds. */
#define NT_SECONDS_DT    (30)        /* Update rate, how many seconds are in a real second. */
#define NT_CYCLE_SECONDS   ((ntime_t)NT_CYCLE_PERIODS*(ntime_t)NT_PERIOD_SECONDS) /* Seconds in a cycle */
#define NT_PERIODS_DIV   ((ntime_t)NT_PERIOD_SECONDS*(ntime_t)NT_SECONDS_DIV) /* Divider for extracting periods. */
#define NT_CYCLES_DIV   ((ntime_t)NT_CYCLE_SECONDS*(ntime_t)NT_SECONDS_DIV) /* Divider for extracting cycles. */


/**
 * @brief Used for storing time increments to not trigger hooks during Lua
 *        calls and such.
 */
typedef struct NTimeUpdate_s {
   struct NTimeUpdate_s *next; /**< Next in the linked list. */
   ntime_t inc; /**< Time increment associated. */
} NTimeUpdate_t;
static NTimeUpdate_t *ntime_inclist = NULL; /**< Time increment list. */


static ntime_t naev_time = 0; /**< Contains the current time in milliseconds. */
static double naev_remainder = 0.; /**< Remainder when updating, to try to keep in perfect sync. */
static int ntime_enable = 1; /** Allow updates? */


/**
 * @brief Updatse the time based on realtime.
 */
void ntime_update( double dt )
{
   double dtt, tu;
   ntime_t inc;

   /* Only if we need to update. */
   if (!ntime_enable)
      return;

   /* Calculate the effective time. */
   dtt = naev_remainder + dt * NT_SECONDS_DT * NT_SECONDS_DIV;

   /* Time to update. */
   tu             = floor( dtt );
   inc            = (ntime_t) tu;
   naev_remainder = dtt - tu; /* Leave remainder. */

   /* Increment. */
   naev_time     += inc;
   hooks_updateDate( inc );
}


/**
 * @brief Creates a time structure.
 */
ntime_t ntime_create( int scu, int stp, int stu )
{
   ntime_t tscu, tstp, tstu;
   tscu = scu;
   tstp = stp;
   tstu = stu;
   return tscu*NT_CYCLES_DIV + tstp*NT_PERIODS_DIV + tstu*NT_SECONDS_DIV;
}


/**
 * @brief Gets the current time.
 *
 *    @return The current time in milliseconds.
 */
ntime_t ntime_get (void)
{
   return naev_time;
}


/**
 * @brief Gets the current time broken into individual components.
 */
void ntime_getR( int *cycles, int *periods, int *seconds, double *rem )
{
   *cycles = ntime_getCycles( naev_time );
   *periods = ntime_getPeriods( naev_time );
   *seconds = ntime_getSeconds( naev_time );
   *rem = ntime_getRemainder( naev_time ) + naev_remainder;
}


/**
 * @brief Gets the cycles of a time.
 */
int ntime_getCycles( ntime_t t )
{
   return (t / NT_CYCLES_DIV);
}


/**
 * @brief Gets the periods of a time.
 */
int ntime_getPeriods( ntime_t t )
{
   return (t / NT_PERIODS_DIV) % NT_CYCLE_PERIODS;
}


/**
 * @brief Gets the seconds of a time.
 */
int ntime_getSeconds( ntime_t t )
{
   return (t / NT_SECONDS_DIV) % NT_PERIOD_SECONDS;
}


/**
 * @brief Converts the time to seconds.
 *    @param t Time to convert.
 *    @return Time in seconds.
 */
double ntime_convertSeconds( ntime_t t )
{
   return ((double)t / (double)NT_SECONDS_DIV);
}


/**
 * @brief Gets the remainder.
 */
double ntime_getRemainder( ntime_t t )
{
   return (double)(t % NT_SECONDS_DIV);
}


/**
 * @brief Gets the time in a pretty human readable format.
 *
 *    @param t Time to print (in seconds), if 0 it'll use the current time.
 *    @param d Number of digits to use.
 *    @return The time in a human readable format (must free).
 */
char* ntime_pretty( ntime_t t, int d )
{
   char str[64];
   ntime_prettyBuf( str, sizeof(str), t, d );
   return strdup(str);
}


/**
 * @brief Gets the time in a pretty human readable format filling a preset buffer.
 *
 *    @param[out] str Buffer to use.
 *    @param max Maximum length of the buffer (recommended 64).
 *    @param t Time to print (in seconds), if 0 it'll use the current time.
 *    @param d Number of digits to use.
 *    @return The time in a human readable format (must free).
 */
void ntime_prettyBuf( char *str, int max, ntime_t t, int d )
{
   ntime_t nt;
   int cycles, periods, seconds;

   if (t==0)
      nt = naev_time;
   else
      nt = t;

   /* UST (Universal Synchronized Time) - unit is seconds */
   cycles = ntime_getCycles( nt );
   periods = ntime_getPeriods( nt );
   seconds = ntime_getSeconds( nt );
   if ((cycles == 0) && (periods == 0)) /* only seconds */
      nsnprintf( str, max, _("%04ds"), seconds );
   else if ((cycles == 0) || (d==0))
      nsnprintf( str, max, _("%.*fp"), d, periods + 0.0001 * seconds );
   else /* UST format */
      nsnprintf( str, max, _("UST %d:%.*f"), cycles, d, periods + 0.0001 * seconds );
}


/**
 * @brief Sets the time absolutely, does NOT generate an event, used at init.
 *
 *    @param t Absolute time to set to in seconds.
 */
void ntime_set( ntime_t t )
{
   naev_time      = t;
   naev_remainder = 0.;
}


/**
 * @brief Loads time including remainder.
 */
void ntime_setR( int cycles, int periods, int seconds, double rem )
{
   naev_time   = ntime_create( cycles, periods, seconds );
   naev_time  += floor(rem);
   naev_remainder = fmod( rem, 1. );
}


/**
 * @brief Sets the time relatively.
 *
 *    @param t Time modifier in seconds.
 */
void ntime_inc( ntime_t t )
{
   naev_time += t;

   /* Run hooks. */
   if (t > 0)
      hooks_updateDate( t );
}


/**
 * @brief Allows the time to update when the game is updating.
 *
 *    @param enable Whether or not to enable time updating.
 */
void ntime_allowUpdate( int enable )
{
   ntime_enable = enable;
}


/**
 * @brief Sets the time relatively.
 *
 * This does NOT call hooks and such, they must be run with ntime_refresh
 *  manually later.
 *
 *    @param t Time modifier in seconds.
 */
void ntime_incLagged( ntime_t t )
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

      /* Remove the increment. */
      ntime_inclist = ntu->next;

      /* Free the increment. */
      free(ntu);
   }
}

