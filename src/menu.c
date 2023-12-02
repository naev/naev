/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file menu.h
 *
 * @brief Handles the important game menus.
 */
/** @cond */
#include "physfs.h"
#include "SDL.h"

#include "naev.h"
/** @endcond */

#include "menu.h"

#include "array.h"
#include "board.h"
#include "camera.h"
#include "comm.h"
#include "conf.h"
#include "dev_mapedit.h"
#include "dev_uniedit.h"
#include "dialogue.h"
#include "gui.h"
#include "hook.h"
#include "info.h"
#include "intro.h"
#include "land.h"
#include "load.h"
#include "log.h"
#include "map.h"
#include "mission.h"
#include "music.h"
#include "ndata.h"
#include "nstring.h"
#include "ntime.h"
#include "options.h"
#include "pause.h"
#include "pilot.h"
#include "player.h"
#include "render.h"
#include "rng.h"
#include "save.h"
#include "space.h"
#include "start.h"
#include "safelanes.h"
#include "tk/toolkit_priv.h" /* Needed for menu_main_resize */
#include "toolkit.h"

#define MAIN_WIDTH      200 /**< Main menu width. */

#define MENU_WIDTH      200 /**< Escape menu width. */
#define MENU_HEIGHT     250 /**< Escape menu height. */

#define DEATH_WIDTH     200 /**< Death menu width. */
#define DEATH_HEIGHT    200 /**< Death menu height. */

#define BUTTON_WIDTH    160 /**< Button width, standard across menus. */
#define BUTTON_HEIGHT   30 /**< Button height, standard across menus. */

#define EDITORS_EXTRA_WIDTH  60 /**< Editors menu extra width. */

#define menu_Open(f)    (menu_open |= (f)) /**< Marks a menu as opened. */
#define menu_Close(f)   (menu_open &= ~(f)) /**< Marks a menu as closed. */
int menu_open = 0; /**< Stores the opened/closed menus. */
int bg_needs_reset = 1; /**< Whether or not it needs a reset. */

static glTexture *main_naevLogo = NULL; /**< Naev Logo texture. */
static int menu_small_allowsave = 1; /** Can save with small menu. */

/*
 * prototypes
 */
/* Generic. */
static void menu_exit( unsigned int wid, const char *str );
/* main menu */
static int menu_main_bkg_system (void);
static void main_menu_promptClose( unsigned int wid, const char *unused );
static void menu_main_load( unsigned int wid, const char *str );
static void menu_main_new( unsigned int wid, const char *str );
static void menu_main_credits( unsigned int wid, const char *str );
static void menu_main_cleanBG( unsigned int wid, const char *str );
/* small menu */
static void menu_small_load( unsigned int wid, const char *str );
static void menu_small_resume( unsigned int wid, const char *str );
static void menu_small_info( unsigned int wid, const char *str );
static void menu_small_exit( unsigned int wid, const char *str );
static void exit_game (void);
/* death menu */
static void menu_death_continue( unsigned int wid, const char *str );
static void menu_death_restart( unsigned int wid, const char *str );
static void menu_death_main( unsigned int wid, const char *str );
static void menu_death_close( unsigned int wid, const char *str );
/* editors menu */
/* - Universe Editor */
/* - Back to Main Menu */
static void menu_editors_open( unsigned int wid_unused, const char *unused );
static void menu_editors_close( unsigned int wid, const char *str );
/* options button. */
static void menu_options_button( unsigned int wid, const char *str );

/*
 * Background system for the menu.
 */
