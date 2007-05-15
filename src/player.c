

#include "player.h"

#include <malloc.h>

#include "main.h"
#include "pilot.h"
#include "log.h"
#include "opengl.h"


#define POW2(x)	((x)*(x))


#define KEY_PRESS		1.
#define KEY_RELEASE	-1.
/* keybinding structure */
typedef struct {
	char *name; /* keybinding name, taken from keybindNames */
	KeybindType type; /* type, defined in playe.h */
	unsigned int key; /* key/axis/button event number */
	double reverse; /* 1. if normal, -1. if reversed, only useful for joystick axis */
} Keybind;
static Keybind** player_input; /* contains the players keybindings */
/* name of each keybinding */
const char *keybindNames[] = { "accel", "left", "right", "primary",
		"mapzoomin", "mapzoomout" };


/*
 * player stuff
 */
Pilot* player = NULL; /* ze player */
static double player_turn = 0.; /* turn velocity from input */
static double player_acc = 0.; /* accel velocity from input */
static int player_primary = 0; /* player is shooting primary weapon */


/*
 * pilot stuff for GUI
 */
extern Pilot** pilot_stack;
extern int pilots;

/*
 * weapon stuff for GUI
 */
extern void weapon_minimap( double res, double w, double h );


/*
 * GUI stuff
 */
/*  colors */
typedef struct {
	double r, g, b, a;
} Color;
#define COLOR(x)		(x).r, (x).g, (x).b, (x).a
Color cRadar_player	=	{ .r = 0.4, .g = 0.8, .b = 0.4, .a = 1. };
Color cRadar_neut		=	{ .r = 0.8, .g = 0.8, .b = 0.8, .a = 1. };
Color cRadar_weap		=	{ .r = 0.8, .g = 0,2, .b = 0.2, .a = 1. };

Color cShield			=	{ .r = 0.2, .g = 0.2, .b = 0.8, .a = 1. };
Color cArmor			=	{ .r = 0.5, .g = 0.5, .b = 0.5, .a = 1. };
Color cEnergy			=	{ .r = 0.2, .g = 0.8, .b = 0.2, .a = 1. };
typedef enum { RADAR_RECT, RADAR_CIRCLE } RadarShape;
typedef struct {
	double w,h; /* dimensions */
	RadarShape shape;
	double res; /* resolution */
} Radar;
/* radar resolutions */
#define RADAR_RES_MAX		100.
#define RADAR_RES_MIN		10.
#define RADAR_RES_INTERVAL	10.

typedef struct {
	double w,h;
} Rect;

typedef struct {
	/* graphics */
	gl_texture* gfx_frame;
	Radar radar;
	Rect shield, armor, energy;

	/* positions */
	Vector2d pos_frame;
	Vector2d pos_radar;
	Vector2d pos_shield, pos_armor, pos_energy;
} GUI;
GUI gui; /* ze GUI */
/* needed to render properly */
double gui_xoff = 0.;
double gui_yoff = 0.;


/* 
 * prototypes
 */
extern void pilot_render( Pilot* pilot ); /* from pilot.c */



/*
 * renders the player
 */
