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
/** @endcond */

#include "input.h"

#include "array.h"
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
#include "toolkit.h"
#include "weapon.h"
#include "utf8.h"

/* keybinding structure */
/**
 * @brief Naev Keybinding.
 */
typedef struct Keybind_ {
   int disabled; /**< Whether or not it's disabled. */
   const char *name; /**< keybinding name, taken from keybind_info */
   KeybindType type; /**< type, defined in player.h */
   SDL_Keycode key; /**< key/axis/button event number */
   SDL_Keymod mod; /**< Key modifiers (where applicable). */
} Keybind;

/* name of each keybinding */
const char *keybind_info[][3] = {
   /* Movement */
   { "accel", N_("Accelerate"), N_("Makes your ship accelerate forward.") },
   { "left", N_("Turn Left"), N_("Makes your ship turn left.") },
   { "right", N_("Turn Right"), N_("Makes your ship turn right.") },
   { "reverse", N_("Reverse"), N_("Makes your ship face the direction you're moving from. Useful for braking.") },
   { "stealth", N_("Stealth"), N_("Tries to enter stealth mode.") },
   /* Targeting */
   { "target_next", N_("Target Next"), N_("Cycles through ship targets.") },
   { "target_prev", N_("Target Previous"), N_("Cycles backwards through ship targets.") },
   { "target_nearest", N_("Target Nearest"), N_("Targets the nearest non-disabled ship.") },
   { "target_nextHostile", N_("Target Next Hostile"), N_("Cycles through hostile ship targets.") },
   { "target_prevHostile", N_("Target Previous Hostile"), N_("Cycles backwards through hostile ship targets.") },
   { "target_hostile", N_("Target Nearest Hostile"), N_("Targets the nearest hostile ship.") },
   { "target_clear", N_("Clear Target"), N_("Clears the currently-targeted ship, spob or jump point.") },
   /* Fighting */
   { "primary", N_("Fire Primary Weapon"), N_("Fires primary weapons.") },
   { "face", N_("Face Target"), N_("Faces the targeted ship if one is targeted, otherwise faces targeted spob or jump point.") },
   /* Secondary Weapons */
   { "secondary", N_("Fire Secondary Weapon"), N_("Fires secondary weapons.") },
   { "weapset1", N_("Weapon Set 1"), N_("Activates weapon set 1.") },
   { "weapset2", N_("Weapon Set 2"), N_("Activates weapon set 2.") },
   { "weapset3", N_("Weapon Set 3"), N_("Activates weapon set 3.") },
   { "weapset4", N_("Weapon Set 4"), N_("Activates weapon set 4.") },
   { "weapset5", N_("Weapon Set 5"), N_("Activates weapon set 5.") },
   { "weapset6", N_("Weapon Set 6"), N_("Activates weapon set 6.") },
   { "weapset7", N_("Weapon Set 7"), N_("Activates weapon set 7.") },
   { "weapset8", N_("Weapon Set 8"), N_("Activates weapon set 8.") },
   { "weapset9", N_("Weapon Set 9"), N_("Activates weapon set 9.") },
   { "weapset0", N_("Weapon Set 0"), N_("Activates weapon set 0.") },
   /* Escorts */
   { "e_targetNext", N_("Target Next Escort"), N_("Cycles through your escorts.") },
   { "e_targetPrev", N_("Target Previous Escort"), N_("Cycles backwards through your escorts.") },
   { "e_attack", N_("Escort Attack Command"), N_("Orders escorts to attack your target.") },
   { "e_hold", N_("Escort Hold Command"), N_("Orders escorts to hold their formation.") },
   { "e_return", N_("Escort Return Command"), N_("Orders escorts to return to your ship hangars.") },
   { "e_clear", N_("Escort Clear Commands"), N_("Clears your escorts of commands.") },
   /* Space Navigation */
   { "autonav", N_("Autonavigation On"), N_("Initializes the autonavigation system.") },
   { "target_spob", N_("Target Spob"), N_("Cycles through space object targets.") },
   { "approach", N_("Approach"), N_("Attempts to approach the targeted ship or space object, or targets the nearest landable space object. Requests landing permission if necessary. Prioritizes ships over space objects.") },
   { "thyperspace", N_("Target Jump Point"), N_("Cycles through jump points.") },
   { "starmap", N_("Star Map"), N_("Opens the star map.") },
   { "jump", N_("Initiate Jump"), N_("Attempts to jump via a jump point.") },
   { "overlay", N_("Overlay Map"), N_("Opens the in-system overlay map.") },
   { "mousefly", N_("Mouse Flight"), N_("Toggles mouse flying.") },
   { "cooldown", N_("Active Cooldown"), N_("Begins active cooldown.") },
   /* Communication */
   { "log_up", N_("Log Scroll Up"), N_("Scrolls the log upwards.") },
   { "log_down", N_("Log Scroll Down"), N_("Scrolls the log downwards.") },
   { "hail", N_("Hail Target"), N_("Attempts to initialize communication with the targeted ship.") },
   { "autohail", N_("Autohail"), N_("Automatically initialize communication with a ship that is hailing you.") },
   /* Misc. */
   { "mapzoomin", N_("Radar Zoom In"), N_("Zooms in on the radar.") },
   { "mapzoomout", N_("Radar Zoom Out"), N_("Zooms out on the radar.") },
   { "screenshot", N_("Screenshot"), N_("Takes a screenshot.") },
   { "togglefullscreen", N_("Toggle Fullscreen"), N_("Toggles between windowed and fullscreen mode.") },
   { "pause", N_("Pause"), N_("Pauses the game.") },
   { "speed", N_("Toggle Speed"), N_("Toggles speed modifier.") },
   { "menu", N_("Small Menu"), N_("Opens the small in-game menu.") },
   { "info", N_("Information Menu"), N_("Opens the information menu.") },
   { "console", N_("Lua Console"), N_("Opens the Lua console.") },
   { "switchtab1", N_("Switch Tab 1"), N_("Switches to tab 1.") },
   { "switchtab2", N_("Switch Tab 2"), N_("Switches to tab 2.") },
   { "switchtab3", N_("Switch Tab 3"), N_("Switches to tab 3.") },
   { "switchtab4", N_("Switch Tab 4"), N_("Switches to tab 4.") },
   { "switchtab5", N_("Switch Tab 5"), N_("Switches to tab 5.") },
   { "switchtab6", N_("Switch Tab 6"), N_("Switches to tab 6.") },
   { "switchtab7", N_("Switch Tab 7"), N_("Switches to tab 7.") },
   { "switchtab8", N_("Switch Tab 8"), N_("Switches to tab 8.") },
   { "switchtab9", N_("Switch Tab 9"), N_("Switches to tab 9.") },
   { "switchtab0", N_("Switch Tab 0"), N_("Switches to tab 0.") },
   /* Console-main. */
   { "paste", N_("Paste"), N_("Paste from the operating system's clipboard.") },
   /* Must terminate in NULL. */
   { NULL, NULL, NULL }
}; /**< Names of possible keybindings. */

