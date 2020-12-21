/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @mainpage Naev
 *
 * Doxygen documentation for the Naev project.
 */
/**
 * @file naev.c
 *
 * @brief Controls the overall game flow: data loading/unloading and game loop.
 */

/*
 * includes
 */
/* localised global */
#include "SDL.h"

#include "SDL_error.h"
#include "log.h" /* for DEBUGGING */
#include "naev.h"


/* global */
#include "nstring.h" /* strdup */

#if HAS_POSIX
#include <time.h>
#include <unistd.h>
#endif /* HAS_POSIX */

#if defined(HAVE_FENV_H) && defined(DEBUGGING) && defined(_GNU_SOURCE)
#include <fenv.h>
#endif /* defined(HAVE_FENV_H) && defined(DEBUGGING) && defined(_GNU_SOURCE) */

#if HAS_LINUX && HAS_BFD && defined(DEBUGGING)
#include <signal.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <bfd.h>
#include <assert.h>
#endif /* HAS_LINUX && HAS_BFD && defined(DEBUGGING) */

/* local */
#include "ai.h"
#include "background.h"
#include "camera.h"
#include "cond.h"
#include "conf.h"
#include "console.h"
#include "damagetype.h"
#include "dev.h"
#include "dialogue.h"
#include "economy.h"
#include "env.h"
#include "event.h"
#include "faction.h"
#include "fleet.h"
#include "font.h"
#include "gui.h"
#include "hook.h"
#include "input.h"
#include "joystick.h"
#include "land.h"
#include "load.h"
#include "map.h"
#include "map_overlay.h"
#include "map_system.h"
#include "menu.h"
#include "mission.h"
#include "music.h"
#include "ndata.h"
#include "nebula.h"
#include "news.h"
#include "nfile.h"
#include "nlua_misn.h"
#include "nlua_var.h"
#include "npc.h"
#include "npng.h"
#include "nxml.h"
#include "opengl.h"
#include "options.h"
#include "outfit.h"
#include "pause.h"
#include "physics.h"
#include "pilot.h"
#include "player.h"
#include "rng.h"
#include "ship.h"
#include "slots.h"
#include "sound.h"
#include "space.h"
#include "spfx.h"
#include "start.h"
#include "tech.h"
#include "threadpool.h"
#include "toolkit.h"
#include "unidiff.h"
#include "weapon.h"
#include "semver.h"

#if defined ENABLE_NLS && ENABLE_NLS
#include <locale.h>
#endif /* defined ENABLE_NLS && ENABLE_NLS */

#define CONF_FILE       "conf.lua" /**< Configuration file by default. */
#define VERSION_FILE    "VERSION" /**< Version file by default. */

#define NAEV_INIT_DELAY 3000 /**< Minimum amount of time_ms to wait with loading screen */


static int quit               = 0; /**< For primary loop */
static unsigned int time_ms   = 0; /**< used to calculate FPS and movement. */
static glTexture *loading     = NULL; /**< Loading screen. */
static char *binary_path      = NULL; /**< argv[0] */
static SDL_Surface *naev_icon = NULL; /**< Icon. */
static int fps_skipped        = 0; /**< Skipped last frame? */
/* Version stuff. */
static semver_t version_binary; /**< Naev binary version. */
//static semver_t version_data; /**< Naev data version. */
static char version_human[256]; /**< Human readable version. */


/*
 * FPS stuff.
 */
static double fps_dt    = 1.; /**< Display fps accumulator. */
static double game_dt   = 0.; /**< Current game deltatick (uses dt_mod). */
static double real_dt   = 0.; /**< Real deltatick. */
const double fps_min    = 1./30.; /**< Minimum fps to run at. */
static double fps_x     =  15.; /**< FPS X position. */
static double fps_y     = -15.; /**< FPS Y position. */

#if HAS_LINUX && HAS_BFD && defined(DEBUGGING)
static bfd *abfd      = NULL;
static asymbol **syms = NULL;
#endif /* HAS_LINUX && HAS_BFD && defined(DEBUGGING) */

/*
 * prototypes
 */
/* Loading. */
static void print_SDLversion (void);
static void loadscreen_load (void);
static void loadscreen_unload (void);
static void load_all (void);
static void unload_all (void);
static void display_fps( const double dt );
static void window_caption (void);
static void debug_sigInit (void);
static void debug_sigClose (void);
/* update */
static void fps_init (void);
static double fps_elapsed (void);
static void fps_control (void);
static void update_all (void);
static void render_all (void);
/* Misc. */
void loadscreen_render( double done, const char *msg ); /* nebula.c */
void main_loop( int update ); /* dialogue.c */


/**
 * @brief Flags naev to quit.
 */
void naev_quit (void)
{
   quit = 1;
}


/**
 * @brief The entry point of Naev.
 *
 *    @param[in] argc Number of arguments.
 *    @param[in] argv Array of argc arguments.
 *    @return EXIT_SUCCESS on success.
 */
