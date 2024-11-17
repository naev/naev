/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "SDL_keycode.h"

#define INFO_MAIN 0      /**< Main info window. */
#define INFO_SHIP 1      /**< Ship info window. */
#define INFO_WEAPONS 2   /**< Weapons info window. */
#define INFO_CARGO 3     /**< Cargo info window. */
#define INFO_MISSIONS 4  /**< Missions info window. */
#define INFO_STANDINGS 5 /**< Standings info window. */
#define INFO_SHIPLOG 6   /**< Ship log info window. */

#define INFO_DEFAULT -1 /**< Default window (only when opening. */

/*
 * Menu opening routines.
 */
void menu_info( int window );
void info_update( void );

/* Custom buttons. */
int  info_buttonRegister( const char *caption, int priority, SDL_Keycode key );
int  info_buttonUnregister( int id );
void info_buttonClear( void );
