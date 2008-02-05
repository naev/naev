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
#include "player.h"
#include "plasmaf.h"
#include "mission.h"


#define MAIN_WIDTH		130
#define MAIN_HEIGHT		250

#define MENU_WIDTH		130
#define MENU_HEIGHT		200

#define INFO_WIDTH		360
#define INFO_HEIGHT		280

#define OUTFITS_WIDTH	400
#define OUTFITS_HEIGHT	200

#define MISSIONS_WIDTH  600
#define MISSIONS_HEIGHT 400

#define DEATH_WIDTH		130
#define DEATH_HEIGHT		150

#define BUTTON_WIDTH  	90
#define BUTTON_HEIGHT 	30

#define menu_Open(f)		(menu_open |= (f))
#define menu_Close(f)	(menu_open ^= (f))
int menu_open = 0;


/*
 * prototypes
 */
/* main menu */
static void menu_main_close (void);
static void menu_main_new( char* str );
/* small menu */
static void menu_small_close( char* str );
static void edit_options (void);
static void exit_game (void);
/* information menu */
static void menu_info_close( char* str );
/* outfits submenu */
static void info_outfits_menu( char* str );
/* mission submenu */
static void info_missions_menu( char* str );
static void mission_menu_abort( char* str );
static void mission_menu_genList( int first );
static void mission_menu_update( char* str );
/* death menu */
static void menu_death_main( char* str );
/* generic */
static void menu_generic_close( char* str );



