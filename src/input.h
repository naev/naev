/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef INPUT_H
#  define INPUT_H


#include "SDL.h"


#define NMOD_NONE    0
#define NMOD_SHIFT   (1<<0)
#define NMOD_CTRL    (1<<1)
#define NMOD_ALT     (1<<2)
#define NMOD_META    (1<<3)
#define NMOD_ALL     0xFFFF /**< Comfort thing SDL is lacking. */


/* input types */
typedef enum {
   KEYBIND_NULL, /**< Null keybinding. */
   KEYBIND_KEYBOARD, /**< Keyboard keybinding. */
   KEYBIND_JAXISPOS, /**< Joystick axis positive side keybinding. */
   KEYBIND_JAXISNEG, /**< Joystick axis negative side keybinding. */
   KEYBIND_JBUTTON /**< Joystick button keybinding. */
} KeybindType; /**< Keybind types. */


/*
 * set input
 */
void input_setDefault (void);
SDLKey input_keyConv( const char *name );
void input_setKeybind( const char *keybind, KeybindType type, SDLKey key, SDLMod mod );
const char* input_modToText( SDLMod mod );
SDLKey input_getKeybind( const char *keybind, KeybindType *type, SDLMod *mod );
const char* input_getKeybindDescription( const char *keybind );
const char *input_keyAlreadyBound( KeybindType type, SDLKey key, SDLMod mod );

/*
 * Misc.
 */
SDLMod input_translateMod( SDLMod mod );
void input_enableAll (void);
void input_disableAll (void);
void input_toggleEnable( const char *key, int enable );

/*
 * handle input
 */
void input_handle( SDL_Event* event );

/*
 * init/exit
 */
void input_init (void);
void input_exit (void);

/*
 * Updating.
 */
void input_update( double dt );


/*
 * Mouse.
 */
void input_mouseShow (void);
void input_mouseHide (void);


#endif /* INPUT_H */