int main( int argc, char** argv )
{
   char buf[PATH_MAX], langbuf[PATH_MAX];

   env_detect( argc, argv );

   if (!log_isTerminal())
      log_copy(1);

   /* Save the binary path. */
   binary_path = strdup( env.argv0 );

#if defined ENABLE_NLS && ENABLE_NLS
   /* Set up locales. */
   /* When using locales with difference in '.' and ',' for splitting numbers it
    * causes pretty much everything to blow up, so we must refer from loading the
    * numeric type of the locale. */
   setlocale( LC_ALL, "" );
   setlocale( LC_NUMERIC, "C" ); /* Disable numeric locale part. */
   /* We haven't loaded the ndata yet, so just try a path quickly. */
   nsnprintf( langbuf, sizeof(langbuf), "%s/"GETTEXT_PATH, nfile_dirname(naev_binary()) );
   bindtextdomain( PACKAGE_NAME, langbuf );
   //bindtextdomain("naev", "po/");
   textdomain( PACKAGE_NAME );
#endif /* defined ENABLE_NLS && ENABLE_NLS */

   /* Parse version. */
   if (semver_parse( VERSION, &version_binary ))
      WARN( _("Failed to parse version string '%s'!"), VERSION );

   /* Print the version */
   LOG( " %s v%s (%s)", APPNAME, naev_version(0), HOST );
#ifdef GIT_COMMIT
   DEBUG( _(" git HEAD at %s"), GIT_COMMIT );
#endif /* GIT_COMMIT */

   if ( env.isAppImage )
      LOG( "AppImage detected. Running from: %s", env.appdir );
   else
      DEBUG( "AppImage not detected." );

   /* Initializes SDL for possible warnings. */
   if ( SDL_Init( 0 ) ) {
      ERR( _( "Unable to initialize SDL: %s" ), SDL_GetError() );
      return -1;
   }

   /* Initialize the threadpool */
   threadpool_init();

   /* Set up debug signal handlers. */
   debug_sigInit();

#if HAS_UNIX
   /* Set window class and name. */
   nsetenv("SDL_VIDEO_X11_WMCLASS", APPNAME, 0);
#endif /* HAS_UNIX */

   /* Must be initialized before input_init is called. */
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
      WARN( _("Unable to initialize SDL Video: %s"), SDL_GetError());
      return -1;
   }

   /* We'll be parsing XML. */
   LIBXML_TEST_VERSION
   xmlInitParser();

   /* Input must be initialized for config to work. */
   input_init();

   lua_init(); /* initializes lua */

   conf_setDefaults(); /* set the default config values */

   /*
    * Attempts to load the data path from datapath.lua
    * At this early point in the load process, the binary path
    * is the only place likely to be checked.
    */
   conf_loadConfigPath();

   /* Parse the user data path override first. */
   conf_parseCLIPath( argc, argv );

   /* Create the home directory if needed. */
   if ( nfile_dirMakeExist( nfile_configPath() ) )
      WARN( _("Unable to create config directory '%s'"), nfile_configPath());

   /* Set the configuration. */
   nsnprintf(buf, PATH_MAX, "%s"CONF_FILE, nfile_configPath());

   conf_loadConfig(buf); /* Lua to parse the configuration file */
   conf_parseCLI( argc, argv ); /* parse CLI arguments */

   if (conf.redirect_file && log_copying()) {
      log_redirect();
      log_copy(0);
   }
   else
      log_purge();

   /* Enable FPU exceptions. */
#if defined(HAVE_FEENABLEEXCEPT) && defined(DEBUGGING) && defined(_GNU_SOURCE)
   if (conf.fpu_except)
      feenableexcept( FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW );
#endif /* defined(HAVE_FEENABLEEXCEPT) && defined(DEBUGGING) && defined(_GNU_SOURCE) */

   /* Open data. */
   if (ndata_open() != 0)
      ERR( _("Failed to open ndata.") );

#if defined ENABLE_NLS && ENABLE_NLS
   /* Try to set the language again if Naev is attempting to override the locale stuff.
    * This is done late because this is the first stage at which we have the conf file
    * fully loaded.
    * Note: We tried setlocale( LC_ALL, ... ), but it bails if no corresponding system
    * locale exists. That's too restrictive when we only need our own language catalogs. */
   if (conf.language == NULL)
      nsetenv( "LANGUAGE", "", 0 );
   else {
      nsetenv( "LANGUAGE", conf.language, 1 );
      DEBUG(_("Reset language to \"%s\""), conf.language);
   }
   /* HACK: All of our code assumes it's working with UTF-8, so force gettext to return it.
    * Testing under Wine shows it may default to Windows-1252, resulting in glitched translations
    * like "Loading Ships..." -> "Schiffe laden \x85" which our font code will render improperly. */
   nsetenv( "OUTPUT_CHARSET", "utf-8", 1 );
   /* Horrible hack taken from https://www.gnu.org/software/gettext/manual/html_node/gettext-grok.html .
    * Not entirely sure it is necessary, but just in case... */
   {
      extern int  _nl_msg_cat_cntr;
      ++_nl_msg_cat_cntr;
   }
   /* If we don't disable LC_NUMERIC, lots of stuff blows up because 1,000 can be interpreted as
    * 1.0 in certain languages. */
   if (setlocale( LC_NUMERIC, "C" )==NULL) /* Disable numeric locale part. */
      WARN(_("Unable to set LC_NUMERIC to 'C'!"));
   nsnprintf( langbuf, sizeof(langbuf), "%s/"GETTEXT_PATH, ndata_getPath() );
   bindtextdomain( PACKAGE_NAME, langbuf );
   textdomain( PACKAGE_NAME );
