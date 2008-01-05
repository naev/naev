/*
 * See Licensing and Copyright notice in naev.h
 */



#include "land.h"

#include "naev.h"
#include "log.h"
#include "toolkit.h"
#include "player.h"
#include "rng.h"
#include "music.h"
#include "economy.h"


/* global/main window */
#define LAND_WIDTH	700
#define LAND_HEIGHT	600
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 40

/* commodity window */
#define COMMODITY_WIDTH 400
#define COMMODITY_HEIGHT 400

/* outfits */
#define OUTFITS_WIDTH	700
#define OUTFITS_HEIGHT	600

/* shipyard */
#define SHIPYARD_WIDTH	700
#define SHIPYARD_HEIGHT	600

/* news window */
#define NEWS_WIDTH	400
#define NEWS_HEIGHT	500

/* bar window */
#define BAR_WIDTH		460
#define BAR_HEIGHT	300


#define MUSIC_TAKEOFF	"liftoff"
#define MUSIC_LAND		"agriculture"


int landed = 0;

static int land_wid = 0; /* used for the primary land window */
static int secondary_wid = 0; /* used for the second opened land window */
static int terciary_wid = 0; /* used for fancy things like news, your ships... */
Planet* land_planet = NULL;


/*
 * prototypes
 */
/*
 * extern
 */
extern char** player_ships( int *nships );
extern Pilot* player_getShip( char* shipname );
extern char* player_getLoc( char* shipname );
/*
 * static
 */
/* commodity exchange */
static void commodity_exchange (void);
static void commodity_exchange_close( char* str );
static void commodity_update( char* str );
static void commodity_buy( char* str );
static void commodity_sell( char* str );
/* outfits */
static void outfits (void);
static void outfits_close( char* str );
static void outfits_update( char* str );
static void outfits_buy( char* str );
static void outfits_sell( char* str );
static int outfits_getMod (void);
static void outfits_renderMod( double bx, double by, double w, double h );
/* shipyard */
static void shipyard (void);
static void shipyard_close( char* str );
static void shipyard_update( char* str );
static void shipyard_info( char* str );
static void shipyard_buy( char* str );
/* your ships */
static void shipyard_yours( char* str );
static void shipyard_yoursClose( char* str );
static void shipyard_yoursUpdate( char* str );
/* spaceport bar */
static void spaceport_bar (void);
static void spaceport_bar_close( char* str );
/* news */
static void news (void);
static void news_close( char* str );


/*
 * the local market
 */
