/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file menu.h
 *
 * @brief Handles the important game menus.
 */


#include "menu.h"

#include <string.h>

#include "SDL.h"

#include "toolkit.h"
#include "dialogue.h"
#include "log.h"
#include "naev.h"
#include "pilot.h"
#include "space.h"
#include "player.h"
#include "mission.h"
#include "ntime.h"
#include "save.h"
#include "land.h"
#include "rng.h"
#include "nebulae.h"
#include "pause.h"


#define MAIN_WIDTH      130 /**< Main menu width. */
#define MAIN_HEIGHT     250 /**< Main menu height. */

#define MENU_WIDTH      130 /**< Escape menu width. */
#define MENU_HEIGHT     200 /**< Escape menu height. */

#define INFO_WIDTH      360 /**< Information menu width. */
#define INFO_HEIGHT     280 /**< Information menu height. */

#define OUTFITS_WIDTH   400 /**< Outfit menu width. */
#define OUTFITS_HEIGHT  200 /**< Outfit menu height. */

#define CARGO_WIDTH     300 /**< Cargo menu width. */
#define CARGO_HEIGHT    300 /**< Cargo menu height. */

#define MISSIONS_WIDTH  600 /**< Mission menu width. */
#define MISSIONS_HEIGHT 400 /**< Mission menu height. */

#define DEATH_WIDTH     130 /**< Death menu width. */
#define DEATH_HEIGHT    150 /**< Death menu height. */

#define BUTTON_WIDTH    90 /**< Button width, standard across menus. */
#define BUTTON_HEIGHT   30 /**< Button height, standard across menus. */

#define menu_Open(f)    (menu_open |= (f)) /**< Marks a menu as opened. */
#define menu_Close(f)   (menu_open &= ~(f)) /**< Marks a menu as closed. */
int menu_open = 0; /**< Stores the opened/closed menus. */


/*
 * prototypes
 */
/* main menu */
void menu_main_close (void); /**< Externed in save.c */
static void menu_main_load( unsigned int wid, char* str );
static void menu_main_new( unsigned int wid, char* str );
static void menu_main_exit( unsigned int wid, char* str );
/* small menu */
static void menu_small_close( unsigned int wid, char* str );
static void menu_small_exit( unsigned int wid, char* str );
static void exit_game (void);
/* information menu */
static void menu_info_close( unsigned int wid, char* str );
/* outfits submenu */
static void info_outfits_menu( unsigned int parent, char* str );
/* cargo submenu */
static void info_cargo_menu( unsigned int parent, char* str );
static void cargo_update( unsigned int wid, char* str );
static void cargo_jettison( unsigned int wid, char* str );
/* mission submenu */
static void info_missions_menu( unsigned int parent, char* str );
static void mission_menu_abort( unsigned int wid, char* str );
static void mission_menu_genList( unsigned int wid, int first );
static void mission_menu_update( unsigned int wid, char* str );
/* death menu */
static void menu_death_main( unsigned int wid, char* str );


/**
 * @fn void menu_main (void)
 *
 * @brief Opens the main menu (titlescreen).
 */
void menu_main (void)
{
   unsigned int bwid, wid;
   glTexture *tex;

   tex = gl_newImage( "gfx/NAEV.png" );
   nebu_prep( 300., 0. ); /* Needed for nebulae to not spaz out */

   /* create background image window */
   bwid = window_create( "BG", -1, -1, SCREEN_W, SCREEN_H );
   window_addRect( bwid, 0, 0, SCREEN_W, SCREEN_H, "rctBG", &cBlack, 0 );
   window_addCust( bwid, 0, 0, SCREEN_W, SCREEN_H, "cstBG", 0,
         (void(*)(double,double,double,double)) nebu_render, NULL );
   window_addImage( bwid, (SCREEN_W-tex->sw)/2., -50, "imgLogo", tex, 0 );
   window_addText( bwid, 0., 50, SCREEN_W, 30., 1, "txtBG", NULL,
         &cWhite, naev_version() );

   /* create menu window */
   wid = window_create( "Main Menu", -1, -70-tex->sh,
         MAIN_WIDTH, MAIN_HEIGHT );
   window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20)*3,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLoad", "Load Game", menu_main_load );
   window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20)*2,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnNew", "New Game", menu_main_new );
   window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20),
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnOptions", "Options", NULL );
   window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnExit", "Exit", menu_main_exit );

   menu_Open(MENU_MAIN);
}
/**
 * @fn void menu_main_close (void)
 * @brief Closes the main menu.
 */
