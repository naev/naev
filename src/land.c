

#include "land.h"

#include "toolkit.h"


int landed = 0;

static int land_wid = 0;
static Planet* planet = NULL;


/*
 * lands the player
 */
void land( Planet* p )
{
	if (landed) return;

	planet = p;
	land_wid = window_create( -1, -1, 400, 300 );
	window_addButton( land_wid, 400-80-20, 20, 80, 40,
			"takeoff", "Takeoff", (void(*)(char*))takeoff );
	landed = 1;
}


/*
 * takes off the player
 */
void takeoff (void)
{
	if (!landed) return;

	planet = NULL;
	window_destroy( land_wid );
	landed = 0;
}

