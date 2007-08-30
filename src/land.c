

#include "land.h"

#include "toolkit.h"
#include "player.h"
#include "rng.h"
#include "music.h"


/* global/main window */
#define LAND_WIDTH	700
#define LAND_HEIGHT	600
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 40

/* commodity window */
#define COMMODITY_WIDTH 400
#define COMMODITY_HEIGHT 400

/* news window */
#define NEWS_WIDTH	400
#define NEWS_HEIGHT	400

/* bar window */
#define BAR_WIDTH		600
#define BAR_HEIGHT	400


#define MUSIC_TAKEOFF	"liftoff"
#define MUSIC_LAND		"agriculture"


int landed = 0;

static int land_wid = 0; /* used for the primary land window */
static int secondary_wid = 0; /* used for the second opened land window (can only have 2) */
static Planet* planet = NULL;


/*
 * prototypes
 */
static void commodity_exchange (void);
static void commodity_exchange_close( char* str );
static void outfits (void);
static void shipyard (void);
static void spaceport_bar (void);
static void spaceport_bar_close( char* str );
static void news (void);
static void news_close( char* str );


/*
 * the local market
 */
static void commodity_exchange (void)
{
	secondary_wid = window_create( "Commodity Exchange",
			-1, -1, COMMODITY_WIDTH, COMMODITY_HEIGHT );

	window_addButton( secondary_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnCommodityClose",
			"Close", commodity_exchange_close );
}
static void commodity_exchange_close( char* str )
{
	if (strcmp(str, "btnCommodityClose")==0)
		window_destroy(secondary_wid);
}


static void outfits (void)
{
}


static void shipyard (void)
{
}


/*
 * the spaceport bar
 */
static void spaceport_bar (void)
{
	secondary_wid = window_create( "Spaceport Bar",
			-1, -1, BAR_WIDTH, BAR_HEIGHT );

	window_addText( secondary_wid, 20, 20 + BUTTON_HEIGHT + 20,
			BAR_WIDTH - 140, BAR_HEIGHT - 40 - BUTTON_HEIGHT - 60, 0,
			"txtDescription", &gl_smallFont, &cBlack, planet->bar_description );
	
	window_addButton( secondary_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseBar",
			"Close", spaceport_bar_close );
}
static void spaceport_bar_close( char* str )
{
	if (strcmp(str,"btnCloseBar")==0)
		window_destroy(secondary_wid);
}



/*
 * the planetary news reports
 */
static void news (void)
{
	secondary_wid = window_create( "News Reports",
			-1, -1, NEWS_WIDTH, NEWS_HEIGHT );

	window_addText( secondary_wid, 20, 20 + BUTTON_HEIGHT + 20,
			NEWS_WIDTH-40, NEWS_HEIGHT - 20 - BUTTON_HEIGHT - 20 - 20 - 20,
			0, "txtNews", &gl_smallFont, &cBlack,
			"News reporters report that they are on strike!");

	window_addButton( secondary_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseNews",
			"Close", news_close );

}
static void news_close( char* str )
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

	/* change music */
	music_load( MUSIC_LAND );
	music_play();

	planet = p;
	land_wid = window_create( p->name, -1, -1, LAND_WIDTH, LAND_HEIGHT );
	
	/*
	 * pretty display
	 */
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

	music_load( MUSIC_TAKEOFF );
	music_play();

	int sw, sh;
	sw = planet->gfx_space->w;
	sh = planet->gfx_space->h;

	/* set player to another position with random facing direction and no vel */
	player_warp( planet->pos.x + RNG(-sw/2,sw/2), planet->pos.y + RNG(-sh/2,sh/2) );
	vect_pset( &player->solid->vel, 0., 0. );
	player->solid->dir = RNG(0,359) * M_PI/180.;

	space_init(NULL);

	planet = NULL;
	window_destroy( land_wid );
	landed = 0;
}

