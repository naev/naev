/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file joystick.c
 *
 * @brief Handles joystick initialization.
 */


#include "joystick.h"

#include <string.h>
#include "SDL.h"

#include "naev.h"
#include "log.h"


static SDL_Joystick* joystick = NULL; /**< Current joystick in use. */


/**
 * @fn int joystick_get( char* namjoystick )
 *
 * @brief Gets the joystick index by name.
 *
 *    @param namjoystick Looks for this string in the joystick name.
 *    @return The index if found, defaults to 0 if it isn't found.
 */
int joystick_get( char* namjoystick )
{
   int i;
   for (i=0; i < SDL_NumJoysticks(); i++)
      if (strstr(SDL_JoystickName(i),namjoystick))
         return i;

   WARN("Joystick '%s' not found, using default joystick '%s'",
         namjoystick, SDL_JoystickName(0));
   return 0;
}


/**
 * @fn int joystick_use( int indjoystick )
 *
 * @brief Makes the game use a joystick by index.
 *
 *    @param indjoystick Index of the joystick to use.
 *    @return 0 on success.
 */
int joystick_use( int indjoystick )
{
   if (indjoystick < 0 || indjoystick >= SDL_NumJoysticks()) {
      WARN("Joystick of index number %d does not existing, switching to default 0",
            indjoystick);
      indjoystick = 0;
   }

   if (joystick) /* close the joystick if it's already open */
      SDL_JoystickClose(joystick);

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


/**
 * @fn int joystick_init (void)
 *
 * @brief Initializes the joystick subsystem.
 *
 *    @return 0 on success.
 */
int joystick_init (void)
{
   int numjoysticks, i;

   /* initialize the sdl subsystem */
   if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
      WARN("Unable to initialize the joystick subsystem");
      return -1;
   }


   /* figure out how many joysticks there are */
   numjoysticks = SDL_NumJoysticks();
   LOG("%d joystick%s detected", numjoysticks, (numjoysticks==1)?"":"s" );
   for ( i=0; i < numjoysticks; i++ )
      LOG("  %d. %s", i, SDL_JoystickName(i));

   /* enables joystick events */
   SDL_JoystickEventState(SDL_ENABLE);

   return 0;
}


/**
 * @fn void joystick_exit (void)
 *
 * @brief Exits the joystick subsystem.
 */
void joystick_exit (void)
{
   SDL_JoystickClose(joystick);
}


