/*
 * See Licensing and Copyright notice in naev.h
 */


#include "map.h"


#include "log.h"
#include "naev.h"
#include "toolkit.h"
#include "space.h"
#include "opengl.h"


#define WINDOW_WIDTH		550
#define WINDOW_HEIGHT	440

#define MAP_WIDTH			(WINDOW_WIDTH-150)
#define MAP_HEIGHT		(WINDOW_HEIGHT-100)

#define BUTTON_WIDTH		60
#define BUTTON_HEIGHT	30


static int map_wid = 0;
static double map_zoom = 1.; /* zoom of the map */
static double map_xpos = 0.; /* map position */
static double map_ypos = 0.;
static int map_selected = 0;

static int map_drag = 0; /* is the user dragging the map? */

/*
 * extern
 */
/* space.c */
extern StarSystem *systems_stack;
extern int systems_nstack;
/* player.c */
extern int planet_target;
extern int hyperspace_target;


/*
 * prototypes
 */
static void map_close( char* str );
static void map_update (void);
static void map_render( double bx, double by, double w, double h );
static void map_mouse( SDL_Event* event, double mx, double my );
static void map_buttonZoom( char* str );


/*
 * opens the map window
 */
void map_open (void)
{
	if (map_wid) {
		map_close(NULL);
		return;
	}

	/* set position to focus on current system */
	map_xpos = cur_system->pos.x;
	map_ypos = cur_system->pos.y;

	map_wid = window_create( "Star Map", -1, -1,
			WINDOW_WIDTH, WINDOW_HEIGHT );

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
			

	window_addCust( map_wid, 20, -40, MAP_WIDTH, MAP_HEIGHT,
			"cstMap", 1, map_render, map_mouse );
	window_addButton( map_wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnClose", "Close", map_close );

	window_addButton( map_wid, 40, 20, 30, 30, "btnZoomIn", "+", map_buttonZoom );
	window_addButton( map_wid, 80, 20, 30, 30, "btnZoomOut", "-", map_buttonZoom );

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
	int i,j;
	double x,y,r, tx,ty;
	StarSystem* sys;
	glColour* col;

	r = 5.;
	x = (bx - map_xpos + w/2) * 1.;//map_zoom;
	y = (by - map_ypos + h/2) * 1.;//map_zoom;

	/* background */
	COLOUR(cBlack);
	glBegin(GL_QUADS);
		glVertex2d( bx, by );
		glVertex2d( bx, by+h );
		glVertex2d( bx+w, by+h );
		glVertex2d( bx+w, by );
	glEnd(); /* GL_QUADS */


	/* render the star systems */
	for (i=0; i<systems_nstack; i++) {
		
		sys = &systems_stack[i];

		/* draw the system */
		if (sys==cur_system) COLOUR(cRadar_targ);
		else if (sys->nplanets==0) COLOUR(cInert);
		else COLOUR(cYellow);
		gl_drawCircleInRect( x + sys->pos.x*map_zoom, y + sys->pos.y*map_zoom,
				r, bx, by, w, h );

		/* draw the hyperspace paths */
		glShadeModel(GL_SMOOTH);
		/* cheaply use transparency instead of actually calculating
		 * from where to where the line must go :) */  
		for (j=0; j<sys->njumps; j++) {
			/* set the colours */
			/* is the route the current one? */
			if (((cur_system==sys) && (j==hyperspace_target)) ||
					((cur_system==&systems_stack[ sys->jumps[j] ]) &&
					 (sys==&systems_stack[ cur_system->jumps[hyperspace_target] ] )))
				col = &cRed;
			else col = &cInert;

			glBegin(GL_LINE_STRIP);
				ACOLOUR(*col,0.);
				tx = x + sys->pos.x * map_zoom;
				ty = y + sys->pos.y * map_zoom;
				if (!((tx < bx) || (tx > bx+w) || (ty < by) || (ty > by+h)))
					glVertex2d( tx, ty );
				COLOUR(*col);
				tx += (systems_stack[ sys->jumps[j] ].pos.x - sys->pos.x)/2. * map_zoom;
				ty += (systems_stack[ sys->jumps[j] ].pos.y - sys->pos.y)/2. * map_zoom;
				if (!((tx < bx) || (tx > bx+w) || (ty < by) || (ty > by+h)))
					glVertex2d( tx, ty );
				ACOLOUR(*col,0.);
				tx = x + systems_stack[ sys->jumps[j] ].pos.x * map_zoom;
				ty = y + systems_stack[ sys->jumps[j] ].pos.y * map_zoom;
				if (!((tx < bx) || (tx > bx+w) || (ty < by) || (ty > by+h)))
					glVertex2d( tx, ty );
			glEnd(); /* GL_LINE_STRIP */
		}
		glShadeModel(GL_FLAT);
	}

	/* selected planet */
	sys = &systems_stack[ map_selected ];
	COLOUR(cRed);
	gl_drawCircleInRect( x + sys->pos.x * map_zoom, y + sys->pos.y * map_zoom,
			r+3., bx, by, w, h );
}
/*
 * map event handling
 */
static void map_mouse( SDL_Event* event, double mx, double my )
{
	int i, j;
	double x,y, t;

	t = 15.*15.; /* threshold */

	mx -= MAP_WIDTH/2 - map_xpos;
	my -= MAP_HEIGHT/2 - map_ypos;

	switch (event->type) {
		
		case SDL_MOUSEBUTTONDOWN:
			/* selecting star system */
			if (event->button.button==SDL_BUTTON_LEFT) {
				for (i=0; i<systems_nstack; i++) {
					x = systems_stack[i].pos.x * map_zoom;
					y = systems_stack[i].pos.y * map_zoom;

					if ((pow2(mx-x)+pow2(my-y)) < t) {
						map_selected = i;
						for (j=0; j<cur_system->njumps; j++)
							if (i==cur_system->jumps[j]) {
								planet_target = -1; /* override planet_target */
								hyperspace_target = j;
								break;
							}

						map_update();
						break;
					}
				}
			}
			/* start dragging */
			else if (event->button.button==SDL_BUTTON_RIGHT)
				map_drag = 1;
			break;

		case SDL_MOUSEBUTTONUP:
			if ((event->button.button==SDL_BUTTON_RIGHT) && map_drag)
				map_drag = 0;
			break;

		case SDL_MOUSEMOTION:
			if (map_drag) {
				/* axis is inverted */
				map_xpos -= event->motion.xrel;
				map_ypos += event->motion.yrel;
			}
			break;
	}
}
static void map_buttonZoom( char* str )
{
	if (strcmp(str,"btnZoomIn")==0)
		map_zoom = MIN(2., map_zoom+0.5);
	else if (strcmp(str,"btnZoomOut")==0)
		map_zoom = MAX(0.5, map_zoom-0.5);
}


