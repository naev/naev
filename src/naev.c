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

#include "naev.h"
#include "log.h" /* for DEBUGGING */


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
#endif /* HAS_LINUX && HAS_BFD && defined(DEBUGGING) */

/* Locale setting. */
#include <locale.h>

/* local */
#include "conf.h"
#include "physics.h"
#include "opengl.h"
#include "font.h"
#include "ship.h"
#include "pilot.h"
#include "fleet.h"
#include "player.h"
#include "input.h"
#include "joystick.h"
#include "space.h"
#include "rng.h"
#include "ai.h"
#include "outfit.h"
#include "weapon.h"
#include "faction.h"
#include "nxml.h"
#include "toolkit.h"
#include "pause.h"
#include "sound.h"
#include "music.h"
#include "spfx.h"
#include "damagetype.h"
#include "economy.h"
#include "menu.h"
#include "mission.h"
#include "nlua_misn.h"
#include "nfile.h"
#include "nebula.h"
#include "unidiff.h"
#include "ndata.h"
#include "gui.h"
#include "news.h"
#include "nlua_var.h"
#include "map.h"
#include "event.h"
#include "cond.h"
#include "land.h"
#include "tech.h"
#include "hook.h"
#include "npc.h"
#include "console.h"
#include "npng.h"
#include "dev.h"
#include "background.h"
#include "camera.h"
#include "map_overlay.h"
#include "start.h"
#include "threadpool.h"
#include "load.h"
#include "options.h"
#include "dialogue.h"
#include "slots.h"


#define CONF_FILE       "conf.lua" /**< Configuration file by default. */
#define VERSION_FILE    "VERSION" /**< Version file by default. */

#define NAEV_INIT_DELAY 3000 /**< Minimum amount of time_ms to wait with loading screen */


static int quit               = 0; /**< For primary loop */
static unsigned int time_ms   = 0; /**< used to calculate FPS and movement. */
static char short_version[64]; /**< Contains version. */
static char human_version[256]; /**< Human readable version. */
static glTexture *loading     = NULL; /**< Loading screen. */
static char *binary_path      = NULL; /**< argv[0] */
static SDL_Surface *naev_icon = NULL; /**< Icon. */
static int fps_skipped        = 0; /**< Skipped last frame? */


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
   char buf[PATH_MAX];

   if (!log_isTerminal())
      log_copy(1);
#if HAS_WIN32
   else {
      /* Windows has no line-buffering, so use unbuffered output
       * when running from a terminal.
       */
      setvbuf( stdout, NULL, _IONBF, 0 );
      setvbuf( stderr, NULL, _IONBF, 0 );
   }
#endif

   /* Set up locales. */
   setlocale(LC_ALL, "");
   //bindtextdomain("naev", LOCALEDIR);
   bindtextdomain("naev", "po/");
   textdomain("naev");

   /* Save the binary path. */
   binary_path = strdup(argv[0]);

   /* Print the version */
   LOG( " %s v%s", APPNAME, naev_version(0) );
#ifdef GIT_COMMIT
   DEBUG( _(" git HEAD at %s"), GIT_COMMIT );
#endif /* GIT_COMMIT */

   /* Initializes SDL for possible warnings. */
   SDL_Init(0);

   /* Initialize the threadpool */
   threadpool_init();

   /* Set up debug signal handlers. */
   debug_sigInit();

#if HAS_UNIX
   /* Set window class and name. */
   setenv("SDL_VIDEO_X11_WMCLASS", APPNAME, 0);
#endif /* HAS_UNIX */

   /* Must be initialized before input_init is called. */
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
      WARN( _("Unable to initialize SDL Video: %s"), SDL_GetError());
      return -1;
   }

   /* Get desktop dimensions. */
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_DisplayMode current;
   SDL_GetCurrentDisplayMode( 0, &current );
   gl_screen.desktop_w = current.w;
   gl_screen.desktop_h = current.h;
#elif SDL_VERSION_ATLEAST(1,2,10)
   const SDL_VideoInfo *vidinfo = SDL_GetVideoInfo();
   gl_screen.desktop_w = vidinfo->current_w;
   gl_screen.desktop_h = vidinfo->current_h;
