/*
 * See Licensing and Copyright notice in naev.h
 */



#include "player.h"

#include <malloc.h>

#include "xml.h"

#include "naev.h"
#include "pilot.h"
#include "log.h"
#include "opengl.h"
#include "font.h"
#include "pack.h"
#include "space.h"
#include "rng.h"
#include "land.h"
#include "sound.h"
#include "economy.h"
#include "pause.h"
#include "menu.h"
#include "toolkit.h"


#define XML_GUI_ID	"GUIs"   /* XML section identifier */
#define XML_GUI_TAG	"gui"

#define XML_START_ID	"Start"

#define GUI_DATA		"dat/gui.xml"
#define GUI_GFX		"gfx/gui/"

#define START_DATA 	"dat/start.xml"


/*
 * player stuff
 */
Pilot* player = NULL; /* ze player */
static Ship* player_ship = NULL; /* temporary ship to hold when naming it */
static double player_px, player_py, player_vx, player_vy, player_dir; /* more hack */
/* player pilot stack - ships he has */
static Pilot** player_stack = NULL;
static char** player_lstack = NULL; /* names of the planet the ships are at */
static int player_nstack = 0;
/* player global properties */
char* player_name = NULL; /* ze name */
int player_credits = 0; /* ze monies */
int player_crating = 0; /* ze rating */
unsigned int player_flags = 0; /* player flags */
/* used in input.c */
double player_turn = 0.; /* turn velocity from input */
double player_acc = 0.; /* accel velocity from input */
unsigned int player_target = PLAYER_ID; /* targetted pilot */
/* pure internal */
int planet_target = -1; /* targetted planet */
int hyperspace_target = -1; /* targetted hyperspace route */
/* for death and such */
static unsigned int player_timer = 0;
static Vector2d player_cam;


/*
 * pilot stuff for GUI
 */
extern Pilot** pilot_stack;
extern int pilots;

/*
 * space stuff for GUI
 */
extern StarSystem *systems_stack;

/*
 * land stuff for player_stack
 */
extern Planet* land_planet;


/*
 * GUI stuff
 */
typedef struct Radar_ {
	double x,y; /* position */
	double w,h; /* dimensions */
	RadarShape shape;
	double res; /* resolution */
} Radar;
/* radar resolutions */
#define RADAR_RES_MAX		100.
#define RADAR_RES_MIN		10.
#define RADAR_RES_INTERVAL	10.
#define RADAR_RES_DEFAULT	40.

typedef struct Rect_ {
	double x,y;
	double w,h;
} Rect;

typedef struct GUI_ {
	/* graphics */
	glTexture *gfx_frame;
	glTexture *gfx_targetPilot, *gfx_targetPlanet;

	/* rects */
	Radar radar;
	Rect nav;
	Rect shield, armour, energy;
	Rect weapon;
	Rect target_health, target_name, target_faction;
	Rect misc;
	Rect mesg;
	
	/* positions */
	Vector2d frame;
	Vector2d target;

} GUI;
GUI gui = { .gfx_frame = NULL,
		.gfx_targetPilot = NULL,
		.gfx_targetPlanet = NULL }; /* ze GUI */
/* needed to render properly */
double gui_xoff = 0.;
double gui_yoff = 0.;

/* messages */
#define MESG_SIZE_MAX	80
int mesg_timeout = 3000;
int mesg_max = 5; /* maximum messages onscreen */
typedef struct Mesg_ {
	char str[MESG_SIZE_MAX];
	unsigned int t;
} Mesg;
static Mesg* mesg_stack;


/* 
 * prototypes
 */
/* external */
extern void pilot_render( const Pilot* pilot ); /* from pilot.c */
extern void weapon_minimap( const double res, const double w, const double h,
		const RadarShape shape ); /* from weapon.c */
extern void planets_minimap( const double res, const double w, const double h,
		const RadarShape shape ); /* from space.c */
/* internal */
static void player_nameClose( char *str );
static void player_newMake (void);
static void player_nameShipClose( char *str );
static void player_newShipMake( char *name );
static void rect_parse( const xmlNodePtr parent,
		double *x, double *y, double *w, double *h );
static int gui_parse( const xmlNodePtr parent, const char *name );
static void gui_renderPilot( const Pilot* p );
static void gui_renderBar( const glColour* c,
		const Rect* r, const double w );
/* externed */
void player_dead (void);
void player_destroyed (void);



/* 
 * prompts for player's name
 */
void player_new (void)
{
	unsigned int wid;

	/* to not segfault due to lack of environment */
	player_setFlag(PLAYER_DESTROYED);
	vectnull( &player_cam );
	gl_bindCamera( &player_cam );

	/* cleanup player stuff if we'll be recreating */
	player_cleanup();

	wid = window_create( "Player Name", -1, -1, 240, 140 );

	window_addText( wid, 30, -30, 180, 20, 0, "txtInfo",
			&gl_smallFont, &cDConsole, "Please write your name:" );
	window_addInput( wid, 20, -50, 200, 20, "inpName", 20, 1 );
	window_addButton( wid, -20, 20, 80, 30, "btnClose", "Done", player_nameClose );
}
static void player_nameClose( char *str )
{
	(void)str;
	unsigned int wid;
	char *name;

	wid = window_get("Player Name");
	name = window_getInput( wid, "inpName" );

	if (strlen(name) < 3) {
		toolkit_alert( "Your name must be at least 3 characters long." );
		return;
	}

	player_name = strdup(name);
	window_destroy( wid );

	player_newMake();
}


/*
 * creates a new player
 */
