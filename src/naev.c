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
#include "SDL_image.h"

#include "naev.h"
#include "log.h" /* for DEBUGGING */


/* global */
#include <string.h> /* strdup */

#if HAS_POSIX
#include <time.h>
#endif /* HAS_POSIX */

#if defined(HAVE_FENV_H) && defined(DEBUGGING)
#include <fenv.h>
#endif /* defined(HAVE_FENV_H) && defined(DEBUGGING) */

#if HAS_LINUX && HAS_BFD && defined(DEBUGGING)
#include <signal.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <bfd.h>
#endif /* HAS_LINUX && HAS_BFD && defined(DEBUGGING) */

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


#define CONF_FILE       "conf.lua" /**< Configuration file by default. */
#define VERSION_FILE    "VERSION" /**< Version file by default. */
#define FONT_SIZE       12 /**< Normal font size. */
#define FONT_SIZE_SMALL 10 /**< Small font size. */

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
const double fps_min    = 1./50.; /**< Minimum fps to run at. */
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
void main_loop (void); /* dialogue.c */


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

   /* Save the binary path. */
   binary_path = strdup(argv[0]);

   /* Print the version */
   LOG( " "APPNAME" v%s", naev_version(0) );
#ifdef GIT_COMMIT
   DEBUG( " git HEAD at " GIT_COMMIT );
#endif /* GIT_COMMIT */

   /* Initializes SDL for possible warnings. */
   SDL_Init(0);

   /* Initialize the threadpool */
   threadpool_init();

   /* Set up debug signal handlers. */
   debug_sigInit();

   /* Create the home directory if needed. */
   if (nfile_dirMakeExist("%s", nfile_basePath()))
      WARN("Unable to create naev directory '%s'", nfile_basePath());

   /* Must be initialized before input_init is called. */
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
      WARN("Unable to initialize SDL Video: %s", SDL_GetError());
      return -1;
   }

   /* Get desktop dimensions. */
#if SDL_VERSION_ATLEAST(1,2,10)
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

   /* Set the configuration. */
   snprintf(buf, PATH_MAX, "%s"CONF_FILE, nfile_basePath());
   conf_setDefaults(); /* set the default config values */
   conf_loadConfig(buf); /* Lua to parse the configuration file */
   conf_parseCLI( argc, argv ); /* parse CLI arguments */

   /* Enable FPU exceptions. */
#if defined(HAVE_FEENABLEEXCEPT) && defined(DEBUGGING)
   if (conf.fpu_except)
      feenableexcept( FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW );
