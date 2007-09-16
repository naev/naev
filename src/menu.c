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
#include "pilot.h"
#include "space.h"


#define MENU_WIDTH	120
#define MENU_HEIGHT	200

#define INFO_WIDTH	320
#define INFO_HEIGHT	280

#define BUTTON_WIDTH  80
#define BUTTON_HEIGHT 30


#define MENU_SMALL		(1<<0)
#define MENU_INFO			(1<<1)
#define menu_isOpen(f)	(menu_open & (f))
#define menu_Open(f)		(menu_open |= (f))
#define menu_Close(f)	(menu_open ^= (f))
static int menu_open = 0;


/*
 * prototypes
 */
static void menu_small_close( char* str );
static void edit_options (void);
static void exit_game (void);
static void info_menu_close( char* str );



/*
 *
 * ingame menu
 *
 */
/*
 * the small ingame menu
 */
void menu_small (void)
{
	if (menu_isOpen(MENU_SMALL)) return; /* menu is already open */
	pause();

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

	menu_Open(MENU_SMALL);
}
static void menu_small_close( char* str )
{
	if (strcmp(str,"btnResume")==0)
		window_destroy( window_get("Menu") );

	unpause();
	menu_Close(MENU_SMALL);
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



/*
 * 
 * information menu
 *
 */
void info_menu (void)
{
	if (menu_isOpen(MENU_INFO)) return;
	pause();

	char str[128];
	unsigned int wid;
	wid = window_create( "Info", -1, -1, INFO_WIDTH, INFO_HEIGHT );

	/* pilot generics */
	window_addText( wid, 20, 20, 120, INFO_HEIGHT-60,
			0, "txtDPilot", &gl_smallFont, &cDConsole,
			"Pilot:\n"
			"Combat Rating:\n"
			);
	snprintf( str, 128, 
			"Foobar\n"
			"Luser\n"
			);
	window_addText( wid, 120, 20,
			INFO_WIDTH-120-BUTTON_WIDTH, INFO_HEIGHT-60,
			0, "txtPilot", &gl_smallFont, &cBlack, str );

	/* menu */
	window_addButton( wid, -20, (20 + BUTTON_HEIGHT)*4 + 20,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			player->ship->name, "Ship", ship_view );
	window_addButton( wid, -20, (20 + BUTTON_HEIGHT)*3 + 20,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnOutfits", "Outfts", ship_view );
	window_addButton( wid, -20, (20 + BUTTON_HEIGHT)*2 + 20,      
			BUTTON_WIDTH, BUTTON_HEIGHT,    
			"btnCargo", "Cargo", ship_view );
	window_addButton( wid, -20, 20 + BUTTON_HEIGHT + 20,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnMissions", "Missions", ship_view );
	window_addButton( wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnClose", "Close", info_menu_close );

	menu_Open(MENU_INFO);
}
static void info_menu_close( char* str )
{
	if (strcmp(str,"btnClose")==0)
		window_destroy( window_get("Info") );

	menu_Close(MENU_INFO);
	unpause();
}