static void player_newMake (void)
{
	Ship *ship;
	char system[20];
	uint32_t bufsize;
	char *buf = pack_readfile( DATA, START_DATA, &bufsize );
	int l,h;
	double x,y;

	xmlNodePtr node, cur, tmp;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	node = doc->xmlChildrenNode;
	if (!xml_isNode(node,XML_START_ID)) {
		ERR("Malformed '"START_DATA"' file: missing root element '"XML_START_ID"'");
		return;
	}

	node = node->xmlChildrenNode; /* first system node */
	if (node == NULL) {
		ERR("Malformed '"START_DATA"' file: does not contain elements");
		return;
	}
	do {
		if (xml_isNode(node, "player")) { /* we are interested in the player */
			cur = node->children;
			do {
				if (xml_isNode(cur,"ship")) ship = ship_get( xml_get(cur) );
				else if (xml_isNode(cur,"credits")) { /* monies range */
					tmp = cur->children;
					do { 
						if (xml_isNode(tmp,"low")) l = xml_getInt(tmp);
						else if (xml_isNode(tmp,"high")) h = xml_getInt(tmp);
					} while ((tmp = tmp->next));
				}
				else if (xml_isNode(cur,"system")) {
					tmp = cur->children;
					do {
						/* system name, TODO percent chance */
						if (xml_isNode(tmp,"name")) snprintf(system,20,xml_get(tmp));
						/* position */
						else if (xml_isNode(tmp,"x")) x = xml_getFloat(tmp);
						else if (xml_isNode(tmp,"y")) y = xml_getFloat(tmp);
					} while ((tmp = tmp->next));
				}
				else if (xml_isNode(cur,"player_crating"))
					player_crating = xml_getInt(cur);

			} while ((cur = cur->next));
		}
	} while ((node = node->next));

	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();


	/* monies */
	player_credits = RNG(l,h);

	/* welcome message - must be before space_init */
	player_message( "Welcome to "APPNAME"!" );
	player_message( " v%d.%d.%d", VMAJOR, VMINOR, VREV );

	/* create the player and start the game */
	player_newShip( ship, x, y, 0., 0., RNG(0,359)/180.*M_PI );
	space_init(system);
}


/*
 * creates a dialogue to name the new ship
 */
void player_newShip( Ship* ship, double px, double py,
		double vx, double vy, double dir )
{
	unsigned int wid;

	/* temporary values while player doesn't exist */
	player_ship = ship;
	player_px = px;
	player_py = py;
	player_vx = vx;
	player_vy = vy;
	player_dir = dir;

	wid = window_create( "Ship Name", -1, -1, 240, 140 );

	window_addText( wid, 30, -30, 180, 20, 0, "txtInfo",
			&gl_smallFont, &cDConsole, "Please name your ship:" );
	window_addInput( wid, 20, -50, 200, 20, "inpName", 20, 1 );
	window_addButton( wid, -20, 20, 80, 30, "btnClose", "Done", player_nameShipClose );
}
static void player_nameShipClose( char *str )
{
	(void)str;
	char* ship_name;
	unsigned int wid;

	wid = window_get("Ship Name");
	ship_name = window_getInput( wid, "inpName" );

	if (strlen(ship_name) < 3) {
		toolkit_alert( "Your ship's name must be at least 3 characters long" );
		return;
	}

	player_newShipMake(ship_name);

	window_destroy( wid );
}

/*
 * change the player's ship
 */
static void player_newShipMake( char* name )
{
	Vector2d vp, vv;

	if (player) {
		player_stack = realloc(player_stack, sizeof(Pilot*)*(player_nstack+1));
		player_stack[player_nstack] = pilot_copy( player );
		player_lstack = realloc(player_lstack, sizeof(char*)*(player_nstack+1));
		player_lstack[player_nstack] = strdup( land_planet->name );
		player_nstack++;
		pilot_destroy( player );
	}

	/* in case we're respawning */
	player_rmFlag(PLAYER_DESTROYED);

	/* hackish position setting */
	vect_cset( &vp, player_px, player_py );
	vect_cset( &vv, player_vx, player_vy );

	pilot_create( player_ship, name, faction_get("Player"), NULL,
			player_dir,  &vp, &vv, PILOT_PLAYER );
	gl_bindCamera( &player->solid->pos ); /* set opengl camera */
}


/*
 * cleans up player stuff like player_stack
 */
void player_cleanup (void)
{
	int i;

	/* cleanup name */
	if (player_name) free(player_name);

	/* clean up the stack */
	if (player_stack) {                
		for (i=0; i<player_nstack; i++) {
			pilot_free(player_stack[i]);
			free(player_lstack[i]);
		}
		free (player_stack);
		player_stack = NULL;
		free(player_lstack);
		player_lstack = NULL;
		/* nothing left */
		player_nstack = 0;
	}
}


/*
 * adds a mesg to the queue to be displayed on screen
 */
void player_message ( const char *fmt, ... )
{
	va_list ap;
	int i;

	if (fmt == NULL) return; /* message not valid */

	/* copy old messages back */
	for (i=1; i<mesg_max; i++)
		if (mesg_stack[mesg_max-i-1].str[0] != '\0') {
			strcpy(mesg_stack[mesg_max-i].str, mesg_stack[mesg_max-i-1].str);
			mesg_stack[mesg_max-i].t = mesg_stack[mesg_max-i-1].t;
		}

	/* add the new one */
	va_start(ap, fmt);
	vsprintf( mesg_stack[0].str, fmt, ap );
	va_end(ap);

	mesg_stack[0].t = SDL_GetTicks() + mesg_timeout;
}


/*
 * warps the player to the new position
 */
void player_warp( const double x, const double y )
{
	vect_cset( &player->solid->pos, x, y );
}


/*
 * clears the targets
 */
void player_clear (void)
{
	player_target = PLAYER_ID;
	planet_target = -1;
	hyperspace_target = -1;
}


/*
 * gets the player's combat rating
 */
static char* player_ratings[] = {
		"None",
		"Smallfry",
		"Weak",
		"Minor",
		"Average",
		"Major",
		"Fearsome",
		"Godlike"
};
const char* player_rating (void)
{
	if (player_crating == 0) return player_ratings[0];
	else if (player_crating < 50) return player_ratings[1];
	else if (player_crating < 200) return player_ratings[2];
	else if (player_crating < 500) return player_ratings[3];
	else if (player_crating < 1000) return player_ratings[4];
	else if (player_crating < 2500) return player_ratings[5];
	else if (player_crating < 10000) return player_ratings[6];
	else return player_ratings[7];
}


/*
 * returns how much weapon space the player has left
 */
int player_freeSpace (void)
{
	int i,s;

	s = player->ship->cap_weapon;
	for (i=0; i<player->noutfits; i++)
		s -= player->outfits[i].quantity * player->outfits[i].outfit->mass;
	
	return s;
}


