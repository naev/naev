/*
 * See Licensing and Copyright notice in naev.h
 */

/*
 * includes
 */
/* localised global */
#include "SDL.h"

/* global */
#include <string.h> /* strdup */

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


/* to get data info */
#define XML_START_ID    "Start"
#define START_DATA      "dat/start.xml"

#define CONF_FILE       "conf"
#define VERSION_FILE    "VERSION"
#define VERSION_LEN     10
#define MINIMUM_FPS     0.5
#define FONT_SIZE       12
#define FONT_SIZE_SMALL 10


static int quit = 0; /* for primary loop */
unsigned int time = 0; /* used to calculate FPS and movement, in pause.c */
static char version[VERSION_LEN];

/* some defaults */
char* data = NULL;
char dataname[DATA_NAME_LEN] = "";
int nosound = 0;
int show_fps = 1; /* shows fps - default yes */
int max_fps = 0;
int indjoystick = -1;
char* namjoystick = NULL;


/*
 * prototypes
 */
static void load_all (void);
static void unload_all (void);
void main_loop (void);
static void display_fps( const double dt );
static void window_caption (void);
static void data_name (void);
/* update */
static void fps_control (void);
static void update_all (void);
static void update_routine( double dt );
static void render_all (void);


/**
 * @brief The entry point of NAEV.
 * @param[in] argc Number of arguments.
 * @param[in] argv Array of argc arguments.
 * @return EXIT_SUCCESS on success.
 */
int main ( int argc, char** argv )
{
   char buf[PATH_MAX];

   /* print the version */
   snprintf( version, VERSION_LEN, "%d.%d.%d", VMAJOR, VMINOR, VREV );
   LOG( " "APPNAME" v%s", version );

   /* initializes SDL for possible warnings */
   SDL_Init(0);

   /* create the home directory if needed */
   if (nfile_dirMakeExist("."))
      WARN("Unable to create naev directory '%s'",nfile_basePath());

   /* input must be initialized for config to work */
   input_init(); 

   /* set the configuration */
   snprintf(buf, PATH_MAX, "%s"CONF_FILE, nfile_basePath());
   conf_setDefaults(); /* set the default config values */
   conf_loadConfig(buf); /* Lua to parse the configuration file */
   conf_parseCLI( argc, argv ); /* parse CLI arguments */

   /* load the data basics */
   data_name();
   LOG(" %s", dataname);
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


   /*
    * OpenAL - Sound
    */
   if (nosound)
      LOG("Sound is disabled!");
   else {
      if (sound_init()) WARN("Problem setting up sound!");
      music_choose("load");
   }


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
   gl_fontInit( NULL, NULL, FONT_SIZE ); /* initializes default font to size */
   gl_fontInit( &gl_smallFont, NULL, FONT_SIZE_SMALL ); /* small font */
   gui_init(); /* initializes the GUI graphics */
   toolkit_init(); /* initializes the toolkit */

   
   /* data loading */
   load_all();


   /* start menu */
   menu_main();
   
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
   gl_exit(); /* kills video output */
   sound_exit(); /* kills the sound */
   SDL_Quit(); /* quits SDL */


   /* all is well */
   exit(EXIT_SUCCESS);
}

/**
 * loads all the data, makes main() simpler
 */
void load_all (void)
{
   /* order is very important as they're interdependent */
   commodity_load(); /* dep for space */
   factions_load(); /* dep for fleet, space, missions */
   missions_load(); /* no dep */
   spfx_load(); /* no dep */
   outfit_load(); /* dep for ships */
   ships_load(); /* dep for fleet */
   fleet_load(); /* dep for space */
   space_load();
}
/**
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
 * @brief Split main loop from main() for secondary loop hack in toolkit.c.
 */
void main_loop (void)
{
   sound_update(); /* do sound stuff */

   glClear(GL_COLOR_BUFFER_BIT);

   fps_control(); /* everyone loves fps control */
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
static double dt = 0.; /* used also a bit in render_all */
/**
 * @brief Controls the FPS.
 */
static void fps_control (void)
{
   unsigned int t;

   /* dt in ms/1000 */
   t = SDL_GetTicks();
   dt = (double)(t - time) / 1000.;
   time = t;

   if (paused) SDL_Delay(10); /* drop paused FPS - we are nice to the CPU :) */

   /* if fps is limited */                       
   if ((max_fps != 0) && (dt < 1./max_fps)) {
      double delay = 1./max_fps - dt;
      SDL_Delay( delay );
      fps_dt += delay; /* makes sure it displays the proper fps */
   }
}


const double fps_min = 1./50.;
/**
 * @brief Updates the game itself (player flying around and friends).
 * @notes
 */
static void update_all (void)
{
   double tempdt;

   if (dt > 0.25) { /* slow timers down and rerun calculations */
      pause_delay((unsigned int)dt*1000);
      return;
   }
   else if (dt > fps_min) { /* we'll force a minimum of 50 FPS */

      tempdt = dt - fps_min;
      pause_delay( (unsigned int)(tempdt*1000));
      update_routine(fps_min);
      
      /* run as many cycles of dt=fps_min as needed */
      while (tempdt > fps_min) {
         pause_delay((unsigned int)(-fps_min*1000)); /* increment counters */
         update_routine(fps_min);
         tempdt -= fps_min;
      }
   }

   update_routine(dt);
}


/**
 * @brief Actually runs the updates
 */
static void update_routine( double dt )
{
   space_update(dt);
   weapons_update(dt);
   spfx_update(dt);
   pilots_update(dt);
}


/**
 * @brief Renders the game itself (player flying around and friends).
 *
 * Blitting order (layers):
 *   BG | @ stars and planets
 *      | @ background player stuff (planet targetting)
 *      | @ background particles
 *      | @ back layer weapons
 *      X
 *   N  | @ NPC ships
 *      | @ front layer weapons
 *      | @ normal layer particles (above ships)
 *      X
 *   FG | @ player
 *      | @ foreground particles
 *      | @ text and GUI
 */
static void render_all (void)
{
   /* setup */
   spfx_start(dt);
   /* BG */
   space_render(dt);
   planets_render();
   player_renderBG();
   weapons_render(WEAPON_LAYER_BG);
   /* N */
   pilots_render();
   weapons_render(WEAPON_LAYER_FG);
   spfx_render(SPFX_LAYER_BACK);
   /* FG */
   player_render();
   spfx_render(SPFX_LAYER_FRONT);
   display_fps(dt);
}


static double fps = 0.;
static double fps_cur = 0.;
/**
 * @brief Displays FPS on the screen.
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
   if (show_fps)
      gl_print( NULL, x, y, NULL, "%3.2f", fps );
}


/**
 * @brief Sets the data module's name.
 */
static void data_name (void)
{
   uint32_t bufsize;
   char *buf;

   /* 
    * check to see if data file is valid
    */
   if (pack_check(DATA)) {
      ERR("Data file '%s' not found",DATA);
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
   xmlCleanupParser();
}


/**
 * @brief Sets the window caption.
 */
static void window_caption (void)
{
   char tmp[DATA_NAME_LEN+10];

   snprintf(tmp,DATA_NAME_LEN+10,APPNAME" - %s",dataname);
   SDL_WM_SetCaption(tmp, NULL );
}