void player_render (void)
{
	pilot_render(player);

	/*
	 *    G U I
	 */
	/* frame */
	gl_blitStatic( gui.gfx_frame, &gui.pos_frame );

	/* radar */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glTranslated( VX(gui.pos_radar) - gl_screen.w/2. + gui.radar.w/2.,
				VY(gui.pos_radar) - gl_screen.h/2. - gui.radar.h/2., 0.);
	glBegin(GL_POINTS);
		/* player */
		glColor4d( COLOR(cRadar_player) );
		glVertex2d(  0.,  2. );
		glVertex2d(  0.,  1. );
		glVertex2d(  0.,  0. );
		glVertex2d(  0., -1. );
		glVertex2d(  0., -2. );
		glVertex2d(  2.,  0. );
		glVertex2d(  1.,  0. );
		glVertex2d( -1.,  0. );
		glVertex2d( -2.,  0. );

	int i;
	double x,y,sx,sy;
	Pilot* p;
	switch (gui.radar.shape) {
		case RADAR_RECT:

			glColor4d( COLOR(cRadar_weap) );
			weapon_minimap(gui.radar.res, gui.radar.w, gui.radar.h);
			glEnd(); /* GL_POINTS */


			for (i=1; i<pilots; i++) {
				p = pilot_stack[i];
				x = (p->solid->pos.x - player->solid->pos.x) / gui.radar.res;
				y = (p->solid->pos.y - player->solid->pos.y) / gui.radar.res;
				sx = PILOT_SIZE_APROX/2. * p->ship->gfx_space->sw / gui.radar.res;
				sy = PILOT_SIZE_APROX/2. * p->ship->gfx_space->sh / gui.radar.res;
				if (sx < 1.) sx = 1.;
				if (sy < 1.) sy = 1.;

				if ( (ABS(x) > gui.radar.w/2+sx) || (ABS(y) > gui.radar.h/2.+sy) )
					continue; /* pilot not in range */

				glBegin(GL_QUADS);
					glColor4d( COLOR(cRadar_neut) );
					glVertex2d( MAX(x-sx,-gui.radar.w/2.),/* top-left */
							MIN(y+sy, gui.radar.h/2.) );
					glVertex2d( MIN(x+sx, gui.radar.w/2.), /* top-right */
							MIN(y+sy, gui.radar.h/2.) );
					glVertex2d( MIN(x+sx, gui.radar.w/2.), /* bottom-right */
							MAX(y-sy,-gui.radar.h/2.) );
					glVertex2d( MAX(x-sx,-gui.radar.w/2.), /* bottom-left */
							MAX(y-sy,-gui.radar.h/2.) );
				glEnd(); /* GL_QUADS */
			}
			break;

		case RADAR_CIRCLE:
			for  (i=1; i<pilots; i++) {
				p = pilot_stack[i];
				glColor4d( COLOR(cRadar_neut) );
				glVertex2d( (p->solid->pos.x - player->solid->pos.x) / gui.radar.res,
						(p->solid->pos.y - player->solid->pos.y) / gui.radar.res );
			}
			glEnd(); /* GL_POINTS */
			break;
	}
	glPopMatrix(); /* GL_PROJECTION */

	/* health */
	glBegin(GL_QUADS); /* shield */
		glColor4d( COLOR(cShield) );
		x = VX(gui.pos_shield) - gl_screen.w/2.;
		y = VY(gui.pos_shield) - gl_screen.h/2.;
		sx = player->shield / player->shield_max * gui.shield.w;
		sy = gui.shield.h;
		glVertex2d( x, y );
		glVertex2d( x + sx, y );
		glVertex2d( x + sx, y - sy );
		glVertex2d( x, y - sy );
	glEnd(); /* GL_QUADS */
	glBegin(GL_QUADS); /* armor */
		glColor4d( COLOR(cArmor) );
		x = VX(gui.pos_armor) - gl_screen.w/2.;
		y = VY(gui.pos_armor) - gl_screen.h/2.;
		sx = player->armor / player->armor_max * gui.armor.w;
		sy = gui.armor.h;
		glVertex2d( x, y );
		glVertex2d( x + sx, y );
		glVertex2d( x + sx, y - sy );
		glVertex2d( x, y - sy );
	glEnd(); /* GL_QUADS */
	glBegin(GL_QUADS); /* energy */
		glColor4d( COLOR(cEnergy) );
		x = VX(gui.pos_energy) - gl_screen.w/2.;
		y = VY(gui.pos_energy) - gl_screen.h/2.;
		sx = player->energy / player->energy_max * gui.energy.w;
		sy = gui.energy.h;
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
	 * frame
	 */
	gui.gfx_frame = gl_newImage( "gfx/gui/frame.png" );
	vect_csetmin( &gui.pos_frame,
			gl_screen.w - gui.gfx_frame->w,		/* x */
			gl_screen.h - gui.gfx_frame->h );	/* y */
	gui_xoff = - gui.gfx_frame->w/2.; /* offset is only horizontal and on right side */


	/*
	 * radar
	 */
	gui.radar.res = 10.;
	gui.radar.w = 128.;
	gui.radar.h = 128.;
	gui.radar.shape = RADAR_RECT;
	vect_csetmin( &gui.pos_radar,
			VX(gui.pos_frame) + 11, 						/* x */
			VY(gui.pos_frame) + gui.gfx_frame->h - 10 );	/* y */

	/*
	 * bars
	 */
	gui.shield.w = gui.armor.w = gui.energy.w = 128;
	gui.shield.h = gui.armor.h = gui.energy.h = 10;
	vect_csetmin( &gui.pos_shield,
			VX(gui.pos_frame) + 10,
			VY(gui.pos_frame) + gui.gfx_frame->h - 201 );
	vect_csetmin( &gui.pos_armor,
			VX(gui.pos_frame) + 10,
			VY(gui.pos_frame) + gui.gfx_frame->h - 218 );
	vect_csetmin( &gui.pos_energy,
			VX(gui.pos_frame) + 10,
			VY(gui.pos_frame) + gui.gfx_frame->h - 236 );

	return 0;
}
/*
 * frees the GUI
 */