/*
 * returns how many of the outfit the player owns
 */
int player_outfitOwned( const char* outfitname )
{
	int i;

	for (i=0; i<player->noutfits; i++)
		if (strcmp(outfitname, player->outfits[i].outfit->name)==0)
			return player->outfits[i].quantity;
	return 0;
}


/*
 * returns how many of the commodity the player has
 */
int player_cargoOwned( const char* commodityname )
{
	int i;

	for (i=0; i<player->ncommodities; i++)
		if (strcmp(commodityname, player->commodities[i].commodity->name)==0)
			return player->commodities[i].quantity;
	return 0;
}



/*
 * renders the background player stuff, namely planet target gfx
 */
void player_renderBG (void)
{
	double x,y;
	glColour *c;
	Planet* planet;

	/* no need to draw if pilot is dead */
	if (player_isFlag(PLAYER_DESTROYED) ||
		pilot_isFlag(player,PILOT_DEAD)) return;

	if (planet_target >= 0) {
		planet = &cur_system->planets[planet_target];

		if (areEnemies(player->faction,planet->faction)) c = &cHostile;
		else c = &cNeutral;

		x = planet->pos.x - planet->gfx_space->sw/2.;
		y = planet->pos.y + planet->gfx_space->sh/2.;
		gl_blitSprite( gui.gfx_targetPlanet, x, y, 0, 0, c ); /* top left */

		x += planet->gfx_space->sw;
		gl_blitSprite( gui.gfx_targetPlanet, x, y, 1, 0, c ); /* top right */

		y -= planet->gfx_space->sh;
		gl_blitSprite( gui.gfx_targetPlanet, x, y, 1, 1, c ); /* bottom right */

		x -= planet->gfx_space->sw;
		gl_blitSprite( gui.gfx_targetPlanet, x, y, 0, 1, c ); /* bottom left */
	}
}

/*
 * renders the player
 */