static int menu_main_bkg_system (void)
{
   if (!bg_needs_reset) {
      pause_setSpeed( 1. );
      sound_setSpeed( 1. );
      return 0;
   }

   const nsave_t *saves;
   const char *sys;
   double cx, cy;

   /* Clean pilots. */
   pilots_cleanAll();
   sys = NULL;

   load_refresh();
   saves = load_getList(NULL);

   if (array_size( saves ) > 0) {
      const nsave_t *ns = &saves[0];

      /* Try to apply unidiff. */
      load_gameDiff( ns[0].path );

      /* Get start position. */
      if (spob_exists( ns[0].spob )) {
         Spob *pnt = spob_get( ns[0].spob );
         if (pnt != NULL) {
            sys = spob_getSystem( ns[0].spob );
            if (sys != NULL) {
               cx = pnt->pos.x;
               cy = pnt->pos.y;
            }
         }
      }
   }

   /* In case save game has no diff. */
   if (!safelanes_calculated())
      safelanes_recalculate();

   /* Fallback if necessary. */
   if (sys == NULL) {
      sys = start_system();
      start_position( &cx, &cy );
   }

   /* Have to normalize values by zoom. */
   cx += SCREEN_W/4. / conf.zoom_far;
   cy += SCREEN_H/8. / conf.zoom_far;

   /* Initialize. */
   space_init( sys, 1 ); /* More lively with simulation. */
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

   /* Clean up land if triggered with player.gameover() while landed. */
   if (landed)
      land_cleanup();

   /* Close all open windows. */
   toolkit_closeAll();

   /* Clean up GUI - must be done before using SCREEN_W or SCREEN_H. */
   gui_cleanup();
   player_soundStop(); /* Stop sound. */
   player_resetSpeed();
   render_postprocessCleanup();

   /* Play load music. */
   music_choose("load");

   /* Load background and friends. */
   gl_freeTexture( main_naevLogo );
   tex = gl_newImage( GFX_PATH"Naev.webp", 0 );
   main_naevLogo = tex;
   menu_main_bkg_system();

   /* Set dimensions */
   y  = 20 + (BUTTON_HEIGHT+20)*4;
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
   bwid = window_create( "wdwBG", "", -1, -1, -1, -1 );
   window_onClose( bwid, menu_main_cleanBG );
   window_setBorder( bwid, 0 );
   window_addImage( bwid, (SCREEN_W-tex->sw)/2., offset_logo, 0, 0, "imgLogo", tex, 0 );
   window_addText( bwid, 0, 10, SCREEN_W, 30., 1, "txtBG", NULL,
         &cWhite, naev_version(1) );

   /* create menu window */
   wid = window_create( "wdwMainMenu", _("Main Menu"), -1, offset_wdw, MAIN_WIDTH, h );
   window_setCancel( wid, main_menu_promptClose );

   /* Buttons. */
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnLoad", _("Load Game"), menu_main_load, SDLK_l );
   y -= BUTTON_HEIGHT+20;
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnNew", _("New Game"), menu_main_new, SDLK_n );
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
         "btnCredits", p_("Menu|", "Credits"), menu_main_credits, SDLK_c );
   y -= BUTTON_HEIGHT+20;
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnExit", _("Exit Game"), menu_exit, SDLK_x );

   /* Disable load button if there are no saves. */
   if (array_size( load_getList(NULL) ) == 0) {
      window_disableButton( wid, "btnLoad" );
      window_setFocus( wid, "btnNew" );
   }
   else
      window_setFocus( wid, "btnLoad" );

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

   menu_id = window_get("wdwMainMenu");
   bg_id   = window_get("wdwBG");

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
static void main_menu_promptClose( unsigned int wid, const char *unused )
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
   if (window_exists( "wdwMainMenu" )) {
      unsigned int wid = window_get("wdwMainMenu");
      window_destroy( wid );
   }
   else
      WARN( _("Main menu does not exist.") );

   menu_Close(MENU_MAIN);
   pause_game();
}
/**
 * @brief Function to active the load game menu.
 *    @param str Unused.
 */
static void menu_main_load( unsigned int wid, const char *str )
{
   (void) str;
   (void) wid;
   load_loadGameMenu();
}
/**
 * @brief Function to active the new game menu.
 *    @param str Unused.
 */
static void menu_main_new( unsigned int wid, const char *str )
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
 * @brief Function to exit the main menu and game.
 *    @param str Unused.
 */
static void menu_main_credits( unsigned int wid, const char *str )
{
   (void) str;
   window_destroy( wid );
   menu_Close(MENU_MAIN);
   intro_display( "AUTHORS", "credits" );
   bg_needs_reset = 0;
   menu_main();
   bg_needs_reset = 1;
   /* We'll need to start music again. */
   music_choose("load");
}
/**
 * @brief Function to exit the main menu and game.
 *    @param str Unused.
 */
static void menu_exit( unsigned int wid, const char *str )
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
static void menu_main_cleanBG( unsigned int wid, const char *str )
{
   (void) wid;
   (void) str;

   gl_freeTexture(main_naevLogo);
   main_naevLogo = NULL;
}

