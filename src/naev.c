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
/** @cond */
#include "linebreak.h"
#include "physfsrwops.h"
#include "SDL.h"
#include "SDL_error.h"
#include "SDL_image.h"

#include "naev.h"

#if HAS_POSIX
#include <time.h>
#include <unistd.h>
#endif /* HAS_POSIX */
/** @endcond */

#include "ai.h"
#include "background.h"
#include "camera.h"
#include "cond.h"
#include "conf.h"
#include "console.h"
#include "damagetype.h"
#include "debug.h"
#include "dialogue.h"
#include "difficulty.h"
#include "economy.h"
#include "env.h"
#include "event.h"
#include "faction.h"
#include "font.h"
#include "gui.h"
#include "hook.h"
#include "input.h"
#include "joystick.h"
#include "land.h"
#include "load.h"
#include "log.h"
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
#include "nlua_tex.h"
#include "nlua_colour.h"
#include "nlua_gfx.h"
#include "nlua_naev.h"
#include "nlua_rnd.h"
#include "nlua_vec2.h"
#include "nlua_file.h"
#include "nlua_data.h"
#include "npc.h"
#include "nstring.h"
#include "nxml.h"
#include "opengl.h"
#include "options.h"
#include "outfit.h"
#include "pause.h"
#include "physics.h"
#include "pilot.h"
#include "player.h"
#include "plugin.h"
#include "render.h"
#include "rng.h"
#include "safelanes.h"
#include "semver.h"
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

#define VERSION_FILE    "VERSION" /**< Version file by default. */

static int quit               = 0; /**< For primary loop */
Uint32 SDL_LOOPDONE           = 0; /**< For custom event to exit loops. */
static unsigned int time_ms   = 0; /**< used to calculate FPS and movement. */
static SDL_Surface *naev_icon = NULL; /**< Icon. */
static int fps_skipped        = 0; /**< Skipped last frame? */
/* Version stuff. */
static semver_t version_binary; /**< Naev binary version. */

/*
 * FPS stuff.
 */
static double fps_dt    = 1.; /**< Display fps accumulator. */
static double game_dt   = 0.; /**< Current game deltatick (uses dt_mod). */
static double real_dt   = 0.; /**< Real deltatick. */
static double fps       = 0.; /**< FPS to finally display. */
static double fps_cur   = 0.; /**< FPS accumulator to trigger change. */
static double fps_x     =  15.; /**< FPS X position. */
static double fps_y     = -15.; /**< FPS Y position. */
const double fps_min    = 1./25.; /**< Minimum fps to run at. 1/25 seems to
                                       be acceptable value for fast ships. 1/15
                                       can cause issues with hyenas and such. */
double elapsed_time_mod = 0.; /**< Elapsed modified time. */

static nlua_env load_env = LUA_NOREF; /**< Environment for displaying load messages and stuff. */
static int load_force_render = 0;
static unsigned int load_last_render = 0;

/*
 * prototypes
 */
/* Loading. */
static void print_SDLversion (void);
static void loadscreen_load (void);
static void loadscreen_unload (void);
static void load_all (void);
static void unload_all (void);
static void window_caption (void);
/* update */
static void fps_init (void);
static double fps_elapsed (void);
static void fps_control (void);
static void update_all( int dohooks );
/* Misc. */
static void loadscreen_update( double done, const char *msg );
void main_loop( int nested ); /* externed in dialogue.c */

/**
 * @brief Flags naev to quit.
 */
void naev_quit (void)
{
   quit = 1;
}

/**
 * @brief Get if Naev is trying to quit.
 */
