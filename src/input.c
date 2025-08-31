/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file input.c
 *
 * @brief Handles all the keybindings and input.
 */
/** @cond */
#include "naev.h"
#include <ctype.h>
/** @endcond */

#include <ctype.h>

#include "input.h"

#include "board.h"
#include "camera.h"
#include "conf.h"
#include "console.h"
#include "escort.h"
#include "gui.h"
#include "hook.h"
#include "info.h"
#include "land.h"
#include "log.h"
#include "map.h"
#include "map_overlay.h"
#include "menu.h"
#include "nstring.h"
#include "pause.h"
#include "pilot.h"
#include "player.h"
#include "player_autonav.h"
#include "toolkit.h"

/* keybinding structure */
/**
 * @brief Naev Keybinding.
 */
typedef struct Keybind_ {
   int         disabled; /**< Whether or not it's disabled. */
   KeybindType type;     /**< type, defined in player.h */
   SDL_Keycode key;      /**< key/axis/button event number */
   SDL_Keymod  mod;      /**< Key modifiers (where applicable). */
} Keybind;

/* Description of each key semantic type */
static const char *keybind_info[KST_END][3] = {
   /* Movement */
   [KST_ACCEL]   = { N_( "Accelerate" ),
                     N_( "Makes your ship accelerate forward." ), "accel" },
   [KST_LEFT]    = { N_( "Turn Left" ), N_( "Makes your ship turn left." ),
                     "left" },
   [KST_RIGHT]   = { N_( "Turn Right" ), N_( "Makes your ship turn right." ),
                     "right" },
   [KST_REVERSE] = { N_( "Reverse" ),
                     N_( "Makes your ship face the direction you're moving "
                         "from. Useful for braking." ),
                     "reverse" },
   [KST_FACE]    = { N_( "Face Target" ),
                     N_( "Faces the targeted ship if one is targeted, otherwise "
                            "faces targeted spob, or jump point." ),
                     "face" },

   /* Gameplay modifiers */
   [KST_STEALTH]    = { N_( "Stealth" ), N_( "Tries to enter stealth mode." ),
                        "stealth" },
   [KST_GAME_SPEED] = { N_( "Toggle Speed" ), N_( "Toggles speed modifier." ),
                        "speed" },
   [KST_PAUSE]      = { N_( "Pause" ), N_( "Pauses the game." ), "pause" },

   /* Movement modifiers */
   [KST_AUTONAV] = { N_( "Autonavigation On" ),
                     N_( "Initializes the autonavigation system." ),
                     "autonav" },
   [KST_APPROACH] =
      { N_( "Approach" ),
        N_( "Attempts to approach the targeted ship or space object, "
            "or targets the nearest landable space object. "
            "Requests landing permission if necessary. "
            "Prioritizes ships over space objects." ),
        "approach" },
   [KST_MOUSE_FLYING] = { N_( "Mouse Flight" ), N_( "Toggles mouse flying." ),
                          "mousefly" },
   [KST_JUMP]         = { N_( "Initiate Jump" ),
                          N_( "Attempts to jump via a jump point." ), "jump" },

   /* Targeting */
   [KST_TARGET_NEXT]  = { N_( "Target Next" ),
                          N_( "Cycles through ship targets." ), "target_next" },
   [KST_TARGET_PREV]  = { N_( "Target Previous" ),
                          N_( "Cycles backwards through ship targets." ),
                          "target_prev" },
   [KST_TARGET_CLOSE] = { N_( "Target Nearest" ),
                          N_( "Targets the nearest non-disabled ship." ),
                          "target_nearest" },
   [KST_TARGET_SPOB]  = { N_( "Target Spob" ),
                          N_( "Cycles through space object targets." ),
                          "target_spob" },
   [KST_TARGET_JUMP]  = { N_( "Target Jumpgate" ),
                          N_( "Cycles through jump points." ), "thyperspace" },

   /* Hostile targets */
   [KST_HTARGET_NEXT]  = { N_( "Target Next Hostile" ),
                           N_( "Cycles through hostile ship targets." ),
                           "target_nextHostile" },
   [KST_HTARGET_PREV]  = { N_( "Target Previous Hostile" ),
                           N_(
                             "Cycles backwards through hostile ship targets." ),
                           "target_prevHostile" },
   [KST_HTARGET_CLOSE] = { N_( "Target Nearest Hostile" ),
                           N_( "Targets the nearest hostile ship." ),
                           "target_hostile" },

   [KST_TARGET_CLEAR] =
      { N_( "Clear Target" ),
        N_( "Clears the currently-targeted ship, spob or jump point." ),
        "target_clear" },

   /* Fighting */
   [KST_FIRE_PRIMARY]   = { N_( "Fire Primary Weapon" ),
                            N_( "Fires primary weapons." ), "primary" },
   [KST_FIRE_SECONDARY] = { N_( "Fire Secondary Weapon" ),
                            N_( "Fires secondary weapons." ), "secondary" },
   [KST_COOLDOWN] = { N_( "Active Cooldown" ), N_( "Begins active cooldown." ),
                      "cooldown" },

   /* Switching tab s*/
   [KST_WEAPSET1] = { N_( "Weapon Set 1" ), N_( "Activates weapon set 1." ),
                      "weapset1" },
   [KST_WEAPSET2] = { N_( "Weapon Set 2" ), N_( "Activates weapon set 2." ),
                      "weapset2" },
   [KST_WEAPSET3] = { N_( "Weapon Set 3" ), N_( "Activates weapon set 3." ),
                      "weapset3" },
   [KST_WEAPSET4] = { N_( "Weapon Set 4" ), N_( "Activates weapon set 4." ),
                      "weapset4" },
   [KST_WEAPSET5] = { N_( "Weapon Set 5" ), N_( "Activates weapon set 5." ),
                      "weapset5" },
   [KST_WEAPSET6] = { N_( "Weapon Set 6" ), N_( "Activates weapon set 6." ),
                      "weapset6" },
   [KST_WEAPSET7] = { N_( "Weapon Set 7" ), N_( "Activates weapon set 7." ),
                      "weapset7" },
   [KST_WEAPSET8] = { N_( "Weapon Set 8" ), N_( "Activates weapon set 8." ),
                      "weapset8" },
   [KST_WEAPSET9] = { N_( "Weapon Set 9" ), N_( "Activates weapon set 9." ),
                      "weapset9" },
   [KST_WEAPSET0] = { N_( "Weapon Set 0" ), N_( "Activates weapon set 0." ),
                      "weapset0" },

   /* Map manipulation */
   [KST_OVERLAY_MAP] = { N_( "Overlay Map" ),
                         N_( "Opens the in-system overlay map." ), "overlay" },
   [KST_STAR_MAP]    = { N_( "Star Map" ), N_( "Opens the star map." ),
                         "starmap" },

   /* Menus */
   [KST_MENU_SMALL] = { N_( "Small Menu" ),
                        N_( "Opens the small in-game menu." ), "menu" },
   [KST_MENU_INFO]  = { N_( "Information Menu" ),
                        N_( "Opens the information menu." ), "info" },
   [KST_CONSOLE]    = { N_( "Lua Console" ), N_( "Opens the Lua console." ),
                        "console" },

   /* Escorts */
   [KST_ESCORT_NEXT]   = { N_( "Target Next Escort" ),
                           N_( "Cycles through your escorts." ), "e_targetNext" },
   [KST_ESCORT_PREV]   = { N_( "Target Previous Escort" ),
                           N_( "Cycles backwards through your escorts." ),
                           "e_targetPrev" },
   [KST_ESCORT_ATTACK] = { N_( "Escort Attack Command" ),
                           N_( "Orders escorts to attack your target." ),
                           "e_attack" },
   [KST_ESCORT_HALT]   = { N_( "Escort Hold Command" ),
                           N_( "Orders escorts to hold their formation." ),
                           "e_hold" },
   [KST_ESCORT_RETURN] =
      { N_( "Escort Return Command" ),
        N_( "Orders escorts to return to your ship hangars." ), "e_return" },
   [KST_ESCORT_CLEAR] = { N_( "Escort Clear Commands" ),
                          N_( "Clears your escorts of commands." ), "e_clear" },

   /* Communication */
   [KST_HAIL] =
      { N_( "Hail Target" ),
        N_( "Attempts to initialize communication with the targeted ship." ),
        "hail" },
   [KST_AUTOHAIL] = { N_( "Autohail" ),
                      N_( "Automatically initialize communication with a ship "
                          "that is hailing you." ),
                      "autohail" },
   [KST_SCAN]     = { N_( "Scan Target" ), N_( "Attempts to scan the target." ),
                      "scan" },
   [KST_LOG_UP]   = { N_( "Log Scroll Up" ), N_( "Scrolls the log upwards." ),
                      "log_up" },
   [KST_LOG_DOWN] = { N_( "Log Scroll Down" ),
                      N_( "Scrolls the log downwards." ), "log_down" },

   /* Display options */
   [KST_ZOOM_IN]  = { N_( "Radar Zoom In" ), N_( "Zooms in on the radar." ),
                      "mapzoomin" },
   [KST_ZOOM_OUT] = { N_( "Radar Zoom Out" ), N_( "Zooms out on the radar." ),
                      "mapzoomout" },

   [KST_FULLSCREEN] = { N_( "Toggle Fullscreen" ),
                        N_( "Toggles between windowed and fullscreen mode." ),
                        "togglefullscreen" },

   [KST_SCREENSHOT] = { N_( "Screenshot" ), N_( "Takes a screenshot." ),
                        "screenshot" },
   [KST_PASTE]      = { N_( "Paste" ),
                        N_( "Paste from the operating system's clipboard." ),
                        "paste" },
};