static Keybind *input_keybinds; /**< contains the players keybindings */
const int input_numbinds = ( sizeof( keybind_info ) / sizeof( keybind_info[ 0 ] ) ) - 1; /**< Number of keybindings. */
static Keybind *input_paste;

/*
 * accel hacks
 */
static int doubletap_key         = -1; /**< Last key double tapped. */
static unsigned int doubletap_t  = 0; /**< Used to see if double tap accel. */

/*
 * Key repeat hack.
 */
static int repeat_key                  = -1; /**< Key to repeat. */
static unsigned int repeat_keyTimer    = 0;  /**< Repeat timer. */
static unsigned int repeat_keyCounter  = 0;  /**< Counter for key repeats. */

/*
 * Mouse.
 */
static double input_mouseTimer         = 1.; /**< Timer for hiding again. */
static int input_mouseCounter          = 1; /**< Counter for mouse display/hiding. */
static unsigned int input_mouseClickLast = 0; /**< Time of last click (in ms) */
static void *input_lastClicked         = NULL; /**< Pointer to the last-clicked item. */

/*
 * from player.c
 */
extern double player_left;  /**< player.c */
extern double player_right; /**< player.c */

/*
 * Prototypes.
 */
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
      input_setKeybind( "accel", KEYBIND_KEYBOARD, SDLK_w, NMOD_ANY );
      input_setKeybind( "left", KEYBIND_KEYBOARD, SDLK_a, NMOD_ANY );
      input_setKeybind( "right", KEYBIND_KEYBOARD, SDLK_d, NMOD_ANY );
      input_setKeybind( "reverse", KEYBIND_KEYBOARD, SDLK_s, NMOD_ANY );
   }
   else {
      input_setKeybind( "accel", KEYBIND_KEYBOARD, SDLK_UP, NMOD_ANY );
      input_setKeybind( "left", KEYBIND_KEYBOARD, SDLK_LEFT, NMOD_ANY );
      input_setKeybind( "right", KEYBIND_KEYBOARD, SDLK_RIGHT, NMOD_ANY );
      input_setKeybind( "reverse", KEYBIND_KEYBOARD, SDLK_DOWN, NMOD_ANY );
   }
   input_setKeybind( "stealth", KEYBIND_KEYBOARD, SDLK_f, NMOD_NONE );

   /* Targeting */
   if (wasd) {
      input_setKeybind( "target_next", KEYBIND_KEYBOARD, SDLK_e, NMOD_CTRL );
      input_setKeybind( "target_prev", KEYBIND_KEYBOARD, SDLK_q, NMOD_CTRL );
      input_setKeybind( "target_nearest", KEYBIND_KEYBOARD, SDLK_t, NMOD_ANY );
      input_setKeybind( "target_nextHostile", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
      input_setKeybind( "target_prevHostile", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
      input_setKeybind( "target_hostile", KEYBIND_KEYBOARD, SDLK_r, NMOD_ANY );
      input_setKeybind( "target_clear", KEYBIND_KEYBOARD, SDLK_c, NMOD_ANY );
   }
   else {
      input_setKeybind( "target_next", KEYBIND_KEYBOARD, SDLK_t, NMOD_NONE );
      input_setKeybind( "target_prev", KEYBIND_KEYBOARD, SDLK_t, NMOD_CTRL );
      input_setKeybind( "target_nearest", KEYBIND_KEYBOARD, SDLK_n, NMOD_NONE );
      input_setKeybind( "target_nextHostile", KEYBIND_KEYBOARD, SDLK_r, NMOD_CTRL );
      input_setKeybind( "target_prevHostile", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
      input_setKeybind( "target_hostile", KEYBIND_KEYBOARD, SDLK_r, NMOD_NONE );
      input_setKeybind( "target_clear", KEYBIND_KEYBOARD, SDLK_BACKSPACE, NMOD_ANY );
   }

   /* Combat */
   input_setKeybind( "primary", KEYBIND_KEYBOARD, SDLK_SPACE, NMOD_ANY );

   if (wasd)
      input_setKeybind( "face", KEYBIND_KEYBOARD, SDLK_q, NMOD_NONE );
   else
      input_setKeybind( "face", KEYBIND_KEYBOARD, SDLK_a, NMOD_ANY );

   /* Secondary Weapons */
   input_setKeybind( "secondary", KEYBIND_KEYBOARD, SDLK_LSHIFT, NMOD_ANY );
   input_setKeybind( "weapset1", KEYBIND_KEYBOARD, SDLK_1, NMOD_ANY );
   input_setKeybind( "weapset2", KEYBIND_KEYBOARD, SDLK_2, NMOD_ANY );
   input_setKeybind( "weapset3", KEYBIND_KEYBOARD, SDLK_3, NMOD_ANY );
   input_setKeybind( "weapset4", KEYBIND_KEYBOARD, SDLK_4, NMOD_ANY );
   input_setKeybind( "weapset5", KEYBIND_KEYBOARD, SDLK_5, NMOD_ANY );
   input_setKeybind( "weapset6", KEYBIND_KEYBOARD, SDLK_6, NMOD_ANY );
   input_setKeybind( "weapset7", KEYBIND_KEYBOARD, SDLK_7, NMOD_ANY );
   input_setKeybind( "weapset8", KEYBIND_KEYBOARD, SDLK_8, NMOD_ANY );
   input_setKeybind( "weapset9", KEYBIND_KEYBOARD, SDLK_9, NMOD_ANY );
   input_setKeybind( "weapset0", KEYBIND_KEYBOARD, SDLK_0, NMOD_ANY );
   /* Escorts */
   input_setKeybind( "e_targetNext", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
   input_setKeybind( "e_targetPrev", KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
   input_setKeybind( "e_attack", KEYBIND_KEYBOARD, SDLK_END, NMOD_ANY );
   input_setKeybind( "e_hold", KEYBIND_KEYBOARD, SDLK_INSERT, NMOD_ANY );
   input_setKeybind( "e_return", KEYBIND_KEYBOARD, SDLK_DELETE, NMOD_ANY );
   input_setKeybind( "e_clear", KEYBIND_KEYBOARD, SDLK_HOME, NMOD_ANY );
   /* Space Navigation */
   input_setKeybind( "autonav", KEYBIND_KEYBOARD, SDLK_j, NMOD_CTRL );
   input_setKeybind( "target_spob", KEYBIND_KEYBOARD, SDLK_p, NMOD_NONE );
   input_setKeybind( "approach", KEYBIND_KEYBOARD, SDLK_l, NMOD_NONE );
   input_setKeybind( "thyperspace", KEYBIND_KEYBOARD, SDLK_h, NMOD_NONE );
   input_setKeybind( "starmap", KEYBIND_KEYBOARD, SDLK_m, NMOD_NONE );
   input_setKeybind( "jump", KEYBIND_KEYBOARD, SDLK_j, NMOD_NONE );
   input_setKeybind( "overlay", KEYBIND_KEYBOARD, SDLK_TAB, NMOD_ANY );
   input_setKeybind( "mousefly", KEYBIND_KEYBOARD, SDLK_x, NMOD_CTRL );
   input_setKeybind( "cooldown", KEYBIND_KEYBOARD, SDLK_s, NMOD_CTRL );
   /* Communication */
   input_setKeybind( "log_up", KEYBIND_KEYBOARD, SDLK_PAGEUP, NMOD_ANY );
   input_setKeybind( "log_down", KEYBIND_KEYBOARD, SDLK_PAGEDOWN, NMOD_ANY );
   input_setKeybind( "hail", KEYBIND_KEYBOARD, SDLK_y, NMOD_NONE );
   input_setKeybind( "autohail", KEYBIND_KEYBOARD, SDLK_y, NMOD_CTRL );
   /* Misc. */
   input_setKeybind( "mapzoomin", KEYBIND_KEYBOARD, SDLK_KP_PLUS, NMOD_ANY );
   input_setKeybind( "mapzoomout", KEYBIND_KEYBOARD, SDLK_KP_MINUS, NMOD_ANY );
   input_setKeybind( "screenshot", KEYBIND_KEYBOARD, SDLK_KP_MULTIPLY, NMOD_ANY );
   input_setKeybind( "togglefullscreen", KEYBIND_KEYBOARD, SDLK_F11, NMOD_ANY );
   input_setKeybind( "pause", KEYBIND_KEYBOARD, SDLK_PAUSE, NMOD_ANY );

   input_setKeybind( "speed", KEYBIND_KEYBOARD, SDLK_BACKQUOTE, NMOD_ANY );
   input_setKeybind( "menu", KEYBIND_KEYBOARD, SDLK_ESCAPE, NMOD_ANY );
   input_setKeybind( "info", KEYBIND_KEYBOARD, SDLK_i, NMOD_NONE );
   input_setKeybind( "console", KEYBIND_KEYBOARD, SDLK_F2, NMOD_ANY );
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
   input_setKeybind( "paste", KEYBIND_KEYBOARD, SDLK_v, NMOD_CTRL );
}

/**
 * @brief Initializes the input subsystem (does not set keys).
 */
void input_init (void)
{
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

   /* Window. */
   SDL_EventState( SDL_WINDOWEVENT,     SDL_ENABLE );

   /* Keyboard. */
   SDL_EventState( SDL_TEXTINPUT,       SDL_DISABLE); /* Enabled on a per-widget basis. */

   /* Mouse. */
   SDL_EventState( SDL_MOUSEWHEEL,      SDL_ENABLE );

   input_keybinds = malloc( input_numbinds * sizeof(Keybind) );

   /* Create safe null keybinding for each. */
   for (int i=0; i<input_numbinds; i++) {
      Keybind *temp     = &input_keybinds[i];
      memset( temp, 0, sizeof(Keybind) );
      temp->name        = keybind_info[i][0];
      temp->type        = KEYBIND_NULL;
      temp->key         = SDLK_UNKNOWN;
      temp->mod         = NMOD_NONE;

      if (strcmp(temp->name,"paste")==0)
         input_paste = temp;
   }
}

/**
 * @brief exits the input subsystem.
 */
void input_exit (void)
{
   free(input_keybinds);
}

/**
 * @brief Enables all the keybinds.
 */
void input_enableAll (void)
{
   for (int i=0; keybind_info[i][0] != NULL; i++)
      input_keybinds[i].disabled = 0;
}

/**
 * @brief Disables all the keybinds.
 */
void input_disableAll (void)
{
   for (int i=0; keybind_info[i][0] != NULL; i++)
      input_keybinds[i].disabled = 1;
}

/**
 * @brief Enables or disables a keybind.
 */
void input_toggleEnable( const char *key, int enable )
{
   for (int i=0; i<input_numbinds; i++) {
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
      input_mouseTimer = MIN( input_mouseTimer, conf.mouse_hide );
      input_mouseCounter = 0;
   }
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
   if (k == SDLK_UNKNOWN)
      WARN(_("Keyname '%s' doesn't match any key."), name);

   return k;
}

/**
 * @brief Binds key of type type to action keybind.
 *
 *    @param keybind The name of the keybind defined above.
 *    @param type The type of the keybind.
 *    @param key The key to bind to.
 *    @param mod Modifiers to check for.
 */
void input_setKeybind( const char *keybind, KeybindType type, SDL_Keycode key, SDL_Keymod mod )
{
   for (int i=0; i<input_numbinds; i++) {
      if (strcmp(keybind, input_keybinds[i].name)==0) {
         input_keybinds[i].type = type;
         input_keybinds[i].key = key;
         /* Non-keyboards get mod NMOD_ANY to always match. */
         input_keybinds[i].mod = (type==KEYBIND_KEYBOARD) ? mod : NMOD_ANY;
         return;
      }
   }
   WARN(_("Unable to set keybinding '%s', that command doesn't exist"), keybind);
}

/**
 * @brief Gets the value of a keybind.
 *
 *    @param[in] keybind Name of the keybinding to get.
 *    @param[out] type Stores the type of the keybinding.
 *    @param[out] mod Stores the modifiers used with the keybinding.
 *    @return The key associated with the keybinding.
 */
SDL_Keycode input_getKeybind( const char *keybind, KeybindType *type, SDL_Keymod *mod )
{
   for (int i=0; i<input_numbinds; i++) {
      if (strcmp(keybind, input_keybinds[i].name)==0) {
         if (type != NULL)
            (*type) = input_keybinds[i].type;
         if (mod != NULL)
            (*mod) = input_keybinds[i].mod;
         return input_keybinds[i].key;
      }
   }
   WARN(_("Unable to get keybinding '%s', that command doesn't exist"), keybind);
   return (SDL_Keycode)-1;
}

/**
 * @brief Gets the display name (translated and human-readable) of a keybind
 *
 *    @param[in] keybind Name of the keybinding to get display name of.
 *    @param[out] buf Buffer to write the display name to.
 *    @param[in] len Length of buffer.
 */
void input_getKeybindDisplay( const char *keybind, char *buf, int len )
{
   /* Get the keybinding. */
   KeybindType type  = KEYBIND_NULL;
   SDL_Keymod mod    = NMOD_NONE;
   SDL_Keycode key   = input_getKeybind( keybind, &type, &mod );

   /* Handle type. */
   switch (type) {
      case KEYBIND_NULL:
         strncpy( buf, _("Not bound"), len );
         break;

      case KEYBIND_KEYBOARD:
      {
         int p = 0;
         /* Handle mod. */
         if ((mod != NMOD_NONE) && (mod != NMOD_ANY))
            p += scnprintf( &buf[p], len-p, "%s + ", input_modToText(mod) );
         /* Print key. Special-case ASCII letters (use uppercase, unlike SDL_GetKeyName.). */
         if (key < 0x100 && isalpha(key))
            p += scnprintf( &buf[p], len-p, "%c", toupper(key) );
         else
            p += scnprintf( &buf[p], len-p, "%s", pgettext_var("keyname", SDL_GetKeyName(key)) );
         break;
      }

      case KEYBIND_JBUTTON:
         snprintf( buf, len, _("joy button %d"), key );
         break;

      case KEYBIND_JHAT_UP:
         snprintf( buf, len, _("joy hat %d up"), key );
         break;

      case KEYBIND_JHAT_DOWN:
         snprintf( buf, len, _("joy hat %d down"), key );
         break;

      case KEYBIND_JHAT_LEFT:
         snprintf( buf, len, _("joy hat %d left"), key );
         break;

      case KEYBIND_JHAT_RIGHT:
         snprintf( buf, len, _("joy hat %d right"), key );
         break;

      case KEYBIND_JAXISPOS:
         snprintf( buf, len, _("joy axis %d-"), key );
         break;

      case KEYBIND_JAXISNEG:
         snprintf( buf, len, _("joy axis %d+"), key );
         break;
   }
}

/**
 * @brief Gets the human readable version of mod.
 *
 *    @param mod Mod to get human readable version from.
 *    @return Human readable version of mod.
 */
const char* input_modToText( SDL_Keymod mod )
{
   switch ((int)mod) {
      case NMOD_NONE:   return _("None");
      case NMOD_CTRL:   return _("Ctrl");
      case NMOD_SHIFT:  return _("Shift");
      case NMOD_ALT:    return _("Alt");
      case NMOD_META:   return _("Meta");
      case NMOD_ANY:    return _("Any");
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
const char *input_keyAlreadyBound( KeybindType type, SDL_Keycode key, SDL_Keymod mod )
{
   for (int i=0; i<input_numbinds; i++) {
      Keybind *k = &input_keybinds[i];

      /* Type must match. */
      if (k->type != type)
         continue;

      /* Must match key. */
      if (key !=  k->key)
         continue;

      /* Handle per case. */
      switch (type) {
         case KEYBIND_KEYBOARD:
            if ((k->mod == NMOD_ANY) || (mod == NMOD_ANY) ||
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
   for (int i=0; keybind_info[i][0] != NULL; i++)
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
SDL_Keymod input_translateMod( SDL_Keymod mod )
{
   SDL_Keymod mod_filtered = 0;
   if (mod & (KMOD_LSHIFT | KMOD_RSHIFT))
      mod_filtered |= NMOD_SHIFT;
   if (mod & (KMOD_LCTRL | KMOD_RCTRL))
      mod_filtered |= NMOD_CTRL;
   if (mod & (KMOD_LALT | KMOD_RALT))
      mod_filtered |= NMOD_ALT;
   if (mod & (KMOD_LGUI | KMOD_RGUI))
      mod_filtered |= NMOD_META;
   return mod_filtered;
}

/**
 * @brief Handles key repeating.
 */
void input_update( double dt )
{
   if (input_mouseTimer > 0.) {
      input_mouseTimer -= dt;

      /* Hide if necessary. */
      if ((input_mouseTimer < 0.) && (input_mouseCounter <= 0))
         SDL_ShowCursor( SDL_DISABLE );
   }

   /* Key repeat if applicable. */
   if (conf.repeat_delay != 0) {
      unsigned int t;

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
#define INGAME()  (!toolkit_isOpen() && ((value==KEY_RELEASE) || !player_isFlag(PLAYER_CINEMATICS))) /**< Makes sure player is in game. */
#define NOHYP()   \
   ((player.p != NULL) && !pilot_isFlag(player.p,PILOT_HYP_PREP) &&\
   !pilot_isFlag(player.p,PILOT_HYP_BEGIN) &&\
   !pilot_isFlag(player.p,PILOT_HYPERSPACE)) /**< Make sure the player isn't jumping. */
#define NODEAD()  ((player.p != NULL) && !pilot_isFlag(player.p,PILOT_DEAD)) /**< Player isn't dead. */
#define NOLAND()  ((player.p != NULL) && (!landed && !pilot_isFlag(player.p,PILOT_LANDING))) /**< Player isn't landed. */
/**
 * @brief Runs the input command.
 *
 *    @param keynum The index of the keybind.
 *    @param value The value of the keypress (defined above).
 *    @param kabs The absolute value.
 *    @param repeat Whether the key is still held down, rather than newly pressed.
 */
static void input_key( int keynum, double value, double kabs, int repeat )
{
   HookParam hparam[3];
   int isdoubletap = 0;

   /* Repetition stuff. */
   if (conf.repeat_delay != 0) {
      if ((value==KEY_PRESS) && !repeat) {
         repeat_key        = keynum;
         repeat_keyTimer   = SDL_GetTicks();
         repeat_keyCounter = 0;
      }
      else if (value==KEY_RELEASE) {
         repeat_key        = -1;
         repeat_keyTimer   = 0;
         repeat_keyCounter = 0;
      }
   }

   /* Detect if double tap. */
   if (value==KEY_PRESS) {
      unsigned int t = SDL_GetTicks();
      if ((keynum == doubletap_key) && (t-doubletap_t <= conf.doubletap_sens))
         isdoubletap = 1;
      else {
         doubletap_key = keynum;
         doubletap_t = t;
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
      }
      else { /* prevent it from getting stuck */
         if (isdoubletap) {
            if (NODEAD()) {
               pilot_outfitLOnkeydoubletap( player.p, OUTFIT_KEY_ACCEL );
               pilot_afterburn( player.p );
            }
         }
         else if (value==KEY_RELEASE) {
            if (NODEAD()) {
               pilot_outfitLOnkeyrelease( player.p, OUTFIT_KEY_ACCEL );
               pilot_afterburnOver( player.p );
            }
         }

         if (value==KEY_PRESS) {
            player_restoreControl( PINPUT_MOVEMENT, NULL );
            player_setFlag(PLAYER_ACCEL);
            player_accel(1.);
         }
         else if (value==KEY_RELEASE) {
            player_rmFlag(PLAYER_ACCEL);
            if (!player_isFlag(PLAYER_REVERSE))
               player_accelOver();
         }
      }

   /* turning left */
   } else if (KEY("left") && !repeat) {
      if (isdoubletap) {
         if (NODEAD())
            pilot_outfitLOnkeydoubletap( player.p, OUTFIT_KEY_LEFT );
      }
      else if (value==KEY_RELEASE) {
         if (NODEAD())
            pilot_outfitLOnkeyrelease( player.p, OUTFIT_KEY_LEFT );
      }

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
      if (isdoubletap) {
         if (NODEAD())
            pilot_outfitLOnkeydoubletap( player.p, OUTFIT_KEY_RIGHT );
      }
      else if (value==KEY_RELEASE) {
         if (NODEAD())
            pilot_outfitLOnkeyrelease( player.p, OUTFIT_KEY_RIGHT );
      }

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

      /* Double tap reverse = cooldown! */
      if (isdoubletap)
         player_cooldownBrake();

   /* try to enter stealth mode. */
   } else if (KEY("stealth") && !repeat && NOHYP() && NODEAD() && INGAME()) {
      if (value==KEY_PRESS)
         player_stealth();

   /* face the target */
   } else if (KEY("face") && !repeat) {
      if (value==KEY_PRESS) {
         player_restoreControl( PINPUT_MOVEMENT, NULL );
         player_setFlag(PLAYER_FACE);
      }
      else if ((value==KEY_RELEASE) && player_isFlag(PLAYER_FACE))
         player_rmFlag(PLAYER_FACE);

   /*
    * Combat
    */
   /* shooting primary weapon */
   } else if (KEY("primary") && !repeat) {
      if (value==KEY_PRESS) {
         player_setFlag(PLAYER_PRIMARY);
      }
      else if (value==KEY_RELEASE)
         player_rmFlag(PLAYER_PRIMARY);
   /* targeting */
   } else if ((INGAME() || map_isOpen()) && NODEAD() && KEY("target_next")) {
      if (value==KEY_PRESS) {
         if (map_isOpen())
            map_cycleMissions(1);
         else
            player_targetNext(0);
      }
   } else if ((INGAME() || map_isOpen()) && NODEAD() && KEY("target_prev")) {
      if (value==KEY_PRESS) {
         if (map_isOpen())
            map_cycleMissions(-1);
         else
            player_targetPrev(0);
      }
   } else if ((INGAME() || map_isOpen()) && NODEAD() && KEY("target_nearest")) {
      if (value==KEY_PRESS) {
         if (map_isOpen())
            map_cycleMissions(1);
         else
            player_targetNearest();
      }
   } else if (INGAME() && NODEAD() && KEY("target_nextHostile")) {
      if (value==KEY_PRESS) player_targetNext(1);
   } else if (INGAME() && NODEAD() && KEY("target_prevHostile")) {
      if (value==KEY_PRESS) player_targetPrev(1);
   } else if (INGAME() && NODEAD() && KEY("target_hostile")) {
      if (value==KEY_PRESS) player_targetHostile();
   } else if (INGAME() && NODEAD() && KEY("target_clear")) {
      if (value==KEY_PRESS) player_targetClear();

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
   } else if (KEY("secondary") && !repeat) {
      if (value==KEY_PRESS) {
         player_setFlag(PLAYER_SECONDARY);
      }
      else if (value==KEY_RELEASE)
         player_rmFlag(PLAYER_SECONDARY);

   /* Weapon sets. */
   } else if (NODEAD() && KEY("weapset1")) {
      player_weapSetPress( 0, value, repeat );
   } else if (NODEAD() && KEY("weapset2")) {
      player_weapSetPress( 1, value, repeat );
   } else if (NODEAD() && KEY("weapset3")) {
      player_weapSetPress( 2, value, repeat );
   } else if (NODEAD() && KEY("weapset4")) {
      player_weapSetPress( 3, value, repeat );
   } else if (NODEAD() && KEY("weapset5")) {
      player_weapSetPress( 4, value, repeat );
   } else if (NODEAD() && KEY("weapset6")) {
      player_weapSetPress( 5, value, repeat );
   } else if (NODEAD() && KEY("weapset7")) {
      player_weapSetPress( 6, value, repeat );
   } else if (NODEAD() && KEY("weapset8")) {
      player_weapSetPress( 7, value, repeat );
   } else if (NODEAD() && KEY("weapset9")) {
      player_weapSetPress( 8, value, repeat );
   } else if (NODEAD() && KEY("weapset0")) {
      player_weapSetPress( 9, value, repeat );

   /*
    * Space
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
   /* target spob (cycles like target) */
   } else if (KEY("target_spob") && INGAME() && NOHYP() && NOLAND() && NODEAD()) {
      if (value==KEY_PRESS) player_targetSpob();
   /* target nearest spob or attempt to land */
   } else if (KEY("approach") && INGAME() && NOHYP() && NOLAND() && NODEAD() && !repeat) {
      if (value==KEY_PRESS) {
         player_restoreControl( 0, NULL );
         /* Only do noisy if there is no land target. */
         int board_status = player_tryBoard( player.p->nav_spob<0 );
         if (board_status == PLAYER_BOARD_RETRY)
            player_board();
         else {
            if (player.p->nav_spob != -1) {
               if (player_land(0) == PLAYER_LAND_AGAIN) {
                  player_autonavSpob( cur_system->spobs[player.p->nav_spob]->name, 1 );
               }
            } else
               player_land(1);
         }
      }
   } else if (KEY("thyperspace") && NOHYP() && NOLAND() && NODEAD()) {
      if (value==KEY_PRESS) player_targetHyperspace();
   } else if (KEY("starmap") && NOHYP() && NODEAD() && !repeat) {
      if (value==KEY_PRESS) map_open();
   } else if (KEY("jump") && INGAME() && !repeat) {
      if (value==KEY_PRESS) {
         player_restoreControl( 0, NULL );
         player_jump();
      }
   } else if (KEY("overlay") && NODEAD() && (INGAME() || map_isOpen()) && !repeat) {
      if (map_isOpen())
         map_toggleNotes();
      else
         ovr_key( value );
   } else if (KEY("mousefly") && NODEAD() && !repeat) {
      if (value==KEY_PRESS)
         player_toggleMouseFly();
   } else if (KEY("cooldown") && NOHYP() && NOLAND() && NODEAD() && !repeat) {
      if (value==KEY_PRESS) {
         player_restoreControl( PINPUT_BRAKING, NULL );
         player_cooldownBrake();
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
   /* toggle fullscreen */
   } else if (KEY("togglefullscreen") && !repeat) {
      if (value==KEY_PRESS) naev_toggleFullscreen();
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
         if (player.speed < 4.) {
            player.speed *= 2.;
         } else {
            player.speed = 1.;
         }
         player_resetSpeed();
      }
   /* opens a small menu */
   } else if (KEY("menu") && NODEAD() && !repeat) {
      if (value==KEY_PRESS) menu_small( 1, 1, 1, 1 );

   /* shows pilot information */
   } else if (KEY("info") && NOHYP() && NODEAD() && !repeat) {
      if (value==KEY_PRESS) menu_info( INFO_DEFAULT );

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
static void input_joyaxis( const SDL_Keycode axis, const int value );
static void input_joyevent( const int event, const SDL_Keycode button );
static void input_keyevent( const int event, const SDL_Keycode key, const SDL_Keymod mod, const int repeat );

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
   for (int i=0; i<input_numbinds; i++) {
      if (input_keybinds[i].key == axis) {
         /* Positive axis keybinding. */
         if ((input_keybinds[i].type == KEYBIND_JAXISPOS)
               && (value >= 0)) {
            int k = (value > 0) ? KEY_PRESS : KEY_RELEASE;
            if ((k == KEY_PRESS) && input_keybinds[i].disabled)
               continue;
            input_key( i, k, FABS(((double)value)/32767.), 0 );
         }

         /* Negative axis keybinding. */
         if ((input_keybinds[i].type == KEYBIND_JAXISNEG)
               && (value <= 0)) {
            int k = (value < 0) ? KEY_PRESS : KEY_RELEASE;
            if ((k == KEY_PRESS) && input_keybinds[i].disabled)
               continue;
            input_key( i, k, FABS(((double)value)/32767.), 0 );
         }
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
   for (int i=0; i<input_numbinds; i++) {
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
   for (int i=0; i<input_numbinds; i++) {
      if (input_keybinds[i].key != hat)
         continue;

      if (input_keybinds[i].type == KEYBIND_JHAT_UP) {
         int event = (value & SDL_HAT_UP) ? KEY_PRESS : KEY_RELEASE;
         if (!((event == KEY_PRESS) && input_keybinds[i].disabled))
            input_key(i, event, -1., 0);
      } else if (input_keybinds[i].type == KEYBIND_JHAT_DOWN) {
         int event = (value & SDL_HAT_DOWN) ? KEY_PRESS : KEY_RELEASE;
         if (!((event == KEY_PRESS) && input_keybinds[i].disabled))
            input_key(i, event, -1., 0);
      } else if (input_keybinds[i].type == KEYBIND_JHAT_LEFT) {
         int event = (value & SDL_HAT_LEFT) ? KEY_PRESS : KEY_RELEASE;
         if (!((event == KEY_PRESS) && input_keybinds[i].disabled))
            input_key(i, event, -1., 0);
      } else if (input_keybinds[i].type == KEYBIND_JHAT_RIGHT) {
         int event = (value & SDL_HAT_RIGHT) ? KEY_PRESS : KEY_RELEASE;
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
 *    @param repeat Whether the key is still held down, rather than newly pressed.
 */
static void input_keyevent( const int event, SDL_Keycode key, const SDL_Keymod mod, const int repeat )
{
   /* Filter to "Naev" modifiers. */
   SDL_Keymod mod_filtered = input_translateMod(mod);
   for (int i=0; i<input_numbinds; i++) {
      if ((event == KEY_PRESS) && input_keybinds[i].disabled)
         continue;
      if ((input_keybinds[i].type == KEYBIND_KEYBOARD) &&
            (input_keybinds[i].key == key)) {
         if ((input_keybinds[i].mod == mod_filtered) ||
               (input_keybinds[i].mod == NMOD_ANY) ||
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
      cam_setZoomTarget( cam_getZoomTarget() * modifier, conf.zoom_speed );
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
   int mx, my;
   int res;
   double x, y, zoom;
   HookParam hparam[3];

   /* Generate hook. */
   hparam[0].type    = HOOK_PARAM_NUMBER;
   hparam[0].u.num   = event->button.button;
   hparam[1].type    = HOOK_PARAM_BOOL;
   hparam[1].u.b     = (event->type == SDL_MOUSEBUTTONDOWN);
   hparam[2].type    = HOOK_PARAM_SENTINEL;
   hooks_runParam( "mouse", hparam );

   /* Disable in cinematics. */
   if (player_isFlag(PLAYER_CINEMATICS))
      return;

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

   if (gui_borderClickEvent( event ))
      return;

   if (gui_radarClickEvent( event ))
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
int input_clickPos( SDL_Event *event, double x, double y, double zoom, double minpr, double minr )
{
   unsigned int pid;
   Pilot *p;
   double r, rp;
   double d, dp;
   int pntid, jpid, astid, fieid;

   /* Don't allow selecting a new target with the right mouse button
    * (prevents pilots from getting in the way of autonav). */
   if (event->button.button == SDL_BUTTON_RIGHT) {
      pid = player.p->target;
      p = pilot_get(pid);
      dp = pow2(x - p->solid.pos.x) + pow2(y - p->solid.pos.y);
   } else {
      dp = pilot_getNearestPos( player.p, &pid, x, y, 1 );
      p  = pilot_get(pid);
   }

   d  = system_getClosest( cur_system, &pntid, &jpid, &astid, &fieid, x, y );
   rp = MAX( 1.5 * PILOT_SIZE_APPROX * p->ship->gfx_space->sw / 2 * zoom,  minpr);

   if (pntid >=0) { /* Spob is closer. */
      Spob *pnt = cur_system->spobs[ pntid ];
      r  = MAX( 1.5 * pnt->radius * zoom, minr );
   }
   else if (jpid >= 0) {
      JumpPoint *jp = &cur_system->jumps[ jpid ];
      r  = MAX( 1.5 * jp->radius * zoom, minr );
   }
   else if (astid >= 0) {
      AsteroidAnchor *field = &cur_system->asteroids[fieid];
      Asteroid *ast = &field->asteroids[astid];

      /* Recover the right gfx */
      r = MAX( MAX( ast->gfx->sw * zoom, minr ), ast->gfx->sh * zoom );
   }
   else
      r  = 0.;

   /* Reject pilot if it's too far or a valid spob is closer. */
   if ((dp > pow2(rp)) || ((d < pow2(r)) && (dp >  d)))
      pid = PLAYER_ID;

   if (d > pow2(r)) /* Spob or jump point is too far. */
      jpid = pntid = astid = fieid =  -1;

   /* Target a pilot, spob or jump, and/or perform an appropriate action. */
   if (event->button.button == SDL_BUTTON_LEFT) {
      if (pid != PLAYER_ID) {
         return input_clickedPilot(pid, 0);
      }
      else if (pntid >= 0) { /* Spob is closest. */
         return input_clickedSpob(pntid, 0);
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
      if ((pid != PLAYER_ID) && input_clickedPilot(pid, 1))
         return 1;
      else if ((pntid >= 0) && input_clickedSpob(pntid, 1))
         return 1;
      else if ((jpid >= 0) && input_clickedJump(jpid, 1))
         return 1;

      /* Go to position, if the position is >= 1500 px away. */
      if ((pow2(x - player.p->solid.pos.x) + pow2(y - player.p->solid.pos.y))
            >= pow2(1500))

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
   JumpPoint *jp = &cur_system->jumps[ jump ];

   if (!jp_isUsable(jp))
      return 0;

   if (autonav)
      return 0;

   if (player.p->nav_hyperspace != jump)
      map_select( jp->target, 0 );

   if ((jump == player.p->nav_hyperspace) && input_isDoubleClick( (void*)jp )) {
      player_targetHyperspaceSet( jump, 0 );
      if (space_canHyperspace(player.p))
         player_jump();
      else
         player_autonavStart();
      return 1;
   }
   else
      player_targetHyperspaceSet( jump, 0 );

   input_clicked( (void*)jp );
   return 0;
}

/**
 * @brief Performs an appropriate action when a spob is clicked.
 *
 *    @param spob Index of the spob.
 *    @param autonav Whether to autonav to the target.
 *    @return Whether the click was used.
 */
int input_clickedSpob( int spob, int autonav )
{
   Spob *pnt = cur_system->spobs[ spob ];

   if (!spob_isKnown(pnt))
      return 0;

   if (autonav) {
      player_targetSpobSet(spob);
      player_autonavSpob(pnt->name, 0);
      return 1;
   }

   if (spob == player.p->nav_spob && input_isDoubleClick((void*)pnt)) {
      player_hyperspacePreempt(0);
      spob_updateLand( pnt );
      if (!spob_isFlag(pnt, SPOB_SERVICE_INHABITED) || pnt->can_land ||
            (pnt->land_override > 0)) {
         int ret = player_land(0);
         if (ret == PLAYER_LAND_AGAIN) {
            player_autonavSpob(pnt->name, 1);
         }
         else if (ret == PLAYER_LAND_DENIED) {
            player_autonavSpob(pnt->name, 0);
         }
      }
      else
         player_hailSpob();
   }
   else
      player_targetSpobSet( spob );

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
   AsteroidAnchor *anchor = &cur_system->asteroids[ field ];
   Asteroid *ast = &anchor->asteroids[ asteroid ];
   player_targetAsteroidSet( field, asteroid );
   input_clicked( (void*)ast );
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
   Pilot *p;

   if (pilot == PLAYER_ID)
      return 0;

   if (autonav) {
      player_targetSet( pilot );
      player_autonavPil( pilot );
      return 1;
   }

   p = pilot_get(pilot);
   if (pilot == player.p->target && input_isDoubleClick( (void*)p )) {
      if (pilot_isDisabled(p) || pilot_isFlag(p, PILOT_BOARDABLE)) {
         if (player_tryBoard(0)==PLAYER_BOARD_RETRY)
            player_board();
      }
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
   int ismouse;

   /* Special case mouse stuff. */
   if ((event->type == SDL_MOUSEMOTION)  ||
         (event->type == SDL_MOUSEBUTTONDOWN) ||
         (event->type == SDL_MOUSEBUTTONUP)) {
      input_mouseTimer = conf.mouse_hide;
      SDL_ShowCursor( SDL_ENABLE );
      ismouse = 1;
   }
   else
      ismouse = 0;

   /* Special case paste. */
   if (event->type == SDL_KEYDOWN && SDL_HasClipboardText() &&
         SDL_EventState( SDL_TEXTINPUT, SDL_QUERY )==SDL_ENABLE) {
      SDL_Keymod mod = input_translateMod( event->key.keysym.mod );
      if ((input_paste->key == event->key.keysym.sym) &&
            (input_paste->mod & mod)) {
         SDL_Event evt;
         char *txt = SDL_GetClipboardText();
         evt.type = SDL_TEXTINPUT;
         size_t i = 0;
         uint32_t ch;
         while ((ch = u8_nextchar( txt, &i ))) {
            size_t e = u8_wc_toutf8( evt.text.text, ch );
            evt.text.text[e] = '\0';
            SDL_PushEvent( &evt );
         }
         return;
      }
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
         if (event->key.repeat != 0)
            return;
         input_keyevent(KEY_PRESS, event->key.keysym.sym, event->key.keysym.mod, 0);
         break;

      case SDL_KEYUP:
         if (event->key.repeat != 0)
            return;
         input_keyevent(KEY_RELEASE, event->key.keysym.sym, event->key.keysym.mod, 0);
         break;

      /* Mouse stuff. */
      case SDL_MOUSEBUTTONDOWN:
         input_clickevent( event );
         break;

      case SDL_MOUSEWHEEL:
         if (event->wheel.y > 0)
            input_clickZoom( 1.1 );
         else if (event->wheel.y < 0)
            input_clickZoom( 0.9 );
         break;

      case SDL_MOUSEMOTION:
         input_mouseMove( event );
         break;

      default:
         break;
   }
}