void player_render (void)
{
	int i, j;
	double x, y;
	char str[10];
	Pilot* p;
	glColour* c;
	glFont* f;

	/* pilot is dead, just render him and stop */
	if (player_isFlag(PLAYER_DESTROYED) || pilot_isFlag(player,PILOT_DEAD)) {
		if (player_isFlag(PLAYER_DESTROYED)) {
			if (!toolkit && (SDL_GetTicks() > player_timer))
				menu_death();
		}
		else
			pilot_render(player);

		/*
		 * draw fancy cinematic scene borders
		 */
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix(); /* translation matrix */
			glTranslated( x-(double)gl_screen.w/2., y-(double)gl_screen.h/2., 0);

		COLOUR(cBlack);
		glBegin(GL_QUADS);
			glVertex2d( 0.,          0.              );
			glVertex2d( 0.,          gl_screen.h*0.2 );
			glVertex2d( gl_screen.w, gl_screen.h*0.2 );
			glVertex2d( gl_screen.w, 0.              );
			glVertex2d( 0.,          gl_screen.h     );
			glVertex2d( gl_screen.w, gl_screen.h     );
			glVertex2d( gl_screen.w, gl_screen.h*0.8 );
			glVertex2d( 0.,          gl_screen.h*0.8 );
		glEnd(); /* GL_QUADS */

		glPopMatrix(); /* translation matrx */

		return;
	}

	/* renders the player target graphics */
	if (player_target != PLAYER_ID) p = pilot_get(player_target);
	else p = NULL;
	if ((p==NULL) || pilot_isFlag(p,PILOT_DEAD))
		player_target = PLAYER_ID; /* no more pilot_target */
	else { /* still is a pilot_target */
		if (pilot_isDisabled(p)) c = &cInert;
		else if (pilot_isFlag(p,PILOT_HOSTILE)) c = &cHostile;
		else c = &cNeutral;

		x = p->solid->pos.x - p->ship->gfx_space->sw * PILOT_SIZE_APROX/2.;
		y = p->solid->pos.y + p->ship->gfx_space->sh * PILOT_SIZE_APROX/2.;
		gl_blitSprite( gui.gfx_targetPilot, x, y, 0, 0, c ); /* top left */

		x += p->ship->gfx_space->sw * PILOT_SIZE_APROX;
		gl_blitSprite( gui.gfx_targetPilot, x, y, 1, 0, c ); /* top right */

		y -= p->ship->gfx_space->sh * PILOT_SIZE_APROX;
		gl_blitSprite( gui.gfx_targetPilot, x, y, 1, 1, c ); /* bottom right */

		x -= p->ship->gfx_space->sw * PILOT_SIZE_APROX;
		gl_blitSprite( gui.gfx_targetPilot, x, y, 0, 1, c ); /* bottom left */
	}

	/* render the player */
	pilot_render(player);

	/*
	 *    G U I
	 */
	/*
	 * frame
	 */
	gl_blitStatic( gui.gfx_frame, gui.frame.x, gui.frame.y, NULL );

	/*
	 * radar
	 */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	if (gui.radar.shape==RADAR_RECT)
		glTranslated( gui.radar.x - gl_screen.w/2. + gui.radar.w/2.,
				gui.radar.y - gl_screen.h/2. - gui.radar.h/2., 0.);
	else if (gui.radar.shape==RADAR_CIRCLE)
		glTranslated( gui.radar.x - gl_screen.w/2.,
				gui.radar.y - gl_screen.h/2., 0.);

	/*
	 * planets
	 */
	planets_minimap(gui.radar.res, gui.radar.w, gui.radar.h, gui.radar.shape);

	/*
	 * weapons
	 */
	glBegin(GL_POINTS);
		COLOUR(cRadar_weap);
		weapon_minimap(gui.radar.res, gui.radar.w, gui.radar.h, gui.radar.shape);
	glEnd(); /* GL_POINTS */


	/* render the pilots */
	for (j=0, i=1; i<pilots; i++) { /* skip the player */
		if (pilot_stack[i]->id == player_target) j = i;
		else gui_renderPilot(pilot_stack[i]);
	}
	/* render the targetted pilot */
	if (j!=0) gui_renderPilot(pilot_stack[j]);


	glBegin(GL_POINTS); /* for the player */
		/* player - drawn last*/
		COLOUR(cRadar_player);
		glVertex2d(  0.,  2. ); /* we represent the player with a small + */
		glVertex2d(  0.,  1. ); 
		glVertex2d(  0.,  0. );
		glVertex2d(  0., -1. );                                             
		glVertex2d(  0., -2. );
		glVertex2d(  2.,  0. );
		glVertex2d(  1.,  0. );
		glVertex2d( -1.,  0. );
		glVertex2d( -2.,  0. );
	glEnd(); /* GL_POINTS */

	glPopMatrix(); /* GL_PROJECTION */


	/*
	 * NAV 
	 */
	if (planet_target >= 0) { /* planet landing target */
		gl_printMid( NULL, (int)gui.nav.w,
				gui.nav.x, gui.nav.y - 5,
				&cConsole, "Land" );

		gl_printMid( &gl_smallFont, (int)gui.nav.w,
				gui.nav.x, gui.nav.y - 10 - gl_smallFont.h,
				NULL, "%s", cur_system->planets[planet_target].name );
	}
	else if (hyperspace_target >= 0) { /* hyperspace target */

		c = space_canHyperspace(player) ? &cConsole : NULL ;
		gl_printMid( NULL, (int)gui.nav.w,
				gui.nav.x, gui.nav.y - 5,
				c, "Hyperspace" );

		gl_printMid( &gl_smallFont, (int)gui.nav.w,
				gui.nav.x, gui.nav.y - 10 - gl_smallFont.h,
				NULL, "%s", systems_stack[cur_system->jumps[hyperspace_target]].name );
	}
	else { /* no NAV target */
		gl_printMid( NULL, (int)gui.nav.w,
				gui.nav.x, gui.nav.y - 5,
				&cConsole, "Navigation" );

		gl_printMid( &gl_smallFont, (int)gui.nav.w,
				gui.nav.x, gui.nav.y - 10 - gl_smallFont.h,
				&cGrey, "Off" );
	}


	/*
	 * health
	 */
	gui_renderBar( &cShield,  &gui.shield,
			player->shield / player->shield_max );
	gui_renderBar( &cArmour, &gui.armour,
			player->armour / player->armour_max );
	gui_renderBar( &cEnergy, &gui.energy,
			player->energy / player->energy_max );


	/* 
	 * weapon 
	 */ 
	if (player->secondary==NULL) { /* no secondary weapon */ 
		gl_printMid( NULL, (int)gui.weapon.w,
				gui.weapon.x, gui.weapon.y - 5,
				&cConsole, "Secondary" ); 

		gl_printMid( &gl_smallFont, (int)gui.weapon.w,
				gui.weapon.x, gui.weapon.y - 10 - gl_defFont.h,
				&cGrey, "None"); 
	}  
	else {
		f = &gl_defFont;
		if (outfit_isLauncher(player->secondary->outfit)) {
			/* use the ammunition's name */
			i = gl_printWidth( f, "%s", player->secondary->outfit->u.lau.ammo);
			if (i > gui.weapon.w) /* font is too big */
				f = &gl_smallFont;
			gl_printMid( f, (int)gui.weapon.w,
					gui.weapon.x, gui.weapon.y - 5,
					&cConsole, "%s", player->secondary->outfit->u.lau.ammo );

			/* print ammo left underneath */
			gl_printMid( &gl_smallFont, (int)gui.weapon.w,
					gui.weapon.x, gui.weapon.y - 10 - gl_defFont.h,
					NULL, "%d", (player->ammo) ? player->ammo->quantity : 0 );
		}
		else { /* just print the item name */
			i = gl_printWidth( f, "%s", player->secondary->outfit->name);
			if (i > (int)gui.weapon.w) /* font is too big */
				f = &gl_smallFont;
			gl_printMid( f, (int)gui.weapon.w,
					gui.weapon.x, gui.weapon.y - (gui.weapon.h - f->h)/2.,
					&cConsole, "%s", player->secondary->outfit->name );

		}
	} 


	/*
	 * target
	 */
	if (player_target != PLAYER_ID) {
		p = pilot_get(player_target);

		gl_blitStatic( p->ship->gfx_target, gui.target.x, gui.target.y, NULL );

		/* target name */
		gl_print( NULL,
				gui.target_name.x,
				gui.target_name.y,
				NULL, "%s", p->name );
		gl_print( &gl_smallFont,
				gui.target_faction.x,
				gui.target_faction.y,
				NULL, "%s", p->faction->name );

		/* target status */
		if (pilot_isDisabled(p)) /* pilot is disabled */
			gl_print( &gl_smallFont,
					gui.target_health.x,
					gui.target_health.y,
					NULL, "Disabled" );

		else if (p->shield > p->shield_max/100.) /* on shields */
			gl_print( &gl_smallFont,
				gui.target_health.x,
					gui.target_health.y, NULL,
					"%s: %.0f%%", "Shield", p->shield/p->shield_max*100. );

		else /* on armour */
			gl_print( &gl_smallFont,
					gui.target_health.x,
					gui.target_health.y, NULL, 
					"%s: %.0f%%", "Armour", p->armour/p->armour_max*100. );
	}
	else { /* no target */
		gl_printMid( NULL, SHIP_TARGET_W,
				gui.target.x, gui.target.y  + (SHIP_TARGET_H - gl_defFont.h)/2.,
				&cGrey, "No Target" );
	}


	/*
	 * misc
	 */
	/* monies */
	j = gui.misc.y - 8 - gl_smallFont.h;
	gl_print( &gl_smallFont,
			gui.misc.x + 8, j,
			&cConsole, "Creds:" );
	credits2str( str, player_credits, 2 );
	i = gl_printWidth( &gl_smallFont, str );
	gl_print( &gl_smallFont,
			gui.misc.x + gui.misc.w - 8 - i, j,
			NULL, str );
	/* cargo and friends */
	if (player->ncommodities > 0) {
		j -= gl_smallFont.h + 5;
		gl_print( &gl_smallFont,
				gui.misc.x + 8, j,
				&cConsole, "Cargo:" );
		for (i=0; i < MIN(player->ncommodities,3); i++) { 
			j -= gl_smallFont.h + 3;
			gl_print( &gl_smallFont,
					gui.misc.x + 13, j,
					NULL, "%d %s", player->commodities[i].quantity,
					player->commodities[i].commodity->name );
		}
	}

	j -= gl_smallFont.h + 5;
	gl_print( &gl_smallFont,
			gui.misc.x + 8, j,
			&cConsole, "Free:" );
	i = gl_printWidth( &gl_smallFont, "%d", player->ship->cap_cargo );
	gl_print( &gl_smallFont,
			gui.misc.x + gui.misc.w - 8 - i, j,
			NULL, "%d", player->cargo_free );


	/*
	 * messages
	 */
	x = gui.mesg.x;
	y = gui.mesg.y + (double)(gl_defFont.h*mesg_max)*1.2;
	for (i=0; i<mesg_max; i++) {
		y -= (double)gl_defFont.h*1.2;
		if (mesg_stack[mesg_max-i-1].str[0]!='\0') {
			if (mesg_stack[mesg_max-i-1].t < SDL_GetTicks())
				mesg_stack[mesg_max-i-1].str[0] = '\0';
			else gl_print( NULL, x, y, NULL, "%s", mesg_stack[mesg_max-i-1].str );
		}
	}


	/*
	 * hyperspace
	 */
	if (pilot_isFlag(player, PILOT_HYPERSPACE) && !paused) {
		i = (int)player->ptimer - HYPERSPACE_FADEOUT;
		j = (int)SDL_GetTicks();
		if (i < j) {
			x = (double)(j-i) / HYPERSPACE_FADEOUT;
			glColor4d(1.,1.,1., x );
			glBegin(GL_QUADS);
				glVertex2d( -gl_screen.w/2., -gl_screen.h/2. );
				glVertex2d( -gl_screen.w/2.,  gl_screen.h/2. );
				glVertex2d(  gl_screen.w/2.,  gl_screen.h/2. );
				glVertex2d(  gl_screen.w/2., -gl_screen.h/2. );
			glEnd(); /* GL_QUADS */
		}
	}
}

