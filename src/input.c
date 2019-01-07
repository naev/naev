/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file input.c
 *
 * @brief Handles all the keybindings and input.
 */


#include "input.h"

#include "naev.h"

#include "log.h"
#include "player.h"
#include "pilot.h"
#include "pause.h"
#include "toolkit.h"
#include "menu.h"
#include "info.h"
#include "board.h"
#include "map.h"
#include "escort.h"
#include "land.h"
#include "nstd.h"
#include "gui.h"
#include "weapon.h"
#include "console.h"
#include "conf.h"
#include "camera.h"
#include "map_overlay.h"
#include "hook.h"


#define MOUSE_HIDE   ( 3.) /**< Time in seconds to wait before hiding mouse again. */


#define KEY_PRESS    ( 1.) /**< Key is pressed. */
#define KEY_RELEASE  (-1.) /**< Key is released. */


/* keybinding structure */
/**
 * @brief Naev Keybinding.
 */
typedef struct Keybind_ {
   int disabled; /**< Whether or not it's disabled. */
   const char *name; /**< keybinding name, taken from keybind_info */
   KeybindType type; /**< type, defined in player.h */
   SDLKey key; /**< key/axis/button event number */
   SDLMod mod; /**< Key modifiers (where applicable). */
} Keybind;


static Keybind* input_keybinds; /**< contains the players keybindings */
static int input_numbinds; /**< Number of keybindings. */

/* name of each keybinding */
const char *keybind_info[][3] = {
   /* Movement */
   { "accel", gettext_noop("Accelerate"), gettext_noop("Makes your ship accelerate forward.") },
   { "left", gettext_noop("Turn Left"), gettext_noop("Makes your ship turn left.") },
   { "right", gettext_noop("Turn Right"), gettext_noop("Makes your ship turn right.") },
   { "reverse", gettext_noop("Reverse"), gettext_noop("Makes your ship face the direction you're moving from. Useful for braking.") },
   /* Targeting */
   { "target_next", gettext_noop("Target Next"), gettext_noop("Cycles through ship targets.") },
   { "target_prev", gettext_noop("Target Previous"), gettext_noop("Cycles backwards through ship targets.") },
   { "target_nearest", gettext_noop("Target Nearest"), gettext_noop("Targets the nearest non-disabled ship.") },
   { "target_nextHostile", gettext_noop("Target Next Hostile"), gettext_noop("Cycles through hostile ship targets.") },
   { "target_prevHostile", gettext_noop("Target Previous Hostile"), gettext_noop("Cycles backwards through hostile ship targets.") },
   { "target_hostile", gettext_noop("Target Nearest Hostile"), gettext_noop("Targets the nearest hostile ship.") },
   { "target_clear", gettext_noop("Clear Target"), gettext_noop("Clears the currently-targeted ship, planet or jump point.") },
   /* Fighting */
   { "primary", gettext_noop("Fire Primary Weapon"), gettext_noop("Fires primary weapons.") },
   { "face", gettext_noop("Face Target"), gettext_noop("Faces the targeted ship if one is targeted, otherwise faces targeted planet or jump point.") },
   { "board", gettext_noop("Board Target"), gettext_noop("Attempts to board the targeted ship.") },
   /* Secondary Weapons */
   { "secondary", gettext_noop("Fire Secondary Weapon"), gettext_noop("Fires secondary weapons.") },
   { "weapset1", gettext_noop("Weapon Set 1"), gettext_noop("Activates weapon set 1.") },
   { "weapset2", gettext_noop("Weapon Set 2"), gettext_noop("Activates weapon set 2.") },
   { "weapset3", gettext_noop("Weapon Set 3"), gettext_noop("Activates weapon set 3.") },
   { "weapset4", gettext_noop("Weapon Set 4"), gettext_noop("Activates weapon set 4.") },
   { "weapset5", gettext_noop("Weapon Set 5"), gettext_noop("Activates weapon set 5.") },
   { "weapset6", gettext_noop("Weapon Set 6"), gettext_noop("Activates weapon set 6.") },
   { "weapset7", gettext_noop("Weapon Set 7"), gettext_noop("Activates weapon set 7.") },
   { "weapset8", gettext_noop("Weapon Set 8"), gettext_noop("Activates weapon set 8.") },
   { "weapset9", gettext_noop("Weapon Set 9"), gettext_noop("Activates weapon set 9.") },
   { "weapset0", gettext_noop("Weapon Set 0"), gettext_noop("Activates weapon set 0.") },
   /* Escorts */
   { "e_targetNext", gettext_noop("Target Next Escort"), gettext_noop("Cycles through your escorts.") },
   { "e_targetPrev", gettext_noop("Target Previous Escort"), gettext_noop("Cycles backwards through your escorts.") },
   { "e_attack", gettext_noop("Escort Attack Command"), gettext_noop("Orders escorts to attack your target.") },
   { "e_hold", gettext_noop("Escort Hold Command"), gettext_noop("Orders escorts to hold their positions.") },
   { "e_return", gettext_noop("Escort Return Command"), gettext_noop("Orders escorts to return to your ship hangars.") },
   { "e_clear", gettext_noop("Escort Clear Commands"), gettext_noop("Clears your escorts of commands.") },
   /* Space Navigation */
   { "autonav", gettext_noop("Autonavigation On"), gettext_noop("Initializes the autonavigation system.") },
   { "target_planet", gettext_noop("Target Planet"), gettext_noop("Cycles through planet targets.") },
   { "land", gettext_noop("Land"), gettext_noop("Attempts to land on the targeted planet or targets the nearest landable planet. Requests permission if necessary.") },
   { "thyperspace", gettext_noop("Target Jumpgate"), gettext_noop("Cycles through jump points.") },
   { "starmap", gettext_noop("Star Map"), gettext_noop("Opens the star map.") },
   { "jump", gettext_noop("Initiate Jump"), gettext_noop("Attempts to jump via a jump point.") },
   { "overlay", gettext_noop("Overlay Map"), gettext_noop("Opens the in-system overlay map.") },
   { "mousefly", gettext_noop("Mouse Flight"), gettext_noop("Toggles mouse flying.") },
   { "autobrake", gettext_noop("Autobrake"), gettext_noop("Begins automatic braking or active cooldown, if stopped.") },
   /* Communication */
   { "log_up", gettext_noop("Log Scroll Up"), gettext_noop("Scrolls the log upwards.") },
   { "log_down", gettext_noop("Log Scroll Down"), gettext_noop("Scrolls the log downwards.") },
   { "hail", gettext_noop("Hail Target"), gettext_noop("Attempts to initialize communication with the targeted ship.") },
   { "autohail", gettext_noop("Autohail"), gettext_noop("Automatically initialize communication with a ship that is hailing you.") },
   /* Misc. */
   { "mapzoomin", gettext_noop("Radar Zoom In"), gettext_noop("Zooms in on the radar.") },
   { "mapzoomout", gettext_noop("Radar Zoom Out"), gettext_noop("Zooms out on the radar.") },
   { "screenshot", gettext_noop("Screenshot"), gettext_noop("Takes a screenshot.") },
   { "togglefullscreen", gettext_noop("Toggle Fullscreen"), gettext_noop("Toggles between windowed and fullscreen mode.") },
   { "pause", gettext_noop("Pause"), gettext_noop("Pauses the game.") },
   { "speed", gettext_noop("Toggle 2x Speed"), gettext_noop("Toggles 2x speed modifier.") },
   { "menu", gettext_noop("Small Menu"), gettext_noop("Opens the small in-game menu.") },
   { "info", gettext_noop("Information Menu"), gettext_noop("Opens the information menu.") },
   { "console", gettext_noop("Lua Console"), gettext_noop("Opens the Lua console.") },
   { "switchtab1", gettext_noop("Switch Tab 1"), gettext_noop("Switches to tab 1.") },
   { "switchtab2", gettext_noop("Switch Tab 2"), gettext_noop("Switches to tab 2.") },
   { "switchtab3", gettext_noop("Switch Tab 3"), gettext_noop("Switches to tab 3.") },
   { "switchtab4", gettext_noop("Switch Tab 4"), gettext_noop("Switches to tab 4.") },
   { "switchtab5", gettext_noop("Switch Tab 5"), gettext_noop("Switches to tab 5.") },
   { "switchtab6", gettext_noop("Switch Tab 6"), gettext_noop("Switches to tab 6.") },
   { "switchtab7", gettext_noop("Switch Tab 7"), gettext_noop("Switches to tab 7.") },
   { "switchtab8", gettext_noop("Switch Tab 8"), gettext_noop("Switches to tab 8.") },
   { "switchtab9", gettext_noop("Switch Tab 9"), gettext_noop("Switches to tab 9.") },
   { "switchtab0", gettext_noop("Switch Tab 0"), gettext_noop("Switches to tab 0.") },
   /* Must terminate in "end". */
   { "end", "end", "end" }
}; /**< Names of possible keybindings. */