void menu_main (void)
{
	unsigned int bwid, wid;
	glTexture *tex;

	tex = pf_genFractal( gl_screen.w, gl_screen.h, 10. );

	/* create background image window */
	bwid = window_create( "BG", -1, -1, gl_screen.w, gl_screen.h );
	window_addRect( bwid, 0, 0, gl_screen.w, gl_screen.h, "rctBG", &cBlack, 0 );
	window_addImage( bwid, 0, 0, "imgBG", tex, 0 );
	window_imgColour( bwid, "imgBG", &cPurple );

	/* create menu window */
	wid = window_create( "Main Menu", -1, -1, MAIN_WIDTH, MAIN_HEIGHT );
	window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20)*3,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnLoad", "Load Game", NULL );
	window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20)*2,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnNew", "New Game", menu_main_new );
	window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20),
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnOptions", "Options", (void(*)(char*)) edit_options );
	window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnExit", "Exit", (void(*)(char*)) exit_game );

	menu_Open(MENU_MAIN);
}
static void menu_main_close (void)
{
	window_destroy( window_get("Main Menu") );

	gl_freeTexture( window_getImage( window_get("BG"), "imgBG" ) );
	window_destroy( window_get("BG") );

	menu_Close(MENU_MAIN);
}
static void menu_main_new( char* str )
{
	(void)str;

	menu_main_close();
	player_new();
}


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
	if ( menu_isOpen(MENU_MAIN) ||
			menu_isOpen(MENU_SMALL) ||
			menu_isOpen(MENU_DEATH) )
		return; /* menu is already open */
	pause();

	unsigned int wid;
	wid = window_create( "Menu", -1, -1, MENU_WIDTH, MENU_HEIGHT );


	window_addButton( wid, 20, 20 + BUTTON_HEIGHT*2 + 20*2,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnResume", "Resume", menu_small_close );
	window_addButton( wid, 20, 20 + BUTTON_HEIGHT + 20,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnOptions", "Options", (void(*)(char*)) edit_options );
	window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, 
			"btnExit", "Exit", (void(*)(char*)) exit_game );

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
void menu_info (void)
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
			"Combat\n"
			" Rating:\n"
			"\n"
			"Ship:\n"
			);
	snprintf( str, 128, 
			"%s\n"
			"\n"
			"%s\n"
			"\n"
			"%s\n"
			, player_name, player_rating(), player->name );
	window_addText( wid, 80, 20,
			INFO_WIDTH-120-BUTTON_WIDTH, INFO_HEIGHT-60,
			0, "txtPilot", &gl_smallFont, &cBlack, str );

	/* menu */
	window_addButton( wid, -20, (20 + BUTTON_HEIGHT)*4 + 20,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			player->ship->name, "Ship", ship_view );
	window_addButton( wid, -20, (20 + BUTTON_HEIGHT)*3 + 20,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnOutfits", "Outfts", info_outfits_menu );
	window_addButton( wid, -20, (20 + BUTTON_HEIGHT)*2 + 20,      
			BUTTON_WIDTH, BUTTON_HEIGHT,    
			"btnCargo", "Cargo", NULL );
	window_addButton( wid, -20, 20 + BUTTON_HEIGHT + 20,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnMissions", "Missions", info_missions_menu );
	window_addButton( wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnClose", "Close", menu_info_close );

	menu_Open(MENU_INFO);
}
static void menu_info_close( char* str )
{
	if (strcmp(str,"btnClose")==0)
		window_destroy( window_get("Info") );

	menu_Close(MENU_INFO);
	unpause();
}


/*
 * shows the player what outfits he has
 */
static void info_outfits_menu( char* str )
{
	(void)str;
	char *buf;
	unsigned int wid;
	wid = window_create( "Outfits", -1, -1, OUTFITS_WIDTH, OUTFITS_HEIGHT );

	window_addText( wid, 20, -40, 100, OUTFITS_HEIGHT-40,
			0, "txtLabel", &gl_smallFont, &cDConsole,
			"Ship Outfits:" );

	buf = pilot_getOutfits( player );
	window_addText( wid, 20, -45-gl_smallFont.h,
			OUTFITS_WIDTH-40, OUTFITS_HEIGHT-60,
			0, "txtOutfits", &gl_smallFont, &cBlack, buf );
	free(buf);
	
	window_addButton( wid, -20, 20,
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"closeOutfits", "Close", menu_generic_close );
}


/*
 * shows the player's active missions
 */
static void info_missions_menu( char* str )
{
	(void)str;
	unsigned int wid;

	/* create the window */
	wid = window_create( "Missions", -1, -1, MISSIONS_WIDTH, MISSIONS_HEIGHT );

	/* buttons */
	window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
			"closeMissions", "Back", menu_generic_close );
	window_addButton( wid, -20, 40 + BUTTON_HEIGHT,
			BUTTON_WIDTH, BUTTON_HEIGHT, "btnAbortMission", "Abort",
			mission_menu_abort );
	
	/* text */
	window_addText( wid, 300+40, -60,
			200, 40, 0, "txtSReward",
			&gl_smallFont, &cDConsole, "Reward:" );
	window_addText( wid, 300+100, -60,
			140, 40, 0, "txtReward", &gl_smallFont, &cBlack, NULL );
	window_addText( wid, 300+40, -100,
			200, MISSIONS_HEIGHT - BUTTON_WIDTH - 120, 0,
			"txtDesc", &gl_smallFont, &cBlack, NULL );

	/* list */
	mission_menu_genList(1);
}
static void mission_menu_genList( int first )
{
	int i,j;
	char** misn_names;
	unsigned int wid;

	wid = window_get("Missions");

	if (!first)
		window_destroyWidget( wid, "lstMission" );

	/* list */
	misn_names = malloc(sizeof(char*) * MISSION_MAX);
	j = 0;
	for (i=0; i<MISSION_MAX; i++)
		if (player_missions[i].id != 0)
			misn_names[j++] = strdup(player_missions[i].title);
	if (j==0) { /* no missions */
		free(misn_names);
		misn_names = malloc(sizeof(char*));                                 
		misn_names[0] = strdup("No Missions");                              
		j = 1;
	}
	window_addList( wid, 20, -40,
			300, MISSIONS_HEIGHT-60,
			"lstMission", misn_names, j, 0, mission_menu_update );

	mission_menu_update(NULL);
}
static void mission_menu_update( char* str )
{
	char *active_misn;
	Mission* misn;
	unsigned int wid;

	(void)str;

	wid = window_get( "Missions" );

	active_misn = toolkit_getList( wid, "lstMission" );
	if (strcmp(active_misn,"No Missions")==0) {
		window_modifyText( wid, "txtReward", "None" );
		window_modifyText( wid, "txtDesc",
				"You currently have no active missions." );
		window_disableButton( wid, "btnAbortMission" );
		return;
	}

	misn = &player_missions[ toolkit_getListPos(wid, "lstMission" ) ];
	window_modifyText( wid, "txtReward", misn->reward );
	window_modifyText( wid, "txtDesc", misn->desc );
	window_enableButton( wid, "btnAbortMission" );
}
static void mission_menu_abort( char* str )
{
	(void)str;
	char *selected_misn;
	int pos;
	unsigned int wid;
	Mission* misn;

	wid = window_get( "Missions" );

	selected_misn = toolkit_getList( wid, "lstMission" );

	if (dialogue_YesNo( "Abort Mission", 
				"Are you sure you want to abort this mission?" )) {
		pos = toolkit_getListPos(wid, "lstMission" );
		misn = &player_missions[pos];
		mission_cleanup( misn );
		memmove( misn, &player_missions[pos+1], 
				sizeof(Mission) * (MISSION_MAX-pos-1) );
		mission_menu_genList(0);
	}
}



/*
 * pilot died
 */
void menu_death (void)
{
	unsigned int wid;
	
	wid = window_create( "Death", -1, -1, DEATH_WIDTH, DEATH_HEIGHT );
	window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20),
			BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnMain", "MainMenu", menu_death_main );
	window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
			"btnExit", "Exit", (void(*)(char*)) exit_game );
	menu_Open(MENU_DEATH);
}
static void menu_death_main( char* str )
{
	(void)str;
	unsigned int wid;

	wid = window_get( "Death" );
	window_destroy( wid );
	menu_Close(MENU_DEATH);

	menu_main();
}


/*
 * generic close approach
 */
static void menu_generic_close( char* str )
{
	window_destroy( window_get( str+5 /* "closeFoo -> Foo" */ ) );
}