/*
 * renders a pilot
 */
static void gui_renderPilot( const Pilot* p )
{
	int x, y, sx, sy;
	double w, h;

	x = (p->solid->pos.x - player->solid->pos.x) / gui.radar.res;
	y = (p->solid->pos.y - player->solid->pos.y) / gui.radar.res;
	sx = PILOT_SIZE_APROX/2. * p->ship->gfx_space->sw / gui.radar.res;
	sy = PILOT_SIZE_APROX/2. * p->ship->gfx_space->sh / gui.radar.res;
	if (sx < 1.) sx = 1.;
	if (sy < 1.) sy = 1.;

	if ( ((gui.radar.shape==RADAR_RECT) &&
				((ABS(x) > gui.radar.w/2+sx) || (ABS(y) > gui.radar.h/2.+sy)) ) ||
			((gui.radar.shape==RADAR_CIRCLE) &&
				((x*x+y*y) > (int)(gui.radar.w*gui.radar.w))) )
		return; /* pilot not in range */

	if (gui.radar.shape==RADAR_RECT) {
		w = gui.radar.w/2.;
		h = gui.radar.h/2.;
	}
	else if (gui.radar.shape==RADAR_CIRCLE) {
		w = gui.radar.w;
		h = gui.radar.w;
	}

	glBegin(GL_QUADS);
		/* colors */
		if (p->id == player_target) COLOUR(cRadar_targ);
		else if (pilot_isDisabled(p)) COLOUR(cInert);
		else if (pilot_isFlag(p,PILOT_HOSTILE)) COLOUR(cHostile);
		else COLOUR(cNeutral);

		/* image */
		glVertex2d( MAX(x-sx,-w), MIN(y+sy, h) ); /* top-left */
		glVertex2d( MIN(x+sx, w), MIN(y+sy, h) );/* top-right */
		glVertex2d( MIN(x+sx, w), MAX(y-sy,-h) );/* bottom-right */
		glVertex2d( MAX(x-sx,-w), MAX(y-sy,-h) );/* bottom-left */
	glEnd(); /* GL_QUADS */
}


/*
 * renders a bar (health)
 */
static void gui_renderBar( const glColour* c,
		const Rect* r, const double w )
{
	int x, y, sx, sy;

	glBegin(GL_QUADS); /* shield */
		COLOUR(*c); 
		x = r->x - gl_screen.w/2.;
		y = r->y - gl_screen.h/2.;
		sx = w * r->w;
		sy = r->h;
		glVertex2d( x, y );
		glVertex2d( x + sx, y );
		glVertex2d( x + sx, y - sy );
		glVertex2d( x, y - sy );                                            
	glEnd(); /* GL_QUADS */

}


/*
 * initializes the GUI
 */
int gui_init (void)
{
	/*
	 * set gfx to NULL
	 */
	gui.gfx_frame = NULL;
	gui.gfx_targetPilot = NULL;
	gui.gfx_targetPlanet = NULL;

	/*
	 * radar
	 */
	gui.radar.res = RADAR_RES_DEFAULT;

	/*
	 * messages
	 */
	gui.mesg.x = 20;
	gui.mesg.y = 30;
   mesg_stack = calloc(mesg_max, sizeof(Mesg));

	return 0;
}


/*
 * attempts to load the actual gui
 */
int gui_load (const char* name)
{
	uint32_t bufsize;
	char *buf = pack_readfile( DATA, GUI_DATA, &bufsize );
	char *tmp;
	int found = 0;

	xmlNodePtr node;
	xmlDocPtr doc = xmlParseMemory( buf, bufsize );

	node = doc->xmlChildrenNode;
	if (!xml_isNode(node,XML_GUI_ID)) {
		ERR("Malformed '"GUI_DATA"' file: missing root element '"XML_GUI_ID"'");
		return -1;
	}

	node = node->xmlChildrenNode; /* first system node */
	if (node == NULL) {
		ERR("Malformed '"GUI_DATA"' file: does not contain elements");
		return -1;
	}                                                                                       
	do {
		if (xml_isNode(node, XML_GUI_TAG)) {

			tmp = xml_nodeProp(node,"name"); /* mallocs */

			/* is the gui we are looking for? */
			if (strcmp(tmp,name)==0) {
				found = 1;

				/* parse the xml node */
				if (gui_parse(node,name)) WARN("Trouble loading GUI '%s'", name);
				free(tmp);
				break;
			}

			free(tmp);
		}
	} while ((node = node->next));

	xmlFreeDoc(doc);
	free(buf);
	xmlCleanupParser();

	if (!found) {
		WARN("GUI '%s' not found in '"GUI_DATA"'",name);
		return -1;
	}

	return 0;
}


