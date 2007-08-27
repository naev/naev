

#include "land.h"

#include "toolkit.h"

/* global/main window */
#define LAND_WIDTH	700
#define LAND_HEIGHT	600
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 40

/* news window */
#define NEWS_WIDTH	400
#define NEWS_HEIGHT	400


int landed = 0;

static int land_wid = 0; /* used for the primary land window */
static int secondary_wid = 0; /* used for the second opened land window (can only have 2) */
static Planet* planet = NULL;


/*
 * prototypes
 */
static void commodity_exchange (void);
static void outfits (void);
static void shipyard (void);
static void spaceport_bar (void);
static void news (void);
static void news_close (char* str);


static void commodity_exchange (void)
{
}


static void outfits (void)
{
}


static void shipyard (void)
{
}


static void spaceport_bar (void)
{
}


static void news (void)
{
	secondary_wid = window_create( -1, -1, NEWS_WIDTH, NEWS_HEIGHT );

	/* window title */
	window_addText( secondary_wid, 0, -20, NEWS_WIDTH, 0, 1,
			"txtTitle", NULL, &cBlack, "News Reports" );

	window_addText( secondary_wid, 20, 20 + BUTTON_HEIGHT + 20,
			NEWS_WIDTH-40, NEWS_HEIGHT - 20 - BUTTON_HEIGHT - 20 - 20 - 20,
			0, "txtNews", &gl_smallFont, &cBlack,
			"News reporters report that they are on strike!");

	window_addButton( secondary_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseNews",
			"Close", news_close );

}
static void news_close (char* str)
{
	if (strcmp(str,"btnCloseNews")==0)
		window_destroy(secondary_wid);
}


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
	window_addText( land_wid, 0., -20, LAND_WIDTH, 0, 1,
			"txtPlanet", NULL, &cBlack, p->name );
	window_addImage( land_wid, 20, -440, "imgPlanet", p->gfx_exterior );
	window_addText( land_wid, 440, 80, 200, 460, 0, 
			"txtPlanetDesc", &gl_smallFont, &cBlack, p->description);


	/*
	 * buttons
	 */
	window_addButton( land_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnTakeoff",
			"Takeoff", (void(*)(char*))takeoff );
	if (planet_hasService(planet, PLANET_SERVICE_COMMODITY))
		window_addButton( land_wid, -20, 20 + BUTTON_HEIGHT + 20,
				BUTTON_WIDTH, BUTTON_HEIGHT, "btnCommodity",
				"Commodity Exchange", (void(*)(char*))commodity_exchange);

	if (planet_hasService(planet, PLANET_SERVICE_SHIPYARD))
		window_addButton( land_wid, -20 - BUTTON_WIDTH - 20, 20,
				BUTTON_WIDTH, BUTTON_HEIGHT, "btnShipyard",
				"Shipyard", (void(*)(char*))shipyard);
	if (planet_hasService(planet, PLANET_SERVICE_OUTFITS))
		window_addButton( land_wid, -20 - BUTTON_WIDTH - 20, 20 + BUTTON_HEIGHT + 20,
				BUTTON_WIDTH, BUTTON_HEIGHT, "btnOutfits",
				"Outfits", (void(*)(char*))outfits);

	if (planet_hasService(planet, PLANET_SERVICE_BASIC)) {
	window_addButton( land_wid, 20, 20,
		BUTTON_WIDTH, BUTTON_HEIGHT, "btnNews",
		"News", (void(*)(char*))news);
	window_addButton( land_wid, 20, 20 + BUTTON_HEIGHT + 20,
		BUTTON_WIDTH, BUTTON_HEIGHT, "btnBar",
		"Spaceport Bar", (void(*)(char*))spaceport_bar);
	}


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

