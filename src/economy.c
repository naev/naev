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
void credits2str( char *str, unsigned int credits, int decimals )
{
	if (decimals < 0)
		snprintf( str, 32, "%d", credits );
	else if (credits >= 1000000000)
		snprintf( str, 16, "%.*fB", decimals, (double)credits / 1000000000. );
	else if (credits >= 1000000)                
		snprintf( str, 16, "%.*fM", decimals, (double)credits / 1000000. );
	else if (credits >= 1000)              
		snprintf( str, 16, "%.*fK", decimals, (double)credits / 1000. );
	else snprintf (str, 16, "%d", credits );
}
