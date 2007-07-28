

#ifndef PLAYER_H
#  define PLAYER_H


#include "SDL.h"
#include "pilot.h"


/*
 * the player
 */
extern Pilot* player;


/*
 * enums
 */
typedef enum { RADAR_RECT, RADAR_CIRCLE } RadarShape; /* for rendering fucntions */
typedef enum { KEYBIND_NULL, KEYBIND_KEYBOARD, KEYBIND_JAXIS, KEYBIND_JBUTTON } KeybindType;



/*
 * render
 */
int gui_init (void);
void gui_free (void);
void player_render (void);


/*
 * input
 */
void input_init (void);
void input_exit (void);
void input_setDefault (void);
void input_setKeybind( char *keybind, KeybindType type, int key, int reverse );
void input_handle( SDL_Event* event );


/*
 * misc
 */
void player_message ( const char *fmt, ... );


#endif /* PLAYER_H */
