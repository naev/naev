

#include "input.h"

#include "main.h"
#include "log.h"
#include "player.h"


#define KEY_PRESS    ( 1.)
#define KEY_RELEASE  (-1.)


/* keybinding structure */
typedef struct {
	char *name; /* keybinding name, taken from keybindNames */
	KeybindType type; /* type, defined in playe.h */
	unsigned int key; /* key/axis/button event number */
	double reverse; /* 1. if normal, -1. if reversed, only useful for joystick axis */
} Keybind;


static Keybind** input_keybinds; /* contains the players keybindings */

/* name of each keybinding */
const char *keybindNames[] = { "accel", "left", "right", /* movement */
	"primary", "target", "target_nearest", "face", "board", /* fighting */
	"secondary", "secondary_next", /* secondary weapons */
	"target_planet", "land", /* space navigation */
	"mapzoomin", "mapzoomout", "screenshot",  /* misc */
	"end" }; /* must terminate in "end" */


/*
 * from player.c
 */
extern double player_turn;
extern double player_acc;
extern unsigned int player_target;
extern int planet_target;


/*
 * sets the default input keys
 */
void input_setDefault (void)
{
	/* movement */
	input_setKeybind( "accel", KEYBIND_KEYBOARD, SDLK_UP, 0 );
	input_setKeybind( "left", KEYBIND_KEYBOARD, SDLK_LEFT, 0 );
	input_setKeybind( "right", KEYBIND_KEYBOARD, SDLK_RIGHT, 0 );
	/* combat */
	input_setKeybind( "primary", KEYBIND_KEYBOARD, SDLK_SPACE, 0 );
	input_setKeybind( "target", KEYBIND_KEYBOARD, SDLK_TAB, 0 );
	input_setKeybind( "target_nearest", KEYBIND_KEYBOARD, SDLK_r, 0 );
	input_setKeybind( "face", KEYBIND_KEYBOARD, SDLK_a, 0 );
	input_setKeybind( "board", KEYBIND_KEYBOARD, SDLK_b, 0 );
	/* secondary weap */
	input_setKeybind( "secondary", KEYBIND_KEYBOARD, SDLK_LSHIFT, 0 );
	input_setKeybind( "secondary_next", KEYBIND_KEYBOARD, SDLK_w, 0 );
	/* space */
	input_setKeybind( "target_planet", KEYBIND_KEYBOARD, SDLK_p, 0 );
	input_setKeybind( "land", KEYBIND_KEYBOARD, SDLK_l, 0 );
	/* misc */
	input_setKeybind( "mapzoomin", KEYBIND_KEYBOARD, SDLK_9, 0 );
	input_setKeybind( "mapzoomout", KEYBIND_KEYBOARD, SDLK_0, 0 );
	input_setKeybind( "screenshot", KEYBIND_KEYBOARD, SDLK_KP_MINUS, 0 );
}


/*
 * initialization/exit functions (does not assign keys)
 */
void input_init (void)
{  
	Keybind *temp;
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++); /* gets number of bindings */
	 	input_keybinds = malloc(i*sizeof(Keybind*));

	/* creates a null keybinding for each */
	for (i=0; strcmp(keybindNames[i],"end"); i++) {
		temp = MALLOC_ONE(Keybind);
		temp->name = (char*)keybindNames[i];
		temp->type = KEYBIND_NULL;
		temp->key = 0;
		temp->reverse = 1.;
		input_keybinds[i] = temp;
	}
}
void input_exit (void)
{
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		free(input_keybinds[i]);
	free(input_keybinds);
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
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (strcmp(keybind, input_keybinds[i]->name)==0) {
			input_keybinds[i]->type = type;
			input_keybinds[i]->key = key;
			input_keybinds[i]->reverse = (reverse) ? -1. : 1. ;
			return;
		}
	WARN("Unable to set keybinding '%s', that command doesn't exist", keybind);
}


/*
 * runs the input command
 *
 * @param keynum is the index of the  keybind
 * @param value is the value of the keypress (defined above)
 * @param abs is whether or not it's an absolute value (for them joystick)
 */
