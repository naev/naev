

/*
 * includes
 */
/* localised global */
#include "SDL.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/* global */
#include <unistd.h> /* getopt */
#include <string.h> /* strdup */
#include <getopt.h> /* getopt_long */

/* local */
#include "main.h"
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
#include "outfit.h"
#include "pack.h"
#include "weapon.h"


#define APPNAME			"GAME"
#define WINDOW_CAPTION  "game"

#define CONF_FILE			"conf"

#define MINIMUM_FPS		0.5

#define FONT_SIZE			12


extern const char *keybindNames[]; /* keybindings */


static int quit = 0; /* for primary loop */
static unsigned int time = 0; /* used to calculate FPS and movement */

/* some defaults */
#define DATA_DEF		"data"
char* data = NULL;
static int show_fps = 1; /* shows fps - default yes */
static int max_fps = 0;


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
	LOG("   -F n, --fps n         limit frames per second");
	LOG("   -d s, --data s        set the data file to be s");
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
	 * print the version
	 */
	LOG( " "APPNAME" v%d.%d.%d", VMAJOR, VMINOR, VREV );


	/*
	 * initializes SDL for possible warnings
	 */
	SDL_Init(0);

	/*
	 * default values
	 */
	/* global */
	data = DATA_DEF;
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
	input_setKeybind( "primary", KEYBIND_KEYBOARD, SDLK_SPACE, 0 );
	input_setKeybind( "target", KEYBIND_KEYBOARD, SDLK_TAB, 0 );
	input_setKeybind( "mapzoomin", KEYBIND_KEYBOARD, SDLK_9, 0 );
	input_setKeybind( "mapzoomout", KEYBIND_KEYBOARD, SDLK_0, 0 );


	/*
	 * Lua to parse the configuration file
	 */
	lua_State *L = luaL_newstate();
	if (luaL_dofile(L, CONF_FILE) == 0) { /* configuration file exists */

		/* global */
		lua_getglobal(L, "data");
		if (lua_isstring(L, -1))
			data = strdup((char*)lua_tostring(L, -1));

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

		lua_getglobal(L, "fps");
		if (lua_isnumber(L, -1))
			max_fps = (int)lua_tonumber(L, -1);

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
			{ "fps", required_argument, 0, 'F' },
			{ "data", required_argument, 0, 'd' },
			{ "joystick", required_argument, 0, 'j' },
			{ "Joystick", required_argument, 0, 'J' },
			{ "help", no_argument, 0, 'h' },
			{ "version", no_argument, 0, 'v' },
			{ NULL, 0, 0, 0 } };
	int option_index = 0;
	int c = 0;
	while ((c = getopt_long(argc, argv, "fF:d:J:j:hv", long_options, &option_index)) != -1) {
		switch (c) {
			case 'f':
				gl_screen.fullscreen = 1;
				break;
			case 'F':
				max_fps = atoi(optarg);
				break;
			case 'd':
				data = strdup(optarg);
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


	/* check to see if data file is valid */
	if (pack_check(data)) {
		ERR("Data file '%s' not found",data);
		WARN("You should specify which data file to use with '-d'");
		WARN("See -h or --help for more information");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	/* random numbers */
	rng_init();


	/*
	 * OpenGL
	 */
	if (gl_init()) { /* initializes video output */
		WARN("Error initializing video output, exiting...");
		SDL_Quit();
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
		if (namjoystick != NULL) { /* use the joystick name to find a joystick */
			joystick_use(joystick_get(namjoystick));
			free(namjoystick);
		}
		else if (indjoystick >= 0) /* use a joystick id instead */
			joystick_use(indjoystick);
	}

	/*
	 * Misc
	 */
	if (ai_init())
		WARN("Error initializing AI");

	gl_fontInit( NULL, NULL, FONT_SIZE ); /* initializes default font to size */
	gui_init(); /* initializes the GUI graphics */

	
	/*
	 * data loading
	 */
	outfit_load();
	ships_load();
	space_load();


	/*
	 * testing
	 */
	pilot_create( get_ship("Llama"), "Player", 0.,  NULL, NULL, PILOT_PLAYER );
	gl_bindCamera( &player->solid->pos );
	space_init("Delta Pavonis");

	pilot_create( get_ship("Mr. Test"), NULL, 2., NULL, NULL, 0 );


	player_message( "Welcome to "APPNAME"!" );
	player_message( " v%d.%d.%d", VMAJOR, VMINOR, VREV );

	
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
	weapon_exit(); /* destroys all active weapons */
	space_exit(); /* cleans up the universe itself */
	pilots_free(); /* frees the pilots, they were locked up :( */
	gui_free(); /* frees up the player's GUI */
	ships_free();
	outfit_free();

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
 *	     | @ back layer weapons
 *      X
 *	  N  | @ NPC ships
 *	     | @ normal layer particles (above ships)
 *	     | @ front layer weapons
 *      X
 *	  FG | @ player
 *	     | @ foreground particles
 *	     | @ text and GUI
 */
static double fps_dt = 1.;
static void update_all(void)
{
	/* dt in us */
	double dt = (double)(SDL_GetTicks() - time) / 1000.;
	time = SDL_GetTicks();

	if (dt > MINIMUM_FPS) { /* TODO needs work */
		Vector2d pos;
		vect_csetmin(&pos, 10., (double)(gl_screen.h-40));
		gl_print( NULL, &pos, "FPS very low, skipping frames" );
		SDL_GL_SwapBuffers();
		return;
	}
	/* if fps is limited */
	else if ( max_fps != 0 && dt < 1./max_fps) {
		double delay = 1./max_fps - dt;
		SDL_Delay( delay );
		fps_dt += delay; /* makes sure it displays the proper fps */
	}

	glClear(GL_COLOR_BUFFER_BIT);

	/* BG */
	space_render(dt);
	planets_render();
	weapons_update(dt,WEAPON_LAYER_BG);
	/* N */
	pilots_update(dt);
	weapons_update(dt,WEAPON_LAYER_FG);
	/* FG */
	player_render();
	display_fps(dt);

	SDL_GL_SwapBuffers();
}


/*
 * displays FPS on the screen
 */
static double fps = 0.;
static double fps_cur = 0.;
static void display_fps( const double dt )
{
	fps_dt += dt;
	fps_cur += 1.;
	if (fps_dt > 1.) {
		fps = fps_cur / fps_dt;
		fps_dt = fps_cur = 0.;
	}
	Vector2d pos;
	vect_csetmin(&pos, 10., (double)(gl_screen.h-20));
	if (show_fps)
		gl_print( NULL, &pos, "%3.2f", fps );
}


