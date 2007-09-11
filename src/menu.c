/*
 * See Licensing and Copyright notice in naev.h
 */



#include "menu.h"

#include <string.h>

#include "SDL.h"

#include "toolkit.h"
#include "log.h"
#include "naev.h"
#include "pause.h"


#define MENU_WIDTH	120
#define MENU_HEIGHT	200

#define BUTTON_WIDTH  80
#define BUTTON_HEIGHT 30


static int menu_open = 0;


/*
 * prototypes
 */
static void menu_small_close( char* str );
static void edit_options (void);
static void exit_game (void);


/*
 * the small ingame menu
 */
void menu_small (void)
{
	if (menu_open) return; /* menu is already open */

	unsigned int wid;

	wid = window_create( "Menu", -1, -1, MENU_WIDTH, MENU_HEIGHT );
	
	window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, 
			"btnExit", "Exit", (void(*)(char*)) exit_game );
	window_addButton( wid, 20, 20 + BUTTON_HEIGHT + 20,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnOptions", "Options", (void(*)(char*)) edit_options );
	window_addButton( wid, 20, 20 + BUTTON_HEIGHT*2 + 20*2,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnResume", "Resume", menu_small_close );

	pause();
	menu_open = 1;
}
void menu_small_close( char* str )
{
	if (strcmp(str,"btnResume")==0)
		window_destroy( window_get("Menu") );

	unpause();
	menu_open = 0;
}

/*
 * edits the options
 */
static void edit_options (void)
{
	/* TODO make options menu */
}


/*
 * exits the game
 */
static void exit_game (void)
{
	SDL_Event quit;
	quit.type = SDL_QUIT;
	SDL_PushEvent(&quit);
}
