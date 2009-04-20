/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file menu.h
 *
 * @brief Handles the important game menus.
 */


#include "menu.h"

#include "naev.h"

#include <string.h>

#include "SDL.h"

#include "toolkit.h"
#include "dialogue.h"
#include "log.h"
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
#include "options.h"
#include "intro.h"
#include "music.h"
#include "map.h"
#include "nfile.h"


#define MAIN_WIDTH      130 /**< Main menu width. */
#define MAIN_HEIGHT     300 /**< Main menu height. */

#define MENU_WIDTH      130 /**< Escape menu width. */
#define MENU_HEIGHT     200 /**< Escape menu height. */

#define INFO_WIDTH      360 /**< Information menu width. */
#define INFO_HEIGHT     280 /**< Information menu height. */

#define OUTFITS_WIDTH   400 /**< Outfit menu width. */
#define OUTFITS_HEIGHT  200 /**< Outfit menu height. */

#define LICENSES_WIDTH  300 /**< Licenses menu width. */
#define LICENSES_HEIGHT  300 /**< Licenses menu height. */

#define CARGO_WIDTH     300 /**< Cargo menu width. */
#define CARGO_HEIGHT    300 /**< Cargo menu height. */

#define MISSIONS_WIDTH  600 /**< Mission menu width. */
#define MISSIONS_HEIGHT 500 /**< Mission menu height. */

#define DEATH_WIDTH     130 /**< Death menu width. */
#define DEATH_HEIGHT    200 /**< Death menu height. */

#define OPTIONS_WIDTH   360 /**< Options menu width. */
#define OPTIONS_HEIGHT  90  /**< Options menu height. */

#define BUTTON_WIDTH    90 /**< Button width, standard across menus. */
#define BUTTON_HEIGHT   30 /**< Button height, standard across menus. */

#define menu_Open(f)    (menu_open |= (f)) /**< Marks a menu as opened. */
#define menu_Close(f)   (menu_open &= ~(f)) /**< Marks a menu as closed. */
int menu_open = 0; /**< Stores the opened/closed menus. */


/*
 * prototypes
 */
/* Generic. */
static void menu_exit( unsigned int wid, char* str );
/* main menu */
void menu_main_close (void); /**< Externed in save.c */
static void menu_main_load( unsigned int wid, char* str );
static void menu_main_new( unsigned int wid, char* str );
static void menu_main_credits( unsigned int wid, char* str );
static void menu_main_cleanBG( unsigned int wid, char* str );
/* small menu */
static void menu_small_close( unsigned int wid, char* str );
static void menu_small_exit( unsigned int wid, char* str );
static void exit_game (void);
/* information menu */
static void menu_info_close( unsigned int wid, char* str );
/* outfits submenu */
static void info_outfits_menu( unsigned int parent, char* str );
/* licenses submenu. */
static void info_licenses_menu( unsigned int parent, char* str );
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
static void menu_death_continue( unsigned int wid, char* str );
static void menu_death_restart( unsigned int wid, char* str );
static void menu_death_main( unsigned int wid, char* str );
/* Options menu. */
static void menu_options_close( unsigned int parent, char* str );


/**
 * @brief Opens the main menu (titlescreen).
 */
void menu_main (void)
{
   int offset_logo, offset_wdw, freespace;
   unsigned int bwid, wid;
   glTexture *tex;

   /* Play load music. */
   music_choose("load");

   /* Load background and friends. */
   tex = gl_newImage( "gfx/NAEV.png", 0 );
   nebu_prep( 300., 0. ); /* Needed for nebulae to not spaz out */

   /* Calculate Logo and window offset. */
   freespace = SCREEN_H - tex->sh - MAIN_HEIGHT;
   if (freespace < 0) { /* Not enough freespace, this can get ugly. */
      offset_logo = SCREEN_W - tex->sh;
      offset_wdw  = 0;
   }
   else {
      /* We'll want a maximum seperation of 30 between logo and text. */
      if (freespace/3 > 25) {
         freespace -= 25;
         offset_logo = -25;
         offset_wdw  = -25 - tex->sh - 25;
      }
      /* Otherwise space evenly. */
      else {
         offset_logo = -freespace/3;
         offset_wdw  = freespace/3;
      }
   }

   /* create background image window */
   bwid = window_create( "BG", -1, -1, SCREEN_W, SCREEN_H );
   window_onClose( bwid, menu_main_cleanBG );
   window_addRect( bwid, 0, 0, SCREEN_W, SCREEN_H, "rctBG", &cBlack, 0 );
   window_addCust( bwid, 0, 0, SCREEN_W, SCREEN_H, "cstBG", 0,
         (void(*)(double,double,double,double)) nebu_render, NULL );
   window_addImage( bwid, (SCREEN_W-tex->sw)/2., offset_logo, "imgLogo", tex, 0 );
   window_addText( bwid, 0., 10, SCREEN_W, 30., 1, "txtBG", NULL,
         &cWhite, naev_version() );

   /* create menu window */
   wid = window_create( "Main Menu", -1, offset_wdw,
         MAIN_WIDTH, MAIN_HEIGHT );
   window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20)*4,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLoad", "Load Game", menu_main_load );
   window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20)*3,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnNew", "New Game", menu_main_new );
   window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20)*2,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnOptions", "Options", (void(*)(unsigned int,char*))menu_options );
   window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20),
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnOptions", "Credits", menu_main_credits );
   window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnExit", "Exit", menu_exit );

   /* Make the background window a parent of the menu. */
   window_setParent( bwid, wid );

   menu_Open(MENU_MAIN);
}
/**
 * @brief Closes the main menu.
 */