#endif /* defined ENABLE_NLS && ENABLE_NLS */

   /* Load the start info. */
   if (start_load())
      ERR( _("Failed to load module start data.") );

   /* Load the data basics. */
   LOG(" %s", ndata_name());
   DEBUG_BLANK();

   /* Display the SDL Version. */
   print_SDLversion();
   DEBUG_BLANK();

   /* random numbers */
   rng_init();

   /*
    * OpenGL
    */
   if (gl_init()) { /* initializes video output */
      ERR( _("Initializing video output failed, exiting...") );
      SDL_Quit();
      exit(EXIT_FAILURE);
   }
   window_caption();

   /* Have to set up fonts before rendering anything. */
   //DEBUG("Using '%s' as main font and '%s' as monospace font.", _(FONT_DEFAULT_PATH), _(FONT_MONOSPACE_PATH));
   gl_fontInit( &gl_defFont, _(FONT_DEFAULT_PATH), conf.font_size_def, FONT_PATH_PREFIX, 0 ); /* initializes default font to size */
   gl_fontInit( &gl_smallFont, _(FONT_DEFAULT_PATH), conf.font_size_small, FONT_PATH_PREFIX, 0 ); /* small font */
   gl_fontInit( &gl_defFontMono, _(FONT_MONOSPACE_PATH), conf.font_size_def, FONT_PATH_PREFIX, 0 );

   /* Detect size changes that occurred after window creation. */
   naev_resize();

   /* Display the load screen. */
   loadscreen_load();
   loadscreen_render( 0., _("Initializing subsystems...") );
   time_ms = SDL_GetTicks();

   /*
    * Input
    */
   if ((conf.joystick_ind >= 0) || (conf.joystick_nam != NULL)) {
      if (joystick_init())
         WARN( _("Error initializing joystick input") );
      if (conf.joystick_nam != NULL) { /* use the joystick name to find a joystick */
         if (joystick_use(joystick_get(conf.joystick_nam))) {
            WARN( _("Failure to open any joystick, falling back to default keybinds") );
            input_setDefault(1);
         }
         free(conf.joystick_nam);
      }
      else if (conf.joystick_ind >= 0) /* use a joystick id instead */
         if (joystick_use(conf.joystick_ind)) {
            WARN( _("Failure to open any joystick, falling back to default keybinds") );
            input_setDefault(1);
         }
   }

   /*
    * OpenAL - Sound
    */
   if (conf.nosound) {
      LOG( _("Sound is disabled!") );
      sound_disabled = 1;
      music_disabled = 1;
   }
   if (sound_init())
      WARN( _("Problem setting up sound!") );
   music_choose("load");

   /* FPS stuff. */
   fps_setPos( 15., (double)(gl_screen.h-15-gl_defFont.h) );

   /* Misc graphics init */
   if (nebu_init() != 0) { /* Initializes the nebula */
      /* An error has happened */
      ERR( _("Unable to initialize the Nebula subsystem!") );
      /* Weirdness will occur... */
   }
   gui_init(); /* initializes the GUI graphics */
   toolkit_init(); /* initializes the toolkit */
   map_init(); /* initializes the map. */
   map_system_init(); /* Initialise the solar system map */
   cond_init(); /* Initialize conditional subsystem. */
   cli_init(); /* Initialize console. */

   /* Data loading */
   load_all();

   /* Detect size changes that occurred during load. */
   naev_resize();

   /* Generate the CSV. */
   if (conf.devcsv)
      dev_csv();

   /* Unload load screen. */
   loadscreen_unload();

   /* Start menu. */
   menu_main();

   LOG( _( "Reached main menu" ) );

   /* Force a minimum delay with loading screen */
   if ((SDL_GetTicks() - time_ms) < NAEV_INIT_DELAY)
      SDL_Delay( NAEV_INIT_DELAY - (SDL_GetTicks() - time_ms) );
   fps_init(); /* initializes the time_ms */


   /*
    * main loop
    */
   SDL_Event event;
   /* flushes the event loop since I noticed that when the joystick is loaded it
    * creates button events that results in the player starting out acceling */
   while (SDL_PollEvent(&event));

   /* Incomplete game note (shows every time version number changes). */
   if ( conf.lastversion == NULL || naev_versionCompare(conf.lastversion) != 0 ) {
      free( conf.lastversion );
      conf.lastversion = strdup( naev_version(0) );
      dialogue_msg(
         _("Welcome to Naev"),
         _("Welcome to Naev version %s, and thank you for playing! We hope you"
            " enjoy this game and all it has to offer. This is a passion"
            " project developed exclusively by volunteers and it gives us all"
            " great joy to know that there are others who love this game as"
            " much as we do!\n"
            "    Of course, please note that this is an incomplete game. You"
            " will encounter dead ends to storylines, missing storylines, and"
            " possibly even some bugs, although we try to keep those to a"
            " minimum of course. So be prepared for some rough edges for the"
            " time being. That said, we are working on this game every day and"
            " hope to one day finish this massive project on our hands."
            " Perhaps you could become one of us, who knows?\n"
            "    For more information about the game and its development"
            " state, take a look at naev.org; it has all the relevant links."
            " And again, thank you for playing!"), conf.lastversion );
   }

   /* primary loop */
   while (!quit) {
      while (SDL_PollEvent(&event)) { /* event loop */
         if (event.type == SDL_QUIT) {
            if (menu_askQuit()) {
               quit = 1; /* quit is handled here */
               break;
            }
         }
         else if (event.type == SDL_WINDOWEVENT &&
               event.window.event == SDL_WINDOWEVENT_RESIZED) {
            naev_resize();
            continue;
         }
         input_handle(&event); /* handles all the events and player keybinds */
      }

      main_loop( 1 );
   }

   /* Save configuration. */
   conf_saveConfig(buf);

   /* data unloading */
   unload_all();

   /* cleanup opengl fonts */
   gl_freeFont(NULL);
   gl_freeFont(&gl_smallFont);
   gl_freeFont(&gl_defFontMono);

   /* Close data. */
   ndata_close();
   start_cleanup();

   /* Destroy conf. */
   conf_cleanup(); /* Frees some memory the configuration allocated. */

   /* exit subsystems */
   cli_exit(); /* Clean up the console. */
   map_system_exit(); /* Destroys the solar system map. */
   map_exit(); /* Destroys the map. */
   ovr_mrkFree(); /* Clear markers. */
   toolkit_exit(); /* Kills the toolkit */
   ai_exit(); /* Stops the Lua AI magic */
   joystick_exit(); /* Releases joystick */
   input_exit(); /* Cleans up keybindings */
   nebu_exit(); /* Destroys the nebula */
   lua_exit(); /* Closes Lua state. */
   gl_exit(); /* Kills video output */
   sound_exit(); /* Kills the sound */
   news_exit(); /* Destroys the news. */

   /* Free the icon. */
   if (naev_icon)
      SDL_FreeSurface(naev_icon);

   SDL_Quit(); /* quits SDL */

   /* Clean up parser. */
   xmlCleanupParser();

   /* Clean up signal handler. */
   debug_sigClose();

   /* Last free. */
   free(binary_path);

   /* Delete logs if empty. */
   log_clean();

   /* all is well */
   exit(EXIT_SUCCESS);
}


