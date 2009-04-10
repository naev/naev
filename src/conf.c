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
   /* opengl */
   gl_screen.w = 800;
   gl_screen.h = 600;
   gl_screen.flags = 0;
   gl_screen.fsaa = 4; /* Only used if activated. */
   gl_setScale(1.); /* No scaling. */
   /* openal */
   nosound = 0;
   /* joystick */
   indjoystick = -1;
   namjoystick = NULL;
   /* input */
   input_setDefault();
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
   int w,h, fsaa;
   SDLMod m;

   i = 0;
   d = 0.;

   /* Check to see if file exists. */
   if (!nfile_fileExists(file))
      return nfile_touch(file);

   /* Load the configuration. */
   lua_State *L = nlua_newState();
   if (luaL_dofile(L, file) == 0) {

      /* global */
      lua_getglobal(L, "data");
      if (lua_isstring(L, -1))
         ndata_setPath( lua_tostring(L, -1) );
      lua_pop(L,1);

      /*
       * opengl properties
       */
      /* Dimensions. */
      w = h = 0;
      conf_loadInt("width",w);
      conf_loadInt("height",h);
      if (w != 0) {
         gl_screen.flags |= OPENGL_DIM_DEF;
         gl_screen.w = w;
      }
      if (h != 0) {
         gl_screen.flags |= OPENGL_DIM_DEF;
         gl_screen.h = h;
      }
      /* Scalefactor. */
      d = 1.;
      conf_loadFloat("scalefactor",d);
      if (d!=1.)
         gl_setScale(d);
      /* FSAA */
      fsaa = 0;
      conf_loadInt("fsaa",fsaa);
      if (fsaa > 0) {
         gl_screen.flags |= OPENGL_FSAA;
         gl_screen.fsaa = fsaa;
      }
      /* Fullscreen. */
      conf_loadBool("fullscreen",i);
      if (i) { gl_screen.flags |= OPENGL_FULLSCREEN; i = 0; }
      /* Anti aliasing. */
      conf_loadBool("aa",i);
      if (i) {
         gl_screen.flags |= OPENGL_AA_POINT | OPENGL_AA_LINE | OPENGL_AA_POLYGON;
         i = 0; }
      conf_loadBool("aa_point",i);
      if (i) { gl_screen.flags |= OPENGL_AA_POINT; i = 0; }
      conf_loadBool("aa_line",i);
      if (i) { gl_screen.flags |= OPENGL_AA_LINE; i = 0; }
      conf_loadBool("aa_polygon",i);
      if (i) { gl_screen.flags |= OPENGL_AA_POLYGON; i = 0; }
      /* Vsync. */
      conf_loadBool("vsync",i);
      if (i) { gl_screen.flags |= OPENGL_VSYNC; i = 0; }

      /* FPS */
      conf_loadBool("showfps",show_fps);
      conf_loadInt("maxfps",max_fps);

      /* input */
      i = 250;
      conf_loadInt("afterburn_sensitivity",i);
      input_afterburnSensitivity = (i < 0) ? 0 : (unsigned int)i;
      i = 0;

      /* 
       * sound
       */
      conf_loadBool("nosound",i)
      nosound = i; i = 0;
      conf_loadFloat("sound",d);
      if (d) {
         sound_defVolume = CLAMP( 0., 1., d );
         if (d == 0.)
            sound_disabled = 1;
         d = 0.;
      }
      conf_loadFloat("music",d);
      if (d) {
         music_defVolume = CLAMP( 0., 1., d );
         if (d == 0.)
            music_disabled = 1;
         d = 0.;
      }


      /* 
       * Joystick.
       */
      lua_getglobal(L, "joystick");
      if (lua_isnumber(L, -1))
         indjoystick = (int)lua_tonumber(L, -1);
      else if (lua_isstring(L, -1))
         namjoystick = strdup(lua_tostring(L, -1));
      lua_pop(L,1);


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
   double d;
   while ((c = getopt_long(argc, argv,
         "fF:Vd:j:J:W:H:MSm:s:Ghv",
         long_options, &option_index)) != -1) {
      switch (c) {
         case 'f':
            gl_screen.flags |= OPENGL_FULLSCREEN;
            break;
         case 'F':
            max_fps = atoi(optarg);
            break;
         case 'V':
            gl_screen.flags |= OPENGL_VSYNC;
            break;
         case 'd': 
            ndata_setPath(optarg);
            break;
         case 'j':
            indjoystick = atoi(optarg);
            break;
         case 'J':
            namjoystick = strdup(optarg);
            break;
         case 'W':
            gl_screen.w = atoi(optarg);
            gl_screen.flags |= OPENGL_DIM_DEF;
            break;
         case 'H':
            gl_screen.h = atoi(optarg);
            gl_screen.flags |= OPENGL_DIM_DEF;
            break;
         case 'M':
            nosound = 1;
            break;
         case 'S':
            nosound = 0;
            break;
         case 'm':
            d = atof(optarg);
            music_defVolume = MAX(MIN(d, 1.), 0.);
            if (d == 0.)
               music_disabled = 1;
            break;
         case 's':
            d = atof(optarg);
            sound_defVolume = MAX(MIN(d, 1.), 0.);
            if (d == 0.)
               sound_disabled = 1;
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
int conf_saveConfig (void)
{
   /** @todo save conf */
   return 0;
}

