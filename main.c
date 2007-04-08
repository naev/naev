

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


#define CONF_FILE	"conf"

static gl_font fdefault;

static int quit = 0;

static unsigned int time = 0;

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
	LOG("Usage: %s [-f] [-j n | -J s] [-hv]", argv[0]);
	LOG("Options are:");
	LOG("   -f         fullscreen");
/*	LOG("   -w n       set width to n");
	LOG("   -h n       set height to n");*/
	LOG("   -j n       use joystick n");
	LOG("   -J s       use joystick whose name contains s");
	LOG("   -h         display this message and exit");
	LOG("   -v         print the version and exit");
}


/*
 * main
 */
int main ( int argc, char** argv )
{
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


	/*
	 * Lua to parse the configuration file
	 */
	lua_State *L = luaL_newstate();
	if (luaL_dofile(L, CONF_FILE) == 0) {
		/* opengl */
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
	}
	lua_close(L);


	/*
	 * parse arguments
	 */
	int c = 0;
	while ((c = getopt(argc, argv, "fJ:j:hv")) != -1) {
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
	 * 
	 * SDL_Init is first called here, so it's important to be first
	 * initializaction
	 */
	if (gl_init()) { /* initializes video output */
		WARN("Error initializing video output, exiting...");
		exit(EXIT_FAILURE);
	}


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

	gl_fontInit( &fdefault, "/usr/share/fonts/truetype/freefont/FreeSans.ttf", 16 );

	
	/*
	 * data loading
	 */
	ships_load();


	/*
	 * testing
	 */
	unsigned int player_id;
	player_id = pilot_create( get_ship("Llama"), "Player", NULL, NULL, PILOT_PLAYER );
	gl_bindCamera( &get_pilot(player_id)->solid->pos );
	space_init();

	pilot_create( get_ship("Mr. Test"), NULL, NULL, NULL, 0 );

	
	time = SDL_GetTicks();
	/* 
	 * main loop
	 */
	SDL_Event event;
	while (!quit) {
		while  (SDL_PollEvent(&event)) { /* event loop */
			if (event.type == SDL_QUIT) quit = 1; /* quit is handled here */

			handle_input(&event);
		}
		update_all();
	}


	space_exit();

	/*
	 * data unloading
	 */
	pilots_free();
	ships_free();

	gl_freeFont(&fdefault);

	/*
	 * exit subsystems
	 */
	ai_exit();
	joystick_exit();
	gl_exit(); /* kills video output */

	exit(EXIT_SUCCESS);
}


/*
 * updates everything
 *
 * @pilots
 *  -> pilot think (AI)
 *  -> pliot solid
 */
static void update_all(void)
{
	double dt = (double)(SDL_GetTicks() - time) / 1000.;
	time = SDL_GetTicks();

	glClear(GL_COLOR_BUFFER_BIT);

	space_render(dt);

	pilots_update(dt);

	display_fps(dt);

	SDL_GL_SwapBuffers();
}

static double fps = 0.;
static double fps_cur = 0.;
static double fps_dt = 1.;
static void display_fps( const double dt )
{
	fps_dt += dt;
	fps_cur += 1.;
	if (fps_dt > 1.) {
		fps = fps_cur;
		fps_dt = fps_cur = 0.;
	}
	Vector2d pos = { .x = 10., .y = (double)(gl_screen.h-20)  };
	gl_print( &fdefault, &pos, "%3.2f", fps );
}


