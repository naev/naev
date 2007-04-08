

#include "player.h"

#include "all.h"
#include "pilot.h"
#include "log.h"


Pilot* player = NULL;
static unsigned int player_flags = PLAYER_FLAG_NULL;
static double player_turn = 0.;
static double player_acc = 0.;

/*
 * used in pilot.c
 */
void player_think( Pilot* player, const double dt )
{
	player->solid->dir_vel = 0.;
	if (player_turn)
		player->solid->dir_vel -= player->ship->turn*player_turn/(double)(1<<15);
/*
	if (player_isFlag(PLAYER_FLAG_MOV_LEFT))
		player->solid->dir_vel += player->ship->turn;
	if (player_isFlag(PLAYER_FLAG_MOV_RIGHT))
		player->solid->dir_vel -= player->ship->turn;
*/

	player->solid->force = player->ship->thrust*player_acc/(double)(1<<15);
}


/*
 * flag manipulation
 */
int player_isFlag( unsigned int flag )
{
	return player_flags & flag;
}
void player_setFlag( unsigned int flag )
{
	if (!player_isFlag(flag))
		player_flags |= flag;
}
void player_rmFlag( unsigned int flag )
{
	if (player_isFlag(flag))
		player_flags ^= flag;
}


/*
 * events
 */

/*
 * joystick
 */
static void handle_joyaxis( int axis, int value )
{
	switch (axis) {
		case 0:
			player_turn = (double)value;
			break;

		case 1:
			if (value <= 0)
				player_acc = (double)-value;
			break;
	}
}
static void handle_joydown( int button )
{
	switch (button) {
		case 0:
			player_acc += (double)(1<<15);
			break;
		case 1:
			break;

	}
}
static void handle_joyup( int button )
{
	switch (button) {
		case 0:
			player_acc -= (double)(1<<15);
			break;
		case 1:
			break;
	}
}


/*
 * keyboard
 */
static void handle_keydown(SDLKey key)
{
	SDL_Event quit;
	switch (key) {
		case SDLK_ESCAPE:
			quit.type = SDL_QUIT;
			SDL_PushEvent(&quit);
			break;
		case SDLK_LEFT:
			player_turn -= (double)(1<<15);
			break;
		case SDLK_RIGHT:
			player_turn += (double)(1<<15);
			break;
		case SDLK_UP:
			player_acc += (double)(1<<15);
			break;

		default:
			break;
	}
}
static void handle_keyup(SDLKey key)
{  
	switch (key) {
		case SDLK_LEFT:
			player_turn += (double)(1<<15);
			break;
		case SDLK_RIGHT:
			player_turn -= (double)(1<<15);
			break;
		case SDLK_UP:
			player_acc -= (double)(1<<15);

		default:
			break;
	}
}


/*
 * gloabal input
 */
void handle_input( SDL_Event* event )
{
	switch (event->type) {
		case SDL_JOYAXISMOTION:
			handle_joyaxis(event->jaxis.axis, event->jaxis.value);
			break;

		case SDL_JOYBUTTONDOWN:
			handle_joydown(event->jbutton.button);
			break;

		case SDL_JOYBUTTONUP:
			handle_joyup(event->jbutton.button);

		case SDL_KEYDOWN:
			handle_keydown(event->key.keysym.sym);
			break;

		case SDL_KEYUP:
			handle_keyup(event->key.keysym.sym);
			break;
	}
}



