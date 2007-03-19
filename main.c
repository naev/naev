

/*
 * includes
 */
/* localised global */
#include "SDL.h"

/* global */
#include <stdlib.h>
#include <math.h>

/* local */
#include "all.h"
#include "log.h"
#include "physics.h"
#include "opengl.h"
#include "ship.h"
#include "pilot.h"



static int quit = 0;

/*
 * prototypes
 */

/* event handling */
static void handle_keydown(SDLKey key);
static void handle_keyup(SDLKey key);

/* update */
static void update_all(void);



/*
 * main
 */
int main ( int argc, const char** argv )
{
	SDL_Event event;

	/*
	 * OpenGL
	 */
	/* default window parameters */
	gl_screen.w = 800;
	gl_screen.h = 640;
	gl_screen.fullscreen = 0;
	if (gl_init()) { /* initializes video output */
		WARN("Error initializing video output, exiting...");
		exit(EXIT_FAILURE);
	}

	
	/*
	 * data loading
	 */
	ships_load();


	/* 
	 * main loop
	 */
	while (!quit) {
		while  (SDL_PollEvent(&event)) { /* event loop */
			switch(event.type) {
				case SDL_KEYDOWN:
					handle_keydown(event.key.keysym.sym);
					break;
				case SDL_KEYUP:
					handle_keyup(event.key.keysym.sym);
					break;
				
				case SDL_QUIT: /* window closed or such */
					quit = 1;
					break;
			}
		}
		update_all();
	}


	/*
	 * data unloading
	 */
	ships_free();

	gl_exit(); /* kills video output */

	exit(EXIT_SUCCESS);
}


/*
 * handles key down events
 */
static void handle_keydown(SDLKey key)
{
	switch (key) {
		case SDLK_ESCAPE:
			quit = 1;
			break;

		default:
			break;
	}
}

/*
 * handles key up events
 */
static void handle_keyup(SDLKey key)
{
	switch (key) {
		
		default:
			break;
	}
}


/*
 * updates everything
 */
static void update_all(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	SDL_GL_SwapBuffers();
}