/*
 * accel hacks
 */
static unsigned int input_accelLast = 0; /**< Used to see if double tap */
static int input_accelButton        = 0; /**< Used to show whether accel is pressed. */


/*
 * Key repeat hack.
 */
static int repeat_key                  = -1; /**< Key to repeat. */
static unsigned int repeat_keyTimer    = 0;  /**< Repeat timer. */
static unsigned int repeat_keyCounter  = 0;  /**< Counter for key repeats. */


/*
 * Mouse.
 */
static double input_mouseTimer         = -1.; /**< Timer for hiding again. */
static int input_mouseCounter          = 1; /**< Counter for mouse display/hiding. */
static unsigned int input_mouseClickLast = 0; /**< Time of last click (in ms) */
static void *input_lastClicked         = NULL; /**< Pointer to the last-clicked item. */


/*
 * from player.c
 */
extern double player_left;  /**< player.c */
extern double player_right; /**< player.c */


/*
 * Key conversion table.
 */
#if !SDL_VERSION_ATLEAST(2,0,0)
#define INPUT_NUMKEYS     SDLK_LAST /**< Number of keys available. */
static char *keyconv[INPUT_NUMKEYS]; /**< Key conversion table. */
#endif /* !SDL_VERSION_ATLEAST(2,0,0) */


/*
 * Prototypes.
 */
#if !SDL_VERSION_ATLEAST(2,0,0)
static void input_keyConvGen (void);
static void input_keyConvDestroy (void);
#endif /* !SDL_VERSION_ATLEAST(2,0,0) */
static void input_key( int keynum, double value, double kabs, int repeat );
static void input_clickZoom( double modifier );
static void input_clickevent( SDL_Event* event );
static void input_mouseMove( SDL_Event* event );


/**
 * @brief Sets the default input keys.
 *
 *    @param wasd Whether to use the WASD layout.
 */