/*
 *
 * in-game menu
 *
 */
/**
 * @brief Opens the small in-game menu.
 */
void menu_small( int docheck, int info, int options, int allowsave )
{
   int can_save;
   unsigned int wid;
   int y, h;

   /* Check if menu should be openable. */
   if (docheck && (player_isFlag(PLAYER_DESTROYED) ||
         dialogue_isOpen() || /* Shouldn't open over dialogues. */
         (menu_isOpen(MENU_MAIN) || menu_isOpen(MENU_DEATH) )))
      return;

   if (menu_isOpen( MENU_SMALL ))
      return;

   can_save = allowsave && landed && !player_isFlag(PLAYER_NOSAVE);
   menu_small_allowsave = allowsave;

   h = MENU_HEIGHT - (BUTTON_HEIGHT+20)*(!info+!options);
   y = 20 + (BUTTON_HEIGHT+20)*(2+!!info+!!options);
   wid = window_create( "wdwMenuSmall", _("Menu"), -1, -1, MENU_WIDTH, h + BUTTON_HEIGHT + 20 );

   window_setCancel( wid, menu_small_resume );

   window_addButtonKey( wid, 20, y,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnResume", _("Resume"), menu_small_resume, SDLK_r );
   y -= BUTTON_HEIGHT+20;
   if (info) {
      window_addButtonKey( wid, 20, y,
            BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnInfo", _("Info"), menu_small_info, SDLK_i );
      y -= BUTTON_HEIGHT+20;
   }
   window_addButtonKey( wid, 20, y,
         BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnSave", can_save ? _("Load / Save") : _("Load"), menu_small_load, SDLK_l );
   y -= BUTTON_HEIGHT+20;
   if (options) {
      window_addButtonKey( wid, 20, y,
            BUTTON_WIDTH, BUTTON_HEIGHT,
            "btnOptions", _("Options"), menu_options_button, SDLK_o );
      y -= BUTTON_HEIGHT+20;
   }
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH, BUTTON_HEIGHT,
         "btnExit", _("Exit to Title"), menu_small_exit, SDLK_x );

   menu_Open(MENU_SMALL);
}

/**
 * @brief Opens the load menu.
 *    @param wid Unused.
 *    @param str Unused.
 */
static void menu_small_load( unsigned int wid, const char *str )
{
   (void) wid;
   (void) str;

   load_refresh(); /* FIXME: Substitute proper cache invalidation in case of save_all() etc. */
   load_loadSnapshotMenu( player.name, !menu_small_allowsave );
}

/**
 * @brief Closes the small in-game menu.
 *    @param str Unused.
 */
static void menu_small_resume( unsigned int wid, const char *str )
{
   (void)str;
   window_destroy( wid );
   menu_Close(MENU_SMALL);
}

/**
 * @brief Closes the small menu.
 */
void menu_small_close (void)
{
   if (window_exists( "wdwMenuSmall" ))
      window_destroy( window_get( "wdwMenuSmall" ) );
   else
      WARN( _("Small menu does not exist.") );

   menu_Close(MENU_SMALL);
}

/**
 * @brief Opens the info window.
 *    @param wid Unused.
 *    @param str Unused.
 */
static void menu_small_info( unsigned int wid, const char *str )
{
   (void) str;
   (void) wid;

   menu_info( INFO_MAIN );
}

static int menu_small_exit_hook( void* unused )
{
   (void) unused;
   unsigned int wid;

   /* Still stuck in a dialogue, so we have to do another hook pass. */
   if (dialogue_isOpen()) {
      hook_addFunc( menu_small_exit_hook, NULL, "safe" );
      return 0;
   }

   /* if landed we must save anyways */
   if (landed && land_canSave()) {
      save_all();
      land_cleanup();
   }

   /* Close info menu if open. */
   if (menu_isOpen(MENU_INFO)) {
      unsigned int info_wid = window_get("wdwInfo");
      window_destroy( info_wid );
      menu_Close(MENU_INFO);
   }

   /* Stop player sounds because sometimes they hang. */
   player_restoreControl( 0, _("Exited game.") );
   player_soundStop();

   /* Clean up. */
   wid = window_get("wdwMenuSmall");
   window_destroy( wid );
   menu_Close(MENU_SMALL);
   menu_main();
   return 0;
}

/**
 * @brief Closes the small in-game menu and goes back to the main menu.
 *    @param str Unused.
 */
static void menu_small_exit( unsigned int wid, const char *str )
{
   (void) wid;
   (void) str;

   if (!menu_small_allowsave && landed && land_canSave()) {
      if (!dialogue_YesNoRaw(_("Exit to Menu?"),_("Are you sure you wish to exit to menu right now? The game #rwill not be saved#0 since last time you landed!") ))
         return;
   }

   /* Break out of potential inner loops. */
   SDL_Event event;
   SDL_memset( &event, 0, sizeof(event) );
   event.type = SDL_LOOPDONE;
   SDL_PushEvent( &event );

   hook_addFunc( menu_small_exit_hook, NULL, "safe" );
}

/**
 * @brief Exits the game.
 */
static void exit_game (void)
{
   /* if landed we must save anyways */
   if (landed && land_canSave()) {
      save_all();
      land_cleanup();
   }
   SDL_Event quit;
   quit.type = SDL_QUIT;
   SDL_PushEvent(&quit);
}

/**
 * @brief Reload the current saved game, when player want to continue after death
 */
static void menu_death_continue( unsigned int wid, const char *str )
{
   (void) str;

   window_destroy( wid );
   menu_Close(MENU_DEATH);

   save_reload();
   player.death_counter++; /* Add death here. */
   player.ps.death_counter++;
}

/**
 * @brief Restart the game, when player want to continue after death but without a saved game
 */
static void menu_death_restart( unsigned int wid, const char *str )
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

   wid = window_create( "wdwRIP", _("Death"), -1, -1, DEATH_WIDTH, DEATH_HEIGHT );
   window_onClose( wid, menu_death_close );

   /* Allow the player to continue if the saved game exists, if not, propose to restart */
   load_refresh();
   if (array_size( load_getList( player.name ) ) > 0)
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
   if (!landed)
      unpause_game();
}
/**
 * @brief Closes the player death menu.
 *    @param str Unused.
 */
static void menu_death_main( unsigned int wid, const char *str )
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
static void menu_death_close( unsigned int wid, const char *str )
{
   (void) wid;
   (void) str;
   pause_game(); /* Repause the game. */
}

/**
 * @brief Opens the menu options from a button.
 */
static void menu_options_button( unsigned int wid, const char *str )
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
static void menu_editors_open( unsigned int wid, const char *unused )
{
   (void) unused;
   int h, y;

   /* Menu already open, quit. */
   if (menu_isOpen( MENU_EDITORS )) {
      return;
   }

   /* Close the Main Menu */
   menu_main_close();
   unpause_game();

   /* Clear known flags - specifically for the SYSTEM_HIDDEN flag. */
   space_clearKnown();

   /* Set dimensions */
   y  = 20 + (BUTTON_HEIGHT+20)*2;
   h  = y + 80;

   wid = window_create( "wdwEditors", _("Editors"), -1, -1, MENU_WIDTH + EDITORS_EXTRA_WIDTH, h );
   window_setCancel( wid, menu_editors_close );

   /* Set buttons for the editors */
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH + EDITORS_EXTRA_WIDTH, BUTTON_HEIGHT,
      "btnUniverse", _("Universe Map"), uniedit_open, SDLK_u );
   y -= BUTTON_HEIGHT+20;
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH + EDITORS_EXTRA_WIDTH, BUTTON_HEIGHT,
      "btnMapEdit", _("Map Outfits"), mapedit_open, SDLK_m );
   y -= BUTTON_HEIGHT+20;
   window_addButtonKey( wid, 20, y, BUTTON_WIDTH + EDITORS_EXTRA_WIDTH, BUTTON_HEIGHT,
      "btnMain", _("Exit to Main Menu"), menu_editors_close, SDLK_x );

    /* Editors menu is open. */
   menu_Open( MENU_EDITORS );

   return;
}

/**
 * @brief Closes the editors menu.
 *    @param str Unused.
 */
static void menu_editors_close( unsigned int wid, const char *str )
{
   (void) str;

   /* Close the Editors Menu and mark it as closed */
   window_destroy( wid );
   menu_Close( MENU_EDITORS );

   /* Restores Main Menu */
   bg_needs_reset = 0;
   menu_main();
   bg_needs_reset = 1;

   return;
}
