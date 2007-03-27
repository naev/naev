

#include "joystick.h"

#include "SDL.h"

#include "all.h"
#include "log.h"


static SDL_Joystick* joystick;


int joystick_init()
{
	int indjoystick, numjoysticks, i;
	indjoystick = 1;

	/* initialize the sdl subsystem */
	SDL_InitSubSystem( SDL_INIT_JOYSTICK );


	/* figure out how many joysticks there are */
	numjoysticks = SDL_NumJoysticks();
	LOG("%d joysticks detected", numjoysticks);
	for ( i=0; i < numjoysticks; i++ )
		LOG("  %d. %s", i, SDL_JoystickName(i));

	/* start using joystick */
	LOG("Using joystick %d", indjoystick);
	joystick = SDL_JoystickOpen(indjoystick);

	return 0;
}