/**
 * @brief Loads a loading screen.
 */
void loadscreen_load (void)
{
   unsigned int i;
   char file_path[PATH_MAX];
   char **loadscreens;
   size_t nload;

   /* Count the loading screens */
   loadscreens = ndata_list( GFX_PATH"loading/", &nload );

   /* Must have loading screens */
   if (nload==0) {
      WARN( _("No loading screens found!") );
      loading = NULL;
      return;
   }

   /* Set the zoom. */
   cam_setZoom( conf.zoom_far );

   /* Load the texture */
   nsnprintf( file_path, PATH_MAX, GFX_PATH"loading/%s", loadscreens[ RNG_BASE(0,nload-1) ] );
   loading = gl_newImage( file_path, 0 );

   /* Create the stars. */
   background_initStars( 1000 );

   /* Clean up. */
   for (i=0; i<nload; i++)
      free(loadscreens[i]);
   free(loadscreens);
}


/**
 * @brief Renders the load screen with message.
 *
 *    @param done Amount done (1. == completed).
 *    @param msg Loading screen message.
 */
void loadscreen_render( double done, const char *msg )
{
   const double SHIP_IMAGE_WIDTH = 512.;  /**< Loadscreen Ship Image Width */
   const double SHIP_IMAGE_HEIGHT = 512.; /**< Loadscreen Ship Image Height */
   glColour col;
   double bx;  /**<  Blit Image X Coord */
   double by;  /**<  Blit Image Y Coord */
   double x;   /**<  Progress Bar X Coord */
   double y;   /**<  Progress Bar Y Coord */
   double w;   /**<  Progress Bar Width Basis */
   double h;   /**<  Progress Bar Height Basis */
   double rh;  /**<  Loading Progress Text Relative Height */
   SDL_Event event;

   /* Clear background. */
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* Draw stars. */
   background_renderStars( 0. );

   /*
    * Dimensions.
    */
   /* Image. */
   bx = (SCREEN_W-SHIP_IMAGE_WIDTH)/2.;
   by = (SCREEN_H-SHIP_IMAGE_HEIGHT)/2.;
   /* Loading bar. */
   w  = SCREEN_W * 0.4;
   h  = SCREEN_H * 0.02;
   rh = h + gl_defFont.h + 4.;
   x  = (SCREEN_W-w)/2.;
   y  = (SCREEN_H-SHIP_IMAGE_HEIGHT)/2. - rh - 5.;

   /* Draw loading screen image. */
   if (loading != NULL)
      gl_blitScale( loading, bx, by, SHIP_IMAGE_WIDTH, SHIP_IMAGE_HEIGHT, NULL );

   /* Draw progress bar. */
   /* BG. */
   col.r = cBlack.r;
   col.g = cBlack.g;
   col.b = cBlack.b;
   col.a = 0.7;
   gl_renderRect( x-2., y-2., w+4., rh+4., &col );
   /* FG. */
   col.r = cGreen.r;
   col.g = cGreen.g;
   col.b = cGreen.b;
   col.a = 0.2;
   gl_renderRect( x+done*w, y, (1.-done)*w, h, &col );
   col.r = cPrimeGreen.r;
   col.g = cPrimeGreen.g;
   col.b = cPrimeGreen.b;
   col.a = 0.7;
   gl_renderRect( x, y, done*w, h, &col );

   /* Draw text. */
   gl_printRaw( &gl_defFont, x, y + h + 3., &cFontGreen, -1., msg );

   /* Get rid of events again. */
   while (SDL_PollEvent(&event));

   /* Flip buffers. HACK: Also try to catch a late-breaking resize from the WM (...or a crazy user?). */
   SDL_GL_SwapWindow( gl_screen.window );
   naev_resize();
}


/**
 * @brief Frees the loading screen.
 */
static void loadscreen_unload (void)
{
   gl_freeTexture(loading);
   loading = NULL;
}


/**
 * @brief Loads all the data, makes main() simpler.
 */
