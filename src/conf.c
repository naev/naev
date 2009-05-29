/*
 * See Licensing and Copyright notice in naev.h
 */



#include "conf.h"

#include "naev.h"

#include <stdlib.h> /* atoi */
#include <unistd.h> /* getopt */
#include <string.h> /* strdup */
#include <getopt.h> /* getopt_long */

#include "nlua.h"
#include "lauxlib.h" /* luaL_dofile */

#include "log.h"
#include "player.h"
#include "input.h"
#include "opengl.h"
#include "music.h"
#include "nebulae.h"
#include "ndata.h"
#include "nfile.h"


#define  conf_loadInt(n,i)    \
lua_getglobal(L,n); \
if (lua_isnumber(L, -1)) { \
   i = (int)lua_tonumber(L, -1); \
} \
lua_pop(L,1);

#define  conf_loadFloat(n,f)    \
lua_getglobal(L,n); \
if (lua_isnumber(L, -1)) { \
   f = (double)lua_tonumber(L, -1); \
} \
lua_pop(L,1);

#define  conf_loadBool(n,b)   \
lua_getglobal(L, n); \
if (lua_isnumber(L,-1)) \
   b = (lua_tonumber(L,-1) != 0.); \
else if (!lua_isnil(L,-1)) \
   b = lua_toboolean(L, -1); \
lua_pop(L,1);

#define  conf_loadString(n,s) \
lua_getglobal(L, n); \
if (lua_isstring(L, -1)) { \
   s = strdup(lua_tostring(L, -1));   \
} \
lua_pop(L,1);


/* Global configuration. */
PlayerConf_t conf;

/* from main.c */
extern int nosound;
extern int show_fps;
extern int max_fps;
extern int indjoystick;
extern char* namjoystick;
/* from player.c */
extern const char *keybindNames[]; /* keybindings */
/* from input.c */
extern unsigned int input_afterburnSensitivity;


/*
 * prototypes
 */
static void print_usage( char **argv );


/*
 * prints usage
 */
static void print_usage( char **argv )
{
   LOG("Usage: %s [OPTIONS]", argv[0]);
   LOG("Options are:");
   LOG("   -f, --fullscreen      activate fullscreen");
   LOG("   -F n, --fps n         limit frames per second to n");
   LOG("   -V, --vsync           enable vsync");
   LOG("   -d s, --data s        set the data file to be s");
   LOG("   -W n                  set width to n");
   LOG("   -H n                  set height to n");
   LOG("   -j n, --joystick n    use joystick n");
   LOG("   -J s, --Joystick s    use joystick whose name contains s");
   LOG("   -M, --mute            disables sound");
   LOG("   -S, --sound           forces sound");
   LOG("   -m f, --mvol f        sets the music volume to f");
   LOG("   -s f, --svol f        sets the sound volume to f");
   LOG("   -G                    regenerates the nebulae (slow)");
   LOG("   -h, --help            display this message and exit");
   LOG("   -v, --version         print the version and exit");
}


/**
 * @brief Sets the default configuration.
 */
void conf_setDefaults (void)
{
   conf_cleanup();

   /* ndata. */
   conf.ndata        = NULL;

   /* opengl. */
   conf.fsaa         = 1;
   conf.vsync        = 0;
   conf.vbo          = 1;

   /* Window. */
   conf.width        = 800;
   conf.height       = 600;
   conf.explicit_dim = 0;
   conf.scalefactor  = 1.;
   conf.fullscreen   = 0.;

   /* Sound. */
   conf.nosound      = 0;
   conf.sound        = 0.4;
   conf.music        = 0.8;

   /* FPS. */
   conf.fps_show     = 0;
   conf.fps_max      = 200;

   /* Joystick. */
   conf.joystick_ind = -1;
   conf.joystick_nam = NULL;

   /* Misc. */
   conf.zoom_min     = 0.5;
   conf.zoom_max     = 1.;
   conf.zoom_speed   = 0.25;
   conf.afterburn_sens = 250;

   /* Input */
   input_setDefault();
}


/*
 * Frees some memory the conf allocated.
 */
void conf_cleanup (void)
{
   if (conf.ndata != NULL)
      free(conf.ndata);
   if (conf.joystick_nam != NULL)
      free(conf.joystick_nam);
}