void menu_main_close (void)
{
   window_destroy( window_get("Main Menu") );

   menu_Close(MENU_MAIN);
}
/**
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
 * @brief Function to exit the main menu and game.
 *    @param str Unused.
 */
static void menu_main_credits( unsigned int wid, char* str )
{
   (void) str;
   (void) wid;
   intro_display( "AUTHORS", "credits" );
   /* We'll need to start music again. */
   music_choose("load");
}
/**
 * @brief Function to exit the main menu and game.
 *    @param str Unused.
 */
static void menu_exit( unsigned int wid, char* str )
{
   (void) str;
   (void) wid;

   exit_game();
}
/**
 * @brief Function to clean up the background window.
 *    @param wid Window to clean.
 *    @param str Unused.
 */
static void menu_main_cleanBG( unsigned int wid, char* str )
{
   (void) str;

   /* 
    * Ugly hack to prevent player.c from segfaulting due to the fact
    * that game will attempt to render while waiting for the quit event
    * pushed by exit_game() to be handled without actually having a player
    * nor anything of the likes (nor toolkit to stop rendering) while
    * not leaking any texture.
    */
   gl_freeTexture( window_getImage(wid, "imgLogo") );
   window_modifyImage( wid, "imgLogo", NULL );
}


/*
 *
 * ingame menu
 *
 */
/**
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

   /* Pauses the player's sounds. */
   player_soundPause();

   wid = window_create( "Menu", -1, -1, MENU_WIDTH, MENU_HEIGHT );

   window_setCancel( wid, menu_small_close );

   window_addButton( wid, 20, 20 + BUTTON_HEIGHT*2 + 20*2,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnResume", "Resume", menu_small_close );
   window_addButton( wid, 20, 20 + BUTTON_HEIGHT + 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnOptions", "Options", (void(*)(unsigned int,char*))menu_options );
   window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT, 
         "btnExit", "Exit", menu_small_exit );

   menu_Open(MENU_SMALL);
}
/**
 * @brief Closes the small ingame menu.
 *    @param str Unused.
 */
static void menu_small_close( unsigned int wid, char* str )
{
   (void)str;
   window_destroy( wid );
   menu_Close(MENU_SMALL);

   /* Resume player sounds. */
   player_soundResume();
}
/**
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

   /* Stop player sounds because sometimes they hang. */
   player_abortAutonav( "Exited game." );
   player_stopSound();

   /* Clean up. */
   window_destroy( wid );
   menu_Close(MENU_SMALL);
   menu_main();
}


/**
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
 * @brief Opens the information menu.
 */
void menu_info (void)
{
   char str[128];
   char *nt;
   unsigned int wid;

   /* Can't open menu twice. */
   if (menu_isOpen(MENU_INFO) || dialogue_isOpen()) return;

   /* Pauses the player's sounds. */
   player_soundPause();

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

   /* Resume player sounds. */
   player_soundResume();
}


/**
 * @brief Shows the player what outfits he has.
 *
 *    @param str Unused.
 */
static void info_outfits_menu( unsigned int parent, char* str )
{
   (void) str;
   char *buf;
   unsigned int wid;

   /* Create window */
   wid = window_create( "Outfits", -1, -1, OUTFITS_WIDTH, OUTFITS_HEIGHT );
   window_setParent( wid, parent );

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
   window_addButton( wid, -20-BUTTON_WIDTH-20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLicenses", "Licenses", info_licenses_menu );
}


/**
 * @brief Opens the licenses menu.
 */
static void info_licenses_menu( unsigned int parent, char* str )
{
   (void) str;
   unsigned int wid;
   char **licenses;
   int nlicenses;
   int i;
   char **buf;

   /* Create window */
   wid = window_create( "Licenses", -1, -1, LICENSES_WIDTH, LICENSES_HEIGHT );
   window_setParent( wid, parent );

   /* List. */
   buf = player_getLicenses( &nlicenses );
   licenses = malloc(sizeof(char*)*nlicenses);
   for (i=0; i<nlicenses; i++)
      licenses[i] = strdup(buf[i]);
   window_addList( wid, 20, -40, LICENSES_WIDTH-40, LICENSES_HEIGHT-80-BUTTON_HEIGHT,
         "lstLicenses", licenses, nlicenses, 0, NULL );

   /* Buttons */
   window_addButton( wid, -20, 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "closeLicenses", "Close", window_close );
}


/**
 * @brief Shows the player his cargo.
 *
 *    @param str Unused.
 */
static void info_cargo_menu( unsigned int parent, char* str )
{
   (void) str;
   unsigned int wid;
   char **buf;
   int nbuf;
   int i;

   /* Create the window */
   wid = window_create( "Cargo", -1, -1, CARGO_WIDTH, CARGO_HEIGHT );
   window_setParent( wid, parent );

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
   unsigned int wid;

   /* create the window */
   wid = window_create( "Missions", -1, -1, MISSIONS_WIDTH, MISSIONS_HEIGHT );
   window_setParent( wid, parent );

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

   /* Put a map. */
   map_show( wid, 20, 20, 300, 260, 0.75 );

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
         300, MISSIONS_HEIGHT-340,
         "lstMission", misn_names, j, 0, mission_menu_update );

   mission_menu_update(wid ,NULL);
}
/**
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

   /* Modify the text. */
   misn = &player_missions[ toolkit_getListPos(wid, "lstMission" ) ];
   window_modifyText( wid, "txtReward", misn->reward );
   window_modifyText( wid, "txtDesc", misn->desc );
   window_enableButton( wid, "btnAbortMission" );

   /* Select the system. */
   if (misn->sys_marker != NULL)
      map_center( misn->sys_marker );
}
/**
 * @brief Aborts a mission in the mission menu.
 *    @param str Unused.
 */