#endif /* defined(HAVE_FEENABLEEXCEPT) && defined(DEBUGGING) */

   /* Open data. */
   if (ndata_open() != 0)
      ERR("Failed to open ndata.");

   /* Load the start info. */
   if (start_load())
      ERR("Failed to load module start data.");

   /* Load the data basics. */
   LOG(" %s", ndata_name());
   DEBUG();

   /* Display the SDL Version. */
   print_SDLversion();
   DEBUG();

   /* random numbers */
   rng_init();

   /*
    * OpenGL
    */
   if (gl_init()) { /* initializes video output */
      ERR("Initializing video output failed, exiting...");
      SDL_Quit();
      exit(EXIT_FAILURE);
   }
   window_caption();
   gl_fontInit( NULL, NULL, FONT_SIZE ); /* initializes default font to size */
   gl_fontInit( &gl_smallFont, NULL, FONT_SIZE_SMALL ); /* small font */

   /* Display the load screen. */
   loadscreen_load();
   loadscreen_render( 0., "Initializing subsystems..." );
   time_ms = SDL_GetTicks();


   /*
    * Input
    */
   if ((conf.joystick_ind >= 0) || (conf.joystick_nam != NULL)) {
      if (joystick_init()) WARN("Error initializing joystick input");
      if (conf.joystick_nam != NULL) { /* use the joystick name to find a joystick */
         if (joystick_use(joystick_get(conf.joystick_nam))) {
            WARN("Failure to open any joystick, falling back to default keybinds");
            input_setDefault();
         }
         free(conf.joystick_nam);
      }
      else if (conf.joystick_ind >= 0) /* use a joystick id instead */
         if (joystick_use(conf.joystick_ind)) {
            WARN("Failure to open any joystick, falling back to default keybinds");
            input_setDefault();
         }
   }


   /*
    * OpenAL - Sound
    */
   if (conf.nosound) {
      LOG("Sound is disabled!");
      sound_disabled = 1;
      music_disabled = 1;
   }
   if (sound_init()) WARN("Problem setting up sound!");
   music_choose("load");

   /* FPS stuff. */
   fps_setPos( 15., (double)(gl_screen.h-15-gl_defFont.h) );

   /* Misc graphics init */
   if (nebu_init() != 0) { /* Initializes the nebula */
      /* An error has happened */
      ERR("Unable to initialize the Nebula subsystem!");
      /* Weirdness will occur... */
   }
   gui_init(); /* initializes the GUI graphics */
   toolkit_init(); /* initializes the toolkit */
   map_init(); /* initializes the map. */
   cond_init(); /* Initialize conditional subsystem. */
   cli_init(); /* Initialize console. */

   /* Data loading */
   load_all();

   /* Generate the CVS. */
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
         if (event.type == SDL_QUIT)
            if (menu_askQuit()) {
               quit = 1; /* quit is handled here */
               break;
            }

         input_handle(&event); /* handles all the events and player keybinds */
      }

      main_loop();
   }


   /* Save configuration. */
   conf_saveConfig(buf);

   /* cleanup some stuff */
   player_cleanup(); /* cleans up the player stuff */
   gui_free(); /* cleans up the player's GUI */
   weapon_exit(); /* destroys all active weapons */
   pilots_free(); /* frees the pilots, they were locked up :( */
   cond_exit(); /* destroy conditional subsystem. */
   land_exit(); /* Destroys landing vbo and friends. */
   npc_clear(); /* In case exitting while landed. */
   background_free(); /* Destroy backgrounds. */

   /* data unloading */
   unload_all();

   /* cleanup opengl fonts */
   gl_freeFont(NULL);
   gl_freeFont(&gl_smallFont);

   /* Close data. */
   ndata_close();
   start_cleanup();

   /* Destroy conf. */
   conf_cleanup(); /* Frees some memory the configuration allocated. */

   /* Clean up loading game stuff stuff. */
   load_free();

   /* exit subsystems */
   cli_exit(); /* CLean up the console. */
   map_exit(); /* destroys the map. */
   toolkit_exit(); /* kills the toolkit */
   ai_exit(); /* stops the Lua AI magic */
   joystick_exit(); /* releases joystick */
   input_exit(); /* cleans up keybindings */
   nebu_exit(); /* destroys the nebula */
   gl_exit(); /* kills video output */
   sound_exit(); /* kills the sound */
   news_exit(); /* destroys the news. */

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
   uint32_t nload;

   /* Count the loading screens */
   loadscreens = ndata_list( "gfx/loading/", &nload );

   /* Must have loading screens */
   if (nload==0) {
      WARN("No loading screens found!");
      loading = NULL;
      return;
   }

   /* Set the zoom. */
   cam_setZoom( conf.zoom_far );

   /* Load the texture */
   snprintf( file_path, PATH_MAX, "gfx/loading/%s", loadscreens[ RNG_SANE(0,nload-1) ] );
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
   SDL_GL_SwapBuffers();

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
   /* order is very important as they're interdependent */
   loadscreen_render( 1./LOADING_STAGES, "Loading Commodities..." );
   commodity_load(); /* dep for space */
   loadscreen_render( 2./LOADING_STAGES, "Loading Factions..." );
   factions_load(); /* dep for fleet, space, missions, AI */
   loadscreen_render( 3./LOADING_STAGES, "Loading AI..." );
   ai_load(); /* dep for fleets */
   loadscreen_render( 4./LOADING_STAGES, "Loading Missions..." );
   missions_load(); /* no dep */
   loadscreen_render( 5./LOADING_STAGES, "Loading Events..." );
   events_load(); /* no dep */
   loadscreen_render( 6./LOADING_STAGES, "Loading Special Effects..." );
   spfx_load(); /* no dep */
   loadscreen_render( 7./LOADING_STAGES, "Loading Outfits..." );
   outfit_load(); /* dep for ships */
   loadscreen_render( 8./LOADING_STAGES, "Loading Ships..." );
   ships_load(); /* dep for fleet */
   loadscreen_render( 9./LOADING_STAGES, "Loading Fleets..." );
   fleet_load(); /* dep for space */
   loadscreen_render( 10./LOADING_STAGES, "Loading Techs..." );
   tech_load(); /* dep for space */
   loadscreen_render( 11./LOADING_STAGES, "Loading the Universe..." );
   space_load();
   background_init();
   loadscreen_render( 1., "Loading Completed!" );
   xmlCleanupParser(); /* Only needed to be run after all the loading is done. */
}
/**
 * @brief Unloads all data, simplifies main().
 */