#else /* #elif SDL_VERSION_ATLEAST(1,2,10) */
   gl_screen.desktop_w = 0;
   gl_screen.desktop_h = 0;
#endif /* #elif SDL_VERSION_ATLEAST(1,2,10) */

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
   if (nfile_dirMakeExist("%s", nfile_configPath()))
      WARN( _("Unable to create config directory '%s'"), nfile_configPath());

   /* Set the configuration. */
   nsnprintf(buf, PATH_MAX, "%s"CONF_FILE, nfile_configPath());

#if HAS_MACOS
   /* TODO get rid of this cruft ASAP. */
   char oldconfig[PATH_MAX] = "";
   if (!nfile_fileExists( buf )) {
      char *home = SDL_getenv( "HOME" );
      if (home != NULL) {
         nsnprintf( oldconfig, PATH_MAX, "%s/.config/naev/"CONF_FILE, home );
         if (!nfile_fileExists( oldconfig ))
            oldconfig[0] = '\0';
      }
   }
#endif /* HAS_MACOS */

   conf_loadConfig(buf); /* Lua to parse the configuration file */
   conf_parseCLI( argc, argv ); /* parse CLI arguments */

   if (conf.redirect_file && log_copying()) {
      log_redirect();
      log_copy(0);
   }
   else
      log_purge();

   /* Enable FPU exceptions. */
#if defined(HAVE_FEENABLEEXCEPT) && defined(DEBUGGING)
   if (conf.fpu_except)
      feenableexcept( FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW );
#endif /* defined(HAVE_FEENABLEEXCEPT) && defined(DEBUGGING) */

   /* Open data. */
   if (ndata_open() != 0)
      ERR( _("Failed to open ndata.") );

   /* Load the start info. */
   if (start_load())
      ERR( _("Failed to load module start data.") );

   /* Load the data basics. */
   LOG(" %s", ndata_name());
   DEBUG("");

   /* Display the SDL Version. */
   print_SDLversion();
   DEBUG("");

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
   gl_fontInit( NULL, "Arial", FONT_DEFAULT_PATH, conf.font_size_def ); /* initializes default font to size */
   gl_fontInit( &gl_smallFont, "Arial", FONT_DEFAULT_PATH, conf.font_size_small ); /* small font */
   gl_fontInit( &gl_defFontMono, "Monospace", FONT_MONOSPACE_PATH, conf.font_size_def );

#if SDL_VERSION_ATLEAST(2,0,0)
   /* Detect size changes that occurred after window creation. */
   naev_resize( -1., -1. );
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

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
   cond_init(); /* Initialize conditional subsystem. */
   cli_init(); /* Initialize console. */

   /* Data loading */
   load_all();

#if SDL_VERSION_ATLEAST(2,0,0)
   /* Detect size changes that occurred during load. */
   naev_resize( -1., -1. );
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

   /* Generate the CSV. */
   if (conf.devcsv)
      dev_csv();

   /* Unload load screen. */
   loadscreen_unload();

   /* Start menu. */
   menu_main();

   /* Force a minimum delay with loading screen */
   if ((SDL_GetTicks() - time_ms) < NAEV_INIT_DELAY)
      SDL_Delay( NAEV_INIT_DELAY - (SDL_GetTicks() - time_ms) );
   fps_init(); /* initializes the time_ms */