/*
 * parses the config file
 */
int conf_loadConfig ( const char* file )
{
   int i;
   double d;
   const char *str, *mod;
   SDLKey key;
   int type;
   int w,h;
   SDLMod m;

   i = 0;
   d = 0.;

   /* Check to see if file exists. */
   if (!nfile_fileExists(file))
      return nfile_touch(file);

   /* Load the configuration. */
   lua_State *L = nlua_newState();
   if (luaL_dofile(L, file) == 0) {

      /* ndata. */
      conf_loadString("data",conf.ndata);

      /* OpenGL. */
      conf_loadInt("fsaa",conf.fsaa);
      conf_loadBool("vsync",conf.vsync);
      conf_loadBool("vbo",conf.vbo);

      /* Window. */
      w = h = 0;
      conf_loadInt("width",w);
      conf_loadInt("height",h);
      if (w != 0) {
         conf.explicit_dim = 1;
         conf.width = w;
      }
      if (h != 0) {
         conf.explicit_dim = 1;
         conf.height = h;
      }
      conf_loadFloat("scalefactor",conf.scalefactor);
      conf_loadBool("fullscreen",conf.fullscreen);

      /* FPS */
      conf_loadBool("showfps",conf.fps_show);
      conf_loadInt("maxfps",conf.fps_max);

      /* Sound. */
      conf_loadBool("nosound",conf.nosound);
      conf_loadFloat("sound",conf.sound);
      conf_loadFloat("music",conf.music);

      /* Joystick. */
      lua_getglobal(L, "joystick");
      if (lua_isnumber(L, -1))
         conf.joystick_ind = (int)lua_tonumber(L, -1);
      else if (lua_isstring(L, -1))
         conf.joystick_nam = strdup(lua_tostring(L, -1));
      lua_pop(L,1);

      /* Misc. */
      conf_loadFloat("zoom_min",conf.zoom_min);
      conf_loadFloat("zoom_max",conf.zoom_max);
      conf_loadFloat("zoom_speed",conf.zoom_speed);
      conf_loadInt("afterburn_sensitivity",conf.afterburn_sens);


      /*
       * Keybindings.
       */
      for (i=0; strcmp(keybindNames[i],"end"); i++) {
         lua_getglobal(L, keybindNames[i]);
         if (lua_istable(L, -1)) { /* it's a table */
            /* gets the event type */
            lua_pushstring(L, "type");
            lua_gettable(L, -2);
            if (lua_isstring(L, -1))
               str = lua_tostring(L, -1);
            else if (lua_isnil(L, -1)) {
               WARN("Found keybind with no type field!");
               str = "null";
            }
            else {
               WARN("Found keybind with invalid type field!");
               str = "null";
            }
            lua_pop(L,1);

            /* gets the key */
            lua_pushstring(L, "key");
            lua_gettable(L, -2);
            if (lua_isnumber(L, -1))
               key = (int)lua_tonumber(L, -1);
            else if (lua_isstring(L, -1))
               key = input_keyConv( lua_tostring(L, -1));
            else if (lua_isnil(L, -1)) {
               WARN("Found keybind with no key field!");
               key = SDLK_UNKNOWN;
            }
            else {
               WARN("Found keybind with invalid key field!");
               key = SDLK_UNKNOWN;
            }
            lua_pop(L,1);

            /* Get the modifier. */
            lua_pushstring(L, "mod");
            lua_gettable(L, -2);
            if (lua_isstring(L, -1))
               mod = lua_tostring(L, -1);
            else
               mod = NULL;
            lua_pop(L,1);

            if (str != NULL) { /* keybind is valid */
               /* get type */
               if (strcmp(str,"null")==0)          type = KEYBIND_NULL;
               else if (strcmp(str,"keyboard")==0) type = KEYBIND_KEYBOARD;
               else if (strcmp(str,"jaxispos")==0) type = KEYBIND_JAXISPOS;
               else if (strcmp(str,"jaxisneg")==0) type = KEYBIND_JAXISNEG;
               else if (strcmp(str,"jbutton")==0)  type = KEYBIND_JBUTTON;
               else {
                  WARN("Unkown keybinding of type %s", str);
                  continue;
               }

               /* Set modifier, probably should be able to handle two at a time. */
               if (mod != NULL) {
                  if (strcmp(mod,"lctrl")==0)         m = KMOD_LCTRL;
                  else if (strcmp(mod,"rctrl")==0)    m = KMOD_RCTRL;
                  else if (strcmp(mod,"lshift")==0)   m = KMOD_LSHIFT;
                  else if (strcmp(mod,"rshift")==0)   m = KMOD_RSHIFT;
                  else if (strcmp(mod,"lalt")==0)     m = KMOD_LALT;
                  else if (strcmp(mod,"ralt")==0)     m = KMOD_RALT;
                  else if (strcmp(mod,"lmeta")==0)    m = KMOD_LMETA;
                  else if (strcmp(mod,"rmeta")==0)    m = KMOD_RMETA;
                  else if (strcmp(mod,"any")==0)      m = KMOD_ALL;
                  else if (strcmp(mod,"none")==0)     m = 0;
                  else {
                     WARN("Unknown keybinding mod of type %s", mod);
                     m = KMOD_NONE;
                  }
               }
               else
                  m = KMOD_NONE;

               /* set the keybind */
               input_setKeybind( keybindNames[i], type, key, m );
            }
            else {
               WARN("Malformed keybind for '%s' in '%s'.", keybindNames[i], file);
            }
         }
         /* clean up after table stuff */
         lua_pop(L,1);
      }
   }
   else { /* failed to load the config file */
      WARN("Config file '%s' has invalid syntax:", file );
      WARN("   %s", lua_tostring(L,-1));
      lua_close(L);
      return 1;
   }

   lua_close(L);
   return 0;
}


