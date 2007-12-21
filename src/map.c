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
static void map_render( double bx, double by, double w, double h );
static void map_close( char* str );


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

	window_addCust( map_wid, 20, 20, MAP_WIDTH - 150, MAP_HEIGHT - 60,
			"cstMap", 1, map_render );
	window_addButton( map_wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnClose", "Close", map_close );
}
static void map_close( char* str )
{
	(void)str;
	if (map_wid) {
		window_destroy( map_wid );
		map_wid = 0;
	}
}


/*
 * renders the map as a custom widget
 */
static void map_render( double bx, double by, double w, double h )
{
	int i;

	/* background */
	COLOUR(cBlack);
	glBegin(GL_QUADS);
		glVertex2d( bx, by );
		glVertex2d( bx, by+h );
		glVertex2d( bx+w, by+h );
		glVertex2d( bx+w, by );
	glEnd(); /* GL_QUADS */


	for (i=0; i<systems_nstack; i++) {
		if (i==map_selected) COLOUR(cRadar_targ);
		else COLOUR(cYellow);
		gl_drawCircleInRect( bx + systems_stack[i].pos.x - map_xpos + w/2,
				by + systems_stack[i].pos.y - map_ypos + h/2,
				5, bx, by, w, h );
	}
}