#define LOADING_STAGES     13. /**< Amount of loading stages. */
void load_all (void)
{
   /* We can do fast stuff here. */
   sp_load();

   /* order is very important as they're interdependent */
   loadscreen_render( 1./LOADING_STAGES, _("Loading Commodities...") );
   commodity_load(); /* dep for space */
   loadscreen_render( 2./LOADING_STAGES, _("Loading Factions...") );
   factions_load(); /* dep for fleet, space, missions, AI */
   loadscreen_render( 3./LOADING_STAGES, _("Loading AI...") );
   ai_load(); /* dep for fleets */
   loadscreen_render( 4./LOADING_STAGES, _("Loading Missions...") );
   missions_load(); /* no dep */
   loadscreen_render( 5./LOADING_STAGES, _("Loading Events...") );
   events_load(); /* no dep */
   loadscreen_render( 6./LOADING_STAGES, _("Loading Special Effects...") );
   spfx_load(); /* no dep */
   loadscreen_render( 6./LOADING_STAGES, _("Loading Damage Types...") );
   dtype_load(); /* no dep */
   loadscreen_render( 7./LOADING_STAGES, _("Loading Outfits...") );
   outfit_load(); /* dep for ships */
   loadscreen_render( 8./LOADING_STAGES, _("Loading Ships...") );
   ships_load(); /* dep for fleet */
   loadscreen_render( 9./LOADING_STAGES, _("Loading Fleets...") );
   fleet_load(); /* dep for space */
   loadscreen_render( 10./LOADING_STAGES, _("Loading Techs...") );
   tech_load(); /* dep for space */
   loadscreen_render( 11./LOADING_STAGES, _("Loading the Universe...") );
   space_load();
   loadscreen_render( 12./LOADING_STAGES, _("Loading the UniDiffs...") );
   diff_loadAvailable();
   loadscreen_render( 13./LOADING_STAGES, _("Populating Maps...") );
   outfit_mapParse();
   background_init();
   map_load();
   map_system_load();
   player_init(); /* Initialize player stuff. */
   loadscreen_render( 1., _("Loading Completed!") );
}
/**
 * @brief Unloads all data, simplifies main().
 */
void unload_all (void)
{
   /* cleanup some stuff */
   player_cleanup(); /* cleans up the player stuff */
   gui_free(); /* cleans up the player's GUI */
   weapon_exit(); /* destroys all active weapons */
   pilots_free(); /* frees the pilots, they were locked up :( */
   cond_exit(); /* destroy conditional subsystem. */
   land_exit(); /* Destroys landing vbo and friends. */
   npc_clear(); /* In case exiting while landed. */
   background_free(); /* Destroy backgrounds. */
   load_free(); /* Clean up loading game stuff stuff. */
   diff_free();
   economy_destroy(); /* must be called before space_exit */
   space_exit(); /* cleans up the universe itself */
   tech_free(); /* Frees tech stuff. */
   fleet_free();
   ships_free();
   outfit_free();
   spfx_free(); /* gets rid of the special effect */
   dtype_free(); /* gets rid of the damage types */
   missions_free();
   events_exit(); /* Clean up events. */
   factions_free();
   commodity_free();
   var_cleanup(); /* cleans up mission variables */
   sp_cleanup();
}


/**
 * @brief Split main loop from main() for secondary loop hack in toolkit.c.
 */
void main_loop( int update )
{
   /*
    * Control FPS.
    */
   fps_control(); /* everyone loves fps control */

   /*
    * Handle update.
    */
   input_update( real_dt ); /* handle key repeats. */
   sound_update( real_dt ); /* Update sounds. */
   if (toolkit_isOpen())
      toolkit_update(); /* to simulate key repetition */
   if (!paused && update) {
      /* Important that we pass real_dt here otherwise we get a dt feedback loop which isn't pretty. */
      player_updateAutonav( real_dt );
      update_all(); /* update game */
   }

   /*
    * Handle render.
    */
   /* Clear buffer. */
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   render_all();
   /* Toolkit is rendered on top. */
   if (toolkit_isOpen())
      toolkit_render();
   gl_checkErr(); /* check error every loop */
   /* Draw buffer. */
   SDL_GL_SwapWindow( gl_screen.window );
}


/**
 * @brief Wrapper for gl_resize that handles non-GL reinitialization.
 */
void naev_resize (void)
{
   /* Auto-detect window size. */
   int w, h;
   SDL_GL_GetDrawableSize( gl_screen.window, &w, &h );

   /* Update options menu, if open. (Never skip, in case the fullscreen mode alone changed.) */
   opt_resize();

   /* Nothing to do. */
   if ((w == gl_screen.rw) && (h == gl_screen.rh))
      return;

   /* Resize the GL context, etc. */
   gl_resize();

   /* Regenerate the background stars. */
   if (cur_system != NULL)
      background_initStars( cur_system->stars );
   else
      background_initStars( 1000 ); /* from loadscreen_load */

   /* Must be before gui_reload */
   fps_setPos( 15., (double)(SCREEN_H-15-gl_defFont.h) );

   /* Reload the GUI (may regenerate land window) */
   gui_reload();

   /* Resets the overlay dimensions. */
   ovr_refresh();

   if (nebu_isLoaded())
      nebu_vbo_init();

   /* Re-center windows. */
   toolkit_reposition();

   /* Reposition main menu, if open. */
   menu_main_resize();
}

/*
 * @brief Toggles between windowed and fullscreen mode.
 */
void naev_toggleFullscreen (void)
{
   opt_setVideoMode( conf.width, conf.height, !conf.fullscreen, 0 );
}


#if HAS_POSIX && defined(CLOCK_MONOTONIC)
static struct timespec global_time; /**< Global timestamp for calculating delta ticks. */
static int use_posix_time; /**< Whether or not to use POSIX time. */
#endif /* HAS_POSIX && defined(CLOCK_MONOTONIC) */
/**
 * @brief Initializes the fps engine.
 */
