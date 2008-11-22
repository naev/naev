/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @mainpage NAEV
 *
 * Doxygen documentation for the NAEV project.
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

/* global */
#include <string.h> /* strdup */
#if defined(LINUX) && !defined(NODEBUG)
#include <signal.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#endif /* defined(LINUX) && !defined(DEBUG) */


/* local */
#include "naev.h"
#include "conf.h"
#include "log.h"
#include "physics.h"
#include "opengl.h"
#include "font.h"
#include "ship.h"
#include "pilot.h"
#include "player.h"
#include "input.h"
#include "joystick.h"
#include "space.h"
#include "rng.h"
#include "ai.h"
#include "outfit.h"
#include "pack.h"
#include "weapon.h"
#include "faction.h"
#include "xml.h"
#include "toolkit.h"
#include "pause.h"
#include "sound.h"
#include "music.h"
#include "spfx.h"
#include "economy.h"
#include "menu.h"
#include "mission.h"
#include "misn_lua.h"
#include "nfile.h"
#include "nebulae.h"
#include "unidiff.h"


/* to get data info */
#define XML_START_ID    "Start"  /**< XML document tag of module start file. */
#define START_DATA      "dat/start.xml" /**< Path to module start file. */

#define CONF_FILE       "conf" /**< Configuration file by default. */
#define VERSION_FILE    "VERSION" /**< Version file by default. */
#define VERSION_LEN     10 /**< Maximum length of the version file. */
#define FONT_SIZE       12 /**< Normal font size. */
#define FONT_SIZE_SMALL 10 /**< Small font size. */

#define NAEV_INIT_DELAY 3000 /**< Minimum amount of time to wait with loading screen */


static int quit = 0; /**< For primary loop */
static unsigned int time = 0; /**< used to calculate FPS and movement, in pause.c */
static char version[VERSION_LEN]; /***< Contains version. */
static glTexture *loading; /**< Loading screen. */

/* some defaults */
char* data = NULL; /**< Path to datafile. */
char dataname[DATA_NAME_LEN] = ""; /**< Name of data file. */
int nosound = 0; /**< Disables sound when loading. */
int show_fps = 1; /**< Shows fps - default yes */
int max_fps = 0; /**< Default fps limit, 0 is no limit. */
int indjoystick = -1; /**< Index of joystick to use, -1 is none. */
char* namjoystick = NULL; /**< Name of joystick to use, NULL is none. */


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
static void data_name (void);
static void debug_sigInit (void);
/* update */
static void fps_control (void);
static void update_all (void);
static void update_routine( double dt );
static void render_all (void);
/* Misc. */
void loadscreen_render( double done, const char *msg ); /* nebulae.c */
void main_loop (void); /* dialogue.c */



/**
 * @fn int main( int argc, char** argv )
 *
 * @brief The entry point of NAEV.
 *
 *    @param[in] argc Number of arguments.
 *    @param[in] argv Array of argc arguments.
 *    @return EXIT_SUCCESS on success.
 */
