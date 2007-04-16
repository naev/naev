

/*
 * includes
 */
/* localised global */
#include "SDL.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/* global */
#include <unistd.h>	/* getopt */
#include <string.h> /* strdup */
#include <getopt.h> /* getopt_long */

/* local */
#include "all.h"
#include "log.h"
#include "physics.h"
#include "opengl.h"
#include "ship.h"
#include "pilot.h"
#include "player.h"
#include "joystick.h"
#include "space.h"
#include "rng.h"
#include "ai.h"


#define WINDOW_CAPTION  "game"

#define CONF_FILE			"conf"

#define MINIMUM_FPS		0.5


extern const char *keybindNames[]; /* keybindings */


static int quit = 0; /* for primary loop */
static unsigned int time = 0; /* used to calculate FPS and movement */


/*
 * prototypes
 */
static void print_usage( char **argv );
static void display_fps( const double dt );
/* update */
static void update_all (void);


/*
 * usage
 */
static void print_usage( char **argv )
{
	LOG("Usage: %s [OPTION]", argv[0]);
	LOG("Options are:");
	LOG("   -f, --fullscreen      fullscreen");
	/*LOG("   -w n                  set width to n");
	LOG("   -h n                  set height to n");*/
	LOG("   -j n, --joystick n    use joystick n");
	LOG("   -J s, --Joystick s    use joystick whose name contains s");
	LOG("   -h, --help            display this message and exit");
	LOG("   -v, --version         print the version and exit");
}


/*
 * main
 */
