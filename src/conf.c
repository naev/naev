/*
 * See Licensing and Copyright notice in naev.h
 */



#include "conf.h"

#include <stdlib.h> /* atoi */
#include <unistd.h> /* getopt */
#include <string.h> /* strdup */
#include <getopt.h> /* getopt_long */

#include "nlua.h"
#include "lauxlib.h" /* luaL_dofile */

#include "naev.h"
#include "log.h"
#include "player.h"
#include "input.h"
#include "opengl.h"
#include "music.h"
#include "nebulae.h"
#include "pack.h"
#include "nfile.h"


#define  conf_loadInt(n,i)    \
lua_getglobal(L,n); \
if (lua_isnumber(L, -1)) { \
   i = (int)lua_tonumber(L, -1); \
} \
lua_remove(L,-1);

#define  conf_loadFloat(n,f)    \
lua_getglobal(L,n); \
if (lua_isnumber(L, -1)) { \
   f = (double)lua_tonumber(L, -1); \
} \
lua_remove(L,-1);

#define  conf_loadBool(n,b)   \
lua_getglobal(L, n); \
if (lua_isnumber(L, -1)) { \
   if ((int)lua_tonumber(L, -1) == 1) \
      b = 1; \
} \
else if (lua_isboolean(L, -1)) \
   b = lua_toboolean(L, -1); \
lua_remove(L,-1);

#define  conf_loadString(n,s) \
   lua_getglobal(L, n); \
if (lua_isstring(L, -1)) { \
   s = strdup((char*)lua_tostring(L, -1));   \
} \
lua_remove(L,-1);


/* from main.c */
extern int nosound;
extern int show_fps;
extern int max_fps;
extern int indjoystick;
extern char* namjoystick;
/* from player.c */
extern const char *keybindNames[]; /* keybindings */
/* from input.c */
extern unsigned int input_afterburnSensibility;


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
   int i, nfiles;
   char **files;
   size_t len;

   /* find data */
   if (nfile_fileExists("%s-%d.%d.%d", DATA_NAME, VMAJOR, VMINOR, VREV )) {
      data = malloc(PATH_MAX);
      snprintf( data, PATH_MAX, "%s-%d.%d.%d", DATA_NAME, VMAJOR, VMINOR, VREV );
   }
   else if (nfile_fileExists(DATA_DEF))
      data = DATA_DEF;
   else {
      files = nfile_readDir( &nfiles, "." );
      len = strlen(DATA_NAME);
      for (i=0; i<nfiles; i++) {
         if (strncmp(files[i], DATA_NAME, len)==0) {
            /* Must be packfile. */
            if (pack_check(files[i]))
               continue;

            data = strdup(files[i]);
         }
      }

      for (i=0; i<nfiles; i++)
         free(files[i]);
      free(files);
   }
   /* opengl */
   gl_screen.w = 800;
   gl_screen.h = 600;
   gl_screen.flags = 0;
   gl_screen.fsaa = 4; /* Only used if activated. */
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
   char *str, *mod;
   int type, key, reverse;
   int w,h, fsaa;
   SDLMod m;

   i = 0;
   d = 0.;

   lua_State *L = nlua_newState();
   if (luaL_dofile(L, file) == 0) { /* configuration file exists */

      /* global */
      conf_loadString("data",data);

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
      conf_loadInt("afterburn",input_afterburnSensibility);

      /* 
       * sound
       */
      conf_loadBool("nosound",i)
      nosound = i; i = 0;
      conf_loadFloat("sound",d);
      if (d) { sound_volume(d); d = 0.; }
      conf_loadFloat("music",d);
      if (d) { music_volume(d); d = 0.; }


      /* 
       * Joystick.
       */
      lua_getglobal(L, "joystick");
      if (lua_isnumber(L, -1)) {
         indjoystick = (int)lua_tonumber(L, -1);
         lua_remove(L,-1);
      }
      else if (lua_isstring(L, -1)) {
         namjoystick = strdup((char*)lua_tostring(L, -1));
         lua_remove(L,-1);
      }


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
               str = (char*)lua_tostring(L, -1);
            else {
               WARN("Found keybind with no type field!");
               str = "null";
            }
            lua_remove(L, -1);

            /* gets the key */
            lua_pushstring(L, "key");
            lua_gettable(L, -2);
            if (lua_isnumber(L, -1))
               key = (int)lua_tonumber(L, -1);
            else {
               WARN("Found keybind with no key field!");
               key = -1;
            }
            lua_remove(L, -1);

            /* is reversed, only useful for axis */
            lua_pushstring(L, "reverse");
            lua_gettable(L, -2);
            if (lua_isnumber(L, -1))
               reverse = !!(int)lua_tonumber(L, -1);
            else if (lua_isboolean(L, -1))
               reverse = lua_toboolean(L, -1);
            else
               reverse = 0;
            lua_remove(L, -1);

            /* Get the modifier. */
            lua_pushstring(L, "mod");
            lua_gettable(L, -2);
            if (lua_isstring(L, -1))
               mod = (char*)lua_tostring(L, -1);
            else
               mod = NULL;
            lua_remove(L, -1);

            if ((key != -1) && (str != NULL)) { /* keybind is valid */
               /* get type */
               if (strcmp(str,"null")==0) type = KEYBIND_NULL;
               else if (strcmp(str,"keyboard")==0) type = KEYBIND_KEYBOARD;
               else if (strcmp(str,"jaxis")==0) type = KEYBIND_JAXIS;
               else if (strcmp(str,"jbutton")==0) type = KEYBIND_JBUTTON;
               else {
                  WARN("Unkown keybinding of type %s", str);
                  continue;
               }

               /* Set modifier, probably should be able to handle two at a time. */
               if (mod != NULL) {
                  if (strcmp(mod,"lctrl")==0) m = KMOD_LCTRL;
                  else if (strcmp(mod,"rctrl")==0) m = KMOD_RCTRL;
                  else if (strcmp(mod,"lshift")==0) m = KMOD_LSHIFT;
                  else if (strcmp(mod,"rshift")==0) m = KMOD_RSHIFT;
                  else if (strcmp(mod,"lalt")==0) m = KMOD_LALT;
                  else if (strcmp(mod,"ralt")==0) m = KMOD_RALT;
                  else if (strcmp(mod,"lmeta")==0) m = KMOD_LMETA;
                  else if (strcmp(mod,"rmeta")==0) m = KMOD_RMETA;
                  else {
                     WARN("Unknown keybinding mod of type %s", mod);
                     m = KMOD_NONE;
                  }
               }
               else
                  m = KMOD_NONE;

               /* set the keybind */
               input_setKeybind( (char*)keybindNames[i], type, key, m, reverse );
            }
            else WARN("Malformed keybind in %s", file);              

            /* clean up after table stuff */
            lua_remove(L,-1);
         }
         lua_remove(L,-1);
      }
   }
   else { /* failed to load the config file */
      lua_close(L);
      DEBUG("config file '%s' not found", file);
      nfile_touch(file);
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
            gl_screen.flags |= OPENGL_FULLSCREEN;
            break;
         case 'F':
            max_fps = atoi(optarg);
            break;
         case 'V':
            gl_screen.flags |= OPENGL_VSYNC;
            break;
         case 'd': 
            data = strdup(optarg);
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
            music_volume( atof(optarg) );
            break;
         case 's':
            /*sound_volume( atof(optarg) );*/
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

