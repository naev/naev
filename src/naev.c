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
#include "SDL.h"
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
#include "debug.h"
#include "dialogue.h"
#include "difficulty.h"
#include "economy.h"
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
#include "nlua_colour.h"
#include "nlua_data.h"
#include "nlua_file.h"
#include "nlua_gfx.h"
#include "nlua_naev.h"
#include "nlua_rnd.h"
#include "nlua_tex.h"
#include "nlua_var.h"
#include "nlua_vec2.h"
#include "npc.h"
#include "ntracing.h"
#include "opengl.h"
#include "options.h"
#include "outfit.h"
#include "pause.h"
#include "pilot.h"
#include "player.h"
#include "player_autonav.h"
#include "plugin.h"
#include "render.h"
#include "safelanes.h"
#include "ship.h"
#include "sound.h"
#include "space.h"
#include "spfx.h"
#include "tech.h"
#include "toolkit.h"
#include "unidiff.h"
#include "weapon.h"

#define VERSION_FILE "VERSION" /**< Version file by default. */

static int          quit         = 0; /**< For primary loop */
Uint32              SDL_LOOPDONE = 0; /**< For custom event to exit loops. */
static Uint64       last_t      = 0; /**< used to calculate FPS and movement. */
static SDL_Surface *naev_icon   = NULL; /**< Icon. */
static int          fps_skipped = 0;    /**< Skipped last frame? */

/*
 * FPS stuff.
 */
static double fps_dt  = 1.;       /**< Display fps accumulator. */
static double game_dt = 0.;       /**< Current game deltatick (uses dt_mod). */
static double real_dt = 0.;       /**< Real deltatick. */
static double fps     = 0.;       /**< FPS to finally display. */
static double fps_cur = 0.;       /**< FPS accumulator to trigger change. */
static double fps_x   = 15.;      /**< FPS X position. */
static double fps_y   = -15.;     /**< FPS Y position. */
const double  fps_min = 1. / 10.; /**< New collisions allow larger fps_min. */
double        elapsed_time_mod = 0.; /**< Elapsed modified time. */

static nlua_env *load_env =
   NULL; /**< Environment for displaying load messages and stuff. */
static int          load_force_render = 0;
static unsigned int load_last_render  = 0;
static SDL_mutex   *load_mutex;

/*
 * prototypes
 */
/* update */
static void update_all( int dohooks );
/* Misc. */
void loadscreen_update( double done, const char *msg );

/**
 * @brief Flags naev to quit.
 */
void naev_quit( void )
{
   quit = 1;
}

/**
 * @brief Get if Naev is trying to quit.
 */
int naev_isQuit( void )
{
   return quit;
}

static char conf_file_path[PATH_MAX];
int         naev_main_setup( void )
{
#ifdef DEBUGGING
   /* Set Debugging flags. */
   memset( debug_flags, 0, DEBUG_FLAGS_MAX );
#endif /* DEBUGGING */

   /* Start counting things and such. */
   SDL_LOOPDONE = SDL_RegisterEvents( 1 );

   /* Set the configuration. */
   snprintf( conf_file_path, sizeof( conf_file_path ), "%s" CONF_FILE,
             nfile_configPath() );

   NTracingMessageL( _( "Reached main menu" ) );

   /* Incomplete translation note (shows once if we pick an incomplete
    * translation based on user's locale). */
   if ( !conf.translation_warning_seen && conf.language == NULL ) {
      const char *language = gettext_getLanguage();
      double      coverage = gettext_languageCoverage( language );

      if ( coverage < 0.8 ) {
         conf.translation_warning_seen = 1;
         dialogue_msg(
            _( "Incomplete Translation" ),
            _( "%s is partially translated (%.0f%%) into your language (%s),"
               " but the remaining text will be English. Language settings"
               " are available in the \"%s\" screen." ),
            APPNAME, 100. * coverage, language, _( "Options" ) );
      }
   }

   /* Incomplete game note (shows every time version number changes). */
   if ( conf.lastversion == NULL ||
        ( ( naev_versionCompare( conf.lastversion ) < 0 ) &&
          // "+" will appear on commits described by git describe, aka
          // development builds
          ( strstr( naev_version( 0 ), "+" ) == NULL ) ) ) {
      free( conf.lastversion );
      conf.lastversion = strdup( naev_version( 0 ) );
      dialogue_msg(
         _( "Welcome to Naev" ),
         _( "Welcome to Naev version %s, and thank you for playing! We hope you"
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
            " And again, thank you for playing!" ),
         conf.lastversion );
   }
   return 0;
}