static void commodity_exchange (void)
{
	int i;
	char **goods;

	/* window */
	secondary_wid = window_create( "Commodity Exchange",
			-1, -1, COMMODITY_WIDTH, COMMODITY_HEIGHT );

	/* buttons */
	window_addButton( secondary_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnCommodityClose",
			"Close", commodity_exchange_close );
	window_addButton( secondary_wid, -40-((BUTTON_WIDTH-20)/2), 20*2 + BUTTON_HEIGHT,
			(BUTTON_WIDTH-20)/2, BUTTON_HEIGHT, "btnCommodityBuy",
			"Buy", commodity_buy );
	window_addButton( secondary_wid, -20, 20*2 + BUTTON_HEIGHT,
			(BUTTON_WIDTH-20)/2, BUTTON_HEIGHT, "btnCommoditySell",
			"Sell", commodity_sell );

	/* text */
	window_addText( secondary_wid, -20, -40, BUTTON_WIDTH, 60, 0,
			"txtSInfo", &gl_smallFont, &cDConsole, 
			"You have:\n"
			"Market price:\n"
			"\n"
			"Free Space:\n" );
	window_addText( secondary_wid, -20, -40, BUTTON_WIDTH/2, 60, 0,
			"txtDInfo", &gl_smallFont, &cBlack, NULL );
	window_addText( secondary_wid, -40, -100, BUTTON_WIDTH-20,
			BUTTON_WIDTH, 0, "txtDesc", &gl_smallFont, &cBlack, NULL );

	/* goods list */
	goods = malloc(sizeof(char*) * land_planet->ncommodities);
	for (i=0; i<land_planet->ncommodities; i++)
		goods[i] = strdup(land_planet->commodities[i]->name);
	window_addList( secondary_wid, 20, -40,
			COMMODITY_WIDTH-BUTTON_WIDTH-60, COMMODITY_HEIGHT-80-BUTTON_HEIGHT,
			"lstGoods", goods, land_planet->ncommodities, 0, commodity_update );

	/* update */
	commodity_update(NULL);
}
static void commodity_exchange_close( char* str )
{
	if (strcmp(str, "btnCommodityClose")==0)
		window_destroy(secondary_wid);
}
static void commodity_update( char* str )
{
	(void)str;
	char buf[128];
	char *comname;
	Commodity *com;

	comname = toolkit_getList( secondary_wid, "lstGoods" );
	com = commodity_get( comname );

	/* modify text */
	snprintf( buf, 128,
			"%d tons\n"
			"%d credits/ton\n"
			"\n"
			"%d tons\n",
			player_cargoOwned( comname ),
			com->medium,
			player->cargo_free);
	window_modifyText( secondary_wid, "txtDInfo", buf );
	window_modifyText( secondary_wid, "txtDesc", com->description );

}
static void commodity_buy( char* str )
{
	(void)str;
	char *comname;
	Commodity *com;
	int q;

	q = 10;
	comname = toolkit_getList( secondary_wid, "lstGoods" );
	com = commodity_get( comname );

	if (player_credits <= q * com->medium) {
		toolkit_alert( "Not enough credits!" );
		return;
	}
	else if (player->cargo_free <= 0) {
		toolkit_alert( "Not enough free space!" );
		return;
	}

	q = pilot_addCargo( player, com, q );
	player_credits -= q * com->medium;
	commodity_update(NULL);
}
static void commodity_sell( char* str )
{
	(void)str;
	char *comname;
	Commodity *com;
	int q;

	q = 10;
	comname = toolkit_getList( secondary_wid, "lstGoods" );
	com = commodity_get( comname );

	q = pilot_rmCargo( player, com, q );
	player_credits += q * com->medium;
	commodity_update(NULL);
}


/*
 * ze outfits
 */