/*
 * used to pull out a rect from an xml node (<x><y><w><h>)
 */
static void rect_parse( const xmlNodePtr parent,
		double *x, double *y, double *w, double *h )
{
	xmlNodePtr cur;
	int param;

	param = 0;

	cur = parent->children;
	do {
		if (xml_isNode(cur,"x")) {
			if (x!=NULL) {
				*x = xml_getFloat(cur);
				param |= (1<<0);
			}
			else WARN("Extra parameter 'x' found for GUI node '%s'", parent->name);
		}
		else if (xml_isNode(cur,"y")) {
			if (y!=NULL) {
				*y = xml_getFloat(cur);
				param |= (1<<1);
			}
			else WARN("Extra parameter 'y' found for GUI node '%s'", parent->name);
		}
		else if (xml_isNode(cur,"w")) {
			if (w!=NULL) {
				*w = xml_getFloat(cur);
				param |= (1<<2);
			}
			else WARN("Extra parameter 'w' found for GUI node '%s'", parent->name);
		}
		else if (xml_isNode(cur,"h")) {
			if (h!=NULL) {
				*h = xml_getFloat(cur);
				param |= (1<<3);
			}
			else WARN("Extra parameter 'h' found for GUI node '%s'", parent->name);
		}
	} while ((cur = cur->next));

	/* check to see if we got everything we asked for */
	if (x && !(param & (1<<0)))
		WARN("Missing parameter 'x' for GUI node '%s'", parent->name);
	else if (y && !(param & (1<<1))) 
		WARN("Missing parameter 'y' for GUI node '%s'", parent->name);
	else if (w && !(param & (1<<2))) 
		WARN("Missing parameter 'w' for GUI node '%s'", parent->name);
	else if (h && !(param & (1<<3))) 
		WARN("Missing parameter 'h' for GUI node '%s'", parent->name);
}


/*
 * parse a gui node
 */
#define RELATIVIZE(a)	\
{(a).x+=VX(gui.frame); (a).y=VY(gui.frame)+gui.gfx_frame->h-(a).y;}
static int gui_parse( const xmlNodePtr parent, const char *name )
{
	xmlNodePtr cur, node;
	char *tmp, *tmp2;


	/*
	 * gfx
	 */
	/* set as a property and not a node because it must be loaded first */
	tmp2 = xml_nodeProp(parent,"gfx");
	if (tmp2==NULL) {
		ERR("GUI '%s' has no gfx property",name);
		return -1;
	}

	/* load gfx */
	tmp = malloc( (strlen(tmp2)+strlen(GUI_GFX)+12) * sizeof(char) );
	/* frame */
	snprintf( tmp, strlen(tmp2)+strlen(GUI_GFX)+5, GUI_GFX"%s.png", tmp2 );
	if (gui.gfx_frame) gl_freeTexture(gui.gfx_frame); /* free if needed */
	gui.gfx_frame = gl_newImage( tmp );
	/* pilot */
	snprintf( tmp, strlen(tmp2)+strlen(GUI_GFX)+11, GUI_GFX"%s_pilot.png", tmp2 );
	if (gui.gfx_targetPilot) gl_freeTexture(gui.gfx_targetPilot); /* free if needed */
	gui.gfx_targetPilot = gl_newSprite( tmp, 2, 2 );
	/* planet */
	snprintf( tmp, strlen(tmp2)+strlen(GUI_GFX)+12, GUI_GFX"%s_planet.png", tmp2 );
	if (gui.gfx_targetPlanet) gl_freeTexture(gui.gfx_targetPlanet); /* free if needed */
	gui.gfx_targetPlanet = gl_newSprite( tmp, 2, 2 );
	free(tmp);
	free(tmp2);

	/*
	 * frame (based on gfx)
	 */
	vect_csetmin( &gui.frame,
			gl_screen.w - gui.gfx_frame->w,     /* x */
			gl_screen.h - gui.gfx_frame->h );   /* y */

	/* now actually parse the data */
	node = parent->children;
	do { /* load all the data */

		/*
		 * offset
		 */
		if (xml_isNode(node,"offset"))
			rect_parse( node, &gui_xoff, &gui_yoff, NULL, NULL );

		/*
		 * radar
		 */
		else if (xml_isNode(node,"radar")) {

			tmp = xml_nodeProp(node,"type");

			/* make sure type is valid */
			if (strcmp(tmp,"rectangle")==0) gui.radar.shape = RADAR_RECT;
			else if (strcmp(tmp,"circle")==0) gui.radar.shape = RADAR_CIRCLE;
			else {
				WARN("Radar for GUI '%s' is missing 'type' tag or has invalid 'type' tag",name);
				gui.radar.shape = RADAR_RECT;
			}

			free(tmp);
		
			/* load the appropriate measurements */
			if (gui.radar.shape == RADAR_RECT)
				rect_parse( node, &gui.radar.x, &gui.radar.y, &gui.radar.w, &gui.radar.h );
			else if (gui.radar.shape == RADAR_CIRCLE)
				rect_parse( node, &gui.radar.x, &gui.radar.y, &gui.radar.w, NULL );
			RELATIVIZE(gui.radar);
		}

		/*
		 * nav computer
		 */
		else if (xml_isNode(node,"nav")) {
			rect_parse( node, &gui.nav.x, &gui.nav.y, &gui.nav.w, &gui.nav.h );
			RELATIVIZE(gui.nav);
			gui.nav.y -= gl_defFont.h;
		}

		/*
		 * health bars
		 */
		else if (xml_isNode(node,"health")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"shield")) {
					rect_parse( cur, &gui.shield.x, &gui.shield.y,
							&gui.shield.w, &gui.shield.h );
					RELATIVIZE(gui.shield);
				}
				if (xml_isNode(cur,"armour")) {
					rect_parse( cur, &gui.armour.x, &gui.armour.y,
							&gui.armour.w, &gui.armour.h );
					RELATIVIZE(gui.armour);
				}
				if (xml_isNode(cur,"energy")) {
					rect_parse( cur, &gui.energy.x, &gui.energy.y,
							&gui.energy.w, &gui.energy.h );
					RELATIVIZE(gui.energy);
				}
			} while ((cur = cur->next));
		}

		/*
		 * secondary weapon
		 */
		else if (xml_isNode(node,"weapon")) {
			rect_parse( node, &gui.weapon.x, &gui.weapon.y,
					&gui.weapon.w, &gui.weapon.h );
			RELATIVIZE(gui.weapon);
			gui.weapon.y -= gl_defFont.h;
		}

		/*
		 * target
		 */
		else if (xml_isNode(node,"target")) {
			cur = node->children;
			do {
				if (xml_isNode(cur,"gfx")) {
					rect_parse( cur, &gui.target.x, &gui.target.y, NULL, NULL );
					RELATIVIZE(gui.target);
					gui.target.y -= SHIP_TARGET_H;
				}
				else if (xml_isNode(cur,"name")) {
					rect_parse( cur, &gui.target_name.x, &gui.target_name.y, NULL, NULL );
					RELATIVIZE(gui.target_name);
					gui.target_name.y -= gl_defFont.h;
				}
				else if (xml_isNode(cur,"faction")) {
					rect_parse( cur, &gui.target_faction.x, &gui.target_faction.y, NULL, NULL );
					RELATIVIZE(gui.target_faction);
					gui.target_faction.y -= gl_smallFont.h;
				}
				else if (xml_isNode(cur,"health")) {
					rect_parse( cur, &gui.target_health.x, &gui.target_health.y, NULL, NULL );
					RELATIVIZE(gui.target_health);
					gui.target_health.y -= gl_smallFont.h;
				}
			} while ((cur = cur->next));
		}

		/*
		 * misc
		 */
		else if (xml_isNode(node,"misc")) {
			rect_parse( node, &gui.misc.x, &gui.misc.y, &gui.misc.w, &gui.misc.h );
			RELATIVIZE(gui.misc);
		}
	} while ((node = node->next));

	return 0;
}
#undef RELATIVIZE
/*
 * frees the GUI
 */