#if HAS_MACOS
   /* Tell the player to migrate their configuration files */
   /* TODO get rid of this cruft ASAP. */
   if ((oldconfig[0] != '\0') && (!conf.datapath)) {
      char path[PATH_MAX], *script, *home;
      size_t scriptsize;
      int ret;

      nsnprintf( path, PATH_MAX, "%s/naev-confupdate.sh", ndata_getDirname() );
      home = SDL_getenv("HOME");
      ret = dialogue_YesNo( _("Warning"), _("Your configuration files are in a deprecated location and must be migrated:\n"
            "   \ar%s\a0\n\n"
            "The update script can likely be found in your Naev data directory:\n"
            "   \ar%s\a0\n\n"
            "Would you like to run it automatically?"), oldconfig, path );

      /* Try to run the script. */
      if (ret) {
         ret = -1;
         /* Running from ndata. */
         if (ndata_getPath() != NULL) {
            script = ndata_read( "naev-confupdate.sh", &scriptsize );
            if (script != NULL)
               ret = system(script);
         }

         /* Running from laid-out files or ndata_read failed. */
         if ((nfile_fileExists(path)) && (ret == -1)) {
            script = nfile_readFile( &scriptsize, path );
            if (script != NULL)
               ret = system(script);
         }

         /* We couldn't find the script. */
         if (ret == -1) {
            dialogue_alert( _("The update script was not found at:\n\ar%s\a0\n\n"
                  "Please locate and run it manually."), path );
         }
         /* Restart, as the script succeeded. */
         else if (!ret) {
            dialogue_msg( _("Update Completed"),
                  _("Configuration files were successfully migrated. Naev will now restart.") );
            execv(argv[0], argv);
         }
         else { /* I sincerely hope this else is never hit. */
            dialogue_alert( _("The update script encountered an error. Please exit Naev and move your config and save files manually:\n\n"
                  "\ar%s/%s\a0 =>\n   \aD%s\a0\n\n"
                  "\ar%s/%s\a0 =>\n   \aD%s\a0\n\n"
                  "\ar%s/%s\a0 =>\n   \aD%snebula/\a0\n\n"),
                  home, ".naev/conf.lua", nfile_configPath(),
                  home, ".naev/{saves,screenshots}/", nfile_dataPath(),
                  home, ".naev/gen/*.png", nfile_cachePath() );
         }
      }
      else {
         dialogue_alert(
               _("To manually migrate your configuration files "
               "please exit Naev and run the update script, "
               "likely found in your Naev data directory:\n"
               "   \ar%s/naev-confupdate.sh\a0"), home, path );
      }
   }
#endif /* HAS_MACOS */

   /*
    * main loop
    */
   SDL_Event event;
   /* flushes the event loop since I noticed that when the joystick is loaded it
    * creates button events that results in the player starting out acceling */
   while (SDL_PollEvent(&event));
   /* primary loop */
   while (!quit) {
      while (SDL_PollEvent(&event)) { /* event loop */
         if (event.type == SDL_QUIT) {
            if (menu_askQuit()) {
               quit = 1; /* quit is handled here */
               break;
            }
         }
#if SDL_VERSION_ATLEAST(2,0,0)
         else if (event.type == SDL_WINDOWEVENT &&
               event.window.event == SDL_WINDOWEVENT_RESIZED) {
            naev_resize( event.window.data1, event.window.data2 );
            continue;
         }
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
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
   nsnprintf( file_path, PATH_MAX, GFX_PATH"loading/%s", loadscreens[ RNG_SANE(0,nload-1) ] );
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
   glColour col;
   double bx,by, bw,bh;
   double x,y, w,h, rh;
   SDL_Event event;

   /* Clear background. */
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* Draw stars. */
   background_renderStars( 0. );

   /*
    * Dimensions.
    */
   /* Image. */
   bw = 512.;
   bh = 512.;
   bx = (SCREEN_W-bw)/2.;
   by = (SCREEN_H-bh)/2.;
   /* Loading bar. */
   w  = gl_screen.w * 0.4;
   h  = gl_screen.h * 0.02;
   rh = h + gl_defFont.h + 4.;
   x  = (SCREEN_W-w)/2.;
   if (SCREEN_H < 768)
      y  = (SCREEN_H-h)/2.;
   else
      y  = (SCREEN_H-bw)/2 - rh - 5.;

   /* Draw loading screen image. */
   if (loading != NULL)
      gl_blitScale( loading, bx, by, bw, bh, NULL );

   /* Draw progress bar. */
   /* BG. */
   col.r = cBlack.r;
   col.g = cBlack.g;
   col.b = cBlack.b;
   col.a = 0.7;
   gl_renderRect( x-2., y-2., w+4., rh+4., &col );
   /* FG. */
   col.r = cDConsole.r;
   col.g = cDConsole.g;
   col.b = cDConsole.b;
   col.a = 0.2;
   gl_renderRect( x+done*w, y, (1.-done)*w, h, &col );
   col.r = cConsole.r;
   col.g = cConsole.g;
   col.b = cConsole.b;
   col.a = 0.7;
   gl_renderRect( x, y, done*w, h, &col );

   /* Draw text. */
   gl_printRaw( &gl_defFont, x, y + h + 3., &cConsole, msg );

   /* Flip buffers. */
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_GL_SwapWindow( gl_screen.window );
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   SDL_GL_SwapBuffers();
#endif /* SDL_VERSION_ATLEAST(2,0,0) */

   /* Get rid of events again. */
   while (SDL_PollEvent(&event));
}


/**
 * @brief Frees the loading screen.
 */
static void loadscreen_unload (void)
{
   /* Free the textures */
   if (loading != NULL)
      gl_freeTexture(loading);
   loading = NULL;
}


/**
 * @brief Loads all the data, makes main() simpler.
 */
#define LOADING_STAGES     12. /**< Amount of loading stages. */
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
   loadscreen_render( 12./LOADING_STAGES, _("Populating Maps...") );
   outfit_mapParse();
   background_init();
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
   economy_destroy(); /* must be called before space_exit */
   space_exit(); /* cleans up the universe itself */
   tech_free(); /* Frees tech stuff. */
   fleet_free();
   ships_free();
   outfit_free();
   spfx_free(); /* gets rid of the special effect */
   dtype_free(); /* gets rid of the damage types */
   missions_free();
   events_cleanup(); /* Clean up events. */
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
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_GL_SwapWindow( gl_screen.window );
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   SDL_GL_SwapBuffers();
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
}