static void fps_init (void)
{
#if HAS_POSIX && defined(CLOCK_MONOTONIC)
   use_posix_time = 1;
   /* We must use clock_gettime here instead of gettimeofday mainly because this
    * way we are not influenced by changes to the time source like say ntp which
    * could skew up the dt calculations. */
   if (clock_gettime(CLOCK_MONOTONIC, &global_time)==0)
      return;
   WARN( _("clock_gettime failed, disabling POSIX time.") );
   use_posix_time = 0;
#endif /* HAS_POSIX && defined(CLOCK_MONOTONIC) */
   time_ms  = SDL_GetTicks();
}
/**
 * @brief Gets the elapsed time.
 *
 *    @return The elapsed time from the last frame.
 */
static double fps_elapsed (void)
{
   double dt;
   unsigned int t;

#if HAS_POSIX && defined(CLOCK_MONOTONIC)
   struct timespec ts;

   if (use_posix_time) {
      if (clock_gettime(CLOCK_MONOTONIC, &ts)==0) {
         dt  = ts.tv_sec - global_time.tv_sec;
         dt += (ts.tv_nsec - global_time.tv_nsec) / 1000000000.0;
         global_time = ts;
         return dt;
      }
      WARN( _("clock_gettime failed!") );
   }
#endif /* HAS_POSIX && defined(CLOCK_MONOTONIC) */

   t        = SDL_GetTicks();
   dt       = (double)(t - time_ms); /* Get the elapsed ms. */
   dt      /= 1000.; /* Convert to seconds. */
   time_ms  = t;

   return dt;
}


/**
 * @brief Controls the FPS.
 */
static void fps_control (void)
{
   double delay;
   double fps_max;
#if HAS_POSIX
   struct timespec ts;
#endif /* HAS_POSIX */

   /* dt in s */
   real_dt  = fps_elapsed();
   game_dt  = real_dt * dt_mod; /* Apply the modifier. */

   /* if fps is limited */
   if (!conf.vsync && conf.fps_max != 0) {
      fps_max = 1./(double)conf.fps_max;
      if (real_dt < fps_max) {
         delay    = fps_max - real_dt;
#if HAS_POSIX
         ts.tv_sec  = floor( delay );
         ts.tv_nsec = fmod( delay, 1. ) * 1e9;
         nanosleep( &ts, NULL );
#else /* HAS_POSIX */
         SDL_Delay( (unsigned int)(delay * 1000) );
#endif /* HAS_POSIX */
         fps_dt  += delay; /* makes sure it displays the proper fps */
      }
   }
}


/**
 * @brief Updates the game itself (player flying around and friends).
 *
 *    @brief Mainly uses game dt.
 */
static void update_all (void)
{
   int i, n;
   double nf, microdt, accumdt;

   if ((real_dt > 0.25) && (fps_skipped==0)) { /* slow timers down and rerun calculations */
      fps_skipped = 1;
      return;
   }
   else if (game_dt > fps_min) { /* we'll force a minimum FPS for physics to work alright. */

      /* Number of frames. */
      nf = ceil( game_dt / fps_min );
      microdt = game_dt / nf;
      n  = (int) nf;

      /* Update as much as needed, evenly. */
      accumdt = 0.;
      for (i=0; i<n; i++) {
         update_routine( microdt, 0 );
         /* OK, so we need a bit of hackish logic here in case we are chopping up a
          * very large dt and it turns out time compression changes so we're now
          * updating in "normal time compression" zone. This amounts to many updates
          * being run when time compression has changed and thus can cause, say, the
          * player to exceed their target position or get mauled by an enemy ship.
          */
         accumdt += microdt;
         if (accumdt > dt_mod*real_dt)
            break;
      }

      /* Note we don't touch game_dt so that fps_display works well */
   }
   else /* Standard, just update with the last dt */
      update_routine( game_dt, 0 );

   fps_skipped = 0;
}


/**
 * @brief Actually runs the updates
 *
 *    @param[in] dt Current delta tick.
 *    @param[in] enter_sys Whether this is the initial update upon entering the system.
 */
void update_routine( double dt, int enter_sys )
{
   if (!enter_sys) {
      hook_exclusionStart();

      /* Update time. */
      ntime_update( dt );
   }

   /* Update engine stuff. */
   space_update(dt);
   weapons_update(dt);
   spfx_update(dt);
   pilots_update(dt);

   /* Update camera. */
   cam_update( dt );

   if (!enter_sys)
      hook_exclusionEnd( dt );
}


/**
 * @brief Renders the game itself (player flying around and friends).
 *
 * Blitting order (layers):
 *   - BG
 *     - stars and planets
 *     - background player stuff (planet targeting)
 *     - background particles
 *     - back layer weapons
 *   - N
 *     - NPC ships
 *     - front layer weapons
 *     - normal layer particles (above ships)
 *   - FG
 *     - player
 *     - foreground particles
 *     - text and GUI
 */
static void render_all (void)
{
   double dt;

   dt = (paused) ? 0. : game_dt;

   /* setup */
   spfx_begin(dt, real_dt);
   /* BG */
   space_render(dt);
   planets_render();
   weapons_render(WEAPON_LAYER_BG, dt);
   /* N */
   pilots_render(dt);
   weapons_render(WEAPON_LAYER_FG, dt);
   spfx_render(SPFX_LAYER_BACK);
   /* FG */
   player_render(dt);
   spfx_render(SPFX_LAYER_FRONT);
   space_renderOverlay(dt);
   gui_renderReticles(dt);
   pilots_renderOverlay(dt);
   spfx_end();
   gui_render(dt);
   ovr_render(dt);
   display_fps( real_dt ); /* Exception. */
}