void gui_free (void)
{
	if (gui.gfx_frame) gl_freeTexture( gui.gfx_frame );
	if (gui.gfx_targetPilot) gl_freeTexture( gui.gfx_targetPilot );
	if (gui.gfx_targetPlanet) gl_freeTexture( gui.gfx_targetPlanet );

	free(mesg_stack);
}


/*
 * used in pilot.c
 *
 * basically uses keyboard input instead of AI input
 */
void player_think( Pilot* player )
{

	/* last i heard, the dead don't think */
	if (pilot_isFlag(player,PILOT_DEAD)) {
		/* no sense in accelerating or turning */
		player->solid->dir_vel = 0.;
		vect_pset( &player->solid->force, 0., 0. );
		return;
	}

	/* turning taken over by PLAYER_FACE */
	if (player_isFlag(PLAYER_FACE)) { 
		if (player_target != PLAYER_ID)
			pilot_face( player,
					vect_angle( &player->solid->pos,
						&pilot_get(player_target)->solid->pos ));
		else if (planet_target != -1)
			pilot_face( player,
					vect_angle( &player->solid->pos,
						&cur_system->planets[ planet_target ].pos ));
	}

	/* turning taken over by PLAYER_REVERSE */
	else if (player_isFlag(PLAYER_REVERSE) && (VMOD(player->solid->vel) > 0.))
		pilot_face( player, VANGLE(player->solid->vel) + M_PI );

	/* normal turning scheme */
	else {
		player->solid->dir_vel = 0.;
		if (player_turn)
			player->solid->dir_vel -= player->turn * player_turn;
	}

	if (player_isFlag(PLAYER_PRIMARY)) pilot_shoot(player,player_target,0);
	if (player_isFlag(PLAYER_SECONDARY)) /* needs target */
		pilot_shoot(player,player_target,1);

	if (player_isFlag(PLAYER_AFTERBURNER)) /* afterburn! */
		vect_pset( &player->solid->force,
				player->thrust * player->afterburner->outfit->u.afb.thrust_perc + 
				player->afterburner->outfit->u.afb.thrust_abs, player->solid->dir );
	else
		vect_pset( &player->solid->force, player->thrust * player_acc,
				player->solid->dir );

	/* set the listener stuff */
	sound_listener( player->solid->dir,
			player->solid->pos.x, player->solid->pos.y,
			player->solid->vel.x, player->solid->vel.y );
}


/*
 *
 * 	For use in keybindings
 *
 */
/*
 * modifies the radar resolution
 */
void player_setRadarRel( int mod )
{
	gui.radar.res += mod * RADAR_RES_INTERVAL;
	if (gui.radar.res > RADAR_RES_MAX) gui.radar.res = RADAR_RES_MAX;
	else if (gui.radar.res < RADAR_RES_MIN) gui.radar.res = RADAR_RES_MIN;

	player_message( "Radar set to %dx", (int)gui.radar.res );
}


/*
 * get the next secondary weapon
 */
void player_secondaryNext (void)
{
	int i = 0;
	
	/* get current secondary weapon pos */
	if (player->secondary != NULL)	
		for (i=0; i<player->noutfits; i++)
			if (&player->outfits[i] == player->secondary) {
				i++;
				break;
			}

	/* get next secondary weapon */
	for (; i<player->noutfits; i++)
		if (outfit_isProp(player->outfits[i].outfit, OUTFIT_PROP_WEAP_SECONDARY)) {
			player->secondary = player->outfits + i;
			break;
		}

	/* found no bugger outfit */
	if (i >= player->noutfits)
		player->secondary = NULL;

	/* set ammo */
	pilot_setAmmo(player);
}


