

/*
 * includes
 */
/* localised global */
#include "SDL.h"

/* global */
#include <stdlib.h>

/* local */
#include "all.h"
#include "log.h"
#include "physics.h"
#include "opengl.h"
#include "ship.h"
#include "pilot.h"
#include "player.h"
#include "joystick.h"



static int quit = 0;

static unsigned int time = 0;

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
	 * Input
	 */
	if (joystick_init())
		WARN("Error initializing joystick input");

	
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

	pilot_create( get_ship("Mr. Test"), NULL, NULL, NULL, 0 );

	
	time = SDL_GetTicks();
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
	pilots_free();
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
		case SDLK_LEFT:
			player_setFlag(PLAYER_FLAG_MOV_LEFT);
			break;
		case SDLK_RIGHT:
			player_setFlag(PLAYER_FLAG_MOV_RIGHT);
			break;
		case SDLK_UP:
			player_setFlag(PLAYER_FLAG_MOV_ACC);
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
		case SDLK_LEFT:
			player_rmFlag(PLAYER_FLAG_MOV_LEFT);
			break;        
		case SDLK_RIGHT: 
			player_rmFlag(PLAYER_FLAG_MOV_RIGHT);
			break;        
		case SDLK_UP:                     
			player_rmFlag(PLAYER_FLAG_MOV_ACC);
			break;
		
		default:
			break;
	}
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

	pilots_update(dt);

	SDL_GL_SwapBuffers();
}
