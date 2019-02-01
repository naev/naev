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

#include "nstring.h"

#include "SDL.h"

#include "toolkit.h"
#include "tk/toolkit_priv.h" /* Needed for menu_main_resize */
#include "dialogue.h"
#include "log.h"
#include "pilot.h"
#include "space.h"
#include "player.h"
#include "mission.h"
#include "ntime.h"
#include "save.h"
#include "load.h"
#include "land.h"
#include "rng.h"
#include "nebula.h"
#include "pause.h"
#include "options.h"
#include "intro.h"
#include "music.h"
#include "map.h"
#include "nfile.h"
#include "info.h"
#include "comm.h"
#include "conf.h"
#include "dev_uniedit.h"
#include "dev_mapedit.h"
#include "gui.h"
#include "start.h"
#include "camera.h"
#include "board.h"
#include "ndata.h"


#define MAIN_WIDTH      130 /**< Main menu width. */

#define MENU_WIDTH      130 /**< Escape menu width. */
#define MENU_HEIGHT     250 /**< Escape menu height. */


#define DEATH_WIDTH     130 /**< Death menu width. */
#define DEATH_HEIGHT    200 /**< Death menu height. */

#define OPTIONS_WIDTH   360 /**< Options menu width. */
#define OPTIONS_HEIGHT  90  /**< Options menu height. */

#define BUTTON_WIDTH    90 /**< Button width, standard across menus. */
#define BUTTON_HEIGHT   30 /**< Button height, standard across menus. */

#define EDITORS_EXTRA_WIDTH  60 /**< Editors menu extra width. */

#define menu_Open(f)    (menu_open |= (f)) /**< Marks a menu as opened. */
#define menu_Close(f)   (menu_open &= ~(f)) /**< Marks a menu as closed. */
int menu_open = 0; /**< Stores the opened/closed menus. */


static glTexture *main_naevLogo = NULL; /**< Naev Logo texture. */


/*
 * prototypes
 */
/* Generic. */
static void menu_exit( unsigned int wid, char* str );
/* main menu */
static int menu_main_bkg_system (void);
static void main_menu_promptClose( unsigned int wid, char *unused );
static void menu_main_load( unsigned int wid, char* str );
static void menu_main_new( unsigned int wid, char* str );
static void menu_main_tutorial( unsigned int wid, char* str );
static void menu_main_credits( unsigned int wid, char* str );
static void menu_main_cleanBG( unsigned int wid, char* str );
/* small menu */
static void menu_small_close( unsigned int wid, char* str );
static void menu_small_info( unsigned int wid, char *str );
static void menu_small_exit( unsigned int wid, char* str );
static void exit_game (void);
/* death menu */
static void menu_death_continue( unsigned int wid, char* str );
static void menu_death_restart( unsigned int wid, char* str );
static void menu_death_main( unsigned int wid, char* str );
static void menu_death_close( unsigned int wid, char* str );
/* editors menu */
/* - Universe Editor */
/* - Back to Main Menu */
static void menu_editors_open( unsigned int wid_unused, char *unused );
static void menu_editors_close( unsigned int wid, char* str );
/* options button. */
static void menu_options_button( unsigned int wid, char *str );


static int menu_main_bkg_system (void)
{
   nsave_t *ns;
   int n;
   const char *sys;
   Planet *pnt;
   double cx, cy;

   /* Clean pilots. */
   pilots_cleanAll();
   sys = NULL;

   /* Refresh saves. */
   load_refresh();

   /* Get start position. */
   ns = load_getList( &n );
   if ((n > 0) && (planet_exists( ns[0].planet ))) {
      pnt = planet_get( ns[0].planet );
      if (pnt != NULL) {
         sys = planet_getSystem( ns[0].planet );
         if (sys != NULL) {
            cx = pnt->pos.x;
            cy = pnt->pos.y;
            cx += 300;
            cy += 200;
         }
      }
   }

   /* Fallback if necessary. */
   if (sys == NULL) {
      sys = start_system();
      start_position( &cx, &cy );
   }

   /* Initialize. */
   space_init( sys );
   cam_setTargetPos( cx, cy, 0 );
   cam_setZoom( conf.zoom_far );
   pause_setSpeed( 1. );
   sound_setSpeed( 1. );

   return 0;
}


/**
 * @brief Opens the main menu (titlescreen).
 */