#if SDL_VERSION_ATLEAST(2,0,0)
/**
 * @brief Wrapper for gl_resize that handles non-GL reinitialization.
 */
void naev_resize( int w, int h )
{
   /* Auto-detect window size. */
   if ((w < 0.) && (h < 0.))
      SDL_GetWindowSize( gl_screen.window, &w, &h );

   /* Nothing to do. */
   if ((w == gl_screen.rw) && (h == gl_screen.rh))
      return;

   /* Resize the GL context, etc. */
   gl_resize( w, h );

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

   /* Update options menu, if open. */
   opt_resize();
}

/*
 * @brief Toggles between windowed and fullscreen mode.
 */
void naev_toggleFullscreen (void)
{
   int w, h, mode;
   SDL_DisplayMode current;

   /* @todo Remove code duplication between this and opt_videoSave */
   if (conf.fullscreen) {
      conf.fullscreen = 0;
      /* Restore windowed mode. */
      SDL_SetWindowFullscreen( gl_screen.window, 0 );

      SDL_SetWindowSize( gl_screen.window, conf.width, conf.height );
      naev_resize( conf.width, conf.height );
      SDL_SetWindowPosition( gl_screen.window,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );

      return;
   }

   conf.fullscreen = 1;

   if (conf.modesetting) {
      mode = SDL_WINDOW_FULLSCREEN;

      SDL_GetWindowDisplayMode( gl_screen.window, &current );

      current.w = conf.width;
      current.h = conf.height;

      SDL_SetWindowDisplayMode( gl_screen.window, &current );
   }
   else
      mode = SDL_WINDOW_FULLSCREEN_DESKTOP;

   SDL_SetWindowFullscreen( gl_screen.window, mode );

   SDL_GetWindowSize( gl_screen.window, &w, &h );
   if ((w != conf.width) || (h != conf.height))
      naev_resize( w, h );
}
#endif /* SDL_VERSION_ATLEAST(2,0,0) */