int main( int argc, char** argv )
{
   char buf[PATH_MAX];

   /* print the version */
   snprintf( version, VERSION_LEN, "%d.%d.%d", VMAJOR, VMINOR, VREV );
   LOG( " "APPNAME" v%s", version );

   /* Initializes SDL for possible warnings. */
   SDL_Init(0);

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

   /* Input must be initialized for config to work. */
   input_init(); 

   /* Set the configuration. */
   snprintf(buf, PATH_MAX, "%s"CONF_FILE, nfile_basePath());
   conf_setDefaults(); /* set the default config values */
   conf_loadConfig(buf); /* Lua to parse the configuration file */
   conf_parseCLI( argc, argv ); /* parse CLI arguments */

   /* Load the data basics. */
   data_name();
   LOG(" %s", dataname);
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
   gl_fontInit( NULL, NULL, FONT_SIZE ); /* initializes default font to size */
   gl_fontInit( &gl_smallFont, NULL, FONT_SIZE_SMALL ); /* small font */
   window_caption();

   /* Display the load screen. */
   loadscreen_load();
   loadscreen_render( 0., "Initializing subsystems..." );
   time = SDL_GetTicks();


   /*
    * OpenAL - Sound
    */
   if (nosound) {
      LOG("Sound is disabled!");
      sound_disabled = 1;
      music_disabled = 1;
   }
   if (sound_init()) WARN("Problem setting up sound!");
   music_choose("load");


   /*
    * Input
    */
   if ((indjoystick >= 0) || (namjoystick != NULL)) {
      if (joystick_init()) WARN("Error initializing joystick input");
      if (namjoystick != NULL) { /* use the joystick name to find a joystick */
         if (joystick_use(joystick_get(namjoystick))) {
            WARN("Failure to open any joystick, falling back to default keybinds");
            input_setDefault();
         }
         free(namjoystick);
      }
      else if (indjoystick >= 0) /* use a joystick id instead */
         if (joystick_use(indjoystick)) {
            WARN("Failure to open any joystick, falling back to default keybinds");
            input_setDefault();
         }
   }

   /* Misc */
   if (ai_init()) WARN("Error initializing AI");

   /* Misc graphics init */
   if (nebu_init() != 0) { /* Initializes the nebulae */
      /* An error has happened */
      ERR("Unable to initialize the Nebulae subsystem!");
      /* Weirdness will occur... */
   }
   gui_init(); /* initializes the GUI graphics */
   toolkit_init(); /* initializes the toolkit */

   /* Data loading */
   load_all();

   /* Unload load screen. */
   loadscreen_unload();

   /* Start menu. */
   menu_main();

   /* Force a minimum delay with loading screen */
   if ((SDL_GetTicks() - time) < NAEV_INIT_DELAY)
      SDL_Delay( NAEV_INIT_DELAY - (SDL_GetTicks() - time) );
   time = SDL_GetTicks(); /* initializes the time */
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
         if (event.type == SDL_QUIT) quit = 1; /* quit is handled here */

         input_handle(&event); /* handles all the events and player keybinds */
      }

      main_loop();
   }


   /* cleanup some stuff */
   player_cleanup(); /* cleans up the player stuff */
   gui_free(); /* cleans up the player's GUI */
   weapon_exit(); /* destroys all active weapons */
   pilots_free(); /* frees the pilots, they were locked up :( */
   space_exit(); /* cleans up the universe itself */

   /* data unloading */
   unload_all();

   /* cleanup opengl fonts */
   gl_freeFont(NULL);
   gl_freeFont(&gl_smallFont);

   /* exit subsystems */
   toolkit_exit(); /* kills the toolkit */
   ai_exit(); /* stops the Lua AI magic */
   joystick_exit(); /* releases joystick */
   input_exit(); /* cleans up keybindings */
   nebu_exit(); /* destroys the nebulae */
   gl_exit(); /* kills video output */
   sound_exit(); /* kills the sound */
   SDL_Quit(); /* quits SDL */


   /* all is well */
   exit(EXIT_SUCCESS);
}


/**
 * @fn void loadscreen_load (void)
 *
 * @brief Loads a loading screen.
 */
void loadscreen_load (void)
{
   int i;
   char file_path[PATH_MAX];
   char **loadscreens;
   char **files;
   uint32_t nfiles;
   size_t len;
   int nload;

   /* Count the loading screens */
   files = pack_listfiles( data, &nfiles );
   len = strlen("gfx/loading/");
   nload = 0;
   loadscreens = malloc(sizeof(char*) * nfiles);
   for (i=0; i<(int)nfiles; i++) {
      if (strncmp(files[i], "gfx/loading/", len)==0) {
         loadscreens[nload] = files[i];
         nload++;
      }
      else
         free(files[i]);
   }
   free(files);


   /* Must have loading screens */
   if (nload==0) {
      WARN("No loading screens found!");
      return;
   }

   /* Load the texture */
   strncpy(file_path, loadscreens[RNG(0,nload-1)], PATH_MAX);
   loading = gl_newImage( file_path );

   /* Clean up. */
   for (i =0; i<nload; i++)
      free(loadscreens[i]);
   free(loadscreens);
}


/**
 * @fn void loadscreen_render( double done, const char *msg )
 *
 * @brief Renders the load screen with message.
 *
 *    @param done Amount done (1. == completed).
 *    @param msg Loading screen message.
 */