/*
 * parses the CLI options
 */
void conf_parseCLI( int argc, char** argv )
{
   static struct option long_options[] = {
      { "fullscreen", no_argument, 0, 'f' },
      { "fps", required_argument, 0, 'F' },
      { "vsync", no_argument, 0, 'V' },
      { "data", required_argument, 0, 'd' },
      { "joystick", required_argument, 0, 'j' },
      { "Joystick", required_argument, 0, 'J' },
      { "width", required_argument, 0, 'W' },
      { "height", required_argument, 0, 'H' },
      { "mute", no_argument, 0, 'M' },
      { "sound", no_argument, 0, 'S' },
      { "mvol", required_argument, 0, 'm' },
      { "svol", required_argument, 0, 's' },
      { "help", no_argument, 0, 'h' }, 
      { "version", no_argument, 0, 'v' },
      { NULL, 0, 0, 0 } };
   int option_index = 0;
   int c = 0;
   while ((c = getopt_long(argc, argv,
         "fF:Vd:j:J:W:H:MSm:s:Ghv",
         long_options, &option_index)) != -1) {
      switch (c) {
         case 'f':
            conf.fullscreen = 1;
            break;
         case 'F':
            conf.fps_max = atoi(optarg);
            break;
         case 'V':
            conf.vsync = 1;
            break;
         case 'd': 
            conf.ndata = strdup(optarg);
            break;
         case 'j':
            conf.joystick_ind = atoi(optarg);
            break;
         case 'J':
            conf.joystick_nam = strdup(optarg);
            break;
         case 'W':
            conf.width = atoi(optarg);
            conf.explicit_dim = 1;
            break;
         case 'H':
            conf.height = atoi(optarg);
            conf.explicit_dim = 1;
            break;
         case 'M':
            conf.nosound = 1;
            break;
         case 'S':
            conf.nosound = 0;
            break;
         case 'm':
            conf.music = atof(optarg);
            break;
         case 's':
            conf.sound = atof(optarg);
            break;
         case 'G':
            nebu_forceGenerate();
            break;

         case 'v':
            /* by now it has already displayed the version
            LOG(APPNAME": version %d.%d.%d", VMAJOR, VMINOR, VREV); */
            exit(EXIT_SUCCESS);
         case 'h':
            print_usage(argv);
            exit(EXIT_SUCCESS);
      }
   }
}


/* 
 * saves the current configuration
 */
int conf_saveConfig ( const char* file )
{
   /** @todo save conf */
   return 0;
}