#if HAS_POSIX && defined(CLOCK_MONOTONIC)
static struct timespec global_time; /**< Global timestamp for calculating delta ticks. */
static int use_posix_time; /**< Whether or not to use posix time. */
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
   WARN( _("clock_gettime failed, disabling posix time.") );
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
         /* Ok, so we need a bit of hackish logic here in case we are chopping up a
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
   if (dt_mod != 1.)
      gl_print( NULL, x, y, NULL, "%3.1fx", dt_mod);

   if (!paused || !player_paused || !conf.pause_show)
      return;

   y = SCREEN_H / 3. - gl_defFontMono.h / 2.;
   gl_printMidRaw( &gl_defFontMono, SCREEN_W, 0., y,
         NULL, _("PAUSED") );
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
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_SetWindowTitle( gl_screen.window, buf );
   SDL_SetWindowIcon(  gl_screen.window, naev_icon );
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   SDL_WM_SetCaption(buf, APPNAME);
   SDL_WM_SetIcon( naev_icon, NULL );
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
}

/**
 * @Brief Gets a short human readable string of the version.
 *
 *    @param[out] str String to output.
 *    @param slen Maximum length of the string.
 *    @param major Major version.
 *    @param minor Minor version.
 *    @param rev Revision.
 *    @return Number of characters written.
 */
int naev_versionString( char *str, size_t slen, int major, int minor, int rev )
{
   int n;
   if (rev<0)
      n = nsnprintf( str, slen, "%d.%d.0-beta%d", major, minor, ABS(rev) );
   else
      n = nsnprintf( str, slen, "%d.%d.%d", major, minor, rev );
   return n;
}


/**
 * @brief Returns the version in a human readable string.
 *
 *    @param long_version Returns the long version if it's long.
 *    @return The human readable version string.
 */
char *naev_version( int long_version )
{
   /* Set short version if needed. */
   if (short_version[0] == '\0')
      naev_versionString( short_version, sizeof(short_version), VMAJOR, VMINOR, VREV );

   /* Set up the long version. */
   if (long_version) {
      if (human_version[0] == '\0')
         nsnprintf( human_version, sizeof(human_version),
               " "APPNAME" v%s%s - %s", short_version,
#ifdef DEBUGGING
               _(" debug"),
#else /* DEBUGGING */
               "",
#endif /* DEBUGGING */
               ndata_name() );
      return human_version;
   }

   return short_version;
}


/**
 * @brief Parses the naev version.
 *
 *    @param[out] version Version parsed.
 *    @param buf Buffer to parse.
 *    @param nbuf Length of the buffer to parse.
 *    @return 0 on success.
 */
int naev_versionParse( int version[3], char *buf, int nbuf )
{
   int i, j, s;
   char cbuf[64];

   /* Check length. */
   if (nbuf > (int)sizeof(cbuf)) {
      WARN( _("Version format is too long!") );
      return -1;
   }

   s = 0;
   j = 0;
   for (i=0; i < MIN(nbuf,(int)sizeof(cbuf)); i++) {
      cbuf[j++] = buf[i];
      if (buf[i] == '.') {
         cbuf[j] = '\0';
         version[s++] = atoi(cbuf);
         if (s >= 3) {
            WARN( _("Version has too many '.'.") );
            return -1;
         }
         j = 0;
      }
   }
   if (s<3) {
      cbuf[j++] = '\0';
      version[s++] = atoi(cbuf);
   }

   return 0;
}


/**
 * @brief Compares the version against the current naev version.
 *
 *    @return positive if version is newer or negative if version is older.
 */
int naev_versionCompare( int version[3] )
{
   if (VMAJOR > version[0])
      return -3;
   else if (VMAJOR < version[0])
      return +3;

   if (VMINOR > version[1])
      return -2;
   else if (VMINOR < version[1])
      return +2;

   if (VREV > version[2])
      return -1;
   else if (VREV < version[2])
      return +1;

   return 0;
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
#if SDL_VERSION_ATLEAST(2,0,0)
   SDL_version ll;
   SDL_GetVersion( &ll );
   linked = &ll;
#else /* SDL_VERSION_ATLEAST(2,0,0) */
   linked = SDL_Linked_Version();
#endif /* SDL_VERSION_ATLEAST(2,0,0) */
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
 */
static void debug_translateAddress( const char *symbol, bfd_vma address )
{
   const char *file, *func;
   unsigned int line;
   asection *section;

   for (section = abfd->sections; section != NULL; section = section->next) {
      if ((bfd_get_section_flags(abfd, section) & SEC_ALLOC) == 0)
         continue;

      bfd_vma vma = bfd_get_section_vma(abfd, section);
      bfd_size_type size = bfd_get_section_size(section);
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
         if (symcount == 0) /* dynamic */
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