static Keybind input_keybinds[KST_END]; /**< contains the players keybindings */
static Keybind *input_paste;

/*
 * accel hacks
 */
static KeySemanticType doubletap_key = KST_END; /**< Last key double tapped. */
static unsigned int    doubletap_t = 0; /**< Used to see if double tap accel. */

/*
 * Key repeat hack.
 */
static int          repeat_key        = -1; /**< Key to repeat. */
static unsigned int repeat_keyTimer   = 0;  /**< Repeat timer. */
static unsigned int repeat_keyCounter = 0;  /**< Counter for key repeats. */

/*
 * Mouse.
 */
static double input_mouseTimer   = 1.; /**< Timer for hiding again. */
static int    input_mouseCounter = 1;  /**< Counter for mouse display/hiding. */
static unsigned int input_mouseClickLast = 0; /**< Time of last click (in ms) */
static const void  *input_lastClicked =
   NULL; /**< Pointer to the last-clicked item. */

/*
 * from player.c
 */
extern double player_left;  /**< player.c */
extern double player_right; /**< player.c */

/*
 * Prototypes.
 */
static void input_key( KeySemanticType keynum, double value, double kabs,
                       int repeat );
static void input_clickZoom( double modifier );
static void input_clickevent( SDL_Event *event );
static void input_mouseMove( SDL_Event *event );
static void input_joyaxis( const SDL_Keycode axis, const int value );
static void input_joyevent( const int event, const SDL_Keycode button );
static void input_keyevent( const int event, const SDL_Keycode key,
                            const SDL_Keymod mod, const int repeat );
static int  input_doubleClickTest( unsigned int *time, const void **last,
                                   const void *clicked );

/**
 * @brief Sets the default input keys.
 *
 *    @param wasd Whether to use the WASD layout.
 */
