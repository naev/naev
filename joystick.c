

#include "joystick.h"

#include "SDL.h"

#include "all.h"
#include "log.h"


static SDL_Joystick* joystick = NULL;


int joystick_use( int indjoystick )
{
	/* start using joystick */
	LOG("Using joystick %d", indjoystick);
	joystick = SDL_JoystickOpen(indjoystick);
	if (joystick==NULL) {
		WARN("Error opening joystick %d [%s]", indjoystick, SDL_JoystickName(indjoystick));
		return -1;
	}
	DEBUG("  with %d axes, %d buttons, %d balls and %d hats",
			SDL_JoystickNumAxes(joystick), SDL_JoystickNumButtons(joystick),
			SDL_JoystickNumBalls(joystick), SDL_JoystickNumHats(joystick));

	return 0;
}


int joystick_init()
{
	int numjoysticks, i;

	/* initialize the sdl subsystem */
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
		WARN("Unable to initialize the joystick subsystem");
		return -1;
	}


	/* figure out how many joysticks there are */
	numjoysticks = SDL_NumJoysticks();
	LOG("%d joysticks detected", numjoysticks);
	for ( i=0; i < numjoysticks; i++ )
		LOG("  %d. %s", i, SDL_JoystickName(i));

	/* enables joystick events */
	SDL_JoystickEventState(SDL_ENABLE);

	return 0;
}


void joystick_exit()
{
	SDL_JoystickClose(joystick);
}