int main ( int argc, char** argv )
{
	int i;

	/*
	 * initializes SDL for possible warnings
	 */
	SDL_Init(0);

	/*
	 * default values
	 */
	/* opengl */
	gl_screen.w = 800;
	gl_screen.h = 640;
	gl_screen.fullscreen = 0;
	/* joystick */
	int indjoystick = -1;
	char* namjoystick = NULL;
	/* input */
	input_init();
	input_setKeybind( "accel", KEYBIND_KEYBOARD, SDLK_UP, 0 );
	input_setKeybind( "left", KEYBIND_KEYBOARD, SDLK_LEFT, 0 ); 
	input_setKeybind( "right", KEYBIND_KEYBOARD, SDLK_RIGHT, 0 );


	/*
	 * Lua to parse the configuration file
	 */
	lua_State *L = luaL_newstate();
	if (luaL_dofile(L, CONF_FILE) == 0) { /* configuration file exists */

		/* opengl properties*/
		lua_getglobal(L, "width");
		if (lua_isnumber(L, -1))
			gl_screen.w = (int)lua_tonumber(L, -1);
		lua_getglobal(L, "height");
		if (lua_isnumber(L, -1))
			gl_screen.h = (int)lua_tonumber(L, -1);
		lua_getglobal(L, "fullscreen");
		if (lua_isnumber(L, -1))
			if ((int)lua_tonumber(L, -1) == 1)
				gl_screen.fullscreen = 1;

		/* joystick */
		lua_getglobal(L, "joystick");
		if (lua_isnumber(L, -1))
			indjoystick = (int)lua_tonumber(L, -1);
		else if (lua_isstring(L, -1))
			namjoystick = strdup((char*)lua_tostring(L, -1));

		/* grab the keybindings if there are any */
		char *str;
		int type, key, reverse;
		for (i=0; keybindNames[i]; i++) {
			lua_getglobal(L, keybindNames[i]);
			str = NULL;
			key = -1;
			reverse = 0;
			if (lua_istable(L, -1)) { /* it's a table */
				/* gets the event type */
				lua_pushstring(L, "type");
				lua_gettable(L, -2);
				if (lua_isstring(L, -1))
					str = (char*)lua_tostring(L, -1);

				/* gets the key */
				lua_pushstring(L, "key");
				lua_gettable(L, -3);
				if (lua_isnumber(L, -1))
					key = (int)lua_tonumber(L, -1);

				/* is reversed, only useful for axis */
				lua_pushstring(L, "reverse");
				lua_gettable(L, -4);
				if (lua_isnumber(L, -1))
					reverse = 1;

				if (key != -1 && str != NULL) { /* keybind is valid */
					/* get type */
					if (strcmp(str,"null")==0) type = KEYBIND_NULL;
					else if (strcmp(str,"keyboard")==0) type = KEYBIND_KEYBOARD;
					else if (strcmp(str,"jaxis")==0) type = KEYBIND_JAXIS;
					else if (strcmp(str,"jbutton")==0) type = KEYBIND_JBUTTON;
					else {
						WARN("Unkown keybinding of type %s", str);
						continue;
					}
					/* set the keybind */
					input_setKeybind( (char*)keybindNames[i], type, key, reverse );
				}
				else WARN("Malformed keybind in %s", CONF_FILE);
			}
		}
	}
	lua_close(L);


	/*
	 * parse arguments
	 */
	static struct option long_options[] = {
			{ "fullscreen", no_argument, 0, 'f' },
			{ "joystick", required_argument, 0, 'j' },
			{ "Joystick", required_argument, 0, 'J' },
			{ "help", no_argument, 0, 'h' },
			{ "version", no_argument, 0, 'v' },
			{ 0, 0, 0, 0 } };
	int option_index = 0;
	int c = 0;
	while ((c = getopt_long(argc, argv, "fJ:j:hv", long_options, &option_index)) != -1) {
		switch (c) {
			case 'f':
				gl_screen.fullscreen = 1;
				break;
			case 'j':
				indjoystick = atoi(optarg);
				break;
			case 'J':
				namjoystick = strdup(optarg);
				break;

			case 'v':
				LOG("main: version %d.%d.%d\n", VMAJOR, VMINOR, VREV);
			case 'h':
				print_usage(argv);
				exit(EXIT_SUCCESS);
		}
	}

	/* random numbers */
	rng_init();


	/*
	 * OpenGL
	 */
	if (gl_init()) { /* initializes video output */
		WARN("Error initializing video output, exiting...");
		exit(EXIT_FAILURE);
	}


	/*
	 * Window
	 */
	SDL_WM_SetCaption( WINDOW_CAPTION, NULL );


	/*
	 * Input
	 */
	if (indjoystick >= 0 || namjoystick != NULL) {
		if (joystick_init())
			WARN("Error initializing joystick input");
		if (namjoystick != NULL) {
			joystick_use(joystick_get(namjoystick));
			free(namjoystick);
		}
		else if (indjoystick >= 0)
			joystick_use(indjoystick);
	}

	/*
	 * Misc
	 */
	if (ai_init())
		WARN("Error initializing AI");

	gl_fontInit( NULL, "/usr/share/fonts/truetype/freefont/FreeSans.ttf", 16 );

	
	/*
	 * data loading
	 */
	ships_load();


	/*
	 * testing
	 */
	pilot_create( get_ship("Llama"), "Player", NULL, NULL, PILOT_PLAYER );
	gl_bindCamera( &player->solid->pos );
	space_init();

	pilot_create( get_ship("Mr. Test"), NULL, NULL, NULL, 0 );

	
	time = SDL_GetTicks();
	/* 
	 * main loop
	 */
	SDL_Event event;
	/* flushes the event loop since I noticed that when the joystick is loaded it
	 * creates button events that results in the player starting out acceling */
	while (SDL_PollEvent(&event));
	/* primary loop */
	while (!quit) {
		while (SDL_PollEvent(&event)) { /* event loop */
			if (event.type == SDL_QUIT) quit = 1; /* quit is handled here */

			input_handle(&event); /* handles all the events and player keybinds */
		}
		update_all();
	}


	/*
	 * data unloading
	 */
	space_exit(); /* cleans up the universe itself */
	pilots_free(); /* frees the pilots, they were locked up :( */
	ships_free();

	gl_freeFont(NULL);

	/*
	 * exit subsystems
	 */
	ai_exit(); /* stops the Lua AI magic */
	joystick_exit(); /* releases joystick */
	input_exit(); /* cleans up keybindings */
	gl_exit(); /* kills video output */

	exit(EXIT_SUCCESS);
}


/*
 * updates everything
 *
 *	Blitting order (layers):
 *	  BG | @ stars and planets
 *	     | @ background particles
 *      X
 *	  N  | @ NPC ships
 *	     | @ normal layer particles (above ships)
 *      X
 *	  FG | @ player
 *	     | @ foreground particles
 *	     | @ text and GUI
 */
static void update_all(void)
{
	/* dt in us */
	double dt = (double)(SDL_GetTicks() - time) / 1000.;
	time = SDL_GetTicks();

	if (dt > MINIMUM_FPS) {
		Vector2d pos;
		vect_cset(&pos, 10., (double)(gl_screen.h-40));
		gl_print( NULL, &pos, "FPS very low, skipping frames" );
		SDL_GL_SwapBuffers();
		return;
	}

	glClear(GL_COLOR_BUFFER_BIT);

	space_render(dt);

	pilots_update(dt);

	display_fps(dt);

	SDL_GL_SwapBuffers();
}


/*
 * displays FPS on the screen
 */
static double fps = 0.;
static double fps_cur = 0.;
static double fps_dt = 1.;
static void display_fps( const double dt )
{
	fps_dt += dt;
	fps_cur += 1.;
	if (fps_dt > 1.) {
		fps = fps_cur / fps_dt;
		fps_dt = fps_cur = 0.;
	}
	Vector2d pos;
	vect_cset(&pos, 10., (double)(gl_screen.h-20));
	gl_print( NULL, &pos, "%3.2f", fps );
}