static void mission_menu_abort( unsigned int wid, char* str )
{
   (void)str;
   char *selected_misn;
   int pos;
   Mission* misn;
   int ret;

   selected_misn = toolkit_getList( wid, "lstMission" );

   if (dialogue_YesNo( "Abort Mission", 
            "Are you sure you want to abort this mission?" )) {

      /* Get the mission. */
      pos = toolkit_getListPos(wid, "lstMission" );
      misn = &player_missions[pos];

      /* We run the "abort" function if it's found. */
      ret = misn_tryRun( misn, "abort" );

      /* Now clean up mission. */
      if (ret != 2) {
         mission_cleanup( misn );
         memmove( misn, &player_missions[pos+1], 
               sizeof(Mission) * (MISSION_MAX-pos-1) );
         memset( &player_missions[MISSION_MAX-1], 0, sizeof(Mission) );
      }

      /* Reset markers. */
      mission_sysMark();

      /* Regenerate list. */
      mission_menu_genList(wid ,0);
   }
}

/**
 * @brief Reload the current savegame, when player want to continue after death
 */
static void menu_death_continue( unsigned int wid, char* str )
{
   (void) str;

   window_destroy( wid );
   menu_Close(MENU_DEATH);

   reload();
}

/**
 * @brief Restart the game, when player want to continue after death but without a savegame
 */
static void menu_death_restart( unsigned int wid, char* str )
{
   (void) str;

   window_destroy( wid );
   menu_Close(MENU_DEATH);

   player_new();
}

/**
 * @brief Player death menu, appears when player got creamed.
 */
void menu_death (void)
{
   unsigned int wid;
   
   wid = window_create( "Death", -1, -1, DEATH_WIDTH, DEATH_HEIGHT );

   /* Propose the player to continue if the samegame exist, if not, propose to restart */
   char path[PATH_MAX];
   snprintf(path, PATH_MAX, "%ssaves/%s.ns", nfile_basePath(), player_name);
   if (nfile_fileExists(path))
      window_addButton( wid, 20, 20 + BUTTON_HEIGHT*2 + 20*2, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnContinue", "Continue", menu_death_continue );
   else
      window_addButton( wid, 20, 20 + BUTTON_HEIGHT*2 + 20*2, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnRestart", "Restart", menu_death_restart );

   window_addButton( wid, 20, 20 + (BUTTON_HEIGHT+20),
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnMain", "Main Menu", menu_death_main );
   window_addButton( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnExit", "Exit Game", menu_exit );
   menu_Open(MENU_DEATH);

   /* Makes it all look cooler since everything still goes on. */
   unpause_game();
}
/**
 * @brief Closes the player death menu.
 *    @param str Unused.
 */
static void menu_death_main( unsigned int wid, char* str )
{
   (void) str;

   window_destroy( wid );
   menu_Close(MENU_DEATH);

   /* Game will repause now since toolkit closes and reopens. */
   menu_main();
}


/**
 * @brief Opens the options menu.
 */
void menu_options (void)
{
   unsigned int wid;

   wid = window_create( "Options", -1, -1, OPTIONS_WIDTH, OPTIONS_HEIGHT );
   window_addButton( wid, -20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnClose", "Close", menu_options_close );
   window_addButton( wid, -20 - (BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnKeybinds", "Keybindings", (void(*)(unsigned int,char*))opt_menuKeybinds );
   window_addButton( wid, -20 - 2 * (BUTTON_WIDTH+20), 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnAudio", "Audio", (void(*)(unsigned int,char*))opt_menuAudio );
   menu_Open(MENU_OPTIONS);
}
/**
 * @brief Closes the options menu.
 *    @param str Unused.
 */
static void menu_options_close( unsigned int wid, char* str )
{
   (void) str;

   window_destroy( wid );
   menu_Close(MENU_OPTIONS);
}