/*
 * cycle through planet targets
 */
void player_targetPlanet (void)
{
	hyperspace_target = -1;
	player_rmFlag(PLAYER_LANDACK);

	/* no target */
	if ((planet_target==-1) && (cur_system->nplanets > 0)) {
		planet_target = 0;
		return;
	}
	
	planet_target++;

	if (planet_target >= cur_system->nplanets) /* last system */
		planet_target = -1;
}


/*
 * try to land or target closest planet if no land target
 */
void player_land (void)
{
	if (landed) { /* player is already landed */
		takeoff();
		return;
	}

	Planet* planet = &cur_system->planets[planet_target];
	if (planet_target >= 0) { /* attempt to land */
		if (!planet_hasService(planet, PLANET_SERVICE_LAND)) {
			player_message( "You can't land here." );
			return;
		}
		else if (!player_isFlag(PLAYER_LANDACK)) { /* no landing authorization */
			if (!areEnemies( player->faction, planet->faction )) {              
				player_message( "%s> Permission to land granted.", planet->name );
				player_setFlag(PLAYER_LANDACK);
			}
			else {
				player_message( "%s> Land request denied.", planet->name );
			}
			return;
		}
		else if (vect_dist(&player->solid->pos,&planet->pos) > planet->gfx_space->sw) {
			player_message("You are too far away to land on %s", planet->name);
			return;
		} else if ((pow2(VX(player->solid->vel)) + pow2(VY(player->solid->vel))) >
				(double)pow2(MAX_HYPERSPACE_VEL)) {
			player_message("You are going to fast to land on %s", planet->name);
			return;
		}

		land(planet); /* land the player */
	}
	else { /* get nearest planet target */

		int i;
		int tp;
		double td, d;

		td = -1; /* temporary distance */
		tp = -1; /* temporary planet */
		for (i=0; i<cur_system->nplanets; i++) {
			d = vect_dist(&player->solid->pos,&cur_system->planets[i].pos);
			if (planet_hasService(&cur_system->planets[i],PLANET_SERVICE_LAND) &&
						((tp==-1) || ((td == -1) || (td > d)))) {
				tp = i;
				td = d;
			}
		}
		planet_target = tp;
		player_rmFlag(PLAYER_LANDACK);
		player_land(); /* rerun land protocol */
	}
}


/*
 * gets a hyperspace target
 */
void player_targetHyperspace (void)
{
	planet_target = -1; /* get rid of planet target */
	player_rmFlag(PLAYER_LANDACK); /* get rid of landing permission */
	hyperspace_target++;

	if (hyperspace_target >= cur_system->njumps)
		hyperspace_target = -1;
}


/*
 * actually attempts to jump in hyperspace
 */
void player_jump (void)
{
	if ((hyperspace_target == -1) ||
			pilot_isFlag(player, PILOT_HYP_PREP) ||
			pilot_isFlag(player, PILOT_HYP_BEGIN) ||
			pilot_isFlag(player, PILOT_HYPERSPACE))
		return;

	int i = space_hyperspace(player);

	if (i == -1)
		player_message("You are too close to gravity centers to initiate hyperspace");
	else if (i == -2)
		player_message("You are moving too fast to enter hyperspace.");
	else
		player_message("Preparing for hyperspace");
}


/*
 * player actually broke hyperspace (entering new system)
 */
void player_brokeHyperspace (void)
{
	/* enter the new system */
	space_init( systems_stack[cur_system->jumps[hyperspace_target]].name );

	/* set position, the pilot_update will handle lowering vel */
	player_warp( -cos( player->solid->dir ) * MIN_HYPERSPACE_DIST * 2.5,
			-sin( player->solid->dir ) * MIN_HYPERSPACE_DIST * 2.5 );

	/* stop hyperspace */
	pilot_rmFlag( player, PILOT_HYPERSPACE | PILOT_HYP_BEGIN | PILOT_HYP_PREP );
}


/*
 * makes player face his hyperspace target
 */
double player_faceHyperspace (void)
{
	double a;
	a = ANGLE( systems_stack[ cur_system->jumps[hyperspace_target] ].pos.x -
				cur_system->pos.x,
			systems_stack[ cur_system->jumps[hyperspace_target] ].pos.y -
				cur_system->pos.y );
	return pilot_face( player, a );
}


/*
 * activate the afterburner
 */
void player_afterburn (void)
{
	/* TODO fancy effect? */
	if (player->afterburner!=NULL) {
		player_setFlag(PLAYER_AFTERBURNER);
		pilot_setFlag(player,PILOT_AFTERBURNER);
	}
}
void player_afterburnOver (void)
{
	if (player->afterburner!=NULL) {
		player_rmFlag(PLAYER_AFTERBURNER);
		pilot_rmFlag(player,PILOT_AFTERBURNER);
	}
}

/*
 * take a screenshot
 */
static int screenshot_cur = 0;
void player_screenshot (void)
{
	FILE *fp;
	int done;
	char filename[PATH_MAX];

	done = 0;
	do {
		if (screenshot_cur >= 128) { /* in case the crap system breaks :) */
			WARN("You have reached the maximum amount of screenshots [128]");
			return;
		}
		snprintf( filename, PATH_MAX, "screenshot%03d.png", screenshot_cur );
		fp = fopen( filename, "r" ); /* yes i know it's a cheesy way to check */
		if (fp==NULL) done = 1;
		else { /* next */
			screenshot_cur++;
			fclose(fp);
		}
		fp = NULL;
	} while (!done);


	/* now proceed to take the screenshot */
	DEBUG( "Taking screenshot [%03d]...", screenshot_cur );
	gl_screenshot(filename);
}


/*
 * player got pwned
 */
void player_dead (void)
{
	gui_xoff = 0.;
	gui_yoff = 0.;
}
/*
 * player blew up in a fireball
 */
void player_destroyed (void)
{
	vectcpy( &player_cam, &player->solid->pos );
	gl_bindCamera( &player_cam );
	player_setFlag(PLAYER_DESTROYED);
	player_timer = SDL_GetTicks() + 5000;
}