void unload_all (void)
{
   /* data unloading - inverse load_all is a good order */
   economy_destroy(); /* must be called before space_exit */
   space_exit(); /* cleans up the universe itself */
   tech_free(); /* Frees tech stuff. */
   fleet_free();
   ships_free();
   outfit_free();
   spfx_free(); /* gets rid of the special effect */
   missions_free();
   factions_free();
   commodity_free();
   var_cleanup(); /* cleans up mission variables */
}

/**
 * @brief Split main loop from main() for secondary loop hack in toolkit.c.
 */
void main_loop (void)
{
   int tk;

   /* Check to see if toolkit is open. */
   tk = toolkit_isOpen();

   /* Clear buffer. */
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   fps_control(); /* everyone loves fps control */

   /* Important that we pass real_dt here otherwise we get a dt feedback loop which isn't pretty. */
   player_updateAutonav( real_dt );

   input_update( real_dt ); /* handle key repeats. */

   sound_update( real_dt ); /* Update sounds. */
   if (tk) toolkit_update(); /* to simulate key repetition */
   if (!paused)
      update_all(); /* update game */
   render_all();
   /* Toolkit is rendered on top. */
   if (tk) toolkit_render();

   gl_checkErr(); /* check error every loop */

   /* Draw buffer. */
   SDL_GL_SwapBuffers();
}


#if HAS_POSIX
static struct timespec global_time; /**< Global timestamp for calculating delta ticks. */
static int use_posix_time; /**< Whether or not to use posix time. */
#endif /* HAS_POSIX */
/**
 * @brief Initializes the fps engine.
 */
static void fps_init (void)
{
#if HAS_POSIX
#ifdef CLOCK_MONOTONIC
   use_posix_time = 1;
   /* We must use clock_gettime here instead of gettimeofday mainly because this
    * way we are not influenced by changes to the time source like say ntp which
    * could skew up the dt calculations. */
   if (clock_gettime(CLOCK_MONOTONIC, &global_time)==0)
      return;
   WARN("clock_gettime failed, disabling posix time.");
#endif /* CLOCK_MONOTONIC */
   use_posix_time = 0;
#endif /* HAS_POSIX */
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

#if HAS_POSIX
#ifdef CLOCK_MONOTONIC
   struct timespec ts;

   if (use_posix_time) {
      if (clock_gettime(CLOCK_MONOTONIC, &ts)==0) {
         dt  = ts.tv_sec - global_time.tv_sec;
         dt += (ts.tv_nsec - global_time.tv_nsec) / 1000000000.0;
         memcpy( &global_time, &ts, sizeof(struct timespec) );
         return dt;
      }
      WARN( "clock_gettime failed!" );
   }
#endif /* CLOCK_MONOTONIC */
#endif /* HAS_POSIX */

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
   double nf, microdt;

   if ((real_dt > 0.25) && (fps_skipped==0)) { /* slow timers down and rerun calculations */
      pause_delay((unsigned int)game_dt*1000);
      fps_skipped = 1;
      return;
   }
   else if (game_dt > fps_min) { /* we'll force a minimum of 50 FPS */

      /* Number of frames. */
      nf = ceil( game_dt / fps_min );
      microdt = game_dt / nf;
      n  = (int) nf;

      /* Update as much as needed, evenly. */
      for (i=0; i<n; i++) {
         pause_delay( (unsigned int)(microdt*1000));
         update_routine(microdt);
      }

      /* Note we don't touch game_dt so that fps_display works well */
   }
   else /* Standard, just update with the last dt */
      update_routine(game_dt);

   fps_skipped = 0;
}


/**
 * @brief Actually runs the updates
 *
 *    @param[in] dt Current delta tick.
 */
void update_routine( double dt )
{
   /* Update time. */
   ntime_update( dt );

   /* Update engine stuff. */
   space_update(dt);
   weapons_update(dt);
   spfx_update(dt);
   pilots_update(dt);
   hooks_update(dt);

   /* Update camera. */
   cam_update( dt );
}


