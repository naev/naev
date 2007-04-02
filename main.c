

/*
 * includes
 */
/* localised global */
#include "SDL.h"

/* global */
#include <unistd.h>	/* getopt */

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



static int quit = 0;

static unsigned int time = 0;

/*
 * prototypes
 */

/* update */
static void update_all (void);


/*
 * usage
 */
void print_usage( char **argv )
{
	LOG("USAGE: %s [-f] [-j n] [-hv]", argv[0]);
	LOG("Options are:");
	LOG("   -f         fullscreen");
/*	LOG("   -w n       set width to n");
	LOG("   -h n       set height to n");*/
	LOG("   -j n       use joystick n");
	LOG("   -h         display this message and exit");
	LOG("   -v         print the version and exit");
}


/*
 * main
 */
int main ( int argc, char** argv )
{
	SDL_Event event;

	/*
	 * defaulte values
	 */
	/* opengl */
	gl_screen.w = 800;
	gl_screen.h = 640;
	gl_screen.fullscreen = 0;
	/* joystick */
	int indjoystick = -1;

	/*
	 * parse arguments
	 */
	int c = 0;
	while ((c = getopt(argc, argv, "fj:hv")) != -1) {
		switch (c) {
			case 'f':
				gl_screen.fullscreen = 1;
				break;
			case 'j':
				indjoystick = atoi(optarg);
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
	/* default window parameters */
	if (gl_init()) { /* initializes video output */
		WARN("Error initializing video output, exiting...");
		exit(EXIT_FAILURE);
	}


	/*
	 * Input
	 */
	if (indjoystick >= 0) {
		if (joystick_init())
			WARN("Error initializing joystick input");
		joystick_use(indjoystick);
	}

	
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

	/*
	 * exit subsystems
	 */
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
	FP dt = (FP)(SDL_GetTicks() - time) / 1000.;
	time = SDL_GetTicks();

	glClear(GL_COLOR_BUFFER_BIT);

	space_render(dt);

	pilots_update(dt);

	SDL_GL_SwapBuffers();
}


