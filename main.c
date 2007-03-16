

#include "SDL.h"

#include <stdlib.h>
#include <math.h>

#include "all.h"
#include "log.h"
#include "physics.h"
#include "opengl.h"
#include "ship.h"
#include "pilot.h"


int main ( int argc, const char** argv )
{
	int quit = 0;
	SDL_Event event;

	gl_screen.w = 800;
	gl_screen.h = 640;
	gl_screen.fullscreen = 0;

	gl_init();




	gl_texture *llama;
	if ((llama = gl_newSprite("llama.png", 6, 6)) == NULL) {
		WARN("Unable to load image");
		return -1;
	}
	Ship* ship = MALLOC_ONE(Ship);
	ship->gfx_ship = llama;
	ship->mass = 1;
	ship->class = SHIP_CLASS_CIVILIAN;
	Pilot* player = pilot_create( ship, "player", NULL, NULL, PILOT_PLAYER );
	gl_bindCamera(&player->solid->pos);

	/*int i,j;
	Vector2d vtemp = { .x=0.0, .y=0.0 };
	i = j = 0;
	while (!quit) {
		while  (SDL_PollEvent(&event)) {
			switch(event.type) {               
				case SDL_KEYDOWN:               
					switch(event.key.keysym.sym) {
						case SDLK_q:
							quit = 1;
							break;
						default:
							break;
					}
			}
		}
		if (j < llama->sy) {
			if (i < llama->sx) {
				gl_blitSprite( llama, &vtemp, i, j );
				SDL_GL_SwapBuffers();
				glClear( GL_COLOR_BUFFER_BIT );
				SDL_Delay( 100 );
				i++;
			}
			else {
				i = 0;
				j++;
			}
		}
		else {
			i = j = 0;
		}
	}*/

	int tflag = 0;
	unsigned int time = SDL_GetTicks();
	FP dt;
	while (!quit) {
		while  (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
						case SDLK_q:
							quit = 1;
							break;
						case SDLK_LEFT:
							tflag ^= 1;
							break;
						case SDLK_RIGHT:
							tflag ^= 2;
							break;
						case SDLK_UP:
							tflag ^= 8;
							break;
						default:
							break;
					}
					break;
				case SDL_KEYUP:
					switch(event.key.keysym.sym) {
						case SDLK_LEFT:
							tflag ^= 1;
							break;
						case SDLK_RIGHT:
							tflag ^= 2;
							break;
						case SDLK_UP:
							tflag ^= 8;
							break;
						default:
							break;
					}
					break;
			}
		}
		dt = (FP)(SDL_GetTicks()-time)/1000.0;
		if (tflag & 1) player->solid->dir += 200.0/180.0*M_PI*dt;
		if (tflag & 2) player->solid->dir -= 200.0/180.0*M_PI*dt;
		if (tflag & 8) player->solid->force = 340;
		else player->solid->force = 0;
		glClear(GL_COLOR_BUFFER_BIT);
		player->update(player, dt);
		SDL_GL_SwapBuffers();
		time = SDL_GetTicks();
		SDL_Delay(5);
	}

	pilot_free(player);
	free(ship);
	gl_free(llama);



	gl_exit();

	return 0;
}

