/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file joystick.c
 *
 * @brief Handles joystick initialization.
 */


/** @cond */
#include "SDL.h"
#include "SDL_haptic.h"
#include "SDL_joystick.h"

#include "naev.h"
/** @endcond */

#include "joystick.h"

#include "log.h"
#include "nstring.h"


static SDL_Joystick *joystick = NULL; /**< Current joystick in use. */
static int has_haptic = 0; /**< Does the player have haptic? */
SDL_Haptic *haptic = NULL; /**< Current haptic in use, externed in spfx.c. */
unsigned int haptic_query = 0; /**< Properties of the haptic device. */


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
      jname = SDL_JoystickNameForIndex(i);
      if (strstr( jname, namjoystick ))
         return i;
   }

   WARN(_("Joystick '%s' not found, using default joystick '%s'"),
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
      WARN(_("Joystick of index number %d does not exist, switching to default 0"),
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
   jname = SDL_JoystickNameForIndex(indjoystick);
   if (joystick == NULL) {
      WARN(_("Error opening joystick %d [%s]"), indjoystick, jname);
      return -1;
   }
   LOG(_("Using joystick %d - %s"), indjoystick, jname);
   DEBUG(_("   with %d axes, %d buttons, %d balls and %d hats"),
         SDL_JoystickNumAxes(joystick), SDL_JoystickNumButtons(joystick),
         SDL_JoystickNumBalls(joystick), SDL_JoystickNumHats(joystick));

   /* Initialize the haptic if possible. */
   joystick_initHaptic();

   /* For style purposes. */
   DEBUG_BLANK();

   return 0;
}


/**
 * @brief Initializes force feedback for the loaded device.
 */
static void joystick_initHaptic (void)
{
   if (has_haptic && SDL_JoystickIsHaptic(joystick)) {

      /* Close haptic if already open. */
      if (haptic != NULL) {
         SDL_HapticClose(haptic);
         haptic = NULL;
      }

      /* Try to create haptic device. */
      haptic = SDL_HapticOpenFromJoystick(joystick);
      if (haptic == NULL) {
         WARN(_("Unable to initialize force feedback: %s"), SDL_GetError());
         return;
      }

      /* Check to see what it supports. */
      haptic_query = SDL_HapticQuery(haptic);
      if (!(haptic_query & SDL_HAPTIC_SINE)) {
         SDL_HapticClose(haptic);
         haptic = NULL;
         return;
      }

      DEBUG(_("   force feedback enabled"));
   }
}


#if DEBUGGING
/**
 * @brief Describes detected joysticks in the debug log.
 */
static void joystick_debug (void)
{
   int numjoysticks, i;

   /* figure out how many joysticks there are */
   numjoysticks = SDL_NumJoysticks();
   DEBUG( n_("%d joystick detected", "%d joysticks detected", numjoysticks), numjoysticks );
   for (i=0; i < numjoysticks; i++) {
      const char *jname;
      jname = SDL_JoystickNameForIndex(i);
      DEBUG("  %d. %s", i, jname);
   }
}
#endif


/**
 * @brief Initializes the joystick subsystem.
 *
 *    @return 0 on success.
 */
int joystick_init (void)
{
   /* initialize the sdl subsystem */
   if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
      WARN(_("Unable to initialize the joystick subsystem"));
      return -1;
   }

   if (SDL_InitSubSystem(SDL_INIT_HAPTIC) == 0)
      has_haptic = 1;

#if DEBUGGING
   joystick_debug();
#endif

   /* enables joystick events */
   SDL_JoystickEventState(SDL_ENABLE);

   return 0;
}


/**
 * @brief Exits the joystick subsystem.
 */
void joystick_exit (void)
{
   if (haptic != NULL) {
      SDL_HapticClose(haptic);
      haptic = NULL;
   }

   if (joystick != NULL) {
      SDL_JoystickClose(joystick);
      joystick = NULL;
   }
}