static void outfits (void)
{
	char **outfits;
	int noutfits;
	char buf[128];

	/* create window */
	snprintf(buf,128,"%s - Outfits", land_planet->name );
	secondary_wid = window_create( buf, -1, -1,
			OUTFITS_WIDTH, OUTFITS_HEIGHT );
	window_setFptr( secondary_wid, outfits_buy ); /* will allow buying from keyboard */

	/* buttons */
	window_addButton( secondary_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseOutfits",
			"Close", outfits_close );
	window_addButton( secondary_wid, -40-BUTTON_WIDTH, 40+BUTTON_HEIGHT,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnBuyOutfit",
			"Buy", outfits_buy );
	window_addButton( secondary_wid, -40-BUTTON_WIDTH, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnSellOutfit",
			"Sell", outfits_sell );

	/* fancy 128x128 image */
	window_addRect( secondary_wid, -20, -50, 128, 128, "rctImage", &cBlack, 0 );
	window_addImage( secondary_wid, -20-128, -50-128, "imgOutfit", NULL, 1 );

	/* cust draws the modifier */
	window_addCust( secondary_wid, -40-BUTTON_WIDTH, 60+2*BUTTON_HEIGHT,
			BUTTON_WIDTH, BUTTON_HEIGHT, "cstMod", 0, outfits_renderMod, NULL );

	/* the descriptive text */
	window_addText( secondary_wid, 40+200+20, -60,
			80, 96, 0, "txtSDesc", &gl_smallFont, &cDConsole,
			"Name:\n"
			"Type:\n"
			"Owned:\n"
			"\n"
			"Space taken:\n"
			"Free Space:\n"
			"\n"
			"Price:\n"
			"Money:\n" );
	window_addText( secondary_wid, 40+200+40+60, -60,
			250, 96, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );
	window_addText( secondary_wid, 20+200+40, -200,
			OUTFITS_WIDTH-300, 200, 0, "txtDescription",
			&gl_smallFont, NULL, NULL );

	/* set up the outfits to buy/sell */
	outfits = outfit_getTech( &noutfits, land_planet->tech, PLANET_TECH_MAX);
	window_addList( secondary_wid, 20, 40,
			200, OUTFITS_HEIGHT-80, "lstOutfits",
			outfits, noutfits, 0, outfits_update );

	/* write the outfits stuff */
	outfits_update( NULL );
}
static void outfits_close( char* str )
{
	if (strcmp(str,"btnCloseOutfits")==0)
		window_destroy(secondary_wid);
}
static void outfits_update( char* str )
{
	(void)str;
	char *outfitname;
	Outfit* outfit;
	char buf[80], buf2[16], buf3[16];

	outfitname = toolkit_getList( secondary_wid, "lstOutfits" );
	outfit = outfit_get( outfitname );

	/* new image */
	window_modifyImage( secondary_wid, "imgOutfit", outfit->gfx_store );

	/* new text */
	window_modifyText( secondary_wid, "txtDescription", outfit->description );
	credits2str( buf2, outfit->price, 2 );
	credits2str( buf3, player_credits, 2 );
	snprintf( buf, 80,
			"%s\n"
			"%s\n"
			"%d\n"
			"\n"
			"%d\n"
			"%d\n"
			"\n"
			"%s credits\n"
			"%s credits\n",
			outfit->name,
			outfit_getType(outfit),
			player_outfitOwned(outfitname),
			outfit->mass,
			pilot_freeSpace(player),
			buf2,
			buf3 );
	window_modifyText( secondary_wid,  "txtDDesc", buf );
}
static void outfits_buy( char* str )
{
	(void)str;
	char *outfitname;
	Outfit* outfit;
	int q;
	char buf[16];

	outfitname = toolkit_getList( secondary_wid, "lstOutfits" );
	outfit = outfit_get( outfitname );

	q = outfits_getMod();

	/* can player actually fit the outfit? */
	if ((pilot_freeSpace(player) - outfit->mass) < 0) {
		toolkit_alert( "Not enough free space (you need %d more).",
				outfit->mass - pilot_freeSpace(player) );
		return;
	}
	/* has too many already */
	else if (player_outfitOwned(outfitname) >= outfit->max) {
		toolkit_alert( "You can only carry %d of this outfit.",
				outfit->max );
		return;
	}
	/* can only have one afterburner */
	else if (outfit_isAfterburner(outfit) && (player->afterburner!=NULL)) {
		toolkit_alert( "You can only have one afterburner." );
		return;
	}
	/* not enough $$ */
	else if (q*(int)outfit->price >= player_credits) {
		credits2str( buf, q*outfit->price - player_credits, 2 );
		toolkit_alert( "You need %s more credits.", buf);
		return;
	}

	player_credits -= outfit->price * pilot_addOutfit( player, outfit,
			MIN(q,outfit->max) );
	outfits_update(NULL);
}
static void outfits_sell( char* str )
{
	(void)str;
	char *outfitname;
	Outfit* outfit;
	int q;

	outfitname = toolkit_getList( secondary_wid, "lstOutfits" );
	outfit = outfit_get( outfitname );

	q = outfits_getMod();

	/* has no outfits to sell */
	if (player_outfitOwned(outfitname) <= 0) {
		toolkit_alert( "You can't sell something you don't have." );
		return;
	}

	player_credits += outfit->price * pilot_rmOutfit( player, outfit, q );
	outfits_update(NULL);
}
/*
 * returns the current modifier status
 */