static double fps     = 0.; /**< FPS to finally display. */
static double fps_cur = 0.; /**< FPS accumulator to trigger change. */
/**
 * @brief Displays FPS on the screen.
 *
 *    @param[in] dt Current delta tick.
 */
static void display_fps( const double dt )
{
   double x,y;
   double dt_mod_base = 1.;

   fps_dt  += dt;
   fps_cur += 1.;
   if (fps_dt > 1.) { /* recalculate every second */
      fps = fps_cur / fps_dt;
      fps_dt = fps_cur = 0.;
   }

   x = fps_x;
   y = fps_y;
   if (conf.fps_show) {
      gl_print( NULL, x, y, NULL, "%3.2f", fps );
      y -= gl_defFont.h + 5.;
   }

   if ((player.p != NULL) && !player_isFlag(PLAYER_DESTROYED) &&
         !player_isFlag(PLAYER_CREATING)) {
      dt_mod_base = player_dt_default();
   }
   if (dt_mod != dt_mod_base)
      gl_print( NULL, x, y, NULL, "%3.1fx", dt_mod / dt_mod_base);

   if (!paused || !player_paused || !conf.pause_show)
      return;

   y = SCREEN_H / 3. - gl_defFontMono.h / 2.;
   gl_printMidRaw( &gl_defFontMono, SCREEN_W, 0., y,
         NULL, -1., _("PAUSED") );
}


/**
 * @brief Sets the position to display the FPS.
 */
void fps_setPos( double x, double y )
{
   fps_x = x;
   fps_y = y;
}


/**
 * @brief Sets the window caption.
 */
static void window_caption (void)
{
   char buf[PATH_MAX];
   SDL_RWops *rw;
   npng_t *npng;

   /* Load icon. */
   rw = ndata_rwops( GFX_PATH"icon.png" );
   if (rw == NULL) {
      WARN( _("Icon (icon.png) not found!") );
      return;
   }
   npng        = npng_open( rw );
   naev_icon   = npng_readSurface( npng, 0, 0 );
   npng_close( npng );
   SDL_RWclose( rw );
   if (naev_icon == NULL) {
      WARN( _("Unable to load icon.png!") );
      return;
   }

   /* Set caption. */
   nsnprintf(buf, PATH_MAX ,APPNAME" - %s", ndata_name());
   SDL_SetWindowTitle( gl_screen.window, buf );
   SDL_SetWindowIcon(  gl_screen.window, naev_icon );
}


/**
 * @brief Returns the version in a human readable string.
 *
 *    @param long_version Returns the long version if it's long.
 *    @return The human readable version string.
 */
char *naev_version( int long_version )
{
   /* Set up the long version. */
   if (long_version) {
      if (version_human[0] == '\0')
         nsnprintf( version_human, sizeof(version_human),
               " "APPNAME" v%s%s - %s", VERSION,
#ifdef DEBUGGING
               _(" debug"),
#else /* DEBUGGING */
               "",
#endif /* DEBUGGING */
               ndata_name() );
      return version_human;
   }

   return VERSION;
}


static int
binary_comparison (int x, int y) {
  if (x == y) return 0;
  if (x > y) return 1;
  return -1;
}
/**
 * @brief Compares the version against the current naev version.
 *
 *    @return positive if version is newer or negative if version is older.
 */
int naev_versionCompare( const char *version )
{
   int res;
   semver_t sv;

   if (semver_parse( version, &sv ))
      WARN( _("Failed to parse version string '%s'!"), version );

   if ((res = 3*binary_comparison(version_binary.major, sv.major)) == 0) {
      if ((res = 2*binary_comparison(version_binary.minor, sv.minor)) == 0) {
         res = semver_compare( version_binary, sv );
      }
   }
   semver_free( &sv );
   return res;
}


/**
 * @brief Returns the naev binary path.
 */
char *naev_binary (void)
{
   return binary_path;
}


/**
 * @brief Prints the SDL version to console.
 */
static void print_SDLversion (void)
{
   const SDL_version *linked;
   SDL_version compiled;
   unsigned int version_linked, version_compiled;

   /* Extract information. */
   SDL_VERSION(&compiled);
   SDL_version ll;
   SDL_GetVersion( &ll );
   linked = &ll;
   DEBUG( _("SDL: %d.%d.%d [compiled: %d.%d.%d]"),
         linked->major, linked->minor, linked->patch,
         compiled.major, compiled.minor, compiled.patch);

   /* Get version as number. */
   version_linked    = linked->major*100 + linked->minor;
   version_compiled  = compiled.major*100 + compiled.minor;

   /* Check if major/minor version differ. */
   if (version_linked > version_compiled)
      WARN( _("SDL is newer than compiled version") );
   if (version_linked < version_compiled)
      WARN( _("SDL is older than compiled version.") );
}


#if HAS_LINUX && HAS_BFD && defined(DEBUGGING)
/**
 * @brief Gets the string related to the signal code.
 *
 *    @param sig Signal to which code belongs.
 *    @param sig_code Signal code to get string of.
 *    @return String of signal code.
 */
