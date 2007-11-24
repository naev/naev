/*
 * See Licensing and Copyright notice in naev.h
 */



#include "economy.h"

#include <stdio.h>

#include "naev.h"


/*
 * converts credits to a usable string for displaying
 * str must have 10 characters alloced
 */
void credits2str( char *str, unsigned int credits )
{
	if (credits >= 1000000000)
		snprintf( str, 10, "%.2fB", (double)credits / 1000000000.);
	else if (credits >= 1000000)                
		snprintf( str, 10, "%.2fM", (double)credits / 1000000.);
	else if (credits >= 1000)              
		snprintf( str, 10, "%.2fK", (double)credits / 1000.);
	else snprintf (str, 10, "%d", credits );
}