#define KEY(s)		strcmp(input_keybinds[keynum]->name,s)==0
static void input_key( int keynum, double value, int abs )
{
	/*
	 * movement
	 */
	/* accelerating */
	if (KEY("accel")) {
		if (abs) player_acc = value;
		else player_acc += value;
		player_acc = ABS(player_acc); /* make sure value is sane */         

	/* turning left */
	} else if (KEY("left")) {

		/* set flags for facing correction */
		if (value==KEY_PRESS) player_setFlag(PLAYER_TURN_LEFT);
		else if (value==KEY_RELEASE) player_rmFlag(PLAYER_TURN_LEFT);

		if (abs) player_turn = -value;
		else player_turn -= value;
		if (player_turn < -1.) player_turn = -1.; /* make sure value is sane */

	/* turning right */
	} else if (KEY("right")) {

		/* set flags for facing correction */
		if (value==KEY_PRESS) player_setFlag(PLAYER_TURN_RIGHT);
		else if (value==KEY_RELEASE) player_rmFlag(PLAYER_TURN_RIGHT);

		if (abs) player_turn = value;
		else player_turn += value;
		if (player_turn > 1.) player_turn = 1.; /* make sure value is sane */


	/*
	 * combat
	 */
	/* shooting primary weapon */
	} else if (KEY("primary")) {
		if (value==KEY_PRESS) player_setFlag(PLAYER_PRIMARY);
		else if (value==KEY_RELEASE) player_rmFlag(PLAYER_PRIMARY);
	/* targetting */
	} else if (KEY("target")) {
		if (value==KEY_PRESS) player_target = pilot_getNext(player_target);

	} else if (KEY("target_nearest")) {
		if (value==KEY_PRESS) player_target = pilot_getHostile();
	/* face the target */
	} else if (KEY("face")) {
		if (value==KEY_PRESS) player_setFlag(PLAYER_FACE);
		else if (value==KEY_RELEASE) {
			player_rmFlag(PLAYER_FACE);
			player_turn = 0; /* turning corrections */
			if (player_isFlag(PLAYER_TURN_LEFT)) player_turn -= 1;
			if (player_isFlag(PLAYER_TURN_RIGHT)) player_turn += 1;
		}
	/* board them ships */
	} else if (KEY("board")) {
		if (value==KEY_PRESS) player_board();


	/*
	 * secondary weapons
	 */
	/* shooting secondary weapon */
	} else if (KEY("secondary")) {
		if (value==KEY_PRESS) player_setFlag(PLAYER_SECONDARY);
		else if (value==KEY_RELEASE) player_rmFlag(PLAYER_SECONDARY);

	/* selecting secondary weapon */
	} else if (KEY("secondary_next")) {
		if (value==KEY_PRESS) player_secondaryNext();


	/*
	 * secondary weapons
	 */
	/* shooting secondary weapon */
	} else if (KEY("secondary")) {
		if (value==KEY_PRESS) player_setFlag(PLAYER_SECONDARY);
		else if (value==KEY_RELEASE) player_rmFlag(PLAYER_SECONDARY);

	/* selecting secondary weapon */
	} else if (KEY("secondary_next")) {
		if (value==KEY_PRESS) player_secondaryNext();


	/*                                                                     
	 * space
	 */
	/* target planet (cycles like target) */
	} else if (KEY("target_planet")) {
		if (value==KEY_PRESS) player_targetPlanet();
	/* target nearest planet or attempt to land */
	} else if (KEY("land")) {
		if (value==KEY_PRESS) player_land();


	/*
	 * misc
	 */
	/* zooming in */
	} else if (KEY("mapzoomin")) {
		if (value==KEY_PRESS) player_setRadarRel(1);
	/* zooming out */
	} else if (KEY("mapzoomout")) {
		if (value==KEY_PRESS) player_setRadarRel(-1);
	/* take a screenshot */
	} else if (KEY("screenshot")) {
		if (value==KEY_PRESS) player_screenshot();
	}
}
#undef KEY


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
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (input_keybinds[i]->type == KEYBIND_JAXIS && input_keybinds[i]->key == axis) {
			input_key(i,-(input_keybinds[i]->reverse)*(double)value/32767.,1);
			return;
		}
}
/* joystick button down */
static void input_joydown( const unsigned int button )
{
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (input_keybinds[i]->type == KEYBIND_JBUTTON && input_keybinds[i]->key == button) {
			input_key(i,KEY_PRESS,0);
			return;
		}
}
/* joystick button up */
static void input_joyup( const unsigned int button )
{  
	int i;
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (input_keybinds[i]->type == KEYBIND_JBUTTON && input_keybinds[i]->key == button) {
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
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (input_keybinds[i]->type == KEYBIND_KEYBOARD && input_keybinds[i]->key == key) {
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
	for (i=0; strcmp(keybindNames[i],"end"); i++)
		if (input_keybinds[i]->type == KEYBIND_KEYBOARD && input_keybinds[i]->key == key) {
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