static const char* debug_sigCodeToStr( int sig, int sig_code )
{
   if (sig == SIGFPE)
      switch (sig_code) {
         case SI_USER: return _("SIGFPE (raised by program)");
         case FPE_INTDIV: return _("SIGFPE (integer divide by zero)");
         case FPE_INTOVF: return _("SIGFPE (integer overflow)");
         case FPE_FLTDIV: return _("SIGFPE (floating-point divide by zero)");
         case FPE_FLTOVF: return _("SIGFPE (floating-point overflow)");
         case FPE_FLTUND: return _("SIGFPE (floating-point underflow)");
         case FPE_FLTRES: return _("SIGFPE (floating-point inexact result)");
         case FPE_FLTINV: return _("SIGFPE (floating-point invalid operation)");
         case FPE_FLTSUB: return _("SIGFPE (subscript out of range)");
         default: return _("SIGFPE");
      }
   else if (sig == SIGSEGV)
      switch (sig_code) {
         case SI_USER: return _("SIGSEGV (raised by program)");
         case SEGV_MAPERR: return _("SIGSEGV (address not mapped to object)");
         case SEGV_ACCERR: return _("SIGSEGV (invalid permissions for mapped object)");
         default: return _("SIGSEGV");
      }
   else if (sig == SIGABRT)
      switch (sig_code) {
         case SI_USER: return _("SIGABRT (raised by program)");
         default: return _("SIGABRT");
      }

   /* No suitable code found. */
   return strsignal(sig);
}

/**
 * @brief Translates and displays the address as something humans can enjoy.
 *
 * @TODO Remove the conditional defines which are to support old BFD (namely Ubuntu 16.04).
 */
static void debug_translateAddress( const char *symbol, bfd_vma address )
{
   const char *file, *func;
   unsigned int line;
   asection *section;

   for (section = abfd->sections; section != NULL; section = section->next) {
#ifdef bfd_get_section_flags
      if ((bfd_get_section_flags(abfd, section) & SEC_ALLOC) == 0)
#else /* bfd_get_section_flags */
      if ((bfd_section_flags(section) & SEC_ALLOC) == 0)
#endif /* bfd_get_section_flags */
         continue;

#ifdef bfd_get_section_vma
      bfd_vma vma = bfd_get_section_vma(abfd, section);
#else /* bfd_get_section_vma */
      bfd_vma vma = bfd_section_vma(section);
#endif /* bfd_get_section_vma */

#ifdef bfd_get_section_size
      bfd_size_type size = bfd_get_section_size(section);
#else /* bfd_get_section_size */
      bfd_size_type size = bfd_section_size(section);
#endif /* bfd_get_section_size */
      if (address < vma || address >= vma + size)
         continue;

      if (!bfd_find_nearest_line(abfd, section, syms, address - vma,
            &file, &func, &line))
         continue;

      do {
         if (func == NULL || func[0] == '\0')
            func = "??";
         if (file == NULL || file[0] == '\0')
            file = "??";
         DEBUG("%s %s(...):%u %s", symbol, func, line, file);
      } while (bfd_find_inliner_info(abfd, &file, &func, &line));

      return;
   }

   DEBUG("%s %s(...):%u %s", symbol, "??", 0, "??");
}


/**
 * @brief Backtrace signal handler for Linux.
 *
 *    @param sig Signal.
 *    @param info Signal information.
 *    @param unused Unused.
 */
static void debug_sigHandler( int sig, siginfo_t *info, void *unused )
{
   (void)sig;
   (void)unused;
   int i, num;
   void *buf[64];
   char **symbols;

   num      = backtrace(buf, 64);
   symbols  = backtrace_symbols(buf, num);

   DEBUG( _("Naev received %s!"),
         debug_sigCodeToStr(info->si_signo, info->si_code) );
   for (i=0; i<num; i++) {
      if (abfd != NULL)
         debug_translateAddress(symbols[i], (bfd_vma) (bfd_hostptr_t) buf[i]);
      else
         DEBUG("   %s", symbols[i]);
   }
   DEBUG( _("Report this to project maintainer with the backtrace.") );

   /* Always exit. */
   exit(1);
}
#endif /* HAS_LINUX && HAS_BFD && defined(DEBUGGING) */


/**
 * @brief Sets up the SignalHandler for Linux.
 */
static void debug_sigInit (void)
{
#if HAS_LINUX && HAS_BFD && defined(DEBUGGING)
   char **matching;
   struct sigaction sa, so;
   long symcount;
   unsigned int size;

   bfd_init();

   /* Read the executable */
   abfd = bfd_openr("/proc/self/exe", NULL);
   if (abfd != NULL) {
      bfd_check_format_matches(abfd, bfd_object, &matching);

      /* Read symbols */
      if (bfd_get_file_flags(abfd) & HAS_SYMS) {

         /* static */
         symcount = bfd_read_minisymbols (abfd, FALSE, (void **)&syms, &size);
         if ( symcount == 0 && abfd != NULL ) /* dynamic */
            symcount = bfd_read_minisymbols (abfd, TRUE, (void **)&syms, &size);
         assert(symcount >= 0);
      }
   }

   /* Set up handler. */
   sa.sa_handler   = NULL;
   sa.sa_sigaction = debug_sigHandler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags     = SA_SIGINFO;

   /* Attach signals. */
   sigaction(SIGSEGV, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( _("Unable to set up SIGSEGV signal handler.") );
   sigaction(SIGFPE, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( _("Unable to set up SIGFPE signal handler.") );
   sigaction(SIGABRT, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG( _("Unable to set up SIGABRT signal handler.") );
   DEBUG( _("BFD backtrace catching enabled.") );
#endif /* HAS_LINUX && HAS_BFD && defined(DEBUGGING) */
}


/**
 * @brief Closes the SignalHandler for Linux.
 */
static void debug_sigClose (void)
{
#if HAS_LINUX && HAS_BFD && defined(DEBUGGING)
   bfd_close( abfd );
#endif /* HAS_LINUX && HAS_BFD && defined(DEBUGGING) */
}