void loadscreen_render( double done, const char *msg )
{
   double x,y, w,h, rh;

   /* Clear background. */
   glClear(GL_COLOR_BUFFER_BIT);

   /* Draw loading screen image. */
   gl_blitScale( loading, 0., 0., SCREEN_W, SCREEN_H, NULL );

   /* Draw progress bar. */
   w = gl_screen.w * 0.4;
   h = gl_screen.h * 0.02;
   rh = h + gl_defFont.h + 4.;
   x = -w/2.;
   y = -h/2.;
   /* BG. */
   ACOLOUR(cBlack, 0.7);
   glBegin(GL_QUADS);
      glVertex2d( x-2.,        y-2. + 0.  );
      glVertex2d( x-2.,        y+2. + rh  );
      glVertex2d( x-2. + w+4., y+2. + rh  );
      glVertex2d( x-2. + w+4., y-2. + 0.  );
   glEnd();
   /* FG. */
   ACOLOUR(cConsole, 0.7);
   glBegin(GL_QUADS);
      glVertex2d( x,          y + 0. );
      glVertex2d( x,          y + h  );
      glVertex2d( x + done*w, y + h  );
      glVertex2d( x + done*w, y + 0. );
   glEnd();

   /* Draw text. */
   gl_print( &gl_defFont, x + gl_screen.w/2., y + gl_screen.h/2 + 2. + h,
         &cConsole, msg );

   /* Flip buffers. */
   SDL_GL_SwapBuffers();
}


/**
 * @fn static void loadscreen_unload (void)
 *
 * @brief Frees the loading screen.
 */
static void loadscreen_unload (void)
{
   /* Free the textures */
   gl_freeTexture(loading);
   loading = NULL;
}


/**
 * @fn void load_all (void)
 *
 * @brief Loads all the data, makes main() simpler.
 */
#define LOADING_STAGES     9. /**< Amount of loading stages. */
void load_all (void)
{
   /* order is very important as they're interdependent */
   loadscreen_render( 1./LOADING_STAGES, "Loading Commodities..." );
   commodity_load(); /* dep for space */
   loadscreen_render( 2./LOADING_STAGES, "Loading Factions..." );
   factions_load(); /* dep for fleet, space, missions */
   loadscreen_render( 3./LOADING_STAGES, "Loading Missions..." );
   missions_load(); /* no dep */
   loadscreen_render( 4./LOADING_STAGES, "Loading Special Effects..." );
   spfx_load(); /* no dep */
   loadscreen_render( 5./LOADING_STAGES, "Loading Outfits..." );
   outfit_load(); /* dep for ships */
   loadscreen_render( 6./LOADING_STAGES, "Loading Ships..." );
   ships_load(); /* dep for fleet */
   loadscreen_render( 7./LOADING_STAGES, "Loading Fleets..." );
   fleet_load(); /* dep for space */
   loadscreen_render( 8./LOADING_STAGES, "Loading the Universe..." );
   space_load();
   loadscreen_render( 1., "Loading Completed!" );
   xmlCleanupParser(); /* Only needed to be run after all the loading is done. */
}
/**
 * @fn void unload_all (void)
 *
 * @brief Unloads all data, simplifies main().
 */