void input_setDefault ( int wasd )
{
   /* Movement */
   if (wasd) {
      input_setKeybind( "accel", KEYBIND_KEYBOARD, SDLK_w, NMOD_ALL );
      input_setKeybind( "left", KEYBIND_KEYBOARD, SDLK_a, NMOD_ALL );
      input_setKeybind( "right", KEYBIND_KEYBOARD, SDLK_d, NMOD_ALL );
      input_setKeybind( "reverse", KEYBIND_KEYBOARD, SDLK_s, NMOD_NONE );
   }
   else {
      input_setKeybind( "accel", KEYBIND_KEYBOARD, SDLK_UP, NMOD_ALL );
      input_setKeybind( "left", KEYBIND_KEYBOARD, SDLK_LEFT, NMOD_ALL );
      input_setKeybind( "right", KEYBIND_KEYBOARD, SDLK_RIGHT, NMOD_ALL );
      input_setKeybind( "reverse", KEYBIND_KEYBOARD, SDLK_DOWN, NMOD_ALL );
   }

   /* Targeting */
   if (wasd) {
      input_setKeybind( "target_next", KEYBIND_KEYBOARD, SDLK_e, NMOD_CTRL );
      input_setKeybind( "target_prev", KEYBIND_KEYBOARD, SDLK_q, NMOD_CTRL );
      input_setKeybind( "target_nearest", KEYBIND_KEYBOARD, SDLK_t, NMOD_ALL );
      input_setKeybind( "target_nextHostile", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
      input_setKeybind( "target_prevHostile", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
      input_setKeybind( "target_hostile", KEYBIND_KEYBOARD, SDLK_r, NMOD_ALL );
      input_setKeybind( "target_clear", KEYBIND_KEYBOARD, SDLK_c, NMOD_ALL );
   }
   else {
      input_setKeybind( "target_next", KEYBIND_KEYBOARD, SDLK_t, NMOD_NONE );
      input_setKeybind( "target_prev", KEYBIND_KEYBOARD, SDLK_t, NMOD_CTRL );
      input_setKeybind( "target_nearest", KEYBIND_KEYBOARD, SDLK_n, NMOD_NONE );
      input_setKeybind( "target_nextHostile", KEYBIND_KEYBOARD, SDLK_r, NMOD_CTRL );
      input_setKeybind( "target_prevHostile", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
      input_setKeybind( "target_hostile", KEYBIND_KEYBOARD, SDLK_r, NMOD_NONE );
      input_setKeybind( "target_clear", KEYBIND_KEYBOARD, SDLK_BACKSPACE, NMOD_ALL );
   }

   /* Combat */
   input_setKeybind( "primary", KEYBIND_KEYBOARD, SDLK_SPACE, NMOD_ALL );

   if (wasd)
      input_setKeybind( "face", KEYBIND_KEYBOARD, SDLK_q, NMOD_NONE );
   else
      input_setKeybind( "face", KEYBIND_KEYBOARD, SDLK_a, NMOD_ALL );

   input_setKeybind( "board", KEYBIND_KEYBOARD, SDLK_b, NMOD_NONE );
   /* Secondary Weapons */
   input_setKeybind( "secondary", KEYBIND_KEYBOARD, SDLK_LSHIFT, NMOD_ALL );
   input_setKeybind( "weapset1", KEYBIND_KEYBOARD, SDLK_1, NMOD_ALL );
   input_setKeybind( "weapset2", KEYBIND_KEYBOARD, SDLK_2, NMOD_ALL );
   input_setKeybind( "weapset3", KEYBIND_KEYBOARD, SDLK_3, NMOD_ALL );
   input_setKeybind( "weapset4", KEYBIND_KEYBOARD, SDLK_4, NMOD_ALL );
   input_setKeybind( "weapset5", KEYBIND_KEYBOARD, SDLK_5, NMOD_ALL );
   input_setKeybind( "weapset6", KEYBIND_KEYBOARD, SDLK_6, NMOD_ALL );
   input_setKeybind( "weapset7", KEYBIND_KEYBOARD, SDLK_7, NMOD_ALL );
   input_setKeybind( "weapset8", KEYBIND_KEYBOARD, SDLK_8, NMOD_ALL );
   input_setKeybind( "weapset9", KEYBIND_KEYBOARD, SDLK_9, NMOD_ALL );
   input_setKeybind( "weapset0", KEYBIND_KEYBOARD, SDLK_0, NMOD_ALL );
   /* Escorts */
   input_setKeybind( "e_targetNext", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
   input_setKeybind( "e_targetPrev", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
   input_setKeybind( "e_attack", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
   input_setKeybind( "e_hold", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
   input_setKeybind( "e_return", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
   input_setKeybind( "e_clear", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
   /* Space Navigation */
   input_setKeybind( "autonav", KEYBIND_KEYBOARD, SDLK_j, NMOD_CTRL );
   input_setKeybind( "target_planet", KEYBIND_KEYBOARD, SDLK_p, NMOD_NONE );
   input_setKeybind( "land", KEYBIND_KEYBOARD, SDLK_l, NMOD_NONE );
   input_setKeybind( "thyperspace", KEYBIND_KEYBOARD, SDLK_h, NMOD_NONE );
   input_setKeybind( "starmap", KEYBIND_KEYBOARD, SDLK_m, NMOD_NONE );
   input_setKeybind( "jump", KEYBIND_KEYBOARD, SDLK_j, NMOD_NONE );
   input_setKeybind( "overlay", KEYBIND_KEYBOARD, SDLK_TAB, NMOD_ALL );
   input_setKeybind( "mousefly", KEYBIND_KEYBOARD, SDLK_x, NMOD_CTRL );
   input_setKeybind( "autobrake", KEYBIND_KEYBOARD, SDLK_s, NMOD_CTRL );
   /* Communication */
   input_setKeybind( "log_up", KEYBIND_KEYBOARD, SDLK_PAGEUP, NMOD_ALL );
   input_setKeybind( "log_down", KEYBIND_KEYBOARD, SDLK_PAGEDOWN, NMOD_ALL );
   input_setKeybind( "hail", KEYBIND_KEYBOARD, SDLK_y, NMOD_NONE );
   input_setKeybind( "autohail", KEYBIND_KEYBOARD, SDLK_y, NMOD_CTRL );
   /* Misc. */
   input_setKeybind( "mapzoomin", KEYBIND_KEYBOARD, SDLK_KP_PLUS, NMOD_ALL );
   input_setKeybind( "mapzoomout", KEYBIND_KEYBOARD, SDLK_KP_MINUS, NMOD_ALL );
   input_setKeybind( "screenshot", KEYBIND_KEYBOARD, SDLK_KP_MULTIPLY, NMOD_ALL );
   input_setKeybind( "togglefullscreen", KEYBIND_KEYBOARD, SDLK_F11, NMOD_ALL );
   input_setKeybind( "pause", KEYBIND_KEYBOARD, SDLK_PAUSE, NMOD_ALL );

   input_setKeybind( "speed", KEYBIND_KEYBOARD, SDLK_BACKQUOTE, NMOD_ALL );
   input_setKeybind( "menu", KEYBIND_KEYBOARD, SDLK_ESCAPE, NMOD_ALL );
   input_setKeybind( "info", KEYBIND_KEYBOARD, SDLK_i, NMOD_NONE );
   input_setKeybind( "console", KEYBIND_KEYBOARD, SDLK_F2, NMOD_ALL );
   input_setKeybind( "switchtab1", KEYBIND_KEYBOARD, SDLK_1, NMOD_ALT );
   input_setKeybind( "switchtab2", KEYBIND_KEYBOARD, SDLK_2, NMOD_ALT );
   input_setKeybind( "switchtab3", KEYBIND_KEYBOARD, SDLK_3, NMOD_ALT );
   input_setKeybind( "switchtab4", KEYBIND_KEYBOARD, SDLK_4, NMOD_ALT );
   input_setKeybind( "switchtab5", KEYBIND_KEYBOARD, SDLK_5, NMOD_ALT );
   input_setKeybind( "switchtab6", KEYBIND_KEYBOARD, SDLK_6, NMOD_ALT );
   input_setKeybind( "switchtab7", KEYBIND_KEYBOARD, SDLK_7, NMOD_ALT );
   input_setKeybind( "switchtab8", KEYBIND_KEYBOARD, SDLK_8, NMOD_ALT );
   input_setKeybind( "switchtab9", KEYBIND_KEYBOARD, SDLK_9, NMOD_ALT );
   input_setKeybind( "switchtab0", KEYBIND_KEYBOARD, SDLK_0, NMOD_ALT );
}


/**
 * @brief Initializes the input subsystem (does not set keys).
 */
void input_init (void)
{
   Keybind *temp;
   int i;

#if !SDL_VERSION_ATLEAST(2,0,0)
   /* We need unicode for the input widget. */
   SDL_EnableUNICODE(1);

   /* Key repeat fscks up stuff like double tap. */
   SDL_EnableKeyRepeat( 0, 0 );
#endif /* !SDL_VERSION_ATLEAST(2,0,0) */

   /* Window. */
   SDL_EventState( SDL_SYSWMEVENT,      SDL_DISABLE );

   /* Keyboard. */
   SDL_EventState( SDL_KEYDOWN,         SDL_ENABLE );
   SDL_EventState( SDL_KEYUP,           SDL_ENABLE );

   /* Mice. */
   SDL_EventState( SDL_MOUSEMOTION,     SDL_ENABLE );
   SDL_EventState( SDL_MOUSEBUTTONDOWN, SDL_ENABLE );
   SDL_EventState( SDL_MOUSEBUTTONUP,   SDL_ENABLE );

   /* Joystick, enabled in joystick.c if needed. */
   SDL_EventState( SDL_JOYAXISMOTION,   SDL_DISABLE );
   SDL_EventState( SDL_JOYHATMOTION,    SDL_DISABLE );
   SDL_EventState( SDL_JOYBUTTONDOWN,   SDL_DISABLE );
   SDL_EventState( SDL_JOYBUTTONUP,     SDL_DISABLE );

   /* Quit. */
   SDL_EventState( SDL_QUIT,            SDL_ENABLE );

#if SDL_VERSION_ATLEAST(1,3,0)
   /* Window. */
   SDL_EventState( SDL_WINDOWEVENT,     SDL_ENABLE );

   /* Keyboard. */
   SDL_EventState( SDL_TEXTINPUT,       SDL_ENABLE );

   /* Mouse. */
   SDL_EventState( SDL_MOUSEWHEEL,      SDL_ENABLE );
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   /* Get the number of keybindings. */
   for (i=0; strcmp(keybind_info[i][0],"end"); i++);
   input_numbinds = i;
   input_keybinds = malloc( input_numbinds * sizeof(Keybind) );

   /* Create sane null keybinding for each. */
   for (i=0; i<input_numbinds; i++) {
      temp = &input_keybinds[i];
      memset( temp, 0, sizeof(Keybind) );
      temp->name        = keybind_info[i][0];
      temp->type        = KEYBIND_NULL;
      temp->key         = SDLK_UNKNOWN;
      temp->mod         = NMOD_NONE;
   }

#if !SDL_VERSION_ATLEAST(2,0,0)
   /* Generate Key translation table. */
   input_keyConvGen();
#endif /* !SDL_VERSION_ATLEAST(2,0,0) */
}


/**
 * @brief Exits the input subsystem.
 */
void input_exit (void)
{
   free(input_keybinds);

#if !SDL_VERSION_ATLEAST(2,0,0)
   input_keyConvDestroy();
#endif /* !SDL_VERSION_ATLEAST(2,0,0) */
}


/**
 * @brief Enables all the keybinds.
 */
void input_enableAll (void)
{
   int i;
   for (i=0; strcmp(keybind_info[i][0],"end"); i++)
      input_keybinds[i].disabled = 0;
}


/**
 * @brief Disables all the keybinds.
 */
void input_disableAll (void)
{
   int i;
   for (i=0; strcmp(keybind_info[i][0],"end"); i++)
      input_keybinds[i].disabled = 1;
}


/**
 * @brief Enables or disables a keybind.
 */
void input_toggleEnable( const char *key, int enable )
{
   int i;
   for (i=0; i<input_numbinds; i++) {
      if (strcmp(key, input_keybinds[i].name)==0) {
         input_keybinds[i].disabled = !enable;
         break;
      }
   }
}


/**
 * @brief Shows the mouse.
 */
void input_mouseShow (void)
{
   SDL_ShowCursor( SDL_ENABLE );
   input_mouseCounter++;
}


/**
 * @brief Hides the mouse.
 */
void input_mouseHide (void)
{
   input_mouseCounter--;
   if (input_mouseCounter <= 0) {
      input_mouseTimer = MOUSE_HIDE;
      input_mouseCounter = 0;
   }
}


#if !SDL_VERSION_ATLEAST(2,0,0)
/**
 * @brief Creates the key conversion table.
 */
static void input_keyConvGen (void)
{
   SDLKey k;

   for (k=0; k < INPUT_NUMKEYS; k++)
      keyconv[k] = strdup( SDL_GetKeyName(k) );
}


/**
 * @brief Destroys the key conversion table.
 */
static void input_keyConvDestroy (void)
{
   int i;

   for (i=0; i < INPUT_NUMKEYS; i++)
      free( keyconv[i] );
}
#endif /* !SDL_VERSION_ATLEAST(2,0,0) */


/**
 * @brief Gets the key id from its name.
 *
 *    @param name Name of the key to get id from.
 *    @return ID of the key.
 */
SDLKey input_keyConv( const char *name )
{
#if SDL_VERSION_ATLEAST(2,0,0)
   SDLKey k;
   k = SDL_GetKeyFromName( name );

   if (k == SDLK_UNKNOWN)
      WARN(_("Keyname '%s' doesn't match any key."), name);

   return k;
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   SDLKey k, m;
   size_t l;
   char buf;

   l = strlen(name);
   buf = tolower(name[0]);

   /* Compare for single character. */
   if (l == 1) {
      m = MIN(256, INPUT_NUMKEYS);
      for (k=0; k < m; k++) { /* Only valid for char range. */
         /* Must not be NULL. */
         if (keyconv[k] == NULL)
            continue;

         /* Check if is also a single char. */
         if ((buf == tolower(keyconv[k][0])) && (keyconv[k][1] == '\0'))
            return k;
      }
   }
   /* Compare for strings. */
   else {
      for (k=0; k < INPUT_NUMKEYS; k++) {
         /* Must not be NULL. */
         if (keyconv[k] == NULL)
            continue;

         /* Compare strings. */
         if (strcmp(name , keyconv[k])==0)
            return k;
      }
   }

   WARN(_("Keyname '%s' doesn't match any key."), name);
   return SDLK_UNKNOWN;
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
}


/**
 * @brief Binds key of type type to action keybind.
 *
 *    @param keybind The name of the keybind defined above.
 *    @param type The type of the keybind.
 *    @param key The key to bind to.
 *    @param mod Modifiers to check for.
 */
void input_setKeybind( const char *keybind, KeybindType type, SDLKey key, SDLMod mod )
{
   int i;
   for (i=0; i<input_numbinds; i++) {
      if (strcmp(keybind, input_keybinds[i].name)==0) {
         input_keybinds[i].type = type;
         input_keybinds[i].key = key;
         /* Non-keyboards get mod NMOD_ALL to always match. */
         input_keybinds[i].mod = (type==KEYBIND_KEYBOARD) ? mod : NMOD_ALL;
         return;
      }
   }
   WARN(_("Unable to set keybinding '%s', that command doesn't exist"), keybind);
}


/**
 * @brief Gets the value of a keybind.
 *
 *    @brief keybind Name of the keybinding to get.
 *    @param[out] type Stores the type of the keybinding.
 *    @param[out] mod Stores the modifiers used with the keybinding.
 *    @return The key associated with the keybinding.
 */
SDLKey input_getKeybind( const char *keybind, KeybindType *type, SDLMod *mod )
{
   int i;
   for (i=0; i<input_numbinds; i++) {
      if (strcmp(keybind, input_keybinds[i].name)==0) {
         if (type != NULL)
            (*type) = input_keybinds[i].type;
         if (mod != NULL)
            (*mod) = input_keybinds[i].mod;
         return input_keybinds[i].key;
      }
   }
   WARN(_("Unable to get keybinding '%s', that command doesn't exist"), keybind);
   return (SDLKey)-1;
}


/**
 * @brief Gets the human readable version of mod.
 *
 *    @brief mod Mod to get human readable version from.
 *    @return Human readable version of mod.
 */
const char* input_modToText( SDLMod mod )
{
   switch ((int)mod) {
      case NMOD_NONE:   return _("None");
      case NMOD_CTRL:   return _("Ctrl");
      case NMOD_SHIFT:  return _("Shift");
      case NMOD_ALT:    return _("Alt");
      case NMOD_META:   return _("Meta");
      case NMOD_ALL:    return _("Any");
      default:          return _("unknown");
   }
}


/**
 * @brief Checks to see if a key is already bound.
 *
 *    @param type Type of key.
 *    @param key Key.
 *    @param mod Key modifiers.
 *    @return Name of the key that is already bound to it.
 */
const char *input_keyAlreadyBound( KeybindType type, SDLKey key, SDLMod mod )
{
   int i;
   Keybind *k;
   for (i=0; i<input_numbinds; i++) {
      k = &input_keybinds[i];

      /* Type must match. */
      if (k->type != type)
         continue;

      /* Must match key. */
      if (key !=  k->key)
         continue;

      /* Handle per case. */
      switch (type) {
         case KEYBIND_KEYBOARD:
            if ((k->mod == NMOD_ALL) || (mod == NMOD_ALL) ||
                  (k->mod == mod))
               return keybind_info[i][0];
            break;

         case KEYBIND_JAXISPOS:
         case KEYBIND_JAXISNEG:
         case KEYBIND_JBUTTON:
         case KEYBIND_JHAT_UP:
         case KEYBIND_JHAT_DOWN:
         case KEYBIND_JHAT_LEFT:
         case KEYBIND_JHAT_RIGHT:
            return keybind_info[i][0];

         default:
            break;
      }
   }

   /* Not found. */
   return NULL;
}


/**
 * @brief Gets the description of the keybinding.
 *
 *    @param keybind Keybinding to get the description of.
 *    @return Description of the keybinding.
 */
const char* input_getKeybindDescription( const char *keybind )
{
   int i;
   for (i=0; strcmp(keybind_info[i][0],"end"); i++)
      if (strcmp(keybind, input_keybinds[i].name)==0)
         return _(keybind_info[i][2]);
   WARN(_("Unable to get keybinding description '%s', that command doesn't exist"), keybind);
   return NULL;
}


/**
 * @brief Translates SDL modifier to Naev modifier.
 *
 *    @param mod SDL modifier to translate.
 *    @return Naev modifier.
 */
SDLMod input_translateMod( SDLMod mod )
{
   SDLMod mod_filtered = 0;
   if (mod & (KMOD_LSHIFT | KMOD_RSHIFT))
      mod_filtered |= NMOD_SHIFT;
   if (mod & (KMOD_LCTRL | KMOD_RCTRL))
      mod_filtered |= NMOD_CTRL;
   if (mod & (KMOD_LALT | KMOD_RALT))
      mod_filtered |= NMOD_ALT;
   if (mod & (KMOD_LMETA | KMOD_RMETA))
      mod_filtered |= NMOD_META;
   return mod_filtered;
}


/**
 * @brief Handles key repeating.
 */
void input_update( double dt )
{
   unsigned int t;

   if (input_mouseTimer > 0.) {
      input_mouseTimer -= dt;

      /* Hide if necessary. */
      if ((input_mouseTimer < 0.) && (input_mouseCounter <= 0))
         SDL_ShowCursor( SDL_DISABLE );
   }

   /* Key repeat if applicable. */
   if (conf.repeat_delay != 0) {

      /* Key must be repeating. */
      if (repeat_key == -1)
         return;

      /* Get time. */
      t = SDL_GetTicks();

      /* Should be repeating. */
      if (repeat_keyTimer + conf.repeat_delay + repeat_keyCounter*conf.repeat_freq > t)
         return;

      /* Key repeat. */
      repeat_keyCounter++;
      input_key( repeat_key, KEY_PRESS, 0., 1 );
   }
}


#define KEY(s)    (strcmp(input_keybinds[keynum].name,s)==0) /**< Shortcut for ease. */
#define INGAME()  (!toolkit_isOpen()) /**< Makes sure player is in game. */
#define NOHYP()   \
((player.p != NULL) && !pilot_isFlag(player.p,PILOT_HYP_PREP) &&\
!pilot_isFlag(player.p,PILOT_HYP_BEGIN) &&\
!pilot_isFlag(player.p,PILOT_HYPERSPACE)) /**< Make sure the player isn't jumping. */
#define NODEAD()  ((player.p != NULL) && !pilot_isFlag(player.p,PILOT_DEAD)) /**< Player isn't dead. */
#define NOLAND()  ((player.p != NULL) && (!landed && !pilot_isFlag(player.p,PILOT_LANDING))) /**< Player isn't landed. */
/**
 * @brief Runs the input command.
 *
 *    @param keynum The index of the  keybind.
 *    @param value The value of the keypress (defined above).
 *    @param kabs The absolute value.
 */
static void input_key( int keynum, double value, double kabs, int repeat )
{
   unsigned int t;
   HookParam hparam[3];

   /* Repetition stuff. */
   if (conf.repeat_delay != 0) {
      if ((value == KEY_PRESS) && !repeat) {
         repeat_key        = keynum;
         repeat_keyTimer   = SDL_GetTicks();
         repeat_keyCounter = 0;
      }
      else if (value == KEY_RELEASE) {
         repeat_key        = -1;
         repeat_keyTimer   = 0;
         repeat_keyCounter = 0;
      }
   }

   /*
    * movement
    */
   /* accelerating */
   if (KEY("accel") && !repeat) {
      if (kabs >= 0.) {
         player_restoreControl( PINPUT_MOVEMENT, NULL );
         player_accel(kabs);
         input_accelButton = 1;
      }
      else { /* prevent it from getting stuck */
         if (value==KEY_PRESS) {
            player_restoreControl( PINPUT_MOVEMENT, NULL );
            player_setFlag(PLAYER_ACCEL);
            player_accel(1.);
            input_accelButton = 1;
         }

         else if (value==KEY_RELEASE) {
            player_accelOver();
            player_rmFlag(PLAYER_ACCEL);
            input_accelButton = 0;
         }

         /* double tap accel = afterburn! */
         t = SDL_GetTicks();
         if ((conf.afterburn_sens != 0) &&
               (value==KEY_PRESS) && INGAME() && NOHYP() && NODEAD() &&
               (t-input_accelLast <= conf.afterburn_sens))
            pilot_afterburn( player.p );
         else if (value==KEY_RELEASE)
            pilot_afterburnOver( player.p );

         if (value==KEY_PRESS)
            input_accelLast = t;
      }

   /* turning left */
   } else if (KEY("left") && !repeat) {
      if (kabs >= 0.) {
         player_restoreControl( PINPUT_MOVEMENT, NULL );
         player_setFlag(PLAYER_TURN_LEFT);
         player_left = kabs;
      }
      else {
         /* set flags for facing correction */
         if (value==KEY_PRESS) {
            player_restoreControl( PINPUT_MOVEMENT, NULL );
            player_setFlag(PLAYER_TURN_LEFT);
            player_left = 1.;
         }
         else if (value==KEY_RELEASE) {
            player_rmFlag(PLAYER_TURN_LEFT);
            player_left = 0.;
         }
      }

   /* turning right */
   } else if (KEY("right") && !repeat) {
      if (kabs >= 0.) {
         player_restoreControl( PINPUT_MOVEMENT, NULL );
         player_setFlag(PLAYER_TURN_RIGHT);
         player_right = kabs;
      }
      else {
         /* set flags for facing correction */
         if (value==KEY_PRESS) {
            player_restoreControl( PINPUT_MOVEMENT, NULL );
            player_setFlag(PLAYER_TURN_RIGHT);
            player_right = 1.;
         }
         else if (value==KEY_RELEASE) {
            player_rmFlag(PLAYER_TURN_RIGHT);
            player_right = 0.;
         }
      }

   /* turn around to face vel */
   } else if (KEY("reverse") && !repeat) {
      if (value==KEY_PRESS) {
         player_restoreControl( PINPUT_MOVEMENT, NULL );
         player_setFlag(PLAYER_REVERSE);
      }
      else if ((value==KEY_RELEASE) && player_isFlag(PLAYER_REVERSE)) {
         player_rmFlag(PLAYER_REVERSE);

         if (!player_isFlag(PLAYER_ACCEL))
            player_accelOver();
      }


   /*
    * combat
    */
   /* shooting primary weapon */
   } else if (KEY("primary") && NODEAD() && !repeat) {
      if (value==KEY_PRESS) {
         player_setFlag(PLAYER_PRIMARY);
      }
      else if (value==KEY_RELEASE)
         player_rmFlag(PLAYER_PRIMARY);
   /* targeting */
   } else if (INGAME() && NODEAD() && KEY("target_next")) {
      if (value==KEY_PRESS) player_targetNext(0);
   } else if (INGAME() && NODEAD() && KEY("target_prev")) {
      if (value==KEY_PRESS) player_targetPrev(0);
   } else if (INGAME() && NODEAD() && KEY("target_nearest")) {
      if (value==KEY_PRESS) player_targetNearest();
   } else if (INGAME() && NODEAD() && KEY("target_nextHostile")) {
      if (value==KEY_PRESS) player_targetNext(1);
   } else if (INGAME() && NODEAD() && KEY("target_prevHostile")) {
      if (value==KEY_PRESS) player_targetPrev(1);
   } else if (INGAME() && NODEAD() && KEY("target_hostile")) {
      if (value==KEY_PRESS) player_targetHostile();
   } else if (INGAME() && NODEAD() && KEY("target_clear")) {
      if (value==KEY_PRESS) player_targetClear();
   /* face the target */
   } else if (INGAME() && NODEAD() && KEY("face") && !repeat) {
      if (value==KEY_PRESS) {
         player_restoreControl( PINPUT_MOVEMENT, NULL );
         player_setFlag(PLAYER_FACE);
      }
      else if ((value==KEY_RELEASE) && player_isFlag(PLAYER_FACE))
         player_rmFlag(PLAYER_FACE);

   /* board them ships */
   } else if (KEY("board") && INGAME() && NOHYP() && NODEAD() && !repeat) {
      if (value==KEY_PRESS) {
         player_restoreControl( 0, NULL );
         player_board();
      }


   /*
    * Escorts.
    */
   } else if (INGAME() && NODEAD() && KEY("e_targetNext") && !repeat) {
      if (value==KEY_PRESS) player_targetEscort(0);
   } else if (INGAME() && NODEAD() && KEY("e_targetPrev") && !repeat) {
      if (value==KEY_PRESS) player_targetEscort(1);
   } else if (INGAME() && NODEAD() && KEY("e_attack") && !repeat) {
      if (value==KEY_PRESS) escorts_attack(player.p);
   } else if (INGAME() && NODEAD() && KEY("e_hold") && !repeat) {
      if (value==KEY_PRESS) escorts_hold(player.p);
   } else if (INGAME() && NODEAD() && KEY("e_return") && !repeat) {
      if (value==KEY_PRESS) escorts_return(player.p);
   } else if (INGAME() && NODEAD() && KEY("e_clear") && !repeat) {
      if (value==KEY_PRESS) escorts_clear(player.p);


   /*
    * secondary weapons
    */
   /* shooting secondary weapon */
   } else if (KEY("secondary") && NOHYP() && NODEAD() && !repeat) {
      if (value==KEY_PRESS) {
         player_setFlag(PLAYER_SECONDARY);
      }
      else if (value==KEY_RELEASE)
         player_rmFlag(PLAYER_SECONDARY);

   /* Weapon sets. */
   } else if (KEY("weapset1")) {
      player_weapSetPress( 0, value, repeat );
   } else if (KEY("weapset2")) {
      player_weapSetPress( 1, value, repeat );
   } else if (KEY("weapset3")) {
      player_weapSetPress( 2, value, repeat );
   } else if (KEY("weapset4")) {
      player_weapSetPress( 3, value, repeat );
   } else if (KEY("weapset5")) {
      player_weapSetPress( 4, value, repeat );
   } else if (KEY("weapset6")) {
      player_weapSetPress( 5, value, repeat );
   } else if (KEY("weapset7")) {
      player_weapSetPress( 6, value, repeat );
   } else if (KEY("weapset8")) {
      player_weapSetPress( 7, value, repeat );
   } else if (KEY("weapset9")) {
      player_weapSetPress( 8, value, repeat );
   } else if (KEY("weapset0")) {
      player_weapSetPress( 9, value, repeat );

   /*
    * space
    */
   } else if (KEY("autonav") && NOHYP() && NODEAD()) {
      if (value==KEY_PRESS) {
         if (map_isOpen()) {
            unsigned int wid = window_get( MAP_WDWNAME );
            player_autonavStartWindow( wid, NULL );
         }
         else if INGAME() {
            player_autonavStart();
         }
      }
   /* target planet (cycles like target) */
   } else if (KEY("target_planet") && INGAME() && NOHYP() && NOLAND() && NODEAD()) {
      if (value==KEY_PRESS) player_targetPlanet();
   /* target nearest planet or attempt to land */
   } else if (KEY("land") && INGAME() && NOHYP() && NOLAND() && NODEAD()) {
      if (value==KEY_PRESS)
         player_land();
   } else if (KEY("thyperspace") && NOHYP() && NOLAND() && NODEAD()) {
      if (value==KEY_PRESS)
         player_targetHyperspace();
   } else if (KEY("starmap") && NOHYP() && NODEAD() && !repeat) {
      if (value==KEY_PRESS) map_open();
   } else if (KEY("jump") && INGAME() && !repeat) {
      if (value==KEY_PRESS) {
         player_restoreControl( 0, NULL );
         player_jump();
      }
   } else if (KEY("overlay") && NODEAD() && INGAME() && !repeat) {
      ovr_key( value );
   } else if (KEY("mousefly") && NODEAD() && !repeat) {
      if (value==KEY_PRESS)
         player_toggleMouseFly();
   } else if (KEY("autobrake") && NOHYP() && NOLAND() && NODEAD() && !repeat) {
      if (value==KEY_PRESS) {
         player_restoreControl( PINPUT_BRAKING, NULL );
         player_brake();
      }

   /*
    * Communication.
    */
   } else if (KEY("log_up") && INGAME() && NODEAD()) {
      if (value==KEY_PRESS) {
         gui_messageScrollUp(5);
      }
   } else if (KEY("log_down") && INGAME() && NODEAD()) {
      if (value==KEY_PRESS) {
         gui_messageScrollDown(5);
      }
   } else if (KEY("hail") && INGAME() && NOHYP() && NODEAD() && !repeat) {
      if (value==KEY_PRESS) {
         player_hail();
      }
   } else if (KEY("autohail") && INGAME() && NOHYP() && NODEAD() && !repeat) {
      if (value==KEY_PRESS) {
         player_autohail();
      }

   /*
    * misc
    */
   /* zooming in */
   } else if (KEY("mapzoomin") && INGAME() && NODEAD()) {
      if (value==KEY_PRESS) gui_setRadarRel(-1);
   /* zooming out */
   } else if (KEY("mapzoomout") && INGAME() && NODEAD()) {
      if (value==KEY_PRESS) gui_setRadarRel(1);
   /* take a screenshot */
   } else if (KEY("screenshot")) {
      if (value==KEY_PRESS) player_screenshot();
#if SDL_VERSION_ATLEAST(2,0,0)
   /* toggle fullscreen */
   } else if (KEY("togglefullscreen") && !repeat) {
      if (value==KEY_PRESS) naev_toggleFullscreen();
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
   /* pause the games */
   } else if (KEY("pause") && !repeat) {
      if (value==KEY_PRESS) {
         if (!toolkit_isOpen()) {
            if (paused)
               unpause_game();
            else
               pause_player();
         }
      }
   /* toggle speed mode */
   } else if (KEY("speed") && !repeat) {
      if ((value==KEY_PRESS) && (!player_isFlag( PLAYER_CINEMATICS_2X ))) {
         if (player_isFlag(PLAYER_DOUBLESPEED)) {
            if (!player_isFlag(PLAYER_AUTONAV)) {
               pause_setSpeed( player.p->ship->dt_default );
               sound_setSpeed( 1. );
            }
            player_rmFlag(PLAYER_DOUBLESPEED);
         } else {
            if (!player_isFlag(PLAYER_AUTONAV)) {
               pause_setSpeed( 2. * player.p->ship->dt_default );
               sound_setSpeed( 2. );
            }
            player_setFlag(PLAYER_DOUBLESPEED);
         }
      }
   /* opens a small menu */
   } else if (KEY("menu") && NODEAD() && !repeat) {
      if (value==KEY_PRESS) menu_small();

   /* shows pilot information */
   } else if (KEY("info") && NOHYP() && NODEAD() && !repeat) {
      if (value==KEY_PRESS) menu_info( INFO_MAIN );

   /* Opens the Lua console. */
   } else if (KEY("console") && NODEAD() && !repeat) {
      if (value==KEY_PRESS) cli_open();
   }

   /* Key press not used. */
   else {
      return;
   }

   /* Run the hook. */
   hparam[0].type    = HOOK_PARAM_STRING;
   hparam[0].u.str   = input_keybinds[keynum].name;
   hparam[1].type    = HOOK_PARAM_BOOL;
   hparam[1].u.b     = (value > 0.);
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "input", hparam );
}
#undef KEY


/*
 * events
 */
/* prototypes */
static void input_joyaxis( const SDLKey axis, const int value );
static void input_joyevent( const int event, const SDLKey button );
static void input_keyevent( const int event, const SDLKey key, const SDLMod mod, const int repeat );

/*
 * joystick
 */
/**
 * @brief Filters a joystick axis event.
 *    @param axis Axis generated by the event.
 *    @param value Value of the axis.
 */
static void input_joyaxis( const SDLKey axis, const int value )
{
   int i, k;
   for (i=0; i<input_numbinds; i++) {
      if (input_keybinds[i].key == axis) {
         /* Positive axis keybinding. */
         if ((input_keybinds[i].type == KEYBIND_JAXISPOS)
               && (value >= 0)) {
            k = (value > 0) ? KEY_PRESS : KEY_RELEASE;
            if ((k == KEY_PRESS) && input_keybinds[i].disabled)
               continue;
            input_key( i, k, fabs(((double)value)/32767.), 0 );
         }

         /* Negative axis keybinding. */
         if ((input_keybinds[i].type == KEYBIND_JAXISNEG)
               && (value <= 0)) {
            k = (value < 0) ? KEY_PRESS : KEY_RELEASE;
            if ((k == KEY_PRESS) && input_keybinds[i].disabled)
               continue;
            input_key( i, k, fabs(((double)value)/32767.), 0 );
         }
      }
   }
}
/**
 * @brief Filters a joystick button event.
 *    @param event Event type (down/up).
 *    @param button Button generating the event.
 */
static void input_joyevent( const int event, const SDLKey button )
{
   int i;
   for (i=0; i<input_numbinds; i++) {
      if ((event == KEY_PRESS) && input_keybinds[i].disabled)
         continue;
      if ((input_keybinds[i].type == KEYBIND_JBUTTON) &&
            (input_keybinds[i].key == button))
         input_key(i, event, -1., 0);
   }
}

/**
 * @brief Filters a joystick hat event.
 *    @param value Direction on hat.
 *    @param hat Hat generating the event.
 */
static void input_joyhatevent( const Uint8 value, const Uint8 hat )
{
   int i, event;
   for (i=0; i<input_numbinds; i++) {
      if (input_keybinds[i].key != hat)
         continue;

      if ((input_keybinds[i].type == KEYBIND_JHAT_UP)) {
         event = (value & SDL_HAT_UP) ? KEY_PRESS : KEY_RELEASE;
         if (!((event == KEY_PRESS) && input_keybinds[i].disabled))
            input_key(i, event, -1., 0);
      } else if ((input_keybinds[i].type == KEYBIND_JHAT_DOWN)) {
         event = (value & SDL_HAT_DOWN) ? KEY_PRESS : KEY_RELEASE;
         if (!((event == KEY_PRESS) && input_keybinds[i].disabled))
            input_key(i, event, -1., 0);
      } else if ((input_keybinds[i].type == KEYBIND_JHAT_LEFT)) {
         event = (value & SDL_HAT_LEFT) ? KEY_PRESS : KEY_RELEASE;
         if (!((event == KEY_PRESS) && input_keybinds[i].disabled))
            input_key(i, event, -1., 0);
      } else if ((input_keybinds[i].type == KEYBIND_JHAT_RIGHT)) {
         event = (value & SDL_HAT_RIGHT) ? KEY_PRESS : KEY_RELEASE;
         if (!((event == KEY_PRESS) && input_keybinds[i].disabled))
            input_key(i, event, -1., 0);
      }
   }
}


/*
 * keyboard
 */
/**
 * @brief Filters a keyboard event.
 *    @param event Event type (down/up).
 *    @param key Key generating the event.
 *    @param mod Modifiers active when event was generated.
 */
static void input_keyevent( const int event, SDLKey key, const SDLMod mod, const int repeat )
{
   int i;
   SDLMod mod_filtered;

   /* Filter to "Naev" modifiers. */
   mod_filtered = input_translateMod(mod);

   for (i=0; i<input_numbinds; i++) {
      if ((event == KEY_PRESS) && input_keybinds[i].disabled)
         continue;
      if ((input_keybinds[i].type == KEYBIND_KEYBOARD) &&
            (input_keybinds[i].key == key)) {
         if ((input_keybinds[i].mod == mod_filtered) ||
               (input_keybinds[i].mod == NMOD_ALL) ||
               (event == KEY_RELEASE)) /**< Release always gets through. */
            input_key(i, event, -1., repeat);
            /* No break so all keys get pressed if needed. */
      }
   }
}


/**
 * @brief Handles zoom.
 */
static void input_clickZoom( double modifier )
{
   if (player.p != NULL)
      cam_setZoomTarget( cam_getZoomTarget() * modifier );
}

/**
 * @brief Provides mouse X and Y coordinates for mouse flying.
 */
static void input_mouseMove( SDL_Event* event )
{
   int mx, my;

   gl_windowToScreenPos( &mx, &my, event->button.x, event->button.y );
   player.mousex = mx;
   player.mousey = my;
}

/**
 * @brief Handles a click event.
 */
static void input_clickevent( SDL_Event* event )
{
   unsigned int pid;
   int mx, my, mxr, myr, pntid, jpid, astid, fieid;
   int rx, ry, rh, rw, res;
   int autonav;
   double x, y, zoom, px, py;
   double ang, angp, mouseang;
   HookParam hparam[2];

   /* Generate hook. */
   hparam[0].type    = HOOK_PARAM_NUMBER;
   hparam[0].u.num   = event->button.button;
   hparam[1].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "mouse", hparam );

#if !SDL_VERSION_ATLEAST(2,0,0)
   /* Handle zoom. */
   if (event->button.button == SDL_BUTTON_WHEELUP) {
      input_clickZoom( 1.1 );
      return;
   }
   else if (event->button.button == SDL_BUTTON_WHEELDOWN) {
      input_clickZoom( 0.9 );
      return;
   }
#endif /* !SDL_VERSION_ATLEAST(2,0,0) */

   /* Player must not be NULL. */
   if ((player.p == NULL) || player_isFlag(PLAYER_DESTROYED))
      return;

   /* Player must not be dead. */
   if (pilot_isFlag(player.p, PILOT_DEAD))
      return;

   /* Middle mouse enables mouse flying. */
   if (event->button.button == SDL_BUTTON_MIDDLE) {
      player_toggleMouseFly();
      return;
   }

   /* Mouse targeting only uses left and right buttons. */
   if (event->button.button != SDL_BUTTON_LEFT &&
            event->button.button != SDL_BUTTON_RIGHT)
      return;

   autonav = (event->button.button == SDL_BUTTON_RIGHT) ? 1 : 0;

   px = player.p->solid->pos.x;
   py = player.p->solid->pos.y;
   gl_windowToScreenPos( &mx, &my, event->button.x, event->button.y );
   if ((mx <= 15 || my <= 15 ) || (my >= gl_screen.h - 15 || mx >= gl_screen.w - 15)) {
      /* Border targeting is handled as a special case, as it uses angles,
       * not coordinates.
       */
      x = (mx - (gl_screen.w / 2.)) + px;
      y = (my - (gl_screen.h / 2.)) + py;
      mouseang = atan2(py - y, px -  x);
      angp = pilot_getNearestAng( player.p, &pid, mouseang, 1 );
      ang  = system_getClosestAng( cur_system, &pntid, &jpid, &astid, &fieid, x, y, mouseang );

      if  ((ABS(angle_diff(mouseang, angp)) > M_PI / 64) ||
            ABS(angle_diff(mouseang, ang)) < ABS(angle_diff(mouseang, angp)))
         pid = PLAYER_ID; /* Pilot angle is too great, or planet/jump is closer. */
      if  (ABS(angle_diff(mouseang, ang)) > M_PI / 64 )
         jpid = pntid = astid = fieid = -1; /* Asset angle difference is too great. */

      if (!autonav && pid != PLAYER_ID) {
         if (input_clickedPilot(pid))
            return;
      }
      else if (pntid >= 0) { /* Planet is closest. */
         if (input_clickedPlanet(pntid, autonav))
            return;
      }
      else if (jpid >= 0) { /* Jump point is closest. */
         if (input_clickedJump(jpid, autonav))
            return;
      }
      else if (astid >= 0) { /* Asteroid is closest. */
         if (input_clickedAsteroid(fieid, astid))
            return;
      }

      /* Fall-through and handle as a normal click. */
   }

   /* Radar targeting requires raw coordinates. */
   mxr = event->button.x;
   myr  = gl_screen.rh - event->button.y;
   gui_radarGetPos( &rx, &ry );
   gui_radarGetDim( &rw, &rh );
   if ((mxr > rx && mxr <= rx + rw ) && (myr > ry && myr <= ry + rh )) { /* Radar */
      zoom = 1.;
      gui_radarGetRes( &res );
      x = (mxr - (rx + rw / 2.)) * res + px;
      y = (myr - (ry + rh / 2.)) * res + py;

      if (input_clickPos( event, x, y, zoom, 10. * res, 15. * res ))
         return;
   }

   /* Visual (on-screen) */
   gl_screenToGameCoords( &x, &y, (double)mx, (double)my );
   zoom = res = 1. / cam_getZoom();

   input_clickPos( event, x, y, zoom, 10. * res, 15. * res );
   return;
}


/**
 * @brief Handles a click at a position in the current system
 *
 *    @brief event The click event itself, used for button information.
 *    @brief x X coordinate within the system.
 *    @brief y Y coordinate within the system.
 *    @brief zoom Camera zoom (mostly for on-screen targeting).
 *    @brief minpr Minimum radius to assign to pilots.
 *    @brief minr Minimum radius to assign to planets and jumps.
 *    @return Whether the click was used to trigger an action.
 */
int input_clickPos( SDL_Event *event, double x, double y, double zoom, double minpr, double minr )
{
   unsigned int pid;
   Pilot *p;
   double r, rp;
   double d, dp;
   Planet *pnt;
   JumpPoint *jp;
   Asteroid *ast;
   AsteroidAnchor *field;
   AsteroidType *at;
   int pntid, jpid, astid, fieid;

   dp = pilot_getNearestPos( player.p, &pid, x, y, 1 );
   p  = pilot_get(pid);

   d  = system_getClosest( cur_system, &pntid, &jpid, &astid, &fieid, x, y );
   rp = MAX( 1.5 * PILOT_SIZE_APROX * p->ship->gfx_space->sw / 2 * zoom,  minpr);

   if (pntid >=0) { /* Planet is closer. */
      pnt = cur_system->planets[ pntid ];
      r  = MAX( 1.5 * pnt->radius * zoom, minr );
   }
   else if (jpid >= 0) {
      jp = &cur_system->jumps[ jpid ];
      r  = MAX( 1.5 * jp->radius * zoom, minr );
   }
   else if (astid >= 0) {
      field = &cur_system->asteroids[fieid];
      ast   = &field->asteroids[astid];

      /* Recover the right gfx */
      at = space_getType( ast->type );
      if (ast->gfxID > at->ngfx+1)
         WARN(_("Gfx index out of range"));
      r  = MAX( MAX( at->gfxs[ast->gfxID]->w * zoom, minr ),
                at->gfxs[ast->gfxID]->h * zoom );
   }
   else
      r  = 0.;

   /* Reject pilot if it's too far or a valid asset is closer. */
   if (dp > pow2(rp) || ((d < pow2(r)) && (dp >  d)))
      pid = PLAYER_ID;

   if (d > pow2(r)) /* Planet or jump point is too far. */
      jpid = pntid = astid = fieid =  -1;

   /* Target a pilot, planet or jump, and/or perform an appropriate action. */
   if (event->button.button == SDL_BUTTON_LEFT) {
      if (pid != PLAYER_ID) {
         return input_clickedPilot(pid);
      }
      else if (pntid >= 0) { /* Planet is closest. */
         return input_clickedPlanet(pntid, 0);
      }
      else if (jpid >= 0) { /* Jump point is closest. */
         return input_clickedJump(jpid, 0);
      }
      else if (astid >= 0) { /* Asteroid is closest. */
         return input_clickedAsteroid(fieid, astid);
      }
   }
   /* Right click only controls autonav. */
   else if (event->button.button == SDL_BUTTON_RIGHT) {
      if ((pntid >= 0) && input_clickedPlanet(pntid, 1))
         return 1;
      else if ((jpid >= 0) && input_clickedJump(jpid, 1))
         return 1;

      /* Go to position, if the position is >= 1500 px away. */
      if ((pow2(x - player.p->solid->pos.x) + pow2(y - player.p->solid->pos.y))
            >= pow2(1500))

      player_autonavPos( x, y );
      return 1;
   }

   return 0;
}


/**
 * @brief Performs an appropriate action when a jump point is clicked.
 *
 *    @param jp Index of the jump point.
 *    @param autonav Whether to autonav to the target.
 *    @return Whether the click was used.
 */
int input_clickedJump( int jump, int autonav )
{
   JumpPoint *jp;
   jp = &cur_system->jumps[ jump ];

   if (!jp_isUsable(jp))
      return 0;

   /* Update map path. */
   if (player.p->nav_hyperspace != jump)
      map_select( jp->target, 0 );

   if (autonav) {
      player_targetHyperspaceSet( jump );
      player_autonavStart();
      return 1;
   }

   if (jump == player.p->nav_hyperspace && input_isDoubleClick( (void*)jp )) {
      if (space_canHyperspace(player.p))
         player_jump();
   }
   else
      player_targetHyperspaceSet( jump );

   input_clicked( (void*)jp );
   return 1;
}

/**
 * @brief Performs an appropriate action when a planet is clicked.
 *
 *    @param planet Index of the planet.
 *    @param autonav Whether to autonav to the target.
 *    @return Whether the click was used.
 */
int input_clickedPlanet( int planet, int autonav )
{
   Planet *pnt;
   pnt = cur_system->planets[ planet ];

   if (!planet_isKnown(pnt))
      return 0;

   if (autonav) {
      player_targetPlanetSet( planet );
      player_autonavPnt( pnt->name );
      return 1;
   }

   if (planet == player.p->nav_planet && input_isDoubleClick((void*)pnt)) {
      player_hyperspacePreempt(0);
      if (planet_hasService(pnt, PLANET_SERVICE_LAND)) {
         if ((pnt->faction >= 0) && (areEnemies( player.p->faction, pnt->faction ) && !pnt->bribed))
            player_hailPlanet();
         else
            player_land();
      }
   }
   else
      player_targetPlanetSet( planet );

   input_clicked( (void*)pnt );
   return 1;
}

/**
 * @brief Performs an appropriate action when an asteroid is clicked.
 *
 *    @param field Index of the parent field of the asteoid.
 *    @param asteroid Index of the oasteoid in the field.
 *    @return Whether the click was used.
 */
int input_clickedAsteroid( int field, int asteroid )
{
   Asteroid *ast;
   AsteroidAnchor *anchor;

   anchor = &cur_system->asteroids[ field ];
   ast = &anchor->asteroids[ asteroid ];

   player_targetAsteroidSet( field, asteroid );

   input_clicked( (void*)ast );
   return 1;
}

/**
 * @brief Performs an appropriate action when a pilot is clicked.
 *
 *    @param pilot Index of the pilot.
 *    @return Whether the click was used.
 */
int input_clickedPilot( unsigned int pilot )
{
   Pilot *p;

   if (pilot == PLAYER_ID)
      return 0;

   p = pilot_get(pilot);
   if (pilot == player.p->target && input_isDoubleClick( (void*)p )) {
      if (pilot_isDisabled(p) || pilot_isFlag(p, PILOT_BOARDABLE))
         player_board();
      else
         player_hail();
   }
   else
      player_targetSet( pilot );

   input_clicked( (void*)p );
   return 1;
}


/**
 * @brief Sets the last-clicked item, for double-click detection.
 *    @param clicked Pointer to the clicked item.
 */
void input_clicked( void *clicked )
{
   if (conf.mouse_doubleclick <= 0.)
      return;

   input_lastClicked = clicked;
   input_mouseClickLast = SDL_GetTicks();
}


/**
 * @brief Checks whether a clicked item is the same as the last-clicked.
 *    @param clicked Pointer to the clicked item.
 */
int input_isDoubleClick( void *clicked )
{
   unsigned int threshold;

   if (conf.mouse_doubleclick <= 0.)
      return 1;

   /* Most recent time that constitutes a valid double-click. */
   threshold = input_mouseClickLast + (int)(conf.mouse_doubleclick * 1000);

   if ((SDL_GetTicks() <= threshold) && (clicked == input_lastClicked))
      return 1;

   return 0;
}


/**
 * @brief Handles global input.
 *
 * Basically separates the event types
 *
 *    @param event Incoming SDL_Event.
 */
void input_handle( SDL_Event* event )
{
   int ismouse = 0;

   /* Special case mouse stuff. */
   if ((event->type == SDL_MOUSEMOTION)  ||
         (event->type == SDL_MOUSEBUTTONDOWN) ||
         (event->type == SDL_MOUSEBUTTONUP)) {
      input_mouseTimer = MOUSE_HIDE;
      SDL_ShowCursor( SDL_ENABLE );
      ismouse = 1;
   }

   if (toolkit_isOpen()) { /* toolkit handled completely separately */
      if (toolkit_input(event))
         return; /* we don't process it if toolkit grabs it */
      if (ismouse)
         return; /* Toolkit absorbs everything mousy. */
   }

   if (ovr_isOpen())
      if (ovr_input(event))
         return; /* Don't process if the map overlay wants it. */

   /* GUI gets event. */
   if (gui_handleEvent(event))
      return;

   switch (event->type) {

      /*
       * game itself
       */
      case SDL_JOYAXISMOTION:
         input_joyaxis(event->jaxis.axis, event->jaxis.value);
         break;

      case SDL_JOYBUTTONDOWN:
         input_joyevent(KEY_PRESS, event->jbutton.button);
         break;

      case SDL_JOYBUTTONUP:
         input_joyevent(KEY_RELEASE, event->jbutton.button);
         break;

      case SDL_JOYHATMOTION:
         input_joyhatevent(event->jhat.value, event->jhat.hat);
         break;

      case SDL_KEYDOWN:
#if SDL_VERSION_ATLEAST(2,0,0)
         if (event->key.repeat != 0)
            return;
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
         input_keyevent(KEY_PRESS, event->key.keysym.sym, event->key.keysym.mod, 0);
         break;

      case SDL_KEYUP:
#if SDL_VERSION_ATLEAST(2,0,0)
         if (event->key.repeat !=0)
            return;
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
         input_keyevent(KEY_RELEASE, event->key.keysym.sym, event->key.keysym.mod, 0);
         break;


      /* Mouse stuff. */
      case SDL_MOUSEBUTTONDOWN:
         input_clickevent( event );
         break;

#if SDL_VERSION_ATLEAST(2,0,0)
      case SDL_MOUSEWHEEL:
         if (event->wheel.y > 0)
            input_clickZoom( 1.1 );
         else
            input_clickZoom( 0.9 );
         break;
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

      case SDL_MOUSEMOTION:
         input_mouseMove( event );
         break;

      default:
         break;
   }
}