void menu_main_close (void)
{
   window_destroy( window_get("Main Menu") );

   gl_freeTexture( window_getImage( window_get("BG"), "imgLogo" ) );
   window_destroy( window_get("BG") );

   menu_Close(MENU_MAIN);
}
/**
 * @fn static void menu_main_load( unsigned int wid, char* str )
 * @brief Function to active the load game menu.
 *    @param str Unused.
 */
static void menu_main_load( unsigned int wid, char* str )
{
   (void) str;
   (void) wid;
   load_game_menu();
}
/**
 * @fn static void menu_main_new( unsigned int wid, char* str )
 * @brief Function to active the new game menu.
 *    @param str Unused.
 */
static void menu_main_new( unsigned int wid, char* str )
{
   (void) str;
   (void) wid;
   menu_main_close();
   player_new();
}
/**
 * @fn static void menu_main_exit( unsigned int wid, char* str )
 * @brief Function to exit the main menu and game.
 *    @param str Unused.
 */
static void menu_main_exit( unsigned int wid, char* str )
{
   (void) str;
   (void) wid;
   unsigned int bg;

   bg = window_get( "BG" );

   /* 
    * Ugly hack to prevent player.c from segfaulting due to the fact
    * that game will attempt to render while waiting for the quit event
    * pushed by exit_game() to be handled without actually having a player
    * nor anything of the likes (nor toolkit to stop rendering) while
    * not leaking any texture.
    */
   gl_freeTexture( window_getImage(bg, "imgLogo") );
   window_modifyImage( bg, "imgLogo", NULL );

   exit_game();
}


/*
 *
 * ingame menu
 *
 */
/**
 * @fnvoid menu_small (void)
 *
 * @brief Opens the small ingame menu.
 */
