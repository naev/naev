/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file joystick.c
 *
 * @brief Handles joystick initialization.
 */


#include "joystick.h"

#include "naev.h"

#include "nstring.h"

#include "SDL.h"
#include "SDL_joystick.h"
#if SDL_VERSION_ATLEAST(1,3,0)
#include "SDL_haptic.h"
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

#include "log.h"


static SDL_Joystick *joystick = NULL; /**< Current joystick in use. */
#if SDL_VERSION_ATLEAST(1,3,0)
static int has_haptic = 0; /**< Does the player have haptic? */
SDL_Haptic *haptic = NULL; /**< Current haptic in use, externed in spfx.c. */
unsigned int haptic_query = 0; /**< Properties of the haptic device. */
#endif /* SDL_VERSION_ATLEAST(1,3,0) */


/*
 * Prototypes.
 */
static void joystick_initHaptic (void);


/**
 * @brief Gets the joystick index by name.
 *
 *    @param namjoystick Looks for this string in the joystick name.
 *    @return The index if found, defaults to 0 if it isn't found.
 */
int joystick_get( const char* namjoystick )
{
   const char *jname;
   int i;
   for (i=0; i < SDL_NumJoysticks(); i++) {
#if SDL_VERSION_ATLEAST(2,0,0)
      jname = SDL_JoystickNameForIndex(i);
#else /* SDL_VERSION_ATLEAST(2,0,0) */
      jname = SDL_JoystickName(i);
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
      if (strstr( jname, namjoystick ))
         return i;
   }

   WARN("Joystick '%s' not found, using default joystick '%s'",
         namjoystick, SDL_JoystickName(0));
   return 0;
}


/**
 * @brief Makes the game use a joystick by index.
 *
 *    @param indjoystick Index of the joystick to use.
 *    @return 0 on success.
 */
int joystick_use( int indjoystick )
{
   const char *jname;

   /* Check to see if it exists. */
   if ((indjoystick < 0) || (indjoystick >= SDL_NumJoysticks())) {
      WARN("Joystick of index number %d does not existing, switching to default 0",
            indjoystick);
      indjoystick = 0;
   }

   /* Close if already open. */
   if (joystick != NULL) {
      SDL_JoystickClose(joystick);
      joystick = NULL;
   }

   /* Start using joystick. */
   joystick = SDL_JoystickOpen(indjoystick);
#if SDL_VERSION_ATLEAST(2,0,0)
   jname = SDL_JoystickNameForIndex(indjoystick);
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   jname = SDL_JoystickName(indjoystick);
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
   if (joystick == NULL) {
      WARN("Error opening joystick %d [%s]", indjoystick, jname);
      return -1;
   }
   LOG("Using joystick %d - %s", indjoystick, jname);
   DEBUG("   with %d axes, %d buttons, %d balls and %d hats",
         SDL_JoystickNumAxes(joystick), SDL_JoystickNumButtons(joystick),
         SDL_JoystickNumBalls(joystick), SDL_JoystickNumHats(joystick));

   /* Initialize the haptic if possible. */
   joystick_initHaptic();

   /* For style purposes. */
   DEBUG();

   return 0;
}


/**
 * @brief Initializes force feedback for the loaded device.
 */
static void joystick_initHaptic (void)
{
#if SDL_VERSION_ATLEAST(1,3,0)
   if (has_haptic && SDL_JoystickIsHaptic(joystick)) {

      /* Close haptic if already open. */
      if (haptic != NULL) {
         SDL_HapticClose(haptic);
         haptic = NULL;
      }

      /* Try to create haptic device. */
      haptic = SDL_HapticOpenFromJoystick(joystick);
      if (haptic == NULL) {
         WARN("Unable to initialize force feedback: %s", SDL_GetError());
         return;
      }

      /* Check to see what it supports. */
      haptic_query = SDL_HapticQuery(haptic);
      if (!(haptic_query & SDL_HAPTIC_SINE)) {
         SDL_HapticClose(haptic);
         haptic = NULL;
         return;
      }

      DEBUG("   force feedback enabled");
   }
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
}


/**
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

#if SDL_VERSION_ATLEAST(1,3,0)
   if (SDL_InitSubSystem(SDL_INIT_HAPTIC) == 0)
      has_haptic = 1;
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   /* figure out how many joysticks there are */
   numjoysticks = SDL_NumJoysticks();
   DEBUG("%d joystick%s detected", numjoysticks, (numjoysticks==1)?"":"s" );
   for (i=0; i < numjoysticks; i++) {
      const char *jname;
#if SDL_VERSION_ATLEAST(2,0,0)
      jname = SDL_JoystickNameForIndex(i);
#else /* SDL_VERSION_ATLEAST(2,0,0) */
      jname = SDL_JoystickName(i);
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
      DEBUG("  %d. %s", i, jname);
   }

   /* enables joystick events */
   SDL_JoystickEventState(SDL_ENABLE);

   return 0;
}


/**
 * @brief Exits the joystick subsystem.
 */
void joystick_exit (void)
{
#if SDL_VERSION_ATLEAST(1,3,0)
   if (haptic != NULL) {
      SDL_HapticClose(haptic);
      haptic = NULL;
   }
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   if (joystick != NULL) {
      SDL_JoystickClose(joystick);
      joystick = NULL;
   }
}