void gui_free (void)
{
	gl_freeTexture( gui.gfx_frame );
}


/*
 * used in pilot.c
 *
 * basically uses keyboard input instead of AI input
 */
void player_think( Pilot* player )
{
	player->solid->dir_vel = 0.;
	if (player_turn)
		player->solid->dir_vel -= player->ship->turn * player_turn;

	if (player_primary) pilot_shoot(player,0);

	vect_pset( &player->solid->force, player->ship->thrust * player_acc, player->solid->dir );
}


/*
 *
 *
 *		I N P U T
 *
 */
/*
 * initialization/exit functions (does not assign keys)
 */
void input_init (void)
{
	Keybind *temp;
	int i;
	for (i=0; keybindNames[i]; i++); /* gets number of bindings */
	player_input = malloc(i*sizeof(Keybind*));

	/* creates a null keybinding for each */
	for (i=0; keybindNames[i]; i++) {
		temp = MALLOC_ONE(Keybind);
		temp->name = (char*)keybindNames[i];
		temp->type = KEYBIND_NULL;
		temp->key = 0;
		temp->reverse = 1.;
		player_input[i] = temp;
	}
}
void input_exit (void)
{
	int i;
	for (i=0; keybindNames[i]; i++)
		free(player_input[i]);
	free(player_input);
}


/*
 * binds key of type type to action keybind
 *
 * @param keybind is the name of the keybind defined above
 * @param type is the type of the keybind
 * @param key is the key to bind to
 * @param reverse is whether to reverse it or not
 */
void input_setKeybind( char *keybind, KeybindType type, int key, int reverse )
{
	int i;
	for (i=0; keybindNames[i]; i++)
		if (strcmp(keybind, player_input[i]->name)==0) {
			player_input[i]->type = type;
			player_input[i]->key = key;
			player_input[i]->reverse = (reverse) ? -1. : 1. ;
			return;
		}
}


/*
 * runs the input command
 *
 * @param keynum is the index of the player_input keybind
 * @param value is the value of the keypress (defined above)
 * @param abs is whether or not it's an absolute value (for them joystick)
 */