void unload_all (void)
{
   /* data unloading - order shouldn't matter, but inverse load_all is good */
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
 * @fn void main_loop (void)
 *
 * @brief Split main loop from main() for secondary loop hack in toolkit.c.
 */
void main_loop (void)
{
   glClear(GL_COLOR_BUFFER_BIT);

   fps_control(); /* everyone loves fps control */

   sound_update(); /* Update sounds. */
   if (toolkit) toolkit_update(); /* to simulate key repetition */
   if (!menu_isOpen(MENU_MAIN)) {
      if (!paused) update_all(); /* update game */
      render_all();
   }
   if (toolkit) toolkit_render();

   gl_checkErr(); /* check error every loop */

   SDL_GL_SwapBuffers();
}


static double fps_dt = 1.;
static double cur_dt = 0.; /* used also a bit in render_all */
/**
 * @fn static void fps_control (void)
 *
 * @brief Controls the FPS.
 */
static void fps_control (void)
{
   unsigned int t;
   double delay;

   /* dt in s */
   t = SDL_GetTicks();
   cur_dt  = (double)(t - time); /* Get the elapsed ms. */
   cur_dt *= dt_mod; /* Apply the modifier. */
   cur_dt /= 1000.; /* Convert to seconds. */
   time = t;

   if (paused) SDL_Delay(10); /* drop paused FPS - we are nice to the CPU :) */

   /* if fps is limited */                       
   if ((max_fps != 0) && (cur_dt < 1./max_fps)) {
      delay = 1./max_fps - cur_dt;
      SDL_Delay( (unsigned int)(delay * 1000) );
      fps_dt += delay; /* makes sure it displays the proper fps */
   }
}


static const double fps_min = 1./50.; /**< Minimum fps to run at. */
/**
 * @fn static void update_all (void)
 *
 * @brief Updates the game itself (player flying around and friends).
 */
static void update_all (void)
{
   double tempdt;

   if (cur_dt > 0.25*dt_mod) { /* slow timers down and rerun calculations */
      pause_delay((unsigned int)cur_dt*1000);
      return;
   }
   else if (cur_dt > fps_min) { /* we'll force a minimum of 50 FPS */

      tempdt = cur_dt - fps_min;
      pause_delay( (unsigned int)(tempdt*1000));
      update_routine(fps_min);

      /* run as many cycles of dt=fps_min as needed */
      while (tempdt > fps_min) {
         pause_delay((unsigned int)(-fps_min*1000)); /* increment counters */
         update_routine(fps_min);
         tempdt -= fps_min;
      }

      update_routine(tempdt); /* leftovers */
      /* Note we don't touch cur_dt so that fps_display works well */
   }
   else /* Standard, just update with the last dt */
      update_routine(cur_dt);
}


/**
 * @fn static void update_routine( double dt )
 *
 * @brief Actually runs the updates
 *
 *    @param[in] dt Current delta tick.
 */
static void update_routine( double dt )
{
   space_update(dt);
   weapons_update(dt);
   spfx_update(dt);
   pilots_update(dt);
   missions_update(dt);
}


/**
 * @fn static void render_all (void)
 *
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

   dt = (paused) ? 0. : cur_dt;

   /* setup */
   spfx_start(dt);
   /* BG */
   space_render(dt);
   planets_render();
   player_renderBG();
   weapons_render(WEAPON_LAYER_BG, dt);
   /* N */
   pilots_render();
   weapons_render(WEAPON_LAYER_FG, dt);
   spfx_render(SPFX_LAYER_BACK);
   /* FG */
   player_render();
   spfx_render(SPFX_LAYER_FRONT);
   space_renderOverlay(dt);
   player_renderGUI();
   display_fps(cur_dt); /* Exception. */
}


static double fps = 0.;
static double fps_cur = 0.;
/**
 * @fn static void display_fps( const double dt )
 *
 * @brief Displays FPS on the screen.
 *
 *    @param[in] dt Current delta tick.
 */
static void display_fps( const double dt )
{
   double x,y;

   fps_dt += dt;
   fps_cur += 1.;
   if (fps_dt > 1.) { /* recalculate every second */
      fps = fps_cur / fps_dt;
      fps_dt = fps_cur = 0.;
   }

   x = 10.;
   y = (double)(gl_screen.h-20);
   if (show_fps) {
      gl_print( NULL, x, y, NULL, "%3.2f", fps );
      y -= gl_defFont.h + 5.;
   }
   if (dt_mod != 1.)
      gl_print( NULL, x, y, NULL, "%3.1fx", dt_mod);
}


/**
 * @fn static void data_name (void)
 *
 * @brief Sets the data module's name.
 */
