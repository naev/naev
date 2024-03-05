/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include "SDL.h"
/** @endcond */

#define NMOD_NONE    0
#define NMOD_SHIFT   (1<<0)
#define NMOD_CTRL    (1<<1)
#define NMOD_ALT     (1<<2)
#define NMOD_META    (1<<3)
#define NMOD_ANY     0xFFFF /**< Comfort thing SDL is lacking. */

#define KEY_PRESS    ( 1.) /**< Key is pressed. */
#define KEY_RELEASE  (-1.) /**< Key is released. */


/**
 * @brief Naev internal key types
 */
typedef enum KeySemanticType_ {
   KST_ACCEL=0,
   KST_LEFT,
   KST_RIGHT,
   KST_REVERSE,
   KST_FACE,

   KST_STEALTH,
   KST_GAME_SPEED,
   KST_PAUSE,

   KST_AUTONAV,
   KST_APPROACH,
   KST_MOUSE_FLYING,
   KST_JUMP,

   KST_TARGET_NEXT,
   KST_TARGET_PREV,
   KST_TARGET_CLOSE,
   KST_TARGET_SPOB,
   KST_TARGET_JUMP,

   KST_HTARGET_NEXT,
   KST_HTARGET_PREV,
   KST_HTARGET_CLOSE,

   KST_TARGET_CLEAR,

   KST_FIRE_PRIMARY,
   KST_FIRE_SECONDARY,
   KST_INIT_COOLDOWN,

   KST_TAB_1,
   KST_TAB_2,
   KST_TAB_3,
   KST_TAB_4,
   KST_TAB_5,
   KST_TAB_6,
   KST_TAB_7,
   KST_TAB_8,
   KST_TAB_9,
   KST_TAB_0,

   KST_LOCAL_MAP,
   KST_GLOBAL_MAP,

   KST_MENU_SMALL,
   KST_MENU_INFO,
   KST_MENU_LUA,

   KST_ESCORT_NEXT,
   KST_ESCORT_PREV,
   KST_ESCORT_ATTACK,
   KST_ESCORT_HALT,
   KST_ESCORT_RETURN,
   KST_ESCORT_CLEAR,

   KST_COMM_HAIL,
   KST_COMM_RECEIVE,
   KST_COMM_UP,
   KST_COMM_DOWN,

   KST_ZOOM_IN,
   KST_ZOOM_OUT,

   KST_FULLSCREEN,

   KST_SCREENSHOT,

   KST_PASTE,

} KeySemanticType;

/* input types */
typedef enum {
   KEYBIND_NULL, /**< Null keybinding. */
   KEYBIND_KEYBOARD, /**< Keyboard keybinding. */
   KEYBIND_JAXISPOS, /**< Joystick axis positive side keybinding. */
   KEYBIND_JAXISNEG, /**< Joystick axis negative side keybinding. */
   KEYBIND_JBUTTON, /**< Joystick button keybinding. */
   KEYBIND_JHAT_UP, /**< Joystick hat up direction keybinding. */
   KEYBIND_JHAT_DOWN, /**< Joystick hat down direction keybinding. */
   KEYBIND_JHAT_LEFT, /**< Joystick hat left direction keybinding. */
   KEYBIND_JHAT_RIGHT /**< Joystick hat right direction keybinding. */
} KeybindType; /**< Keybind types. */

extern const char *keybind_info[KST_PASTE+1][2];
extern const int   input_numbinds;

/*
 * set input
 */
void input_setDefault( int wasd );
SDL_Keycode input_keyConv( const char *name );
void input_setKeybind( KeySemanticType keybind, KeybindType type, SDL_Keycode key, SDL_Keymod mod );
const char *input_modToText( SDL_Keymod mod );
SDL_Keycode input_getKeybind( KeySemanticType keybind, KeybindType *type, SDL_Keymod *mod );
void input_getKeybindDisplay( KeySemanticType keybind, char *buf, int len );
const char *input_getKeybindDescription( KeySemanticType keybind );
const char *input_keyAlreadyBound( KeybindType type, SDL_Keycode key, SDL_Keymod mod );
KeySemanticType find_key( const char *name );
/*
 * Misc.
 */
SDL_Keymod input_translateMod( SDL_Keymod mod );
void input_enableAll (void);
void input_disableAll (void);
void input_toggleEnable( KeySemanticType key, int enable );
int input_clickPos( SDL_Event *event, double x, double y, double zoom, double minpr, double minr );
int input_clickedJump( int jump, int autonav );
int input_clickedSpob( int spob, int autonav );
int input_clickedAsteroid( int field, int asteroid );
int input_clickedPilot( unsigned int pilot, int autonav );
void input_clicked( void *clicked );
int input_isDoubleClick( void *clicked );

/*
 * handle input
 */
void input_handle( SDL_Event* event );

/*
 * init/exit
 */
void input_init (void);

/*
 * Updating.
 */
void input_update( double dt );

/*
 * Mouse.
 */
void input_mouseShow (void);
void input_mouseHide (void);