static void input_key( int keynum, double value, int abs )
{
	/* accelerating */
	if (strcmp(player_input[keynum]->name,"accel")==0) {
		if (abs) player_acc = value;
		else player_acc += value;
	/* turning left */
	} else if (strcmp(player_input[keynum]->name,"left")==0) {
		if (abs) player_turn = -value;
		else player_turn -= value;
	/* turning right */
	} else if (strcmp(player_input[keynum]->name,"right")==0) {
		if (abs)	player_turn = value;
		else player_turn += value;
	/* shooting primary weapon */
	} else if (strcmp(player_input[keynum]->name, "primary")==0) {
		if (value==KEY_PRESS) player_primary = 1;
		else if (value==KEY_RELEASE) player_primary = 0;
	} else if (strcmp(player_input[keynum]->name, "mapzoomin")==0) {
		if (value==KEY_PRESS && gui.radar.res < RADAR_RES_MAX)
			gui.radar.res += RADAR_RES_INTERVAL;
	} else if (strcmp(player_input[keynum]->name, "mapzoomout")==0) {
		if (value==KEY_PRESS && gui.radar.res > RADAR_RES_MIN)
			gui.radar.res -= RADAR_RES_INTERVAL;
	}

	/* make sure values are sane */
	player_acc = ABS(player_acc);
	if (player_acc > 1.) player_acc = 1.;
	if (player_turn > 1.) player_turn = 1.;
	else if (player_turn < -1.) player_turn = -1.;
}


/*
 * events
 */
/* prototypes */
static void input_joyaxis( const unsigned int axis, const int value );
static void input_joydown( const unsigned int button );
static void input_joyup( const unsigned int button );
static void input_keydown( SDLKey key );
static void input_keyup( SDLKey key );

/*
 * joystick
 */
/* joystick axis */
static void input_joyaxis( const unsigned int axis, const int value )
{
	int i;
	for (i=0; keybindNames[i]; i++)
		if (player_input[i]->type == KEYBIND_JAXIS && player_input[i]->key == axis) {
			input_key(i,-(player_input[i]->reverse)*(double)value/32767.,1);
			return;
		}
}
/* joystick button down */
static void input_joydown( const unsigned int button )
{
	int i;
	for (i=0; keybindNames[i]; i++)
		if (player_input[i]->type == KEYBIND_JBUTTON && player_input[i]->key == button) {
			input_key(i,KEY_PRESS,0);
			return;
		}  
}
/* joystick button up */
static void input_joyup( const unsigned int button )
{
	int i;
	for (i=0; keybindNames[i]; i++)
		if (player_input[i]->type == KEYBIND_JBUTTON && player_input[i]->key == button) {
			input_key(i,KEY_RELEASE,0);
			return;
		} 
}


/*
 * keyboard
 */
/* key down */
static void input_keydown( SDLKey key )
{
	int i;
	for (i=0; keybindNames[i]; i++)
		if (player_input[i]->type == KEYBIND_KEYBOARD && player_input[i]->key == key) {
			input_key(i,KEY_PRESS,0);
			return;
		}

	/* ESC = quit */
	SDL_Event quit;
	if (key == SDLK_ESCAPE) {
		quit.type = SDL_QUIT;
		SDL_PushEvent(&quit);
	}

}
/* key up */
static void input_keyup( SDLKey key )
{
	int i;
	for (i=0; keybindNames[i]; i++)
		if (player_input[i]->type == KEYBIND_KEYBOARD && player_input[i]->key == key) {
			input_key(i,KEY_RELEASE,0);
			return;
		}
}


/*
 * global input
 *
 * basically seperates the event types
 */
void input_handle( SDL_Event* event )
{
	switch (event->type) {
		case SDL_JOYAXISMOTION:
			input_joyaxis(event->jaxis.axis, event->jaxis.value);
			break;

		case SDL_JOYBUTTONDOWN:
			input_joydown(event->jbutton.button);
			break;

		case SDL_JOYBUTTONUP:
			input_joyup(event->jbutton.button);
			break;

		case SDL_KEYDOWN:
			input_keydown(event->key.keysym.sym);
			break;

		case SDL_KEYUP:
			input_keyup(event->key.keysym.sym);
			break;
	}
}