void menu_small (void)
{
   unsigned int wid;

   /* Check if menu should be openable. */
   if ((player == NULL) || player_isFlag(PLAYER_DESTROYED) ||
         pilot_isFlag(player,PILOT_DEAD) ||
         dialogue_isOpen() || /* Shouldn't open over dialogues. */
         (menu_isOpen(MENU_MAIN) ||
            menu_isOpen(MENU_SMALL) ||
            menu_isOpen(MENU_DEATH) ))
      return;

   wid = window_create( "Menu", -1, -1, MENU_WIDTH, MENU_HEIGHT );

   window_setCancel( wid, menu_small_close );

   window_addButton( wid, 20, 20 + BUTTON_HEIGHT*2 + 20*2,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnResume", "Resume", menu_small_close );
   window_addButton( wid, 20, 20 + BUTTON_HEIGHT + 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnOptions", "Options", NULL );
   window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, 
         "btnExit", "Exit", menu_small_exit );

   menu_Open(MENU_SMALL);
}
/**
 * @fn static void menu_small_close( unsigned int wid, char* str )
 * @brief Closes the small ingame menu.
 *    @param str Unused.
 */
static void menu_small_close( unsigned int wid, char* str )
{
   (void)str;
   window_destroy( wid );
   menu_Close(MENU_SMALL);
}
/**
 * @fn static void menu_small_exit( unsigned int wid, char* str )
 * @brief Closes the small ingame menu and goes back to the main menu.
 *    @param str Unused.
 */
static void menu_small_exit( unsigned int wid, char* str )
{
   (void) str;
   unsigned int info_wid;
   
   /* if landed we must save anyways */
   if (landed) {
      /* increment time to match takeoff */
      ntime_inc( RNG( 2*NTIME_UNIT_LENGTH, 3*NTIME_UNIT_LENGTH ) );
      save_all();
      land_cleanup();
   }

   /* Close info menu if open. */
   if (menu_isOpen(MENU_INFO)) {
      info_wid = window_get("Info");
      window_destroy( info_wid );
      menu_Close(MENU_INFO);
   }

   window_destroy( wid );
   menu_Close(MENU_SMALL);
   menu_main();
}


/**
 * @fn static void exit_game (void)
 *
 * @brief Exits the game.
 */
static void exit_game (void)
{
   /* if landed we must save anyways */
   if (landed) {
      /* increment time to match takeoff */
      ntime_inc( RNG( 2*NTIME_UNIT_LENGTH, 3*NTIME_UNIT_LENGTH ) );
      save_all();
      land_cleanup();
   }
   SDL_Event quit;
   quit.type = SDL_QUIT;
   SDL_PushEvent(&quit);
}



/*
 * 
 * information menu
 *
 */
/**
 * @fn void menu_info (void)
 *
 * @brief Opens the information menu.
 */
void menu_info (void)
{
   char str[128];
   char *nt;
   unsigned int wid;

   /* Can't open menu twice. */
   if (menu_isOpen(MENU_INFO)) return;

   wid = window_create( "Info", -1, -1, INFO_WIDTH, INFO_HEIGHT );

   /* pilot generics */
   nt = ntime_pretty( ntime_get() );
   window_addText( wid, 20, 20, 120, INFO_HEIGHT-60,
         0, "txtDPilot", &gl_smallFont, &cDConsole,
         "Pilot:\n"
         "Date:\n"
         "Combat\n"
         " Rating:\n"
         "\n"
         "Ship:\n"
         "Fuel:\n"
         );
   snprintf( str, 128, 
         "%s\n"
         "%s\n"
         "\n"
         "%s\n"
         "\n"
         "%s\n"
         "%d (%d jumps)"
         , player_name, nt, player_rating(), player->name,
         (int)player->fuel, pilot_getJumps(player) );
   window_addText( wid, 80, 20,
         INFO_WIDTH-120-BUTTON_WIDTH, INFO_HEIGHT-60,
         0, "txtPilot", &gl_smallFont, &cBlack, str );
   free(nt);

   /* menu */
   window_addButton( wid, -20, (20 + BUTTON_HEIGHT)*4 + 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         player->ship->name, "Ship", ship_view );
   window_addButton( wid, -20, (20 + BUTTON_HEIGHT)*3 + 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnOutfits", "Outfts", info_outfits_menu );
   window_addButton( wid, -20, (20 + BUTTON_HEIGHT)*2 + 20,      
         BUTTON_WIDTH, BUTTON_HEIGHT,    
         "btnCargo", "Cargo", info_cargo_menu );
   window_addButton( wid, -20, 20 + BUTTON_HEIGHT + 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnMissions", "Missions", info_missions_menu );
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", menu_info_close );

   menu_Open(MENU_INFO);
}
/**
 * @brief Closes the information menu.
 *    @param str Unused.
 */
static void menu_info_close( unsigned int wid, char* str )
{
   (void) str;
   window_destroy( wid );
   menu_Close(MENU_INFO);
}


/**
 * @brief Shows the player what outfits he has.
 *
 *    @param str Unused.
 */
static void info_outfits_menu( unsigned int parent, char* str )
{
   (void) str;
   (void) parent;
   char *buf;
   unsigned int wid;

   /* Create window */
   wid = window_create( "Outfits", -1, -1, OUTFITS_WIDTH, OUTFITS_HEIGHT );

   /* Text */
   window_addText( wid, 20, -40, 100, OUTFITS_HEIGHT-40,
         0, "txtLabel", &gl_smallFont, &cDConsole,
         "Ship Outfits:" );
   buf = pilot_getOutfits( player );
   window_addText( wid, 20, -45-gl_smallFont.h,
         OUTFITS_WIDTH-40, OUTFITS_HEIGHT-60,
         0, "txtOutfits", &gl_smallFont, &cBlack, buf );
   free(buf);

   /* Buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "closeOutfits", "Close", window_close );
}


/**
 * @brief Shows the player his cargo.
 *
 *    @param str Unused.
 */
static void info_cargo_menu( unsigned int parent, char* str )
{
   (void) str;
   (void) parent;
   unsigned int wid;
   char **buf;
   int nbuf;
   int i;

   /* Create the window */
   wid = window_create( "Cargo", -1, -1, CARGO_WIDTH, CARGO_HEIGHT );

   /* Buttons */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "closeCargo", "Back", window_close );
   window_addButton( wid, -40 - BUTTON_WIDTH, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT, "btnJettisonCargo", "Jettison",
         cargo_jettison );
   window_disableButton( wid, "btnJettisonCargo" );

   /* List */
   if (player->ncommodities==0) {
      /* No cargo */
      buf = malloc(sizeof(char*));
      buf[0] = strdup("None");
      nbuf = 1;
   }
   else {
      /* List the player's cargo */
      buf = malloc(sizeof(char*)*player->ncommodities);
      for (i=0; i<player->ncommodities; i++) {
         buf[i] = malloc(sizeof(char)*128);
         snprintf(buf[i],128, "%s%s %d",
               player->commodities[i].commodity->name,
               (player->commodities[i].id != 0) ? "*" : "",
               player->commodities[i].quantity);
      }
      nbuf = player->ncommodities;
   }
   window_addList( wid, 20, -40,
         CARGO_WIDTH - 40, CARGO_HEIGHT - BUTTON_HEIGHT - 80,
         "lstCargo", buf, nbuf, 0, cargo_update );

   cargo_update(wid, NULL);
}
/**
 * @fn static void cargo_update( unsigned int wid, char* str )
 * @brief Updates the player's cargo in the cargo menu.
 *    @param str Unused.
 */
static void cargo_update( unsigned int wid, char* str )
{
   (void)str;
   int pos;

   if (player->ncommodities==0) return; /* No cargo */

   pos = toolkit_getListPos( wid, "lstCargo" );

   /* Can jettison all but mission cargo when not landed*/
   if (landed || (player->commodities[pos].id != 0))
      window_disableButton( wid, "btnJettisonCargo" );
   else
      window_enableButton( wid, "btnJettisonCargo" );
}
/**
 * @fn static void cargo_jettison( unsigned int wid, char* str )
 * @brief Makes the player jettison the currently selected cargo.
 *    @param str Unused.
 */
static void cargo_jettison( unsigned int wid, char* str )
{
   (void)str;
   int pos;

   if (player->ncommodities==0) return; /* No cargo, redundant check */

   pos = toolkit_getListPos( wid, "lstCargo" );

   /* Remove the cargo */
   commodity_Jettison( player->id, player->commodities[pos].commodity,
         player->commodities[pos].quantity );
   pilot_rmCargo( player, player->commodities[pos].commodity,
         player->commodities[pos].quantity );

   /* We reopen the menu to recreate the list now. */
   window_destroy( wid );
   info_cargo_menu(0, NULL);
}


/**
 * @brief Shows the player's active missions.
 *
 *    @param parent Unused.
 *    @param str Unused.
 */
static void info_missions_menu( unsigned int parent, char* str )
{
   (void) str;
   (void) parent;
   unsigned int wid;

   /* create the window */
   wid = window_create( "Missions", -1, -1, MISSIONS_WIDTH, MISSIONS_HEIGHT );

   /* buttons */
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "closeMissions", "Back", window_close );
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
   mission_menu_genList(wid ,1);
}
/**
 * @brief Creates the current mission list for the mission menu.
 *    @param first 1 if it's the first time run.
 */
static void mission_menu_genList( unsigned int wid, int first )
{
   int i,j;
   char** misn_names;

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

   mission_menu_update(wid ,NULL);
}
/**
 * @fn static void mission_menu_update( unsigned int wid, char* str )
 * @brief Updates the mission menu mission information based on what's selected.
 *    @param str Unusued.
 */
static void mission_menu_update( unsigned int wid, char* str )
{
   (void)str;
   char *active_misn;
   Mission* misn;

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
/**
 * @fn static void mission_menu_abort( unsigned int wid, char* str )
 * @brief Aborts a mission in the mission menu.
 *    @param str Unused.
 */
static void mission_menu_abort( unsigned int wid, char* str )
{
   (void)str;
   char *selected_misn;
   int pos;
   Mission* misn;

   selected_misn = toolkit_getList( wid, "lstMission" );

   if (dialogue_YesNo( "Abort Mission", 
            "Are you sure you want to abort this mission?" )) {
      pos = toolkit_getListPos(wid, "lstMission" );
      misn = &player_missions[pos];
      mission_cleanup( misn );
      memmove( misn, &player_missions[pos+1], 
            sizeof(Mission) * (MISSION_MAX-pos-1) );
      mission_menu_genList(wid ,0);
   }
}



/**
 * @fn void menu_death (void)
 *
 * @brief Player death menu, appears when player got creamed.
 */
void menu_death (void)
{
   unsigned int wid;
   
   wid = window_create( "Death", -1, -1, DEATH_WIDTH, DEATH_HEIGHT );
   window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20),
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnMain", "Main Menu", menu_death_main );
   window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnExit", "Exit Game", (void(*)(unsigned int, char*)) exit_game );
   menu_Open(MENU_DEATH);

   /* Makes it all look cooler since everything still goes on. */
   unpause_game();
}
/**
 * @brief Closes the player death menu.
 *    @param str Unused.
 */
static void menu_death_main( unsigned int parent, char* str )
{
   (void) parent;
   (void) str;
   unsigned int wid;

   wid = window_get( "Death" );
   window_destroy( wid );
   menu_Close(MENU_DEATH);

   /* Game will repause now since toolkit closes and reopens. */
   menu_main();
}