static int outfits_getMod (void)
{
	SDLMod mods;
	int q;

	mods = SDL_GetModState();
	q = 1;
	if (mods & (KMOD_LCTRL | KMOD_RCTRL)) q *= 5;
	if (mods & (KMOD_LSHIFT | KMOD_RSHIFT)) q *= 10;

	return q;
}
static void outfits_renderMod( double bx, double by, double w, double h )
{
	(void) h;
	int q;
	char buf[8];

	q = outfits_getMod();
	if (q==1) return;

	snprintf( buf, 8, "%dx", q );
	gl_printMid( &gl_smallFont, w,
			bx + (double)gl_screen.w/2.,
			by + (double)gl_screen.h/2.,
			&cBlack, buf );
}




/*
 * ze shipyard
 */
static void shipyard (void)
{
	char **ships;
	int nships;
	char buf[128];

	/* window creation */
	snprintf( buf, 128, "%s - Shipyard", land_planet->name );
	secondary_wid = window_create( buf,
			-1, -1, SHIPYARD_WIDTH, SHIPYARD_HEIGHT );

	/* buttons */
	window_addButton( secondary_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseShipyard",
			"Close", shipyard_close );
	window_addButton( secondary_wid, -20, 40+BUTTON_HEIGHT,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnYourShips",
			"Your Ships", shipyard_yours );
	window_addButton( secondary_wid, -40-BUTTON_WIDTH, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnBuyShip",
			"Buy", shipyard_buy );
	window_addButton( secondary_wid, -40-BUTTON_WIDTH, 40+BUTTON_HEIGHT,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnInfoShip",
			"Info", shipyard_info );

	/* target gfx */
	window_addRect( secondary_wid, -40, -50,
			128, 96, "rctTarget", &cBlack, 0 );
	window_addImage( secondary_wid, -40-128, -50-96,
			"imgTarget", NULL, 1 );

	/* text */
	window_addText( secondary_wid, 40+200+40, -55,
			80, 96, 0, "txtSDesc", &gl_smallFont, &cDConsole,
			"Name:\n"
			"Class:\n"
			"Fabricator:\n"
			"\n"
			"Price:\n"
			"Money:\n" );
	window_addText( secondary_wid, 40+200+40+80, -55,
			130, 96, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );
	window_addText( secondary_wid, 20+200+40, -160,
			SHIPYARD_WIDTH-300, 200, 0, "txtDescription",
			&gl_smallFont, NULL, NULL );

	/* set up the ships to buy/sell */
	ships = ship_getTech( &nships, land_planet->tech, PLANET_TECH_MAX );
	window_addList( secondary_wid, 20, 40,
			200, SHIPYARD_HEIGHT-80, "lstShipyard",
			ships, nships, 0, shipyard_update );

	/* write the shipyard stuff */
	shipyard_update( NULL );
}
static void shipyard_close( char* str )
{
	if (strcmp(str,"btnCloseShipyard")==0)
		window_destroy(secondary_wid);
}
static void shipyard_update( char* str )
{
	(void)str;
	char *shipname;
	Ship* ship;
	char buf[80], buf2[16], buf3[16];
	
	shipname = toolkit_getList( secondary_wid, "lstShipyard" );
	ship = ship_get( shipname );

	/* update image */
	window_modifyImage( secondary_wid, "imgTarget", ship->gfx_target );

	/* update text */
	window_modifyText( secondary_wid, "txtDescription", ship->description );
	credits2str( buf2, ship->price, 2 );
	credits2str( buf3, player_credits, 2 );
	snprintf( buf, 80,
			"%s\n"
			"%s\n"
			"%s\n"
			"\n"
			"%s credits\n"
			"%s credits\n",
			ship->name,
			ship_class(ship),
			ship->fabricator,
			buf2,
			buf3);
	window_modifyText( secondary_wid,  "txtDDesc", buf );
}
static void shipyard_info( char* str )
{
	(void)str;
	char *shipname;

	shipname = toolkit_getList( secondary_wid, "lstShipyard" );
	ship_view(shipname);
}
static void shipyard_buy( char* str )
{
	(void)str;
	char *shipname;
	Ship* ship;

	shipname = toolkit_getList( secondary_wid, "lstShipyard" );
	ship = ship_get( shipname );

	/* player just gots a new ship */
	player_newShip( ship, player->solid->pos.x, player->solid->pos.y,
			0., 0., player->solid->dir );
}
static void shipyard_yours( char* str )
{
	(void)str;
	char **ships;
	int nships;

	/* create window */
	terciary_wid = window_create( "Your Ships",
			-1, -1, SHIPYARD_WIDTH, SHIPYARD_HEIGHT );

	/* buttons */
	window_addButton( terciary_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseYourShips",
			"Shipyard", shipyard_yoursClose );

	/* image */
	window_addRect( terciary_wid, -40, -50,
			128, 96, "rctTarget", &cBlack, 0 );
	window_addImage( terciary_wid, -40-128, -50-96,
			"imgTarget", NULL, 1 );

	/* text */
	window_addText( terciary_wid, 40+200+40, -55,
			100, 96, 0, "txtSDesc", &gl_smallFont, &cDConsole,
			"Name:\n"
			"Ship:\n"
			"Class:\n"
			"Where:\n"
			"\n"
			"Cargo free:\n"
			"Weapons free:\n"
			"\n"
			"Transportation:\n"
			"Sell price:\n"
			);
	window_addText( terciary_wid, 40+200+40+100, -55,
		130, 96, 0, "txtDDesc", &gl_smallFont, &cBlack, NULL );
	window_addText( terciary_wid, 40+200+40, -215,
		100, 20, 0, "txtSOutfits", &gl_smallFont, &cDConsole,
		"Outfits:\n"
		);
	window_addText( terciary_wid, 40+200+40, -215-gl_smallFont.h-5,
		SHIPYARD_WIDTH-40-200-40-20, 200, 0, "txtDOutfits",
		&gl_smallFont, &cBlack, NULL );

	/* ship list */
	ships = player_ships( &nships );
	window_addList( terciary_wid, 20, 40,
			200, SHIPYARD_HEIGHT-80, "lstYourShips",
			ships, nships, 0, shipyard_yoursUpdate );

	shipyard_yoursUpdate(NULL);
}
static void shipyard_yoursClose( char* str )
{
	(void)str;
	window_destroy( terciary_wid );
}
static void shipyard_yoursUpdate( char* str )
{
	(void)str;
	char buf[256], buf2[16], buf3[16], *buf4;
	char *shipname;
	Pilot *ship;
	char* loc;

	shipname = toolkit_getList( terciary_wid, "lstYourShips" );
	if (strcmp(shipname,"None")==0) return; /* no ships */
	ship = player_getShip( shipname );
	loc = player_getLoc(ship->name);

	/* update image */
	window_modifyImage( terciary_wid, "imgTarget", ship->ship->gfx_target );

	/* update text */
	credits2str( buf2, (strcmp(loc,land_planet->name)==0) ? 0 :
			ship->solid->mass*500, 2 ); /* transport */
	credits2str( buf3, 0, 2 ); /* sell price */
	snprintf( buf, 80,
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"\n"
			"%d tons\n"
			"%d tons\n"
			"\n"
			"%s credits\n"
			"%s credits\n",
			ship->name,
			ship->ship->name,
			ship_class(ship->ship),
			loc,
			ship->cargo_free,
			pilot_freeSpace(ship),
			buf2,
			buf3);
	window_modifyText( terciary_wid, "txtDDesc", buf );

	buf4 = pilot_getOutfits( ship );
	window_modifyText( terciary_wid, "txtDOutfits", buf4 );
	free(buf4);
}