void menu_main (void)
{
   int offset_logo, offset_wdw, freespace;
   unsigned int bwid, wid;
   glTexture *tex;
   int h, y;

   if (menu_isOpen(MENU_MAIN)) {
      WARN( _("Menu main is already open.") );
      return;
   }

   /* Clean up GUI - must be done before using SCREEN_W or SCREEN_H. */
   gui_cleanup();
   player_soundStop(); /* Stop sound. */

   /* Play load music. */
   music_choose("load");

   /* Load background and friends. */
   tex = gl_newImage( GFX_PATH"Naev.png", 0 );
   main_naevLogo = tex;
   menu_main_bkg_system();

   /* Set dimensions */
   y  = 20 + (BUTTON_HEIGHT+20)*5;
   h  = y + 80;
   if (conf.devmode) {
      h += BUTTON_HEIGHT + 20;
      y += BUTTON_HEIGHT + 20;
   }

   /* Calculate Logo and window offset. */
   freespace = SCREEN_H - tex->sh - h;
   if (freespace < 0) { /* Not enough freespace, this can get ugly. */
      offset_logo = SCREEN_W - tex->sh;
      offset_wdw  = 0;
   }
   /* Otherwise space evenly. */
   else {
      offset_logo = -freespace/4;
      offset_wdw  = freespace/2;
   }

   /* create background image window */
   bwid = window_create( "BG", -1, -1, -1, -1 );
   window_onClose( bwid, menu_main_cleanBG );
   window_setBorder( bwid, 0 );
   window_addImage( bwid, (SCREEN_W-tex->sw)/2., offset_logo, 0, 0, "imgLogo", tex, 0 );
   window_addText( bwid, 0, 10, SCREEN_W, 30., 1, "txtBG", NULL,
         &cWhite, naev_version(1) );

   /* create menu window */
   wid = window_create( "Main Menu", -1, offset_wdw, MAIN_WIDTH, h );
   window_setCancel( wid, main_menu_promptClose );

   /* Buttons. */
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLoad", _("Load Game"), menu_main_load, SDLK_l );
   y -= BUTTON_HEIGHT+20;
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnNew", _("New Game"), menu_main_new, SDLK_n );
   y -= BUTTON_HEIGHT+20;
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnTutorial", _("Tutorial"), menu_main_tutorial, SDLK_t );
   y -= BUTTON_HEIGHT+20;
   if (conf.devmode) {
      window_addButtonKey( wid, 20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnEditor", _("Editors"), menu_editors_open, SDLK_e );
      y -= BUTTON_HEIGHT+20;
   }
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnOptions", _("Options"), menu_options_button, SDLK_o );
   y -= BUTTON_HEIGHT+20;
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnCredits", _("Credits"), menu_main_credits, SDLK_c );
   y -= BUTTON_HEIGHT+20;
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnExit", _("Exit"), menu_exit, SDLK_x );

   /* Disable load button if there are no saves. */
   if (!save_hasSave())
      window_disableButton( wid, "btnLoad" );

   /* Make the background window a child of the menu. */
   window_setParent( bwid, wid );

   unpause_game();
   menu_Open(MENU_MAIN);
}


/**
 * @brief Resizes the main menu and its background.
 *
 * This is a one-off function that ensures the main menu's appearance
 * is consistent regardless of window resizing.
 */
void menu_main_resize (void)
{
   int w, h, bgw, bgh, tw, th;
   int offset_logo, offset_wdw, freespace;
   int menu_id, bg_id;
   Widget *wgt;

   if (!menu_isOpen(MENU_MAIN))
      return;

   menu_id = window_get("Main Menu");
   bg_id   = window_get("BG");

   window_dimWindow( menu_id, &w, &h );
   window_dimWindow( bg_id, &bgw, &bgh );

   freespace = SCREEN_H - main_naevLogo->sh - h;
   if (freespace < 0) {
      offset_logo = SCREEN_H - main_naevLogo->sh;
      offset_wdw  = 0;
   }
   else {
      offset_logo = -freespace/4;
      offset_wdw  = freespace/2;
   }

   window_moveWidget( bg_id, "imgLogo",
         (bgw - main_naevLogo->sw)/2., offset_logo );

   window_dimWidget( bg_id, "txtBG", &tw, &th );

   if (tw > SCREEN_W) {
      /* RIP abstractions. X must be set manually because window_moveWidget
       * transforms negative coordinates. */
      wgt = window_getwgt( bg_id, "txtBG" );
      if (wgt)
         wgt->x = (SCREEN_W - tw) / 2;
   }
   else
      window_moveWidget( bg_id, "txtBG", (SCREEN_W - tw)/2, 10. );

   window_move( menu_id, -1, offset_wdw );
}


/**
 * @brief Main menu closing prompt.
 */
static void main_menu_promptClose( unsigned int wid, char *unused )
{
   (void) wid;
   (void) unused;
   exit_game();
}


/**
 * @brief Closes the main menu.
 */
void menu_main_close (void)
{
   if (window_exists("Main Menu"))
      window_destroy( window_get("Main Menu") );
   else
      WARN( _("Main menu does not exist.") );

   menu_Close(MENU_MAIN);
   pause_game();
}
/**
 * @brief Function to active the load game menu.
 *    @param str Unused.
 */
static void menu_main_load( unsigned int wid, char* str )
{
   (void) str;
   (void) wid;
   load_loadGameMenu();
}
/**
 * @brief Function to active the new game menu.
 *    @param str Unused.
 */
static void menu_main_new( unsigned int wid, char* str )
{
   (void) str;
   (void) wid;

   /* Closes the main menu window. */
   window_destroy( wid );
   menu_Close(MENU_MAIN);
   pause_game();

   /* Start the new player. */
   player_new();
}
/**
 * @brief Function to active the new game menu.
 *    @param str Unused.
 */
static void menu_main_tutorial( unsigned int wid, char* str )
{
   (void) str;
   (void) wid;
   window_destroy( wid );
   menu_Close(MENU_MAIN);
   pause_game();
   player_newTutorial();
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

   naev_quit();
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
   if (main_naevLogo != NULL)
      gl_freeTexture(main_naevLogo);
   main_naevLogo = NULL;
   window_modifyImage( wid, "imgLogo", NULL, 0, 0 );
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
   if ((player.p == NULL) || player_isFlag(PLAYER_DESTROYED) ||
         pilot_isFlag(player.p,PILOT_DEAD) ||
         comm_isOpen() ||
         dialogue_isOpen() || /* Shouldn't open over dialogues. */
         (menu_isOpen(MENU_MAIN) ||
            menu_isOpen(MENU_SMALL) ||
            menu_isOpen(MENU_DEATH) ))
      return;

   wid = window_create( "Menu", -1, -1, MENU_WIDTH, MENU_HEIGHT );

   window_setCancel( wid, menu_small_close );

   window_addButtonKey( wid, 20, 20 + BUTTON_HEIGHT*3 + 20*3,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnResume", _("Resume"), menu_small_close, SDLK_r );
   window_addButtonKey( wid, 20, 20 + BUTTON_HEIGHT*2 + 20*2,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnInfo", _("Info"), menu_small_info, SDLK_i );
   window_addButtonKey( wid, 20, 20 + BUTTON_HEIGHT + 20,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnOptions", _("Options"), menu_options_button, SDLK_o );
   window_addButtonKey( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnExit", _("Exit"), menu_small_exit, SDLK_x );

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
}


/**
 * @brief Opens the info window.
 *    @param wid Unused.
 *    @param str Unused.
 */
static void menu_small_info( unsigned int wid, char *str )
{
   (void) str;
   (void) wid;

   menu_info( INFO_MAIN );
}

/**
 * @brief Closes the small ingame menu and goes back to the main menu.
 *    @param str Unused.
 */
static void menu_small_exit( unsigned int wid, char* str )
{
   (void) str;
   unsigned int info_wid, board_wid;

   /* if landed we must save anyways */
   if (landed) {
      save_all();
      land_cleanup();
   }

   /* Close info menu if open. */
   if (menu_isOpen(MENU_INFO)) {
      info_wid = window_get("Info");
      window_destroy( info_wid );
      menu_Close(MENU_INFO);
   }

   /* Force unboard. */
   if (player_isBoarded()) {
      board_wid = window_get("Boarding");
      board_exit(board_wid, NULL);
   }

   /* Stop player sounds because sometimes they hang. */
   player_restoreControl( 0, _("Exited game.") );
   player_soundStop();

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
      save_all();
      land_cleanup();
   }
   SDL_Event quit;
   quit.type = SDL_QUIT;
   SDL_PushEvent(&quit);
}


/**
 * @brief Reload the current savegame, when player want to continue after death
 */
static void menu_death_continue( unsigned int wid, char* str )
{
   (void) str;

   window_destroy( wid );
   menu_Close(MENU_DEATH);

   save_reload();
}

/**
 * @brief Restart the game, when player want to continue after death but without a savegame
 */
static void menu_death_restart( unsigned int wid, char* str )
{
   (void) str;

   window_destroy( wid );
   menu_Close(MENU_DEATH);

   if (player_isTut())
      player_newTutorial();
   else
      player_new();
}

/**
 * @brief Player death menu, appears when player got creamed.
 */
void menu_death (void)
{
   unsigned int wid;
   char path[PATH_MAX];

   wid = window_create( "Death", -1, -1, DEATH_WIDTH, DEATH_HEIGHT );
   window_onClose( wid, menu_death_close );

   /* Allow the player to continue if the savegame exists, if not, propose to restart */
   nsnprintf(path, PATH_MAX, "%ssaves/%s.ns", nfile_dataPath(), player.name);
   if (!player_isTut() && nfile_fileExists(path))
      window_addButtonKey( wid, 20, 20 + BUTTON_HEIGHT*2 + 20*2, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnContinue", _("Continue"), menu_death_continue, SDLK_c );
   else
      window_addButtonKey( wid, 20, 20 + BUTTON_HEIGHT*2 + 20*2, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnRestart", _("Restart"), menu_death_restart, SDLK_r );

   window_addButtonKey( wid, 20, 20 + (BUTTON_HEIGHT+20),
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnMain", _("Main Menu"), menu_death_main, SDLK_m );
   window_addButtonKey( wid, 20, 20, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnExit", _("Exit Game"), menu_exit, SDLK_x );
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
 * @brief Hack to get around the fact the death menu unpauses the game.
 */
static void menu_death_close( unsigned int wid, char* str )
{
   (void) wid;
   (void) str;
   pause_game(); /* Repause the game. */
}


/**
 * @brief Opens the menu options from a button.
 */
static void menu_options_button( unsigned int wid, char *str )
{
   (void) wid;
   (void) str;
   opt_menu();
}


/**
 * @brief Menu to ask if player really wants to quit.
 */
int menu_askQuit (void)
{
   /* Asked twice, quit. */
   if (menu_isOpen( MENU_ASKQUIT )) {
      exit_game();
      return 1;
   }

   /* Ask if should quit. */
   menu_Open( MENU_ASKQUIT );
   if (dialogue_YesNoRaw( _("Quit Naev"), _("Are you sure you want to quit Naev?") )) {
      exit_game();
      return 1;
   }
   menu_Close( MENU_ASKQUIT );

   return 0;
}

/**
 * @brief Provisional Menu for when there will be multiple editors
 */
static void menu_editors_open( unsigned int wid, char *unused )
{
   (void) unused;
   int h, y;

   /*WARN("Entering function.");*/

   /* Menu already open, quit. */
   if (menu_isOpen( MENU_EDITORS )) {
      return;
   }

   /* Close the Main Menu */
   menu_main_close();
   unpause_game();

   /* Set dimensions */
   y  = 20 + (BUTTON_HEIGHT+20)*2;
   h  = y + 80;

   wid = window_create( "Editors", -1, -1, MENU_WIDTH + EDITORS_EXTRA_WIDTH, h );
   window_setCancel( wid, menu_editors_close );

   /* Set buttons for the editors */
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH + EDITORS_EXTRA_WIDTH, BUTTON_HEIGHT,
      "btnUniverse", "Universe Map", uniedit_open, SDLK_u );
   y -= BUTTON_HEIGHT+20;
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH + EDITORS_EXTRA_WIDTH, BUTTON_HEIGHT,
      "btnMapEdit", "Map Outfits", mapedit_open, SDLK_m );
   y -= BUTTON_HEIGHT+20;
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH + EDITORS_EXTRA_WIDTH, BUTTON_HEIGHT,
      "btnMain", "Exit to Main Menu", menu_editors_close, SDLK_x );

    /* Editors menu is open. */
   menu_Open( MENU_EDITORS );

   /*WARN("Exiting function.");*/

   return;
}

/**
 * @brief Closes the editors menu.
 *    @param str Unused.
 */
static void menu_editors_close( unsigned int wid, char* str )
{
   (void)str;
   
   /* Close the Editors Menu and mark it as closed */
   /*WARN("Entering function.");*/
   window_destroy( wid );
   menu_Close( MENU_EDITORS );
   
   /* Restores Main Menu */
   /*WARN("Restoring Main Menu.");*/
   menu_main();
   /*WARN("Exiting function.");*/
   
   return;
}