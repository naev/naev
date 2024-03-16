/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/*
 * Menu status.
 */
#define MENU_MAIN ( 1 << 0 )    /**< Main menu (titlescreen). */
#define MENU_SMALL ( 1 << 1 )   /**< Small in-game menu. */
#define MENU_INFO ( 1 << 2 )    /**< Player information menu. */
#define MENU_DEATH ( 1 << 3 )   /**< Player death menu. */
#define MENU_OPTIONS ( 1 << 4 ) /**< Player's options menu. */
#define MENU_ASKQUIT ( 1 << 5 ) /**< Really quit naev? menu. */
#define MENU_EDITORS ( 1 << 6 ) /**< Editors menu. */
#define menu_isOpen( f )                                                       \
   ( menu_open & ( f ) ) /**< Checks if a certain menu is opened. */
extern int menu_open;    /**< Used internally by menu_isOpen() */

/* Menu opening routines. */
void menu_main( void );
void menu_small( int docheck, int info, int options, int allowsave );
void menu_death( void );
int  menu_askQuit( void );

/* Closing. */
void menu_main_close( void );
void menu_small_close( void );

/* Misc. */
void menu_main_resize( void );
