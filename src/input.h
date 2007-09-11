/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef INPUT_H
#  define INPUT_H


#include "SDL.h"


/* input types */
typedef enum { KEYBIND_NULL, KEYBIND_KEYBOARD, KEYBIND_JAXIS, KEYBIND_JBUTTON } KeybindType;


/*
 * set input
 */
void input_setDefault (void);
void input_setKeybind( char *keybind, KeybindType type, int key, int reverse );

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