/*
 * the spaceport bar
 */
static void spaceport_bar (void)
{
	/* window */
	secondary_wid = window_create( "Spaceport Bar",
			-1, -1, BAR_WIDTH, BAR_HEIGHT );

	/* buttons */
	window_addButton( secondary_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseBar",
			"Close", spaceport_bar_close );
	window_addButton( secondary_wid, 20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnNews",
			"News", (void(*)(char*))news);

	/* text */
	window_addText( secondary_wid, 20, -30,
			BAR_WIDTH - 40, BAR_HEIGHT - 40 - BUTTON_HEIGHT, 0,
			"txtDescription", &gl_smallFont, &cBlack, land_planet->bar_description );
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
	/* create window */
	terciary_wid = window_create( "News Reports",
			-1, -1, NEWS_WIDTH, NEWS_HEIGHT );

	/* buttons */
	window_addButton( terciary_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnCloseNews",
			"Close", news_close );

	/* text */
	window_addText( terciary_wid, 20, 20 + BUTTON_HEIGHT + 20,
			NEWS_WIDTH-40, NEWS_HEIGHT - 20 - BUTTON_HEIGHT - 20 - 20 - 20,
			0, "txtNews", &gl_smallFont, &cBlack,
			"News reporters report that they are on strike!");
}
static void news_close( char* str )
{
	if (strcmp(str,"btnCloseNews")==0)
		window_destroy( terciary_wid );
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

	land_planet = p;
	land_wid = window_create( p->name, -1, -1, LAND_WIDTH, LAND_HEIGHT );
	
	/*
	 * pretty display
	 */
	window_addImage( land_wid, 20, -40, "imgPlanet", p->gfx_exterior, 1 );
	window_addText( land_wid, 440, 80, LAND_WIDTH-460, 460, 0, 
			"txtPlanetDesc", &gl_smallFont, &cBlack, p->description);

	/*
	 * buttons
	 */
	/* first column */
	window_addButton( land_wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnTakeoff",
			"Takeoff", (void(*)(char*))takeoff );
	if (planet_hasService(land_planet, PLANET_SERVICE_COMMODITY))
		window_addButton( land_wid, -20, 20 + BUTTON_HEIGHT + 20,
				BUTTON_WIDTH, BUTTON_HEIGHT, "btnCommodity",
				"Commodity Exchange", (void(*)(char*))commodity_exchange);
	/* second column */
	if (planet_hasService(land_planet, PLANET_SERVICE_SHIPYARD))
		window_addButton( land_wid, -20 - BUTTON_WIDTH - 20, 20,
				BUTTON_WIDTH, BUTTON_HEIGHT, "btnShipyard",
				"Shipyard", (void(*)(char*))shipyard);
	if (planet_hasService(land_planet, PLANET_SERVICE_OUTFITS))
		window_addButton( land_wid, -20 - BUTTON_WIDTH - 20, 20 + BUTTON_HEIGHT + 20,
				BUTTON_WIDTH, BUTTON_HEIGHT, "btnOutfits",
				"Outfits", (void(*)(char*))outfits);
	/* third column */
	if (planet_hasService(land_planet, PLANET_SERVICE_BASIC)) {
		window_addButton( land_wid, 20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnNews",
			"Mission Terminal", NULL);
		window_addButton( land_wid, 20, 20 + BUTTON_HEIGHT + 20,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnBar",
			"Spaceport Bar", (void(*)(char*))spaceport_bar);
	}


	/* player is now officially landed */
	landed = 1;
}


/*
 * takes off the player
 */
void takeoff (void)
{
	int sw, sh;

	if (!landed) return;

	/* ze music */
	music_load( MUSIC_TAKEOFF );
	music_play();

	/* to randomize the takeoff a bit */
	sw = land_planet->gfx_space->w;
	sh = land_planet->gfx_space->h;

	/* no longer authorized to land */
	player_rmFlag(PLAYER_LANDACK);

	/* set player to another position with random facing direction and no vel */
	player_warp( land_planet->pos.x + RNG(-sw/2,sw/2),
			land_planet->pos.y + RNG(-sh/2,sh/2) );
	vect_pset( &player->solid->vel, 0., 0. );
	player->solid->dir = RNG(0,359) * M_PI/180.;

	/* heal the player */
	player->armour = player->armour_max;
	player->shield = player->shield_max;
	player->energy = player->energy_max;

	/* initialize the new space */
	space_init(NULL);

	/* cleanup */
	land_planet = NULL;
	window_destroy( land_wid );
	landed = 0;
}

