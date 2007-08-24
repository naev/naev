

#include "land.h"

#include "toolkit.h"


#define LAND_WIDTH	500
#define LAND_HEIGHT	400
#define BUTTON_WIDTH  80
#define BUTTON_HEIGHT 40


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
	land_wid = window_create( -1, -1, LAND_WIDTH, LAND_HEIGHT );

	/*
	 * pretty display
	 */
	window_addText( land_wid, 0., -20., LAND_WIDTH, 1, p->name, NULL, &cBlack );

	/*
	 * buttons
	 */
	window_addButton( land_wid, -20., 20.,
			BUTTON_WIDTH, BUTTON_HEIGHT, "takeoff",
			"Takeoff", (void(*)(char*))takeoff );
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

