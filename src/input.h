/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef INPUT_H
#  define INPUT_H


#include "SDL.h"


#define KMOD_ALL  0xffff /**< Comfort thing SDL is lacking. */


/* input types */
typedef enum {
   KEYBIND_NULL, /**< Null keybinding. */
   KEYBIND_KEYBOARD, /**< Keyboard keybinding. */
   KEYBIND_JAXIS, /**< Joystick axis keybinding. */
   KEYBIND_JBUTTON /**< Joystick button keybinding. */
} KeybindType; /**< Keybind types. */


/*
 * set input
 */
void input_setDefault (void);
void input_setKeybind( char *keybind, KeybindType type, int key, SDLMod mod, int reverse );
SDLKey input_getKeybind( const char *keybind, KeybindType *type, SDLMod *mod, int *reverse );
const char* input_getKeybindDescription( char *keybind );

/*
 * handle input
 */
void input_handle( SDL_Event* event );

/*
 * init/exit
 */
void input_init (void);
void input_exit (void);


#endif /* INPUT_H */
