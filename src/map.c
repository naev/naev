/*
 * See Licensing and Copyright notice in naev.h
 */


#include "map.h"


#include "log.h"
#include "naev.h"
#include "toolkit.h"
#include "space.h"
#include "opengl.h"


#define MAP_WIDTH			550
#define MAP_HEIGHT		400

#define BUTTON_WIDTH		60
#define BUTTON_HEIGHT	30


static int map_wid = 0;
static double map_xpos = 0.;
static double map_ypos = 0.;
static int map_selected = 0;

/*
 * extern
 */
extern StarSystem *systems_stack;
extern int systems_nstack;


/*
 * prototypes
 */
static void map_close( char* str );
static void map_update (void);
static void map_render( double bx, double by, double w, double h );
static void map_mouse( Uint8 type, double mx, double my );


/*
 * opens the map window
 */
void map_open (void)
{
	if (map_wid) map_close(NULL);

	/* set position to focus on current system */
	map_xpos = cur_system->pos.x;
	map_ypos = cur_system->pos.y;

	map_wid = window_create( "Star Map", -1, -1, MAP_WIDTH, MAP_HEIGHT );

	window_addText( map_wid, -20, -20, 100, 20, 1, "txtSysname",
			&gl_defFont, &cDConsole, systems_stack[ map_selected ].name );
	window_addText( map_wid, -20, -60, 90, 20, 0, "txtSFaction",
			&gl_smallFont, &cDConsole, "Faction:" );
	window_addText( map_wid, -20, -60-gl_smallFont.h-5, 80, 100, 0, "txtFaction",
			&gl_smallFont, &cBlack, NULL );
	window_addText( map_wid, -20, -110, 90, 20, 0, "txtSPlanets",
			&gl_smallFont, &cDConsole, "Planets:" );
	window_addText( map_wid, -20, -110-gl_smallFont.h-5, 80, 100, 0, "txtPlanets",
			&gl_smallFont, &cBlack, NULL );
			

	window_addCust( map_wid, 20, 20, MAP_WIDTH - 150, MAP_HEIGHT - 60,
			"cstMap", 1, map_render, map_mouse );
	window_addButton( map_wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnClose", "Close", map_close );

	map_update();
}
static void map_close( char* str )
{
	(void)str;
	if (map_wid) {
		window_destroy( map_wid );
		map_wid = 0;
	}
}
static void map_update (void)
{
	int i;
	StarSystem* sys;
	Faction* f;
	char buf[100];

	sys = &systems_stack[ map_selected ];

	window_modifyText( map_wid, "txtSysname", sys->name );

	if (sys->nplanets == 0) /* no planets -> no factions */
		snprintf( buf, 100, "NA" );
	else {
		f = NULL;
		for (i=0; i<sys->nplanets; i++) {
			if (f==NULL)
				f = sys->planets[i].faction;
			else if (f!= sys->planets[i].faction) { /* TODO more verbosity */
				snprintf( buf, 100, "Multiple" );
				break;
			}
		}
		if (i==sys->nplanets) /* saw them all and all the same */
			snprintf( buf, 100, "%s", f->name );
	}
	window_modifyText( map_wid, "txtFaction", buf );

	buf[0] = '\0';
	if (sys->nplanets == 0)
		snprintf( buf, 100, "None" );
	else {
		for (i=0; i<sys->nplanets; i++) {
			strcat( buf, sys->planets[i].name );
			strcat( buf, "\n" );
		}
	}
	window_modifyText( map_wid, "txtPlanets", buf );
}


/*
 * renders the map as a custom widget
 */
static void map_render( double bx, double by, double w, double h )
{
	int i;
	double x,y,r;
	StarSystem* sys;

	r = 5.;
	x = bx - map_xpos + w/2;
	y = by - map_ypos + h/2;

	/* background */
	COLOUR(cBlack);
	glBegin(GL_QUADS);
		glVertex2d( bx, by );
		glVertex2d( bx, by+h );
		glVertex2d( bx+w, by+h );
		glVertex2d( bx+w, by );
	glEnd(); /* GL_QUADS */


	for (i=0; i<systems_nstack; i++) {
		if (&systems_stack[i]==cur_system) COLOUR(cRadar_targ);
		else COLOUR(cYellow);
		gl_drawCircleInRect( x + systems_stack[i].pos.x,
				y + systems_stack[i].pos.y,
				r, bx, by, w, h );
	}

	/* selected planet */
	sys = &systems_stack[ map_selected ];
	COLOUR(cRed);
	gl_drawCircleInRect( x + sys->pos.x, y + sys->pos.y, r+3., bx, by, w, h );
}
/*
 * map event handling
 */
static void map_mouse( Uint8 type, double mx, double my )
{
}