void input_setDefault( int wasd )
{
   /* Movement */
   if ( wasd ) {
      input_setKeybind( KST_ACCEL, KEYBIND_KEYBOARD, SDLK_W, NMOD_ANY );
      input_setKeybind( KST_LEFT, KEYBIND_KEYBOARD, SDLK_A, NMOD_ANY );
      input_setKeybind( KST_RIGHT, KEYBIND_KEYBOARD, SDLK_D, NMOD_ANY );
      input_setKeybind( KST_REVERSE, KEYBIND_KEYBOARD, SDLK_S, NMOD_ANY );
   } else {
      input_setKeybind( KST_ACCEL, KEYBIND_KEYBOARD, SDLK_UP, NMOD_ANY );
      input_setKeybind( KST_LEFT, KEYBIND_KEYBOARD, SDLK_LEFT, NMOD_ANY );
      input_setKeybind( KST_RIGHT, KEYBIND_KEYBOARD, SDLK_RIGHT, NMOD_ANY );
      input_setKeybind( KST_REVERSE, KEYBIND_KEYBOARD, SDLK_DOWN, NMOD_ANY );
   }
   input_setKeybind( KST_STEALTH, KEYBIND_KEYBOARD, SDLK_F, NMOD_NONE );

   /* Targeting */
   if ( wasd ) {
      input_setKeybind( KST_TARGET_NEXT, KEYBIND_KEYBOARD, SDLK_E, NMOD_CTRL );
      input_setKeybind( KST_TARGET_PREV, KEYBIND_KEYBOARD, SDLK_Q, NMOD_CTRL );
      input_setKeybind( KST_TARGET_CLOSE, KEYBIND_KEYBOARD, SDLK_T, NMOD_ANY );
      input_setKeybind( KST_HTARGET_NEXT, KEYBIND_NULL, SDLK_UNKNOWN,
                        NMOD_NONE );
      input_setKeybind( KST_HTARGET_PREV, KEYBIND_NULL, SDLK_UNKNOWN,
                        NMOD_NONE );
      input_setKeybind( KST_HTARGET_CLOSE, KEYBIND_KEYBOARD, SDLK_R, NMOD_ANY );
      input_setKeybind( KST_TARGET_CLEAR, KEYBIND_KEYBOARD, SDLK_C, NMOD_ANY );
   } else {
      input_setKeybind( KST_TARGET_NEXT, KEYBIND_KEYBOARD, SDLK_T, NMOD_NONE );
      input_setKeybind( KST_TARGET_PREV, KEYBIND_KEYBOARD, SDLK_T, NMOD_CTRL );
      input_setKeybind( KST_TARGET_CLOSE, KEYBIND_KEYBOARD, SDLK_N, NMOD_NONE );
      input_setKeybind( KST_HTARGET_NEXT, KEYBIND_KEYBOARD, SDLK_R, NMOD_CTRL );
      input_setKeybind( KST_HTARGET_PREV, KEYBIND_NULL, SDLK_UNKNOWN,
                        NMOD_NONE );
      input_setKeybind( KST_HTARGET_CLOSE, KEYBIND_KEYBOARD, SDLK_R,
                        NMOD_NONE );
      input_setKeybind( KST_TARGET_CLEAR, KEYBIND_KEYBOARD, SDLK_BACKSPACE,
                        NMOD_ANY );
   }

   /* Combat */
   input_setKeybind( KST_FIRE_PRIMARY, KEYBIND_KEYBOARD, SDLK_SPACE, NMOD_ANY );

   if ( wasd )
      input_setKeybind( KST_FACE, KEYBIND_KEYBOARD, SDLK_Q, NMOD_NONE );
   else
      input_setKeybind( KST_FACE, KEYBIND_KEYBOARD, SDLK_A, NMOD_ANY );

   /* Secondary Weapons */
   input_setKeybind( KST_FIRE_SECONDARY, KEYBIND_KEYBOARD, SDLK_LSHIFT,
                     NMOD_ANY );
   input_setKeybind( KST_WEAPSET1, KEYBIND_KEYBOARD, SDLK_1, NMOD_ANY );
   input_setKeybind( KST_WEAPSET2, KEYBIND_KEYBOARD, SDLK_2, NMOD_ANY );
   input_setKeybind( KST_WEAPSET3, KEYBIND_KEYBOARD, SDLK_3, NMOD_ANY );
   input_setKeybind( KST_WEAPSET4, KEYBIND_KEYBOARD, SDLK_4, NMOD_ANY );
   input_setKeybind( KST_WEAPSET5, KEYBIND_KEYBOARD, SDLK_5, NMOD_ANY );
   input_setKeybind( KST_WEAPSET6, KEYBIND_KEYBOARD, SDLK_6, NMOD_ANY );
   input_setKeybind( KST_WEAPSET7, KEYBIND_KEYBOARD, SDLK_7, NMOD_ANY );
   input_setKeybind( KST_WEAPSET8, KEYBIND_KEYBOARD, SDLK_8, NMOD_ANY );
   input_setKeybind( KST_WEAPSET9, KEYBIND_KEYBOARD, SDLK_9, NMOD_ANY );
   input_setKeybind( KST_WEAPSET0, KEYBIND_KEYBOARD, SDLK_0, NMOD_ANY );
   /* Escorts */
   input_setKeybind( KST_ESCORT_NEXT, KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
   input_setKeybind( KST_ESCORT_PREV, KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
   input_setKeybind( KST_ESCORT_ATTACK, KEYBIND_KEYBOARD, SDLK_END, NMOD_ANY );
   input_setKeybind( KST_ESCORT_HALT, KEYBIND_KEYBOARD, SDLK_INSERT, NMOD_ANY );
   input_setKeybind( KST_ESCORT_RETURN, KEYBIND_KEYBOARD, SDLK_DELETE,
                     NMOD_ANY );
   input_setKeybind( KST_ESCORT_CLEAR, KEYBIND_KEYBOARD, SDLK_HOME, NMOD_ANY );
   /* Space Navigation */
   input_setKeybind( KST_AUTONAV, KEYBIND_KEYBOARD, SDLK_J, NMOD_CTRL );
   input_setKeybind( KST_TARGET_SPOB, KEYBIND_KEYBOARD, SDLK_P, NMOD_NONE );
   input_setKeybind( KST_APPROACH, KEYBIND_KEYBOARD, SDLK_L, NMOD_NONE );
   input_setKeybind( KST_TARGET_JUMP, KEYBIND_KEYBOARD, SDLK_H, NMOD_NONE );
   input_setKeybind( KST_STAR_MAP, KEYBIND_KEYBOARD, SDLK_M, NMOD_NONE );
   input_setKeybind( KST_JUMP, KEYBIND_KEYBOARD, SDLK_J, NMOD_NONE );
   input_setKeybind( KST_OVERLAY_MAP, KEYBIND_KEYBOARD, SDLK_TAB, NMOD_ANY );
   input_setKeybind( KST_MOUSE_FLYING, KEYBIND_KEYBOARD, SDLK_X, NMOD_CTRL );
   input_setKeybind( KST_COOLDOWN, KEYBIND_KEYBOARD, SDLK_S, NMOD_CTRL );
   /* Communication */
   input_setKeybind( KST_HAIL, KEYBIND_KEYBOARD, SDLK_Y, NMOD_NONE );
   input_setKeybind( KST_AUTOHAIL, KEYBIND_KEYBOARD, SDLK_Y, NMOD_CTRL );
   input_setKeybind( KST_SCAN, KEYBIND_KEYBOARD, SDLK_U, NMOD_ANY ),
      input_setKeybind( KST_LOG_UP, KEYBIND_KEYBOARD, SDLK_PAGEUP, NMOD_ANY );
   input_setKeybind( KST_LOG_DOWN, KEYBIND_KEYBOARD, SDLK_PAGEDOWN, NMOD_ANY );
   /* Misc. */
   input_setKeybind( KST_ZOOM_IN, KEYBIND_KEYBOARD, SDLK_KP_PLUS, NMOD_ANY );
   input_setKeybind( KST_ZOOM_OUT, KEYBIND_KEYBOARD, SDLK_KP_MINUS, NMOD_ANY );
   input_setKeybind( KST_SCREENSHOT, KEYBIND_KEYBOARD, SDLK_KP_MULTIPLY,
                     NMOD_ANY );
   input_setKeybind( KST_SCREENSHOT, KEYBIND_KEYBOARD, SDLK_F11, NMOD_ANY );
   input_setKeybind( KST_PAUSE, KEYBIND_KEYBOARD, SDLK_PAUSE, NMOD_ANY );

   input_setKeybind( KST_GAME_SPEED, KEYBIND_KEYBOARD, SDLK_GRAVE, NMOD_ANY );
   input_setKeybind( KST_MENU_SMALL, KEYBIND_KEYBOARD, SDLK_ESCAPE, NMOD_ANY );
   input_setKeybind( KST_MENU_INFO, KEYBIND_KEYBOARD, SDLK_I, NMOD_NONE );
   input_setKeybind( KST_CONSOLE, KEYBIND_KEYBOARD, SDLK_F2, NMOD_ANY );

#if SDL_PLATFORM_MACOS
   input_setKeybind( KST_PASTE, KEYBIND_KEYBOARD, SDLK_V, NMOD_META );
#else  /* SDL_PLATFORM_MACOS */
   input_setKeybind( KST_PASTE, KEYBIND_KEYBOARD, SDLK_V, NMOD_CTRL );
#endif /* SDL_PLATFORM_MACOS */
}

/**
 * @brief Initializes the input subsystem (does not set keys).
 */
void input_init( void )
{
   /* Keyboard. */
   SDL_SetEventEnabled( SDL_EVENT_KEY_DOWN, 1 );
   SDL_SetEventEnabled( SDL_EVENT_KEY_UP, 1 );

   /* Mice. */
   SDL_SetEventEnabled( SDL_EVENT_MOUSE_MOTION, 1 );
   SDL_SetEventEnabled( SDL_EVENT_MOUSE_BUTTON_DOWN, 1 );
   SDL_SetEventEnabled( SDL_EVENT_MOUSE_BUTTON_UP, 1 );

   /* Joystick, enabled in joystick.c if needed. */
   SDL_SetEventEnabled( SDL_EVENT_JOYSTICK_AXIS_MOTION, 0 );
   SDL_SetEventEnabled( SDL_EVENT_JOYSTICK_HAT_MOTION, 0 );
   SDL_SetEventEnabled( SDL_EVENT_JOYSTICK_BUTTON_DOWN, 0 );
   SDL_SetEventEnabled( SDL_EVENT_JOYSTICK_BUTTON_UP, 0 );

   /* Quit. */
   SDL_SetEventEnabled( SDL_EVENT_QUIT, 1 );

   /* Window. */
   SDL_SetEventEnabled( SDL_EVENT_WINDOW_RESIZED, 1 );

   /* Keyboard. */
   SDL_SetEventEnabled( SDL_EVENT_TEXT_INPUT,
                        0 ); /* Enabled on a per-widget basis. */

   /* Mouse. */
   SDL_SetEventEnabled( SDL_EVENT_MOUSE_WHEEL, 1 );

   /* Create safe null keybinding for each. */
   for ( int i = 0; i < KST_END; i++ ) {
      Keybind *k = &input_keybinds[i];
      memset( k, 0, sizeof( Keybind ) );
      k->type = KEYBIND_NULL;
      k->key  = SDLK_UNKNOWN;
      k->mod  = NMOD_NONE;

      if ( i == KST_PASTE )
         input_paste = k;
   }
}

/**
 * @brief Exits the input system.
 */
void input_exit( void )
{
}

/**
 * @brief Enables all the key bindings.
 */
void input_enableAll( void )
{
   for ( int i = 0; i < KST_END; i++ )
      input_keybinds[i].disabled = 0;
}

/**
 * @brief Disables all the key bindings.
 */
void input_disableAll( void )
{
   for ( int i = 0; i < KST_END; i++ )
      input_keybinds[i].disabled = 1;
}

/**
 * @brief Enables or disables a key binding.
 */
void input_toggleEnable( KeySemanticType key, int enable )
{
   input_keybinds[key].disabled = !enable;
}

/**
 * @brief Shows the mouse.
 */
void input_mouseShow( void )
{
   SDL_ShowCursor();
   input_mouseCounter++;
}

/**
 * @brief Hides the mouse.
 */
void input_mouseHide( void )
{
   input_mouseCounter--;
   if ( input_mouseCounter <= 0 ) {
      input_mouseTimer   = MIN( input_mouseTimer, conf.mouse_hide );
      input_mouseCounter = 0;
   }
}

/**
 * @brief Gets whether or not the mouse is currently shown.
 */
int input_mouseIsShown( void )
{
   return SDL_CursorVisible();
}

/**
 * @brief Gets the key id from its name.
 *
 *    @param name Name of the key to get id from.
 *    @return ID of the key.
 */
SDL_Keycode input_keyConv( const char *name )
{
   SDL_Keycode k = SDL_GetKeyFromName( name );
   if ( k == SDLK_UNKNOWN )
      WARN( _( "Keyname '%s' doesn't match any key." ), name );

   return k;
}

/**
 * @brief Binds key of type type to action key binding.
 *
 *    @param keybind The `KeySemanticType` of the key binding (as defined
 * above).
 *    @param type The type of the key binding.
 *    @param key The key to bind to.
 *    @param mod Modifiers to check for.
 */
void input_setKeybind( KeySemanticType keybind, KeybindType type,
                       SDL_Keycode key, SDL_Keymod mod )
{
   if ( ( keybind >= 0 ) && ( keybind < KST_END ) ) {
      Keybind *k = &input_keybinds[keybind];
      k->type    = type;
      k->key     = key;
      /* Non-keyboards get mod NMOD_ANY to always match. */
      k->mod = ( type == KEYBIND_KEYBOARD ) ? mod : NMOD_ANY;
      return;
   }
   WARN( _( "Unable to set keybinding '%d', that command doesn't exist" ),
         keybind );
}

/**
 * @brief Gets the value of a key binding.
 *
 *    @param[in] keybind `KeySemanticType` of the keybinding to get.
 *    @param[out] type Stores the type of the keybinding.
 *    @param[out] mod Stores the modifiers used with the keybinding.
 *    @return The key associated with the keybinding.
 */
SDL_Keycode input_getKeybind( KeySemanticType keybind, KeybindType *type,
                              SDL_Keymod *mod )
{
   if ( keybind < KST_END ) {
      if ( type != NULL )
         ( *type ) = input_keybinds[keybind].type;
      if ( mod != NULL )
         ( *mod ) = input_keybinds[keybind].mod;
      return input_keybinds[keybind].key;
   }
   WARN( _( "Unable to get keybinding '%d', that command doesn't exist" ),
         keybind );
   return (SDL_Keycode)-1;
}

/**
 * @brief Gets the display name (translated and human-readable) of a key binding
 *
 *    @param[in] keybind Name of the keybinding to get display name of.
 *    @param[out] buf Buffer to write the display name to.
 *    @param[in] len Length of buffer.
 */
void input_getKeybindDisplay( KeySemanticType keybind, char *buf, int len )
{
   /* Get the keybinding. */
   KeybindType type = KEYBIND_NULL;
   SDL_Keymod  mod  = NMOD_NONE;
   SDL_Keycode key  = input_getKeybind( keybind, &type, &mod );

   /* Handle type. */
   switch ( type ) {
   case KEYBIND_NULL:
      strncpy( buf, _( "Not bound" ), len );
      break;

   case KEYBIND_KEYBOARD: {
      int p = 0;
      /* Handle mod. */
      if ( ( mod != NMOD_NONE ) && ( mod != NMOD_ANY ) )
         p += scnprintf( &buf[p], len - p, "%s + ", input_modToText( mod ) );
      /* Print key. Special-case ASCII letters (use uppercase, unlike
       * SDL_GetKeyName.). */
      if ( key < 0x100 && isalpha( key ) )
         /*p +=*/scnprintf( &buf[p], len - p, "%c", toupper( key ) );
      else
         /*p +=*/scnprintf( &buf[p], len - p, "%s",
                            pgettext_var( "keyname", SDL_GetKeyName( key ) ) );
      break;
   }

   case KEYBIND_JBUTTON:
      snprintf( buf, len, _( "joy button %d" ), key );
      break;

   case KEYBIND_JHAT_UP:
      snprintf( buf, len, _( "joy hat %d up" ), key );
      break;

   case KEYBIND_JHAT_DOWN:
      snprintf( buf, len, _( "joy hat %d down" ), key );
      break;

   case KEYBIND_JHAT_LEFT:
      snprintf( buf, len, _( "joy hat %d left" ), key );
      break;

   case KEYBIND_JHAT_RIGHT:
      snprintf( buf, len, _( "joy hat %d right" ), key );
      break;

   case KEYBIND_JAXISPOS:
      snprintf( buf, len, _( "joy axis %d-" ), key );
      break;

   case KEYBIND_JAXISNEG:
      snprintf( buf, len, _( "joy axis %d+" ), key );
      break;
   }
}

/**
 * @brief Gets the human readable version of mod.
 *
 *    @param mod Mod to get human readable version from.
 *    @return Human readable version of mod.
 */
const char *input_modToText( SDL_Keymod mod )
{
   switch ( (int)mod ) {
   case NMOD_NONE:
      return _( "None" );
   case NMOD_CTRL:
      return _( "Ctrl" );
   case NMOD_SHIFT:
      return _( "Shift" );
   case NMOD_ALT:
      return _( "Alt" );
   case NMOD_META:
      return _( "Meta" );
   case NMOD_ANY:
      return _( "Any" );
   default:
      return _( "unknown" );
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
KeySemanticType input_keyAlreadyBound( KeybindType type, SDL_Keycode key,
                                       SDL_Keymod mod )
{
   for ( int i = 0; i < KST_END; i++ ) {
      const Keybind *k = &input_keybinds[i];

      /* Type must match. */
      if ( k->type != type )
         continue;

      /* Must match key. */
      if ( key != k->key )
         continue;

      /* Handle per case. */
      switch ( type ) {
      case KEYBIND_KEYBOARD:
         if ( ( k->mod == NMOD_ANY ) || ( mod == NMOD_ANY ) ||
              ( k->mod == mod ) )
            return i;
         break;

      case KEYBIND_JAXISPOS:
      case KEYBIND_JAXISNEG:
      case KEYBIND_JBUTTON:
      case KEYBIND_JHAT_UP:
      case KEYBIND_JHAT_DOWN:
      case KEYBIND_JHAT_LEFT:
      case KEYBIND_JHAT_RIGHT:
         return i;

      default:
         break;
      }
   }

   /* Not found. */
   return -1;
}

/**
 * @brief Gets the brief description of the keybinding.
 */
const char *input_getKeybindBrief( KeySemanticType keybind )
{
   if ( ( keybind >= 0 ) && ( keybind < KST_END ) )
      return keybind_info[keybind][2];
   WARN( _( "Unable to get keybinding '%d', that command doesn't exist" ),
         keybind );
   return NULL;
}

/**
 * @brief Gets the name of the keybinding.
 */
const char *input_getKeybindName( KeySemanticType keybind )
{
   if ( ( keybind >= 0 ) && ( keybind < KST_END ) )
      return _( keybind_info[keybind][0] );
   WARN( _( "Unable to get keybinding '%d', that command doesn't exist" ),
         keybind );
   return NULL;
}

/**
 * @brief Gets the description of the keybinding.
 *
 *    @param keybind Keybinding to get the description of.
 *    @return Description of the keybinding.
 */
const char *input_getKeybindDescription( KeySemanticType keybind )
{
   if ( ( keybind >= 0 ) && ( keybind < KST_END ) )
      return _( keybind_info[keybind][1] );
   WARN( _( "Unable to get keybinding '%d', that command doesn't exist" ),
         keybind );
   return NULL;
}

/**
 * @brief Translates SDL modifier to Naev modifier.
 *
 *    @param mod SDL modifier to translate.
 *    @return Naev modifier.
 */
SDL_Keymod input_translateMod( SDL_Keymod mod )
{
   SDL_Keymod mod_filtered = 0;
   if ( mod & ( SDL_KMOD_LSHIFT | SDL_KMOD_RSHIFT ) )
      mod_filtered |= NMOD_SHIFT;
   if ( mod & ( SDL_KMOD_LCTRL | SDL_KMOD_RCTRL ) )
      mod_filtered |= NMOD_CTRL;
   if ( mod & ( SDL_KMOD_LALT | SDL_KMOD_RALT ) )
      mod_filtered |= NMOD_ALT;
   if ( mod & ( SDL_KMOD_LGUI | SDL_KMOD_RGUI ) )
      mod_filtered |= NMOD_META;
   return mod_filtered;
}

/**
 * @brief Handles key repeating.
 */
void input_update( double dt )
{
   if ( input_mouseTimer > 0. ) {
      input_mouseTimer -= dt;

      /* Hide if necessary. */
      if ( ( input_mouseTimer < 0. ) && ( input_mouseCounter <= 0 ) )
         SDL_HideCursor();
   }

   /* Key repeat if applicable. */
   if ( conf.repeat_delay != 0 ) {
      unsigned int t;

      /* Key must be repeating. */
      if ( repeat_key == -1 )
         return;

      /* Get time. */
      t = SDL_GetTicks();

      /* Should be repeating. */
      if ( repeat_keyTimer + conf.repeat_delay +
              repeat_keyCounter * conf.repeat_freq >
           t )
         return;

      /* Key repeat. */
      repeat_keyCounter++;
      input_key( repeat_key, KEY_PRESS, 0., 1 );
   }
}

#define INGAME()                                                               \
   ( !toolkit_isOpen() &&                                                      \
     ( ( value == KEY_RELEASE ) || !player_isFlag( PLAYER_CINEMATICS ) ) &&    \
     ( player.p != NULL ) &&                                                   \
     !pilot_isFlag( player.p,                                                  \
                    PILOT_DEAD ) ) /**< Makes sure player is in game. */
#define HYP()                                                                  \
   ( ( player.p == NULL ) || pilot_isFlag( player.p, PILOT_HYP_PREP ) ||       \
     pilot_isFlag( player.p, PILOT_HYP_BEGIN ) ||                              \
     pilot_isFlag(                                                             \
        player.p,                                                              \
        PILOT_HYPERSPACE ) ) /**< Make sure the player isn't jumping. */
#define NOHYP()                                                                \
   ( ( player.p != NULL ) && !pilot_isFlag( player.p, PILOT_HYP_PREP ) &&      \
     !pilot_isFlag( player.p, PILOT_HYP_BEGIN ) &&                             \
     !pilot_isFlag(                                                            \
        player.p,                                                              \
        PILOT_HYPERSPACE ) ) /**< Make sure the player isn't jumping. */
#define DEAD()                                                                 \
   ( ( player.p == NULL ) ||                                                   \
     pilot_isFlag( player.p, PILOT_DEAD ) ) /**< Player is dead. */
#define NODEAD()                                                               \
   ( ( player.p != NULL ) &&                                                   \
     !pilot_isFlag( player.p, PILOT_DEAD ) ) /**< Player isn't dead. */
#define LAND()                                                                 \
   ( ( player.p == NULL ) || landed ||                                         \
     pilot_isFlag( player.p, PILOT_LANDING ) ) /**< Player isn't landed. */
#define NOLAND()                                                               \
   ( ( player.p != NULL ) &&                                                   \
     ( !landed &&                                                              \
       !pilot_isFlag( player.p,                                                \
                      PILOT_LANDING ) ) ) /**< Player isn't landed. */
#define MAP() ( map_isOpen() )
/**
 * @brief Runs the input command.
 *
 *    @param keynum The index of the key binding.
 *    @param value The value of the keypress (defined above).
 *    @param kabs The absolute value.
 *    @param repeat Whether the key is still held down, rather than newly
 * pressed.
 */
static void input_key( KeySemanticType keynum, double value, double kabs,
                       int repeat )
{
   HookParam hparam[3];
   int       isdoubletap = 0;

   /* Repetition stuff. */
   if ( conf.repeat_delay != 0 ) {
      if ( ( value == KEY_PRESS ) && !repeat ) {
         repeat_key        = keynum;
         repeat_keyTimer   = SDL_GetTicks();
         repeat_keyCounter = 0;
      } else if ( value == KEY_RELEASE ) {
         repeat_key        = -1;
         repeat_keyTimer   = 0;
         repeat_keyCounter = 0;
      }
   }

   /* Detect if double tap. */
   if ( value == KEY_PRESS ) {
      unsigned int t = SDL_GetTicks();
      if ( ( keynum == doubletap_key ) &&
           ( t - doubletap_t <= conf.doubletap_sens ) )
         isdoubletap = 1;
      else {
         doubletap_key = keynum;
         doubletap_t   = t;
      }
   }

   /*
    * movement
    */
   /* accelerating */
   switch ( keynum ) {
   case KST_ACCEL:
      if ( repeat )
         break;

      if ( kabs >= 0. ) {
         player_restoreControl( PINPUT_MOVEMENT, NULL );
         player_accel( kabs );
      } else { /* prevent it from getting stuck */
         if ( isdoubletap ) {
            if ( NOHYP() && NODEAD() ) {
               pilot_outfitLOnkeydoubletap( player.p, OUTFIT_KEY_ACCEL );
               pilot_afterburn( player.p );
               /* Allow keeping it on outside of weapon sets. */
               if ( player.p->afterburner != NULL ) {
                  player.p->afterburner->flags |= PILOTOUTFIT_ISON_TOGGLE;
                  pilot_weapSetUpdateOutfitState( player.p );
               }
            }
         } else if ( value == KEY_RELEASE ) {
            if ( NOHYP() && NODEAD() ) {
               pilot_outfitLOnkeyrelease( player.p, OUTFIT_KEY_ACCEL );
               /* Make sure to release the weapon set lock. */
               if ( player.p->afterburner != NULL ) {
                  player.p->afterburner->flags &= ~PILOTOUTFIT_ISON_TOGGLE;
                  pilot_weapSetUpdateOutfitState( player.p );
               }
            }
         }

         if ( value == KEY_PRESS ) {
            player_restoreControl( PINPUT_MOVEMENT, NULL );
            player_setFlag( PLAYER_ACCEL );
            player_accel( 1. );
         } else if ( value == KEY_RELEASE ) {
            player_rmFlag( PLAYER_ACCEL );
            if ( !player_isFlag( PLAYER_REVERSE ) )
               player_accelOver();
         }
      }
      break;
   /* turning left */
   case KST_LEFT:
      if ( repeat )
         break;
      if ( kabs >= 0. ) {
         player_restoreControl( PINPUT_MOVEMENT, NULL );
         player_setFlag( PLAYER_TURN_LEFT );
         player_left = kabs;
      } else {
         if ( isdoubletap ) {
            if ( NOHYP() && NODEAD() )
               pilot_outfitLOnkeydoubletap( player.p, OUTFIT_KEY_LEFT );
         } else if ( value == KEY_RELEASE ) {
            player_rmFlag( PLAYER_TURN_LEFT );
            player_left = 0.;
         }
         if ( value == KEY_PRESS ) {
            player_restoreControl( PINPUT_MOVEMENT, NULL );
            player_setFlag( PLAYER_TURN_LEFT );
            player_left = 1.;
         }
      }
      break;
   /* turning right */
   case KST_RIGHT:
      if ( repeat )
         break;
      if ( kabs >= 0. ) {
         player_restoreControl( PINPUT_MOVEMENT, NULL );
         player_setFlag( PLAYER_TURN_RIGHT );
         player_right = kabs;
      } else {
         if ( isdoubletap ) {
            if ( NOHYP() && NODEAD() )
               pilot_outfitLOnkeydoubletap( player.p, OUTFIT_KEY_RIGHT );
         } else if ( value == KEY_RELEASE ) {
            player_rmFlag( PLAYER_TURN_RIGHT );
            player_right = 0.;
         }
         if ( value == KEY_PRESS ) {
            player_restoreControl( PINPUT_MOVEMENT, NULL );
            player_setFlag( PLAYER_TURN_RIGHT );
            player_right = 1.;
         }
      }
      break;
   /* turn around to face vel */
   case KST_REVERSE:
      if ( repeat || HYP() || !INGAME() )
         break;
      if ( value == KEY_PRESS ) {
         player_restoreControl( PINPUT_MOVEMENT, NULL );
         player_setFlag( PLAYER_REVERSE );
         /* Double tap reverse = cooldown! */
         if ( isdoubletap )
            player_cooldownBrake();
      } else if ( ( value == KEY_RELEASE ) &&
                  player_isFlag( PLAYER_REVERSE ) ) {
         player_rmFlag( PLAYER_REVERSE );

         if ( !player_isFlag( PLAYER_ACCEL ) )
            player_accelOver();
      }
      break;
   /* try to enter stealth mode. */
   case KST_STEALTH:
      if ( repeat || HYP() || !INGAME() )
         break;
      if ( value == KEY_PRESS )
         player_stealth();
      break;

   /* face the target */
   case KST_FACE:
      if ( repeat )
         break;
      if ( value == KEY_PRESS ) {
         player_restoreControl( PINPUT_MOVEMENT, NULL );
         player_setFlag( PLAYER_FACE );
      } else if ( ( value == KEY_RELEASE ) && player_isFlag( PLAYER_FACE ) )
         player_rmFlag( PLAYER_FACE );
      break;

   /*
    * Combat
    */
   /* targeting */
   case KST_TARGET_NEXT:
      if ( !INGAME() && !MAP() )
         break;
      if ( value == KEY_PRESS ) {
         if ( MAP() )
            map_cycleMissions( 1 );
         else
            player_targetNext( 0 );
      }
      break;
   case KST_TARGET_PREV:
      if ( !INGAME() && !MAP() )
         break;
      if ( value == KEY_PRESS ) {
         if ( MAP() )
            map_cycleMissions( -1 );
         else
            player_targetPrev( 0 );
      }
      break;
   case KST_TARGET_CLOSE:
      if ( !INGAME() && !MAP() )
         break;
      if ( value == KEY_PRESS ) {
         if ( MAP() )
            map_cycleMissions( 1 );
         else
            player_targetNearest();
      }
      break;
   case KST_HTARGET_NEXT:
      if ( !INGAME() )
         break;
      if ( value == KEY_PRESS )
         player_targetNext( 1 );
      break;
   case KST_HTARGET_PREV:
      if ( !INGAME() )
         break;
      if ( value == KEY_PRESS )
         player_targetPrev( 1 );
      break;
   case KST_HTARGET_CLOSE:
      if ( !INGAME() )
         break;
      if ( value == KEY_PRESS )
         player_targetHostile();
      break;
   case KST_TARGET_CLEAR:
      if ( !INGAME() )
         break;
      if ( value == KEY_PRESS )
         player_targetClear();
      break;

   /*
    * Escorts.
    */
   case KST_ESCORT_NEXT:
      if ( !( INGAME() && !repeat ) )
         break;
      if ( value == KEY_PRESS )
         player_targetEscort( 0 );
      break;
   case KST_ESCORT_PREV:
      if ( !( INGAME() && !repeat ) )
         break;
      if ( value == KEY_PRESS )
         player_targetEscort( 1 );
      break;
   case KST_ESCORT_ATTACK:
      if ( !( INGAME() && !repeat ) )
         break;
      if ( value == KEY_PRESS )
         escorts_attack( player.p );
      break;
   case KST_ESCORT_HALT:
      if ( !( INGAME() && !repeat ) )
         break;
      if ( value == KEY_PRESS )
         escorts_hold( player.p );
      break;
   case KST_ESCORT_RETURN:
      if ( !( INGAME() && !repeat ) )
         break;
      if ( value == KEY_PRESS )
         escorts_return( player.p );
      break;
   case KST_ESCORT_CLEAR:
      if ( !( INGAME() && !repeat ) )
         break;
      if ( value == KEY_PRESS )
         escorts_clear( player.p );
      break;

   /*
    * secondary weapons
    */
   /* shooting primary weapon */
   case KST_FIRE_PRIMARY:
      if ( DEAD() )
         break;
      player_weapSetPress( 0, value, repeat );
      break;
   /* shooting secondary weapon */
   case KST_FIRE_SECONDARY:
      if ( DEAD() )
         break;
      player_weapSetPress( 1, value, repeat );
      break;
   /* Weapon sets. */
   case KST_WEAPSET1:
      if ( DEAD() )
         break;
      player_weapSetPress( 2, value, repeat );
      break;
   case KST_WEAPSET2:
      if ( DEAD() )
         break;
      player_weapSetPress( 3, value, repeat );
      break;
   case KST_WEAPSET3:
      if ( DEAD() )
         break;
      player_weapSetPress( 4, value, repeat );
      break;
   case KST_WEAPSET4:
      if ( DEAD() )
         break;
      player_weapSetPress( 5, value, repeat );
      break;
   case KST_WEAPSET5:
      if ( DEAD() )
         break;
      player_weapSetPress( 6, value, repeat );
      break;
   case KST_WEAPSET6:
      if ( DEAD() )
         break;
      player_weapSetPress( 7, value, repeat );
      break;
   case KST_WEAPSET7:
      if ( DEAD() )
         break;
      player_weapSetPress( 8, value, repeat );
      break;
   case KST_WEAPSET8:
      if ( DEAD() )
         break;
      player_weapSetPress( 9, value, repeat );
      break;
   case KST_WEAPSET9:
      if ( DEAD() )
         break;
      player_weapSetPress( 10, value, repeat );
      break;
   case KST_WEAPSET0:
      if ( DEAD() )
         break;
      player_weapSetPress( 11, value, repeat );
      break;

   /*
    * Space
    */
   case KST_AUTONAV:
      if ( HYP() || DEAD() )
         break;
      if ( value == KEY_PRESS ) {
         if ( MAP() ) {
            unsigned int wid = window_get( MAP_WDWNAME );
            player_autonavStartWindow( wid, NULL );
         } else if INGAME ()
            player_autonavStart();
      }
      break;
   /* target spob (cycles like target) */
   case KST_TARGET_SPOB:
      if ( HYP() || LAND() || !INGAME() )
         break;
      if ( value == KEY_PRESS )
         player_targetSpob();
      break;
   /* target nearest spob or attempt to land */
   case KST_APPROACH:
      if ( repeat || LAND() || HYP() || !INGAME() )
         break;
      if ( value == KEY_PRESS ) {
         player_restoreControl( 0, NULL );
         player_approach();
      }
      break;
   case KST_TARGET_JUMP:
      if ( DEAD() || HYP() || LAND() )
         break;
      if ( value == KEY_PRESS )
         player_targetHyperspace();
      break;
   case KST_STAR_MAP:
      if ( repeat || HYP() || DEAD() )
         break;
      if ( value == KEY_PRESS )
         map_open();
      break;
   case KST_JUMP:
      if ( !( INGAME() && !repeat ) )
         break;
      if ( value == KEY_PRESS ) {
         player_restoreControl( 0, NULL );
         player_jump();
      }
      break;
   case KST_OVERLAY_MAP:
      if ( ( repeat || !INGAME() ) && !MAP() )
         break;
      if ( MAP() )
         map_toggleNotes();
      else
         ovr_key( value );
      break;
   case KST_MOUSE_FLYING:
      if ( DEAD() || repeat )
         break;
      if ( value == KEY_PRESS )
         player_toggleMouseFly();
      break;
   case KST_COOLDOWN:
      if ( repeat || DEAD() || LAND() || HYP() )
         break;
      if ( value == KEY_PRESS ) {
         player_restoreControl( PINPUT_BRAKING, NULL );
         player_cooldownBrake();
      }
      break;

   /*
    * Communication.
    */
   case KST_HAIL:
      if ( repeat || !INGAME() || HYP() )
         break;
      if ( value == KEY_PRESS )
         player_hail();
      break;
   case KST_AUTOHAIL:
      if ( repeat || !INGAME() || HYP() )
         break;
      if ( value == KEY_PRESS )
         player_autohail();
      break;
   case KST_SCAN:
      if ( repeat || !INGAME() || HYP() )
         break;
      if ( value == KEY_PRESS )
         player_scan();
      break;
   case KST_LOG_UP:
      if ( !INGAME() )
         break;
      if ( value == KEY_PRESS )
         gui_messageScrollUp( 5 );
      break;
   case KST_LOG_DOWN:
      if ( !INGAME() )
         break;
      if ( value == KEY_PRESS )
         gui_messageScrollDown( 5 );
      break;

   /*
    * misc
    */
   /* zooming in */
   case KST_ZOOM_IN:
      if ( !INGAME() )
         break;
      if ( value == KEY_PRESS )
         gui_setRadarRel( -1 );
      break;
   /* zooming out */
   case KST_ZOOM_OUT:
      if ( !INGAME() )
         break;
      if ( value == KEY_PRESS )
         gui_setRadarRel( 1 );
      break;
   /* take a screenshot */
   case KST_SCREENSHOT:
      if ( repeat )
         break;
      if ( value == KEY_PRESS )
         player_screenshot();
      break;
   /* toggle fullscreen */
   case KST_FULLSCREEN:
      if ( repeat )
         break;
      if ( value == KEY_PRESS )
         naev_toggleFullscreen();
      break;
   /* pause the games */
   case KST_PAUSE:
      if ( repeat )
         break;
      if ( value == KEY_PRESS ) {
         if ( !toolkit_isOpen() ) {
            if ( paused )
               unpause_game();
            else
               pause_player();
         }
      }
      break;
   /* toggle speed mode */
   case KST_GAME_SPEED:
      if ( repeat )
         break;
      if ( ( value == KEY_PRESS ) &&
           ( !player_isFlag( PLAYER_CINEMATICS_2X ) ) ) {
         if ( player.speed < 4. * conf.game_speed )
            player.speed *= 2.;
         else
            player.speed = conf.game_speed;
         player_resetSpeed();
      }
      break;
   /* opens a small menu */
   case KST_MENU_SMALL:
      if ( DEAD() || repeat )
         break;
      if ( value == KEY_PRESS )
         menu_small( 1, 1, 1, 1 );
      break;

   /* shows pilot information */
   case KST_MENU_INFO:
      if ( repeat || DEAD() || HYP() )
         break;
      if ( value == KEY_PRESS )
         menu_info( INFO_DEFAULT );
      break;

   /* Opens the Lua console. */
   case KST_CONSOLE:
      if ( DEAD() || repeat )
         break;
      if ( value == KEY_PRESS )
         cli_open();
      break;

   /* Key not used. */
   default:
      return;
   }

   /* Run the hook. */
   hparam[0].type  = HOOK_PARAM_STRING;
   hparam[0].u.str = input_getKeybindBrief( keynum );
   hparam[1].type  = HOOK_PARAM_BOOL;
   hparam[1].u.b   = ( value > 0. );
   hparam[2].type  = HOOK_PARAM_SENTINEL;
   hooks_runParam( "input", hparam );
}

/*
 * joystick
 */
/**
 * @brief Filters a joystick axis event.
 *    @param axis Axis generated by the event.
 *    @param value Value of the axis.
 */
static void input_joyaxis( const SDL_Keycode axis, const int value )
{
   for ( int i = 0; i < KST_END; i++ ) {
      const Keybind *k = &input_keybinds[i];
      if ( k->key != axis )
         continue;
      /* Positive axis keybinding. */
      if ( ( k->type == KEYBIND_JAXISPOS ) && ( value >= 0 ) ) {
         int press = ( value > 0 ) ? KEY_PRESS : KEY_RELEASE;
         if ( ( press == KEY_PRESS ) && k->disabled )
            continue;
         input_key( i, press, FABS( ( (double)value ) / 32767. ), 0 );
      }

      /* Negative axis keybinding. */
      if ( ( k->type == KEYBIND_JAXISNEG ) && ( value <= 0 ) ) {
         int press = ( value < 0 ) ? KEY_PRESS : KEY_RELEASE;
         if ( ( press == KEY_PRESS ) && k->disabled )
            continue;
         input_key( i, press, FABS( ( (double)value ) / 32767. ), 0 );
      }
   }
}
/**
 * @brief Filters a joystick button event.
 *    @param event Event type (down/up).
 *    @param button Button generating the event.
 */
static void input_joyevent( const int event, const SDL_Keycode button )
{
   for ( int i = 0; i < KST_END; i++ ) {
      const Keybind *k = &input_keybinds[i];
      if ( ( event == KEY_PRESS ) && k->disabled )
         continue;
      if ( ( k->type == KEYBIND_JBUTTON ) && ( k->key == button ) )
         input_key( i, event, -1., 0 );
   }
}

/**
 * @brief Filters a joystick hat event.
 *    @param value Direction on hat.
 *    @param hat Hat generating the event.
 */
static void input_joyhatevent( const Uint8 value, const Uint8 hat )
{
   for ( int i = 0; i < KST_END; i++ ) {
      const Keybind *k = &input_keybinds[i];
      if ( k->key != hat )
         continue;

      if ( k->type == KEYBIND_JHAT_UP ) {
         int event = ( value & SDL_HAT_UP ) ? KEY_PRESS : KEY_RELEASE;
         if ( !( ( event == KEY_PRESS ) && k->disabled ) )
            input_key( i, event, -1., 0 );
      } else if ( k->type == KEYBIND_JHAT_DOWN ) {
         int event = ( value & SDL_HAT_DOWN ) ? KEY_PRESS : KEY_RELEASE;
         if ( !( ( event == KEY_PRESS ) && k->disabled ) )
            input_key( i, event, -1., 0 );
      } else if ( k->type == KEYBIND_JHAT_LEFT ) {
         int event = ( value & SDL_HAT_LEFT ) ? KEY_PRESS : KEY_RELEASE;
         if ( !( ( event == KEY_PRESS ) && k->disabled ) )
            input_key( i, event, -1., 0 );
      } else if ( k->type == KEYBIND_JHAT_RIGHT ) {
         int event = ( value & SDL_HAT_RIGHT ) ? KEY_PRESS : KEY_RELEASE;
         if ( !( ( event == KEY_PRESS ) && k->disabled ) )
            input_key( i, event, -1., 0 );
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
 *    @param repeat Whether the key is still held down, rather than newly
 * pressed.
 */
static void input_keyevent( const int event, SDL_Keycode key,
                            const SDL_Keymod mod, const int repeat )
{
   /* Filter to "Naev" modifiers. */
   SDL_Keymod mod_filtered = input_translateMod( mod );
   for ( int i = 0; i < KST_END; i++ ) {
      const Keybind *k = &input_keybinds[i];
      if ( ( event == KEY_PRESS ) && k->disabled )
         continue;
      if ( k->type != KEYBIND_KEYBOARD )
         continue;
      if ( k->key != key )
         continue;
      /* Release always gets through. */
      if ( ( k->mod == mod_filtered ) || ( k->mod == NMOD_ANY ) ||
           ( event == KEY_RELEASE ) )
         input_key( i, event, -1., repeat );
      /* No break since multiple keys can be bound to one symbol. */
   }
}

/**
 * @brief Handles zoom.
 */
static void input_clickZoom( double modifier )
{
   if ( player.p != NULL )
      cam_setZoomTarget( cam_getZoomTarget() * modifier, conf.zoom_speed );
}

/**
 * @brief Provides mouse X and Y coordinates for mouse flying.
 */
static void input_mouseMove( SDL_Event *event )
{
   float mx, my;
   gl_windowToScreenPos( &mx, &my, event->button.x, event->button.y );
   player.mousex = mx;
   player.mousey = my;
}

/**
 * @brief Handles a click event.
 */
static void input_clickevent( SDL_Event *event )
{
   float     mx, my;
   int       res;
   double    x, y, zoom;
   HookParam hparam[3];

   /* Generate hook. */
   hparam[0].type  = HOOK_PARAM_NUMBER;
   hparam[0].u.num = event->button.button;
   hparam[1].type  = HOOK_PARAM_BOOL;
   hparam[1].u.b   = ( event->type == SDL_EVENT_MOUSE_BUTTON_DOWN );
   hparam[2].type  = HOOK_PARAM_SENTINEL;
   hooks_runParam( "mouse", hparam );

   /* Disable in cinematics. */
   if ( player_isFlag( PLAYER_CINEMATICS ) )
      return;

   /* Player must not be NULL. */
   if ( ( player.p == NULL ) || player_isFlag( PLAYER_DESTROYED ) )
      return;

   /* Player must not be dead. */
   if ( pilot_isFlag( player.p, PILOT_DEAD ) )
      return;

   /* Middle mouse enables mouse flying. */
   if ( event->button.button == SDL_BUTTON_MIDDLE ) {
      player_toggleMouseFly();
      return;
   }

   /* Mouse targeting only uses left and right buttons. */
   if ( event->button.button != SDL_BUTTON_LEFT &&
        event->button.button != SDL_BUTTON_RIGHT )
      return;

   if ( gui_radarClickEvent( event ) )
      return;

   /* Visual (on-screen) */
   gl_windowToScreenPos( &mx, &my, event->button.x, event->button.y );
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
 *    @brief minr Minimum radius to assign to spobs and jumps.
 *    @return Whether the click was used to trigger an action.
 */
int input_clickPos( SDL_Event *event, double x, double y, double zoom,
                    double minpr, double minr )
{
   unsigned int pid;
   double       dp;
   int          pntid, jpid, astid, fieid;

   /* Don't allow selecting a new target with the right mouse button
    * (prevents pilots from getting in the way of autonav). */
   if ( event->button.button == SDL_BUTTON_RIGHT ) {
      pid            = player.p->target;
      const Pilot *p = pilot_get( pid );
      dp             = pow2( x - p->solid.pos.x ) + pow2( y - p->solid.pos.y );
   } else {
      dp = pilot_getNearestPos( player.p, &pid, x, y, 1 );
   }

   system_getClosest( cur_system, &pntid, &jpid, &astid, &fieid, x, y );

   if ( pntid >= 0 ) { /* Spob is closer. */
      const Spob *spb  = cur_system->spobs[pntid];
      double      rspb = MAX( 1.5 * spb->radius * zoom, minr );
      double      dspb = hypotf( spb->pos.x - x, spb->pos.y - y );
      if ( dspb > rspb )
         pntid = -1;
   }
   if ( jpid >= 0 ) {
      const JumpPoint *jmp  = &cur_system->jumps[jpid];
      double           rjmp = MAX( 1.5 * jmp->radius * zoom, minr );
      double           djmp = hypotf( jmp->pos.x - x, jmp->pos.y - y );
      if ( djmp > rjmp )
         jpid = -1;
   }
   if ( astid >= 0 ) {
      const AsteroidAnchor *field = &cur_system->asteroids[fieid];
      const Asteroid       *ast   = &field->asteroids[astid];
      /* Recover the right gfx */
      double rast = MAX( MAX( tex_sw( ast->gfx ) * zoom, minr ),
                         tex_sh( ast->gfx ) * zoom );
      double dast = hypotf( ast->sol.pos.x - x, ast->sol.pos.y - y );
      if ( dast > rast )
         astid = -1;
   }
   if ( pid != PLAYER_ID ) {
      const Pilot *p = pilot_get( pid );
      double       rp =
         MAX( 1.5 * PILOT_SIZE_APPROX * p->ship->size / 2 * zoom, minpr );
      /* Reject pilot if it's too far or a valid spob is closer. */
      if ( dp > pow2( rp ) )
         pid = PLAYER_ID;
   }

   /* Target a pilot, spob or jump, and/or perform an appropriate action. */
   if ( event->button.button == SDL_BUTTON_LEFT ) {
      if ( ( pntid >= 0 ) && input_clickedSpob( pntid, 0, 1 ) )
         return 1;
      else if ( ( jpid >= 0 ) && input_clickedJump( jpid, 0 ) )
         return 1;
      else if ( ( pid != PLAYER_ID ) && input_clickedPilot( pid, 0 ) )
         return 1;
      else if ( ( astid >= 0 ) && input_clickedAsteroid( fieid, astid ) )
         return 1;
      else if ( ( pntid >= 0 ) && input_clickedSpob( pntid, 0, 0 ) )
         return 1;
   }
   /* Right click only controls autonav. */
   else if ( event->button.button == SDL_BUTTON_RIGHT ) {
      if ( ( pntid >= 0 ) && input_clickedSpob( pntid, 0, 0 ) )
         return 1;
      else if ( ( jpid >= 0 ) && input_clickedJump( jpid, 1 ) )
         return 1;
      else if ( ( pid != PLAYER_ID ) && input_clickedPilot( pid, 1 ) )
         return 1;

      /* Go to position, if the position is >= 1500 px away. */
      if ( ( pow2( x - player.p->solid.pos.x ) +
             pow2( y - player.p->solid.pos.y ) ) >= pow2( 1500 ) )
         player_autonavPos( x, y );
      return 1;
   }

   return 0;
}

/**
 * @brief Performs an appropriate action when a jump point is clicked.
 *
 *    @param jump Index of the jump point.
 *    @param autonav Whether to autonav to the target.
 *    @return Whether the click was used.
 */
int input_clickedJump( int jump, int autonav )
{
   const JumpPoint *jp = &cur_system->jumps[jump];

   if ( !jp_isUsable( jp ) )
      return 0;

   if ( autonav )
      return 0;

   if ( player.p->nav_hyperspace != jump )
      map_select( jp->target, 0 );

   static unsigned int     lastclick_time = 0;
   static const JumpPoint *lastclick_jump = NULL;
   int                     doubleclick    = input_doubleClickTest(
      &lastclick_time, (const void **)&lastclick_jump, jp );

   if ( jump == player.p->nav_hyperspace ) {
      if ( doubleclick ) {
         player_targetHyperspaceSet( jump, 0 );
         if ( space_canHyperspace( player.p ) )
            player_jump();
         else
            player_autonavStart();
         return 1;
      } else
         return 0; /* Already selected, ignore. */
   } else
      player_targetHyperspaceSet( jump, 0 );

   return 0;
}

/**
 * @brief Performs an appropriate action when a spob is clicked.
 *
 *    @param spob Index of the spob.
 *    @param autonav Whether to autonav to the target.
 *    @param priority Whether to consider priority targets.
 *    @return Whether the click was used.
 */
int input_clickedSpob( int spob, int autonav, int priority )
{
   Spob *pnt = cur_system->spobs[spob];

   if ( !spob_isKnown( pnt ) )
      return 0;

   if ( autonav ) {
      player_targetSpobSet( spob );
      player_autonavSpob( pnt->name, 0 );
      return 1;
   }

   static unsigned int lastclick_time = 0;
   static const Spob  *lastclick_spob = NULL;
   int                 doubleclick    = input_doubleClickTest(
      &lastclick_time, (const void **)&lastclick_spob, pnt );

   /* If not priority, ignore non-landable / uninhabited. */
   if ( priority &&
        ( !spob_isFlag( pnt, SPOB_SERVICE_INHABITED ) || !pnt->can_land ) )
      return 0;

   if ( spob == player.p->nav_spob ) {
      if ( doubleclick ) {
         player_hyperspacePreempt( 0 );
         spob_updateLand( pnt );
         if ( !spob_isFlag( pnt, SPOB_SERVICE_INHABITED ) || pnt->can_land ||
              ( pnt->land_override > 0 ) ) {
            int ret = player_land( 0 );
            if ( ret == PLAYER_LAND_AGAIN ) {
               player_autonavSpob( pnt->name, 1 );
            } else if ( ret == PLAYER_LAND_DENIED ) {
               player_autonavSpob( pnt->name, 0 );
            }
         } else
            player_hailSpob();
         return 1;
      } else
         return 0; /* Already selected, ignore. */
   } else
      player_targetSpobSet( spob );

   return 1;
}

/**
 * @brief Performs an appropriate action when an asteroid is clicked.
 *
 *    @param field Index of the parent field of the asteroid.
 *    @param asteroid Index of the asteroid in the field.
 *    @return Whether the click was used.
 */
int input_clickedAsteroid( int field, int asteroid )
{
   // const AsteroidAnchor *anchor = &cur_system->asteroids[field];
   // const Asteroid       *ast    = &anchor->asteroids[asteroid];
   player_targetAsteroidSet( field, asteroid );
   return 1;
}

/**
 * @brief Performs an appropriate action when a pilot is clicked.
 *
 *    @param pilot Index of the pilot.
 *    @param autonav Whether this is an autonav action.
 *    @return Whether the click was used.
 */
int input_clickedPilot( unsigned int pilot, int autonav )
{
   if ( pilot == PLAYER_ID )
      return 0;

   if ( autonav ) {
      player_targetSet( pilot );
      player_autonavPil( pilot );
      return 1;
   }
   const Pilot *p = pilot_get( pilot );

   static unsigned int lastclick_time  = 0;
   static const Pilot *lastclick_pilot = NULL;
   int                 doubleclick     = input_doubleClickTest(
      &lastclick_time, (const void **)&lastclick_pilot, p );

   if ( pilot == player.p->target ) {
      if ( doubleclick ) {
         if ( pilot_isDisabled( p ) || pilot_isFlag( p, PILOT_BOARDABLE ) ) {
            if ( player_tryBoard( 0 ) == PLAYER_BOARD_RETRY )
               player_autonavBoard( player.p->target );
         } else
            player_hail();
         return 1;
      } else
         return 0; /* Ignore reselection. */
   } else
      player_targetSet( pilot );

   return 1;
}

/**
 * @brief Sets the last-clicked item, for double-click detection.
 *    @param clicked Pointer to the clicked item.
 */
void input_clicked( const void *clicked )
{
   if ( conf.mouse_doubleclick <= 0. )
      return;

   input_lastClicked    = clicked;
   input_mouseClickLast = SDL_GetTicks();
}

/**
 * @brief Checks whether a clicked item is the same as the last-clicked.
 *    @param clicked Pointer to the clicked item.
 */
int input_isDoubleClick( const void *clicked )
{
   unsigned int threshold;

   if ( conf.mouse_doubleclick <= 0. )
      return 0;

   /* Most recent time that constitutes a valid double-click. */
   threshold =
      input_mouseClickLast + (int)floor( conf.mouse_doubleclick * 1000. );

   if ( ( SDL_GetTicks() <= threshold ) && ( clicked == input_lastClicked ) )
      return 1;

   return 0;
}

static int input_doubleClickTest( unsigned int *time, const void **last,
                                  const void *clicked )
{
   unsigned int threshold, ticks;

   if ( conf.mouse_doubleclick <= 0. )
      return 0;

   /* Most recent time that constitutes a valid double-click. */
   threshold = *time + (int)floor( conf.mouse_doubleclick * 1000. );
   ticks     = SDL_GetTicks();

   if ( ( ticks <= threshold ) && ( *last == clicked ) )
      return 1;

   *last = clicked;
   *time = ticks;
   return 0;
}

/**
 * @brief Handles global input.
 *
 * Basically separates the event types
 *
 *    @param event Incoming SDL_Event.
 */
void input_handle( SDL_Event *event )
{
   int ismouse;

   /* Special case mouse stuff. */
   if ( ( event->type == SDL_EVENT_MOUSE_MOTION ) ||
        ( event->type == SDL_EVENT_MOUSE_BUTTON_DOWN ) ||
        ( event->type == SDL_EVENT_MOUSE_BUTTON_UP ) ) {
      input_mouseTimer = conf.mouse_hide;
      SDL_ShowCursor();
      ismouse = 1;
   } else
      ismouse = 0;

   /* Special case paste. */
   if ( event->type == SDL_EVENT_KEY_DOWN && SDL_HasClipboardText() &&
        SDL_EventEnabled( SDL_EVENT_TEXT_INPUT ) == 1 ) {
      SDL_Keymod mod = input_translateMod( event->key.mod );
      if ( ( input_paste->key == event->key.key ) &&
           ( input_paste->mod & mod ) ) {
         SDL_Event   evt;
         const char *txt    = SDL_GetClipboardText();
         evt.type           = SDL_EVENT_TEXT_INPUT;
         Uint32 timestamp   = SDL_GetTicks();
         evt.text.text      = strdup( txt );
         evt.text.timestamp = timestamp;
         evt.text.windowID  = 0;
         SDL_PushEvent( &evt );
         return;
      }
   }

   if ( toolkit_isOpen() ) { /* toolkit handled completely separately */
      if ( toolkit_input( event ) )
         return; /* we don't process it if toolkit grabs it */
      if ( ismouse )
         return; /* Toolkit absorbs everything mousy. */
   }

   if ( ovr_isOpen() )
      if ( ovr_input( event ) )
         return; /* Don't process if the map overlay wants it. */

   /* GUI gets event. */
   if ( gui_handleEvent( event ) )
      return;

   switch ( event->type ) {
   case SDL_EVENT_JOYSTICK_AXIS_MOTION:
      input_joyaxis( event->jaxis.axis, event->jaxis.value );
      break;
   case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
      input_joyevent( KEY_PRESS, event->jbutton.button );
      break;
   case SDL_EVENT_JOYSTICK_BUTTON_UP:
      input_joyevent( KEY_RELEASE, event->jbutton.button );
      break;
   case SDL_EVENT_JOYSTICK_HAT_MOTION:
      input_joyhatevent( event->jhat.value, event->jhat.hat );
      break;

   case SDL_EVENT_KEY_DOWN:
      if ( event->key.repeat != 0 )
         return;
      input_keyevent( KEY_PRESS, event->key.key, event->key.mod, 0 );
      break;
   case SDL_EVENT_KEY_UP:
      if ( event->key.repeat != 0 )
         return;
      input_keyevent( KEY_RELEASE, event->key.key, event->key.mod, 0 );
      break;

   case SDL_EVENT_MOUSE_BUTTON_DOWN:
      input_clickevent( event );
      break;
   case SDL_EVENT_MOUSE_WHEEL:
      if ( event->wheel.y > 0 )
         input_clickZoom( 1.1 );
      else if ( event->wheel.y < 0 )
         input_clickZoom( 0.9 );
      break;
   case SDL_EVENT_MOUSE_MOTION:
      input_mouseMove( event );
      break;

   default:
      break;
   }
}

/**
 * Gets the semantic key binding from a brief description name.
 */
KeySemanticType input_keyFromBrief( const char *target )
{
   for ( int i = 0; i < KST_END; i++ ) {
      if ( strcmp( input_getKeybindBrief( i ), target ) == 0 )
         return i;
   }
   WARN( _( "Key '%s' not found!" ), target );
   return -1;
}