/**
 * @brief Renders the game itself (player flying around and friends).
 *
 * Blitting order (layers):
 *   - BG
 *     - stars and planets
 *     - background player stuff (planet targetting)
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

   /* Set caption. */
   snprintf(buf, PATH_MAX ,APPNAME" - %s", ndata_name());
   SDL_WM_SetCaption(buf, APPNAME);

   /* Set icon. */
   rw = ndata_rwops( "gfx/icon.png" );
   if (rw == NULL) {
      WARN("Icon (gfx/icon.png) not found!");
      return;
   }
   npng        = npng_open( rw );
   naev_icon   = npng_readSurface( npng, 0, 0 );
   npng_close( npng );
   SDL_RWclose( rw );
   if (naev_icon == NULL) {
      WARN("Unable to load gfx/icon.png!");
      return;
   }
   SDL_WM_SetIcon( naev_icon, NULL );
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
      snprintf( short_version, sizeof(short_version),
#if VREV < 0
            "%d.%d.0-beta%d",
            VMAJOR, VMINOR, ABS(VREV)
#else /* VREV < 0 */
            "%d.%d.%d",
            VMAJOR, VMINOR, VREV
#endif /* VREV < 0 */
            );

   /* Set up the long version. */
   if (long_version) {
      if (human_version[0] == '\0')
         snprintf( human_version, sizeof(human_version),
               " "APPNAME" v%s%s - %s", short_version,
#ifdef DEBUGGING
               " debug",
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
   char cbuf[8];

   /* Check length. */
   if (nbuf > (int)sizeof(cbuf)) {
      WARN("Version format is too long!");
      return -1;
   }

   s = 0;
   j = 0;
   for (i=0; i < nbuf; i++) {
      cbuf[j++] = buf[i];
      if (buf[i] == '.') {
         cbuf[j] = '\0';
         version[s++] = atoi(cbuf);
         if (s >= 3) {
            WARN("Version has too many '.'.");
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
 * @brief Comparse the version against the current naev version.
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
      return +2;

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
   linked = SDL_Linked_Version();
   DEBUG("SDL: %d.%d.%d [compiled: %d.%d.%d]",
         linked->major, linked->minor, linked->patch,
         compiled.major, compiled.minor, compiled.patch);

   /* Get version as number. */
   version_linked    = linked->major*100 + linked->minor;
   version_compiled  = compiled.major*100 + compiled.minor;

   /* Check if major/minor version differ. */
   if (version_linked > version_compiled)
      WARN("SDL is newer than compiled version");
   if (version_linked < version_compiled)
      WARN("SDL is older than compiled version.");
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
         case SI_USER: return "SIGFPE (raised by program)";
         case FPE_INTDIV: return "SIGFPE (integer divide by zero)";
         case FPE_INTOVF: return "SIGFPE (integer overflow)";
         case FPE_FLTDIV: return "SIGFPE (floating-point divide by zero)";
         case FPE_FLTOVF: return "SIGFPE (floating-point overflow)";
         case FPE_FLTUND: return "SIGFPE (floating-point underflow)";
         case FPE_FLTRES: return "SIGFPE (floating-point inexact result)";
         case FPE_FLTINV: return "SIGFPE (floating-point invalid operation)";
         case FPE_FLTSUB: return "SIGFPE (subscript out of range)";
         default: return "SIGFPE";
      }
   else if (sig == SIGSEGV)
      switch (sig_code) {
         case SI_USER: return "SIGSEGV (raised by program)";
         case SEGV_MAPERR: return "SIGSEGV (address not mapped to object)";
         case SEGV_ACCERR: return "SIGSEGV (invalid permissions for mapped object)";
         default: return "SIGSEGV";
      }
   else if (sig == SIGABRT)
      switch (sig_code) {
         case SI_USER: return "SIGABRT (raised by program)";
         default: return "SIGABRT";
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

   DEBUG("Naev recieved %s!",
         debug_sigCodeToStr(info->si_signo, info->si_code) );
   for (i=0; i<num; i++) {
      if (abfd != NULL)
         debug_translateAddress(symbols[i], (bfd_vma) (bfd_hostptr_t) buf[i]);
      else
         DEBUG("   %s", symbols[i]);
   }
   DEBUG("Report this to project maintainer with the backtrace.");

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
      DEBUG("Unable to set up SIGSEGV signal handler.");
   sigaction(SIGFPE, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG("Unable to set up SIGFPE signal handler.");
   sigaction(SIGABRT, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG("Unable to set up SIGABRT signal handler.");
   DEBUG("BFD backtrace catching enabled.");
#endif /* HAS_LINUX && HAS_BFD && defined(DEBUGGING) */
}


/**
 * @brief Closes the SignalHandler for linux.
 */
static void debug_sigClose (void)
{
#if HAS_LINUX && HAS_BFD && defined(DEBUGGING)
   bfd_close( abfd );
#endif /* HAS_LINUX && HAS_BFD && defined(DEBUGGING) */
}