static void data_name (void)
{
   uint32_t bufsize;
   char *buf;

   /* 
    * check to see if data file is valid
    */
   if ((DATA == NULL) || (pack_check(DATA))) {
      WARN("Data file '%s' not found",DATA);
      WARN("You should specify which data file to use with '-d'");
      WARN("See -h or --help for more information");
      SDL_Quit();
      exit(EXIT_FAILURE);
   }


   /*
    * check the version
    */
   buf = pack_readfile( DATA, VERSION_FILE, &bufsize );

   if (strncmp(buf, version, bufsize) != 0) {
      WARN("NAEV version and data module version differ!");
      WARN("NAEV is v%s, data is for v%s", version, buf );
   }
   
   free(buf);
   
   
   /*
    * load the datafiles name
    */
   buf = pack_readfile( DATA, START_DATA, &bufsize );

   xmlNodePtr node;
   xmlDocPtr doc = xmlParseMemory( buf, bufsize );

   node = doc->xmlChildrenNode;
   if (!xml_isNode(node,XML_START_ID)) {
      ERR("Malformed '"START_DATA"' file: missing root element '"XML_START_ID"'");
      return;
   }

   node = node->xmlChildrenNode; /* first system node */
   if (node == NULL) {
      ERR("Malformed '"START_DATA"' file: does not contain elements");
      return;
   }
   do {
      if (xml_isNode(node,"name")) {
         strncpy(dataname,xml_get(node),DATA_NAME_LEN);
         dataname[DATA_NAME_LEN-1] = '\0';
      }
   } while (xml_nextNode(node));

   xmlFreeDoc(doc);
   free(buf);
}


/**
 * @fn static void window_caption (void)
 *
 * @brief Sets the window caption.
 */
static void window_caption (void)
{
   char tmp[DATA_NAME_LEN+10];

   snprintf(tmp,DATA_NAME_LEN+10,APPNAME" - %s",dataname);
   SDL_WM_SetCaption(tmp, NULL );
}


static char human_version[50]; /**< Stores the human readable version string. */
/**
 * @fn char *naev_version (void)
 *
 * @brief Returns the version in a human readable string.
 *
 *    @return The human readable version string.
 */
char *naev_version (void)
{
   if (human_version[0] == '\0')
      snprintf( human_version, 50, " "APPNAME" v%s - %s", version, dataname );

   return human_version;
}


/**
 * @fn static void print_SDLversion.
 *
 * @brief Prints the SDL version to console.
 */
static void print_SDLversion (void)
{
   const SDL_version *linked;
   SDL_version compiled;
   SDL_VERSION(&compiled);
   linked = SDL_Linked_Version();
   DEBUG("SDL: %d.%d.%d [compiled: %d.%d.%d]",
         linked->major, linked->minor, linked->patch,
         compiled.major, compiled.minor, compiled.patch);

   /* Check if major/minor version differ. */
   if ((linked->major*100 + linked->minor) > compiled.major*100 + compiled.minor)
      WARN("SDL is newer then compiled version");
   if ((linked->major*100 + linked->minor) < compiled.major*100 + compiled.minor)
      WARN("SDL is older then compiled version.");
}


#if defined(LINUX) && !defined(NODEBUG)
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
   else if (sig == SIGTRAP)
      switch (sig_code) {
         case SI_USER: return "SIGTRAP (raised by program)";
         case TRAP_BRKPT: return "SIGTRAP (process breakpoint)";
         case TRAP_TRACE: return "SIGTRAP (process trace trap)";
         default: return "SIGTRAP";
      }

   /* No suitable code found. */
   return strsignal(sig);
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

   num = backtrace(buf, 64);
   symbols = backtrace_symbols(buf, num);

   DEBUG("NAEV recieved %s!",
         debug_sigCodeToStr(info->si_signo, info->si_code) );
   for (i=0; i<num; i++)
      DEBUG("   %s", symbols[i]);
   DEBUG("Report this to project maintainer with the backtrace.");

   exit(1);
}
#endif /* defined(LINUX) && !defined(DEBUG) */


/**
 * @brief Sets up the SignalHandler for Linux.
 */
static void debug_sigInit (void)
{
#if defined(LINUX) && !defined(NODEBUG)
   struct sigaction sa, so;

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
   sigaction(SIGTRAP, &sa, &so);
   if (so.sa_handler == SIG_IGN)
      DEBUG("Unable to set up SIGTRAP signal handler.");
#endif /* defined(LINUX) && !defined(DEBUG) */
}
