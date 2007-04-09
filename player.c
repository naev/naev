

#include "player.h"

#include <malloc.h>

#include "all.h"
#include "pilot.h"
#include "log.h"


#define KEY_PRESS		1.
#define KEY_RELEASE	-1.
/* keybinding structure */
typedef struct {
	char *name; /* keybinding name, taken from keybindNames */
	KeybindType type; /* type, defined in playe.h */
	int key; /* key/axis/button event number */
	double reverse; /* 1. if normal, -1. if reversed, only useful for joystick axis */
} Keybind;
static Keybind** player_input; /* contains the players keybindings */
/* name of each keybinding */
const char *keybindNames[] = { "accel", "left", "right" };


Pilot* player = NULL; /* in pilot.h as extern */
static double player_turn = 0.; /* turn velocity from input */
static double player_acc = 0.; /* accel velocity from input */



/*
 * used in pilot.c
 *
 * basically uses keyboard input instead of AI input
 */
void player_think( Pilot* player, const double dt )
{
	player->solid->dir_vel = 0.;
	if (player_turn)
		player->solid->dir_vel -= player->ship->turn * player_turn;

	player->solid->force = player->ship->thrust * player_acc;
}


/*
 * initialization/exit functions (does not assign keys)
 */
void input_init (void)
{
	Keybind *temp;
	int i;
	for (i=0; keybindNames[i]; i++); /* gets number of bindings */
	player_input = (Keybind**)malloc(i*sizeof(Keybind*));

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
 * @keynum is the index of the player_input keybind
 * @value is the value of the keypress (defined above)
 * @abs is whether or not it's an absolute value (for them joystick)
 */
static void input_key( int keynum, double value, int abs )
{
	if (strcmp(player_input[keynum]->name,"accel")==0) {
		if (abs) player_acc = value;
		else player_acc += value;
	} else if (strcmp(player_input[keynum]->name,"left")==0) {
		if (abs) player_turn = -value;
		else player_turn -= value;
	} else if (strcmp(player_input[keynum]->name,"right")==0) {
		if (abs)	player_turn = value;
		else player_turn += value;
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
static void input_joyaxis( int axis, int value );
static void input_joydown( int button );
static void input_joyup( int button );
static void input_keydown( SDLKey key );
static void input_keyup( SDLKey key );

/*
 * joystick
 */
/* joystick axis */
static void input_joyaxis( int axis, int value )
{
	int i;
	for (i=0; keybindNames[i]; i++)
		if (player_input[i]->type == KEYBIND_JAXIS && player_input[i]->key == axis) {
			input_key(i,-(player_input[i]->reverse)*(double)value/32767.,1);
			return;
		}
}
/* joystick button down */
static void input_joydown( int button )
{
	int i;
	for (i=0; keybindNames[i]; i++)
		if (player_input[i]->type == KEYBIND_JBUTTON && player_input[i]->key == button) {
			input_key(i,KEY_PRESS,0);
			return;
		}  
}
/* joystick button up */
static void input_joyup( int button )
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

		case SDL_KEYDOWN:
			input_keydown(event->key.keysym.sym);
			break;

		case SDL_KEYUP:
			input_keyup(event->key.keysym.sym);
			break;
	}
}