int naev_isQuit (void)
{
   return quit;
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
   char conf_file_path[PATH_MAX], **search_path;
   Uint32 starttime;

#ifdef DEBUGGING
   /* Set Debugging flags. */
   memset( debug_flags , 0, DEBUG_FLAGS_MAX );
#endif /* DEBUGGING */

   env_detect( argc, argv );

   log_init();

   /* Set up PhysicsFS. */
   if (PHYSFS_init( env.argv0 ) == 0) {
      ERR( "PhysicsFS initialization failed: %s",
            _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      return -1;
   }
   PHYSFS_permitSymbolicLinks( 1 );

   /* Set up locales. */
   gettext_init();
   init_linebreak();

   /* Parse version. */
   if (semver_parse( naev_version( 0 ), &version_binary ))
      WARN( _("Failed to parse version string '%s'!"), naev_version( 0 ) );

   /* Print the version */
   LOG( " %s v%s (%s)", APPNAME, naev_version(0), HOST );

   if (env.isAppImage)
      LOG( "AppImage detected. Running from: %s", env.appdir );
   else
      DEBUG( "AppImage not detected." );

   /* Initializes SDL for possible warnings. */
   if (SDL_Init( 0 )) {
      ERR( _( "Unable to initialize SDL: %s" ), SDL_GetError() );
      return -1;
   }
   starttime = SDL_GetTicks();
   SDL_LOOPDONE = SDL_RegisterEvents(1);

   /* Initialize the threadpool */
   threadpool_init();

   /* Set up debug signal handlers. */
   debug_sigInit();

#if HAS_UNIX
   /* Set window class and name. */
   SDL_setenv("SDL_VIDEO_X11_WMCLASS", APPNAME, 0);
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

   /* Create the home directory if needed. */
   if (nfile_dirMakeExist( nfile_configPath() ))
      WARN( _("Unable to create config directory '%s'"), nfile_configPath());

   /* Set the configuration. */
   snprintf(conf_file_path, sizeof(conf_file_path), "%s"CONF_FILE, nfile_configPath());

   conf_loadConfig(conf_file_path); /* Lua to parse the configuration file */
   conf_parseCLI( argc, argv ); /* parse CLI arguments */

   /* Set up I/O. */
   ndata_setupWriteDir();
   log_redirect();
   ndata_setupReadDirs();
   gettext_setLanguage( conf.language ); /* now that we can find translations */
   LOG( _("Loaded configuration: %s"), conf_file_path );
   search_path = PHYSFS_getSearchPath();
   LOG( "%s", _("Read locations, searched in order:") );
   for (char **p = search_path; *p != NULL; p++)
      LOG( "    %s", *p );
   PHYSFS_freeList( search_path );
   /* Logging the cache path is noisy, noisy is good at the DEBUG level. */
   DEBUG( _("Cache location: %s"), nfile_cachePath() );
   LOG( _("Write location: %s\n"), PHYSFS_getWriteDir() );

   /* Enable FPU exceptions. */
   if (conf.fpu_except)
      debug_enableFPUExcept();

   /* Load the start info. */
   if (start_load())
      ERR( _("Failed to load module start data.") );
   LOG(" %s", start_name());
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
      ERR( _("Initializing video output failed, exiting…") );
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
   loadscreen_update( 0., _("Initializing subsystems…") );
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
   fps_setPos( 15., (double)(gl_screen.h-15-gl_defFontMono.h) );

   /* Misc graphics init */
   render_init();
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

   /* Unload load screen. */
   loadscreen_unload();

   /* Start menu. */
   menu_main();

   if (conf.devmode)
      LOG( _( "Reached main menu in %.3f s" ), (SDL_GetTicks()-starttime)/1000. );
   else
      LOG( _( "Reached main menu" ) );

   fps_init(); /* initializes the time_ms */

   /*
    * main loop
    */
   SDL_Event event;
   /* flushes the event loop since I noticed that when the joystick is loaded it
    * creates button events that results in the player starting out acceling */
   while (SDL_PollEvent(&event));

   /* Show plugin compatibility. */
   plugin_check();

   /* Incomplete translation note (shows once if we pick an incomplete translation based on user's locale). */
   if ( !conf.translation_warning_seen && conf.language == NULL ) {
      const char* language = gettext_getLanguage();
      double coverage = gettext_languageCoverage(language);

      if (coverage < 0.8) {
         conf.translation_warning_seen = 1;
         dialogue_msg(
               _("Incomplete Translation"),
               _("%s is partially translated (%.0f%%) into your language (%s),"
                  " but the remaining text will be English. Language settings"
                  " are available in the \"%s\" screen."),
               APPNAME, 100.*coverage, language, _("Options") );
      }
   }

   /* Incomplete game note (shows every time version number changes). */
   if (conf.lastversion == NULL || naev_versionCompare(conf.lastversion) != 0) {
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
      while (!quit && SDL_PollEvent(&event)) { /* event loop */
         if (event.type == SDL_QUIT) {
            SDL_FlushEvent( SDL_QUIT ); /* flush event to prevent it from quitting when lagging a bit. */
            if (quit || menu_askQuit()) {
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

      main_loop( 0 );
   }

   /* Save configuration. */
   conf_saveConfig(conf_file_path);

   /* data unloading */
   unload_all();

   /* cleanup opengl fonts */
   gl_freeFont(NULL);
   gl_freeFont(&gl_smallFont);
   gl_freeFont(&gl_defFontMono);

   start_cleanup(); /* Cleanup from start.c, not the first cleanup step. :) */

   /* exit subsystems */
   plugin_exit();
   cli_exit(); /* Clean up the console. */
   map_system_exit(); /* Destroys the solar system map. */
   map_exit(); /* Destroys the map. */
   ovr_mrkFree(); /* Clear markers. */
   toolkit_exit(); /* Kills the toolkit */
   ai_exit(); /* Stops the Lua AI magic */
   joystick_exit(); /* Releases joystick */
   input_exit(); /* Cleans up keybindings */
   nebu_exit(); /* Destroys the nebula */
   render_exit(); /* Cleans up post-processing. */
   news_exit(); /* Destroys the news. */
   difficulty_free(); /* Clean up difficulties. */
   music_exit(); /* Kills Lua state. */
   lua_exit(); /* Closes Lua state, and invalidates all Lua. */
   sound_exit(); /* Kills the sound */
   gl_exit(); /* Kills video output */

   /* Has to be run last or it will mess up sound settings. */
   conf_cleanup(); /* Free some memory the configuration allocated. */

   /* Free the icon. */
   if (naev_icon)
      SDL_FreeSurface(naev_icon);

   IMG_Quit(); /* quits SDL_image */
   SDL_Quit(); /* quits SDL */

   /* Clean up parser. */
   xmlCleanupParser();
   xmlMemoryDump();

   /* Clean up signal handler. */
   debug_sigClose();

   /* Delete logs if empty. */
   log_clean();

   /* Really turn the lights off. */
   PHYSFS_deinit();
   gl_fontExit();
   gettext_exit();

   /* all is well */
   debug_enableLeakSanitizer();
   return 0;
}

/**
 * @brief Loads a loading screen.
 */
void loadscreen_load (void)
{
   int r;

   load_env = nlua_newEnv();
   r  = nlua_loadStandard( load_env );
   r |= nlua_loadNaev( load_env );
   r |= nlua_loadRnd( load_env );
   r |= nlua_loadVector( load_env );
   r |= nlua_loadFile( load_env );
   r |= nlua_loadData( load_env );
   r |= nlua_loadTex( load_env );
   r |= nlua_loadCol( load_env );
   r |= nlua_loadGFX( load_env );
   if (r)
      WARN(_("Something went wrong when loading Lua libraries for '%s'!"), LOADSCREEN_DATA_PATH);

   size_t bufsize;
   char *buf = ndata_read( LOADSCREEN_DATA_PATH, &bufsize );
   if (nlua_dobufenv(load_env, buf, bufsize, LOADSCREEN_DATA_PATH) != 0) {
      WARN( _("Error loading file: %s\n"
            "%s\n"
            "Most likely Lua file has improper syntax, please check"),
            LOADSCREEN_DATA_PATH, lua_tostring(naevL,-1));
      free(buf);
      return;
   }
   free(buf);
}

/**
 * @brief Renders the loadscreen if necessary.
 */
void naev_renderLoadscreen (void)
{
   SDL_Event event;
   unsigned int t = SDL_GetTicks();

   /* Only render if forced or try for low 10 FPS. */
   if (!load_force_render && (t-load_last_render) < 100 )
      return;
   load_last_render = t;

   /* Clear background. */
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* Run Lua. */
   nlua_getenv( naevL, load_env, "render" );
   if (nlua_pcall(load_env, 0, 0)) { /* error has occurred */
      WARN( _("Loadscreen '%s': '%s'"), "render", lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }

   /* Get rid of events again. */
   while (SDL_PollEvent(&event));

   /* Flip buffers. HACK: Also try to catch a late-breaking resize from the WM (...or a crazy user?). */
   SDL_GL_SwapWindow( gl_screen.window );
   naev_resize();

   /* Clear forcing. */
   load_force_render = 0;
}

/**
 * @brief Renders the load screen with message.
 *
 *    @param done Amount done (1. == completed).
 *    @param msg Loading screen message.
 */
void loadscreen_update( double done, const char *msg )
{
   /* Run Lua. */
   nlua_getenv( naevL, load_env, "update" );
   lua_pushnumber( naevL, done );
   lua_pushstring( naevL, msg );
   if (nlua_pcall(load_env, 2, 0)) { /* error has occurred */
      WARN( _("Loadscreen '%s': '%s'"), "update", lua_tostring(naevL,-1));
      lua_pop(naevL,1);
   }

   /* Force rerender. */
   load_force_render = 1;
   naev_renderLoadscreen();
}

/**
 * @brief Frees the loading screen.
 */
static void loadscreen_unload (void)
{
   nlua_freeEnv( load_env );
}

/**
 * @brief Loads all the data, makes main() simpler.
 */
#define LOADING_STAGES     17. /**< Amount of loading stages. */
void load_all (void)
{
   int stage = 0;
   /* We can do fast stuff here. */
   sp_load();

   /* order is very important as they're interdependent */
   loadscreen_update( ++stage/LOADING_STAGES, _("Loading Commodities…") );
   commodity_load(); /* dep for space */

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading Special Effects…") );
   spfx_load(); /* no dep */

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading Effects…") );
   effect_load(); /* no dep */

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading Damage Types…") );
   dtype_load(); /* dep for outfits */

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading Outfits…") );
   outfit_load(); /* dep for ships, factions */

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading Ships…") );
   ships_load(); /* dep for fleet */

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading Factions…") );
   factions_load(); /* dep for fleet, space, missions, AI */

   /* Handle outfit loading part that may use ships and factions. */
   outfit_loadPost();

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading AI…") );
   ai_load(); /* dep for fleets */

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading Techs…") );
   tech_load(); /* dep for space */

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading the Universe…") );
   space_load(); /* dep for events/missions */

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading Events…") );
   events_load();

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading Missions…") );
   missions_load();

   loadscreen_update( ++stage/LOADING_STAGES, _("Loading the UniDiffs…") );
   diff_loadAvailable();

   loadscreen_update( ++stage/LOADING_STAGES, _("Populating Maps…") );
   outfit_mapParse();

   loadscreen_update( ++stage/LOADING_STAGES, _("Calculating Patrols…") );
   safelanes_init();

   loadscreen_update( ++stage/LOADING_STAGES, _("Initializing Details…") );
#if DEBUGGING
   if (stage > LOADING_STAGES)
      WARN(_("Too many loading stages, please increase LOADING_STAGES"));
#endif /* DEBUGGING */
   difficulty_load();
   background_init();
   map_load();
   map_system_load();
   space_loadLua();
   pilots_init();
   weapon_init();
   player_init(); /* Initialize player stuff. */
   loadscreen_update( 1., _("Loading Completed!") );
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
   safelanes_destroy();
   diff_free();
   economy_destroy(); /* must be called before space_exit */
   space_exit(); /* cleans up the universe itself */
   tech_free(); /* Frees tech stuff. */
   ships_free();
   outfit_free();
   spfx_free(); /* gets rid of the special effect */
   effect_exit();
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
void main_loop( int nested )
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
   toolkit_update(); /* to simulate key repetition and get rid of windows */
   if (!paused) {
      /* Important that we pass real_dt here otherwise we get a dt feedback loop which isn't pretty. */
      player_updateAutonav( real_dt );
      update_all( !nested ); /* update game */
   }
   else if (!nested) {
      /* We run the exclusion end here to handle any hooks that are potentially manually triggered by hook.trigger. */
      hook_exclusionEnd( 0. );
   }

   /* Safe hook should be run every frame regardless of whether game is paused or not. */
   if (!nested)
      hooks_run( "safe" );

   /* Checks to see if we want to land. */
   space_checkLand();

   /*
    * Handle render.
    */
   if (!quit) { /* So if update sets up a nested main loop, we can end up in a
                   state where things are corrupted when trying to exit the game.
                   Avoid rendering when quitting just in case. */
      /* Clear buffer. */
      render_all( game_dt, real_dt );
      /* Draw buffer. */
      SDL_GL_SwapWindow( gl_screen.window );
   }
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

   /* Regenerate the background space dust. */
   if (cur_system != NULL)
      background_initDust( cur_system->spacedust );
   else
      background_initDust( 1000. ); /* from loadscreen_load */

   /* Must be before gui_reload */
   fps_setPos( 15., (double)(SCREEN_H-15-gl_defFontMono.h) );

   /* Reload the GUI (may regenerate land window) */
   gui_reload();

   /* Resets dimensions in other components which care. */
   ovr_refresh();
   toolkit_reposition();
   menu_main_resize();
   nebu_resize();

   /* Lua stuff. */
   nlua_resize();

   /* Finally do a render pass to avoid half-rendered stuff. */
   render_all( 0., 0. );
   SDL_GL_SwapWindow( gl_screen.window );

   /* Force render. */
   load_force_render = 1;
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
         dt += (ts.tv_nsec - global_time.tv_nsec) / 1e9;
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
#if HAS_POSIX
   struct timespec ts;
#endif /* HAS_POSIX */

   /* dt in s */
   real_dt  = fps_elapsed();
   game_dt  = real_dt * dt_mod; /* Apply the modifier. */

   /* if fps is limited */
   if (!conf.vsync && conf.fps_max != 0) {
      const double fps_max = 1./(double)conf.fps_max;
      if (real_dt < fps_max) {
         double delay = fps_max - real_dt;
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
 * @brief Sets the position to display the FPS.
 */
void fps_setPos( double x, double y )
{
   fps_x = x;
   fps_y = y;
}

/**
 * @brief Displays FPS on the screen.
 *
 *    @param[in] dt Current delta tick.
 */
void fps_display( double dt )
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
      gl_print( &gl_defFontMono, x, y, &cFontWhite, "%3.2f", fps );
      y -= gl_defFontMono.h + 5.;
   }

   if ((player.p != NULL) && !player_isFlag(PLAYER_DESTROYED) &&
         !player_isFlag(PLAYER_CREATING)) {
      dt_mod_base = player_dt_default();
   }
   if (dt_mod != dt_mod_base)
      gl_print( &gl_defFontMono, x, y, &cFontWhite, "%3.1fx", dt_mod / dt_mod_base);

   if (!paused || !player_paused || !conf.pause_show)
      return;

   y = SCREEN_H / 3. - gl_defFontMono.h / 2.;
   gl_printMidRaw( &gl_defFontMono, SCREEN_W, 0., y,
         &cFontWhite, -1., _("PAUSED") );
}

/**
 * @brief Gets the current FPS.
 *
 *    @return Current FPS as displayed to the player.
 */
double fps_current (void)
{
   return fps;
}

/**
 * @brief Updates the game itself (player flying around and friends).
 *
 *    @brief Mainly uses game dt.
 */
static void update_all( int dohooks )
{
   if ((real_dt > 0.25) && (fps_skipped==0)) { /* slow timers down and rerun calculations */
      fps_skipped = 1;
      return;
   }
   else if (game_dt > fps_min) { /* We'll force a minimum FPS for physics to work alright. */
      int n;
      double nf, microdt, accumdt;

      /* Number of frames. */
      nf = ceil( game_dt / fps_min );
      microdt = game_dt / nf;
      n  = (int) nf;

      /* Update as much as needed, evenly. */
      accumdt = 0.;
      for (int i=0; i<n; i++) {
         update_routine( microdt, dohooks );
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
      update_routine( game_dt, dohooks );

   fps_skipped = 0;
}

/**
 * @brief Actually runs the updates
 *
 *    @param[in] dt Current delta tick.
 *    @param[in] dohooks Whether or not we want to do hooks, such as the initial update upon entering a system.
 */
void update_routine( double dt, int dohooks )
{
   if (dohooks) {
      hook_exclusionStart();

      /* Update time. */
      ntime_update( dt );
   }

   /* Clean up dead elements and build quadtrees. */
   pilots_updatePurge();
   weapons_updatePurge();

   /* Core stuff independent of collisions. */
   space_update( dt, real_dt );
   spfx_update( dt, real_dt );

   /* First compute weapon collisions. */
   weapons_updateCollide( dt );
   pilots_update( dt );
   weapons_update( dt ); /* Has weapons think and update positions. */

   /* Update camera. */
   cam_update( dt );

   /* Update the elapsed time, should be with all the modifications and such. */
   elapsed_time_mod += dt;

   if (dohooks) {
      HookParam h[3];
      hook_exclusionEnd( dt );
      /* Hook set up. */
      h[0].type = HOOK_PARAM_NUMBER;
      h[0].u.num = dt;
      h[1].type = HOOK_PARAM_NUMBER;
      h[1].u.num = real_dt;
      h[2].type = HOOK_PARAM_SENTINEL;
      /* Run the update hook. */
      hooks_runParam( "update", h );
   }
}

/**
 * @brief Sets the window caption.
 */
static void window_caption (void)
{
   char *buf;
   SDL_RWops *rw;

   /* Load icon. */
   rw = PHYSFSRWOPS_openRead( GFX_PATH"icon.webp" );
   if (rw == NULL) {
      WARN( _("Icon (icon.webp) not found!") );
      return;
   }
   naev_icon   = IMG_Load_RW( rw, 1 );
   if (naev_icon == NULL) {
      WARN( _("Unable to load icon.webp!") );
      return;
   }

   /* Set caption. */
   SDL_asprintf( &buf, APPNAME" - %s", _(start_name()) );
   SDL_SetWindowTitle( gl_screen.window, buf );
   SDL_SetWindowIcon( gl_screen.window, naev_icon );
   free( buf );
}

static int binary_comparison( int x, int y )
{
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

   if (semver_parse( version, &sv )) {
      WARN( _("Failed to parse version string '%s'!"), version );
      return -1;
   }

   if ((res = 3*binary_comparison(version_binary.major, sv.major)) == 0) {
      if ((res = 2*binary_comparison(version_binary.minor, sv.minor)) == 0) {
         res = semver_compare( version_binary, sv );
      }
   }
   semver_free( &sv );
   return res;
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

/**
 * @brief Gets the last delta-tick.
 */
double naev_getrealdt (void)
{
   return real_dt;
}