/**
 * @brief The entry point of Naev.
 *
 *    @return EXIT_SUCCESS on success.
 */
int naev_main_events( void )
{
   SDL_Event event;
   while ( !quit && SDL_PollEvent( &event ) ) { /* event loop */
      if ( event.type == SDL_QUIT ) {
         SDL_FlushEvent( SDL_QUIT ); /* flush event to prevent it from
                                          quitting when lagging a bit. */
         if ( quit || menu_askQuit() ) {
            quit = 1; /* quit is handled here */
            break;
         }
      } else if ( event.type == SDL_WINDOWEVENT &&
                  event.window.event == SDL_WINDOWEVENT_RESIZED ) {
         naev_resize();
         continue;
      }
      input_handle( &event ); /* handles all the events and player keybinds */
   }
   return 0;
}

int naev_main_cleanup( void )
{

   /* Save configuration. */
   conf_saveConfig( conf_file_path );

   /* data unloading */
   unload_all();

   /* cleanup opengl fonts */
   gl_freeFont( NULL );
   gl_freeFont( &gl_smallFont );
   gl_freeFont( &gl_defFontMono );

   /* exit subsystems */
   plugin_exit();
   cli_exit();        /* Clean up the console. */
   map_system_exit(); /* Destroys the solar system map. */
   map_exit();        /* Destroys the map. */
   ovr_mrkFree();     /* Clear markers. */
   toolkit_exit();    /* Kills the toolkit */
   ai_exit();         /* Stops the Lua AI magic */
   joystick_exit();   /* Releases joystick */
   input_exit();      /* Cleans up keybindings */
   nebu_exit();       /* Destroys the nebula */
   render_exit();     /* Cleans up post-processing. */
   news_exit();       /* Destroys the news. */
   difficulty_free(); /* Clean up difficulties. */
   music_exit();      /* Kills Lua state. */
   lua_exit();        /* Closes Lua state, and invalidates all Lua. */
   sound_exit();      /* Kills the sound */
   gl_exit();         /* Kills video output */

   /* Has to be run last or it will mess up sound settings. */
   conf_cleanup(); /* Free some memory the configuration allocated. */

   /* Free the icon. */
   if ( naev_icon )
      SDL_FreeSurface( naev_icon );

   IMG_Quit(); /* quits SDL_image */
   SDL_Quit(); /* quits SDL */

   /* Clean up parser. */
   xmlCleanupParser();

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
nlua_env *loadscreen_load( void )
{
   int r;

   load_mutex = SDL_CreateMutex();
   load_env   = nlua_newEnv( "loadscreen" );

   r = nlua_loadStandard( load_env );
   r |= nlua_loadNaev( load_env );
   r |= nlua_loadRnd( load_env );
   r |= nlua_loadVector( load_env );
   r |= nlua_loadFile( load_env );
   r |= nlua_loadData( load_env );
   r |= nlua_loadTex( load_env );
   r |= nlua_loadCol( load_env );
   r |= nlua_loadGFX( load_env );
   if ( r )
      WARN( _( "Something went wrong when loading Lua libraries for '%s'!" ),
            LOADSCREEN_DATA_PATH );

   size_t bufsize;
   char  *buf = ndata_read( LOADSCREEN_DATA_PATH, &bufsize );
   if ( nlua_dobufenv( load_env, buf, bufsize, LOADSCREEN_DATA_PATH ) != 0 ) {
      WARN( _( "Error loading file: %s\n"
               "%s\n"
               "Most likely Lua file has improper syntax, please check" ),
            LOADSCREEN_DATA_PATH, lua_tostring( naevL, -1 ) );
      free( buf );
      return NULL;
   }
   free( buf );
   return load_env;
}

/**
 * @brief Whether or not we want to render the loadscreen.
 */
int naev_shouldRenderLoadscreen( void )
{
   unsigned int t = SDL_GetTicks();
   int          ret;
   SDL_mutexP( load_mutex );
   /* Only render if forced or try for low 10 FPS. */
   if ( !load_force_render && ( t - load_last_render ) < 100 )
      ret = 0;
   else
      ret = 1;
   SDL_mutexV( load_mutex );
   return ret;
}

void naev_doRenderLoadscreen( void )
{
   /* Stop from being unresponsive. */
   SDL_Event event;
   while ( SDL_PollEvent( &event ) )
      ;

   SDL_mutexP( load_mutex );
   load_last_render = SDL_GetTicks();

   /* Clear background. */
   glClearColor( 0., 0., 0., 1. );
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   glClearColor( 0., 0., 0., 0. );

   /* Run Lua. */
   nlua_getenv( naevL, load_env, "render" );
   if ( nlua_pcall( load_env, 0, 0 ) ) { /* error has occurred */
      WARN( _( "Loadscreen '%s': '%s'" ), "render", lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
   }

   /* Flip buffers. HACK: Also try to catch a late-breaking resize from the WM
    * (...or a crazy user?). */
   SDL_GL_SwapWindow( gl_screen.window );
   naev_resize();

   /* Clear forcing. */
   load_force_render = 0;
   SDL_mutexV( load_mutex );
}

/**
 * @brief Renders the loadscreen if necessary.
 */
void naev_renderLoadscreen( void )
{
   if ( naev_shouldRenderLoadscreen() )
      naev_doRenderLoadscreen();
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
   if ( nlua_pcall( load_env, 2, 0 ) ) { /* error has occurred */
      WARN( _( "Loadscreen '%s': '%s'" ), "update", lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
   }

   /* Force rerender. */
   load_force_render = 1;
   naev_renderLoadscreen();
}

/**
 * @brief Frees the loading screen.
 */
void loadscreen_unload( void )
{
   nlua_freeEnv( load_env );
   SDL_DestroyMutex( load_mutex );
}

/**
 * @brief Unloads all data, simplifies main().
 */
void unload_all( void )
{
   /* cleanup some stuff */
   player_cleanup();  /* cleans up the player stuff */
   gui_free();        /* cleans up the player's GUI */
   weapon_exit();     /* destroys all active weapons */
   pilots_free();     /* frees the pilots, they were locked up :( */
   cond_exit();       /* destroy conditional subsystem. */
   land_exit();       /* Destroys landing vbo and friends. */
   npc_clear();       /* In case exiting while landed. */
   background_free(); /* Destroy backgrounds. */
   load_free();       /* Clean up loading game stuff stuff. */
   diff_exit();
   safelanes_destroy();
   economy_destroy(); /* must be called before space_exit */
   space_exit();      /* cleans up the universe itself */
   tech_free();       /* Frees tech stuff. */
   ships_free();
   outfit_free();
   spfx_free(); /* gets rid of the special effect */
   effect_exit();
   missions_free();
   events_exit(); /* Clean up events. */
   factions_free();
   commodity_free();
   var_cleanup(); /* cleans up mission variables */
}

/**
 * @brief Split main loop from main() for secondary loop hack in toolkit.c.
 */
void main_loop( int nested )
{
   NTracingZone( _ctx, 1 );

   /* Update elapsed time */
   {
      Uint64 t = SDL_GetPerformanceCounter();
      double dt =
         (double)( t - last_t ) / (double)SDL_GetPerformanceFrequency();
      last_t  = t;
      real_dt = dt;
      game_dt = real_dt * dt_mod; /* Apply the modifier. */
   }

   /*
    * Handle update.
    */
   input_update( real_dt ); /* handle key repeats. */
   sound_update( real_dt ); /* Update sounds. */
   toolkit_update(); /* to simulate key repetition and get rid of windows */
   if ( !paused ) {
      update_all( !nested ); /* update game */
   } else if ( !nested ) {
      /* We run the exclusion end here to handle any hooks that are potentially
       * manually triggered by hook.trigger. */
      hook_exclusionEnd( 0. );
   }

   /* Safe hook should be run every frame regardless of whether game is paused
    * or not. */
   if ( !nested )
      hooks_run( "safe" );

   /* Checks to see if we want to land. */
   space_checkLand();

   /*
    * Handle render.
    */
   if ( !quit ) { /* So if update sets up a nested main loop, we can end up in a
                     state where things are corrupted when trying to exit the
                     game. Avoid rendering when quitting just in case. */
      /* Clear buffer. */
      render_all( game_dt, real_dt );
      /* Draw buffer. */
      SDL_GL_SwapWindow( gl_screen.window );

      /* if fps is limited */
      if ( !conf.vsync && conf.fps_max != 0 ) {
#if !SDL_VERSION_ATLEAST( 3, 0, 0 ) && HAS_POSIX
         struct timespec ts;
#endif /* HAS_POSIX */
         const double fps_max = 1. / (double)conf.fps_max;
         Uint64       t       = SDL_GetPerformanceCounter();
         double       dt =
            (double)( t - last_t ) / (double)SDL_GetPerformanceFrequency();
         double delay = fps_max - dt;
         if ( delay > 0. ) {
#if SDL_VERSION_ATLEAST( 3, 0, 0 )
            SDL_DelayNS( delay * 1e9 );
#elif HAS_POSIX
            ts.tv_sec  = floor( delay );
            ts.tv_nsec = fmod( delay, 1. ) * 1e9;
            nanosleep( &ts, NULL );
#else  /* HAS_POSIX */
            SDL_Delay( (unsigned int)round( delay * 1000. ) );
#endif /* HAS_POSIX */
         }
      }

      NTracingFrameMark;
   }

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Wrapper for gl_resize that handles non-GL reinitialization.
 */
void naev_resize( void )
{
   /* Auto-detect window size. */
   int w, h;
   SDL_GL_GetDrawableSize( gl_screen.window, &w, &h );

   /* Update options menu, if open. (Never skip, in case the fullscreen mode
    * alone changed.) */
   opt_resize();

   /* Nothing to do. */
   if ( ( w == gl_screen.rw ) && ( h == gl_screen.rh ) )
      return;

   /* Resize the GL context, etc. */
   gl_resize();

   /* Regenerate the background space dust. */
   if ( cur_system != NULL ) {
      background_initDust( cur_system->spacedust );
      background_load( cur_system->background );
   } else
      background_initDust( 1000. ); /* from loadscreen_load */

   /* Must be before gui_reload */
   fps_setPos( 15., (double)( SCREEN_H - 15 - gl_defFontMono.h ) );

   /* Reload the GUI (may regenerate land window) */
   gui_reload();

   /* Resets dimensions in other components which care. */
   ovr_refresh();
   toolkit_resize();
   menu_main_resize();
   nebu_resize();
   ships_resize();

   /* Lua stuff. */
   nlua_resize();

   /* Have to rerender the toolkit too. */
   toolkit_rerender();

   /* Finally do a render pass to avoid half-rendered stuff. */
   render_all( 0., 0. );
   SDL_GL_SwapWindow( gl_screen.window );

   /* Force render. */
   load_force_render = 1;
}

/*
 * @brief Toggles between windowed and fullscreen mode.
 */
void naev_toggleFullscreen( void )
{
   opt_setVideoMode( conf.width, conf.height, !conf.fullscreen, 0 );
}

/**
 * @brief Initializes the fps engine.
 */
void fps_init( void )
{
   last_t = SDL_GetPerformanceCounter();
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
   double x, y;
   double dt_mod_base = 1.;

   fps_dt += dt;
   fps_cur += 1.;
   if ( fps_dt > 1. ) { /* recalculate every second */
      fps    = fps_cur / fps_dt;
      fps_dt = fps_cur = 0.;
   }

   x = fps_x;
   y = fps_y;
   if ( conf.fps_show ) {
      gl_print( &gl_defFontMono, x, y, &cFontWhite, "%3.2f", fps );
      y -= gl_defFontMono.h + 5.;
   }

   if ( ( player.p != NULL ) && !player_isFlag( PLAYER_DESTROYED ) &&
        !player_isFlag( PLAYER_CREATING ) ) {
      dt_mod_base = player_dt_default();
   }
   if ( dt_mod != dt_mod_base )
      gl_print( &gl_defFontMono, x, y, &cFontWhite, "%3.1fx",
                dt_mod / dt_mod_base );

   if ( !paused || !player_paused || !conf.pause_show )
      return;

   y = SCREEN_H / 3. - gl_defFontMono.h / 2.;
   gl_printMidRaw( &gl_defFontMono, SCREEN_W, 0., y, &cFontWhite, -1.,
                   _( "PAUSED" ) );
}

/**
 * @brief Gets the current FPS.
 *
 *    @return Current FPS as displayed to the player.
 */
double fps_current( void )
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
   NTracingZone( _ctx, 1 );

   if ( ( real_dt > 0.25 ) &&
        ( fps_skipped == 0 ) ) { /* slow timers down and rerun calculations */
      fps_skipped = 1;
      NTracingZoneEnd( _ctx );
      return;
   } else if ( game_dt > fps_min ) { /* We'll force a minimum FPS for physics to
                                        work alright. */
      int    n;
      double nf, microdt, accumdt;

      /* Number of frames. */
      nf      = ceil( game_dt / fps_min );
      microdt = game_dt / nf;
      n       = (int)nf;

      /* Update as much as needed, evenly. */
      accumdt = 0.;
      for ( int i = 0; i < n; i++ ) {
         update_routine( microdt, dohooks );
         /* OK, so we need a bit of hackish logic here in case we are chopping
          * up a very large dt and it turns out time compression changes so
          * we're now updating in "normal time compression" zone. This amounts
          * to many updates being run when time compression has changed and thus
          * can cause, say, the player to exceed their target position or get
          * mauled by an enemy ship.
          */
         accumdt += microdt;
         if ( accumdt > dt_mod * real_dt )
            break;
      }

      /* Note we don't touch game_dt so that fps_display works well */
   } else /* Standard, just update with the last dt */
      update_routine( game_dt, dohooks );

   fps_skipped = 0;

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Actually runs the updates
 *
 *    @param[in] dt Current delta tick.
 *    @param[in] dohooks Whether or not we want to do hooks, such as the initial
 * update upon entering a system.
 */
void update_routine( double dt, int dohooks )
{
   NTracingZone( _ctx, 1 );

   double real_update = dt / dt_mod;

   if ( dohooks ) {
      hook_exclusionStart();

      /* Update time. */
      ntime_update( dt );
   }

   /* Clean up dead elements and build quadtrees. */
   pilots_updatePurge();
   weapons_updatePurge();

   /* Core stuff independent of collisions. */
   space_update( dt, real_update );
   spfx_update( dt, real_update );

   if ( dt > 0. ) {
      /* First compute weapon collisions. */
      weapons_updateCollide( dt );
      pilots_update( dt );
      weapons_update( dt ); /* Has weapons think and update positions. */

      /* Update camera. */
      cam_update( dt );
   }

   /* Player autonav. */
   player_updateAutonav( real_update );

   if ( dohooks ) {
      NTracingZoneName( _ctx_hook, "hooks[update]", 1 );
      HookParam h[3];
      hook_exclusionEnd( dt );
      /* Hook set up. */
      h[0].type  = HOOK_PARAM_NUMBER;
      h[0].u.num = dt;
      h[1].type  = HOOK_PARAM_NUMBER;
      h[1].u.num = real_update;
      h[2].type  = HOOK_PARAM_SENTINEL;
      /* Run the update hook. */
      hooks_runParam( "update", h );
      NTracingZoneEnd( _ctx_hook );
   }

   /* Update the elapsed time, should be with all the modifications and such. */
   elapsed_time_mod += dt;

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Prints the SDL version to console.
 */
void print_SDLversion( void )
{
   const SDL_version *linked;
   SDL_version        compiled;
   unsigned int       version_linked, version_compiled;

   /* Extract information. */
   SDL_VERSION( &compiled );
   SDL_version ll;
   SDL_GetVersion( &ll );
   linked = &ll;
   DEBUG( _( "SDL: %d.%d.%d [compiled: %d.%d.%d]" ), linked->major,
          linked->minor, linked->patch, compiled.major, compiled.minor,
          compiled.patch );
#ifndef DEBUGGING /* Shuts up cppcheck. */
   (void)compiled.patch;
#endif /* DEBUGGING */

   /* Get version as number. */
   version_linked   = linked->major * 100 + linked->minor;
   version_compiled = compiled.major * 100 + compiled.minor;

   /* Check if major/minor version differ. */
   if ( version_linked > version_compiled )
      WARN( _( "SDL is newer than compiled version" ) );
   if ( version_linked < version_compiled )
      WARN( _( "SDL is older than compiled version." ) );
}

/**
 * @brief Gets the last delta-tick.
 */
double naev_getrealdt( void )
{
   return real_dt;
}
