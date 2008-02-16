/*
 * See Licensing and Copyright notice in naev.h
 */



#include "ntime.h"

#include <stdio.h>
#include <string.h>

#include "naev.h"
#include "hook.h"


static unsigned int naev_time = 0;



/*
 * gets the current time
 */
unsigned int ntime_get (void)
{
   return naev_time;
}


/*
 * returns the time in pretty text
 */
char* ntime_pretty( unsigned int t )
{
   unsigned int nt;
   char str[128], *ret;

   if (t==0) nt = naev_time;
   else nt = t;

   /* Galactic Date */
   snprintf( str, 128, "GD %d.%03d",
         nt / (1000*NTIME_UNIT_LENGTH),
         (nt / (NTIME_UNIT_LENGTH)) % 1000);

   ret = strdup(str);

   return ret;
}


/*
 * sets the time absolutely, does NOT generate an event, used at init
 */
void ntime_set( unsigned int t )
{
   naev_time = t;
}


/*
 * sets the time relatively
 */
void ntime_inc( unsigned int t )
{
   naev_time += t;

   hooks_run("time");
}



