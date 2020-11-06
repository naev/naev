/*
 * See Licensing and Copyright notice in naev.h
 */



#include "conf.h"

#include "naev.h"

#include <stdlib.h> /* atoi */
#include <unistd.h> /* getopt */
#include "nstring.h" /* strdup */
#include <getopt.h> /* getopt_long */

#include "nlua.h"

#include "env.h"
#include "input.h"
#include "log.h"
#include "music.h"
#include "ndata.h"
#include "nebula.h"
#include "nfile.h"
#include "nstring.h"
#include "opengl.h"
#include "player.h"
#include "utf8.h"


#define conf_loadInt( env, n, i )            \
   {                                         \
      nlua_getenv( env, n );                 \
      if ( lua_isnumber( naevL, -1 ) ) {     \
         i = (int)lua_tonumber( naevL, -1 ); \
      }                                      \
      lua_pop( naevL, 1 );                   \
   }

#define conf_loadFloat( env, n, f )             \
   {                                            \
      nlua_getenv( env, n );                    \
      if ( lua_isnumber( naevL, -1 ) ) {        \
         f = (double)lua_tonumber( naevL, -1 ); \
      }                                         \
      lua_pop( naevL, 1 );                      \
   }

#define conf_loadBool( env, n, b )                \
   {                                              \
      nlua_getenv( env, n );                      \
      if ( lua_isnumber( naevL, -1 ) )            \
         b = ( lua_tonumber( naevL, -1 ) != 0. ); \
      else if ( !lua_isnil( naevL, -1 ) )         \
         b = lua_toboolean( naevL, -1 );          \
      lua_pop( naevL, 1 );                        \
   }

#define conf_loadString( env, n, s )              \
   {                                              \
      nlua_getenv( env, n );                      \
      if ( lua_isstring( naevL, -1 ) ) {          \
         if ( s != NULL )                         \
            free( s );                            \
         s = strdup( lua_tostring( naevL, -1 ) ); \
      }                                           \
      lua_pop( naevL, 1 );                        \
   }


/* Global configuration. */
PlayerConf_t conf = {
   .ndata = NULL,
   .language=NULL,
   .sound_backend = NULL,
   .joystick_nam = NULL
};

/* from main.c */
extern int show_fps;
extern int max_fps;
extern int indjoystick;
extern char* namjoystick;
/* from input.c */
extern unsigned int input_afterburnSensitivity;


/*
 * prototypes
 */
static void print_usage( void );


/*
 * prints usage
 */
static void print_usage( void )
{
   LOG( _( "Usage: %s [OPTIONS] [DATA]" ), env.argv0 );
   LOG(_("Options are:"));
   LOG(_("   -f, --fullscreen      activate fullscreen"));
   LOG(_("   -F n, --fps n         limit frames per second to n"));
   LOG(_("   -V, --vsync           enable vsync"));
   LOG(_("   -W n                  set width to n"));
   LOG(_("   -H n                  set height to n"));
   LOG(_("   -j n, --joystick n    use joystick n"));
   LOG(_("   -J s, --Joystick s    use joystick whose name contains s"));
   LOG(_("   -M, --mute            disables sound"));
   LOG(_("   -S, --sound           forces sound"));
   LOG(_("   -m f, --mvol f        sets the music volume to f"));
   LOG(_("   -s f, --svol f        sets the sound volume to f"));
   LOG(_("   -G, --generate        regenerates the nebula (slow)"));
   LOG(_("   -d, --datapath        specifies a custom path for all user data (saves, screenshots, etc.)"));
   LOG(_("   -X, --scale           defines the scale factor"));
#ifdef DEBUGGING
   LOG(_("   --devmode             enables dev mode perks like the editors"));
   LOG(_("   --devcsv              generates csv output from the ndata for development purposes"));
#endif /* DEBUGGING */
   LOG(_("   -h, --help            display this message and exit"));
   LOG(_("   -v, --version         print the version and exit"));
}


/**
 * @brief Sets the default configuration.
 */
void conf_setDefaults (void)
{
   conf_cleanup();

   /* ndata. */
   if (conf.ndata != NULL)
      free(conf.ndata);
   conf.ndata        = NULL;

   /* Language. */
   if (conf.language != NULL)
      free(conf.language);
   conf.language     = NULL;

   /* Joystick. */
   conf.joystick_ind = -1;
   if (conf.joystick_nam != NULL)
      free(conf.joystick_nam);
   conf.joystick_nam = NULL;

   /* GUI. */
   conf.mesg_visible = 5;

   /* Repeat. */
   conf.repeat_delay = 500;
   conf.repeat_freq  = 30;

   /* Dynamic zoom. */
   conf.zoom_manual  = 0;
   conf.zoom_far     = 0.5;
   conf.zoom_near    = 1.;
   conf.zoom_speed   = 0.25;
   conf.zoom_stars   = 1.;

   /* Font sizes. */
   conf.font_size_console = 10;
   conf.font_size_intro   = 18;
   conf.font_size_def     = 12;
   conf.font_size_small   = 11;
   conf.font_name_default = NULL;
   conf.font_name_monospace = NULL;

   /* Misc. */
   conf.redirect_file = 1;
   conf.nosave       = 0;
   conf.devmode      = 0;
   conf.devautosave  = 0;
   conf.devcsv       = 0;
   if (conf.lastversion != NULL)
      free( conf.lastversion );
   conf.lastversion = strdup( "" );

   /* Gameplay. */
   conf_setGameplayDefaults();

   /* Audio. */
   conf_setAudioDefaults();

   /* Video. */
   conf_setVideoDefaults();

   /* Input */
   input_setDefault(1);

   /* Debugging. */
   conf.fpu_except   = 0; /* Causes many issues. */

   /* Editor. */
   if (conf.dev_save_sys != NULL)
      free( conf.dev_save_sys );
   conf.dev_save_sys = strdup( DEV_SAVE_SYSTEM_DEFAULT );
   if (conf.dev_save_map != NULL)
      free( conf.dev_save_map );
   conf.dev_save_map = strdup( DEV_SAVE_MAP_DEFAULT );
   if (conf.dev_save_asset != NULL)
      free( conf.dev_save_asset );
   conf.dev_save_asset = strdup( DEV_SAVE_ASSET_DEFAULT );
}


/**
 * @brief Sets the gameplay defaults.
 */
void conf_setGameplayDefaults (void)
{
   conf.afterburn_sens        = AFTERBURNER_SENSITIVITY_DEFAULT;
   conf.compression_velocity  = TIME_COMPRESSION_DEFAULT_MAX;
   conf.compression_mult      = TIME_COMPRESSION_DEFAULT_MULT;
   conf.save_compress         = SAVE_COMPRESSION_DEFAULT;
   conf.mouse_thrust          = MOUSE_THRUST_DEFAULT;
   conf.mouse_doubleclick     = MOUSE_DOUBLECLICK_TIME;
   conf.autonav_reset_speed   = AUTONAV_RESET_SPEED_DEFAULT;
   conf.zoom_manual           = MANUAL_ZOOM_DEFAULT;
}


/**
 * @brief Sets the audio defaults.
 */
void conf_setAudioDefaults (void)
{
   if (conf.sound_backend != NULL) {
      free(conf.sound_backend);
      conf.sound_backend = NULL;
   }

   /* Sound. */
   conf.sound_backend = strdup(BACKEND_DEFAULT);
   conf.snd_voices   = VOICES_DEFAULT;
   conf.snd_pilotrel = PILOT_RELATIVE_DEFAULT;
   conf.al_efx       = USE_EFX_DEFAULT;
   conf.al_bufsize   = BUFFER_SIZE_DEFAULT;
   conf.nosound      = MUTE_SOUND_DEFAULT;
   conf.sound        = SOUND_VOLUME_DEFAULT;
   conf.music        = MUSIC_VOLUME_DEFAULT;
}


/**
 * @brief Sets the video defaults.
 */
void conf_setVideoDefaults (void)
{
   int w, h, f;
   SDL_DisplayMode resolution;

   /* More complex resolution handling. */
   f = 0;
   if (SDL_GetCurrentDisplayMode( 0, &resolution ) == 0) {
      /* Try higher resolution. */
      w = RESOLUTION_W_DEFAULT;
      h = RESOLUTION_H_DEFAULT;

      /* Fullscreen and fit everything onscreen. */
      if ((resolution.w <= w) || (resolution.h <= h)) {
         w = resolution.w;
         h = resolution.h;
         f = FULLSCREEN_DEFAULT;
      }
   }
   else {
      w = 800;
      h = 600;
   }

   /* OpenGL. */
   conf.fsaa         = FSAA_DEFAULT;
   conf.vsync        = VSYNC_DEFAULT;
   conf.mipmaps      = MIPMAP_DEFAULT; /* Also cause for issues. */
   conf.compress     = TEXTURE_COMPRESSION_DEFAULT;
   conf.interpolate  = INTERPOLATION_DEFAULT;
   conf.npot         = NPOT_TEXTURES_DEFAULT;

   /* Window. */
   conf.fullscreen   = f;
   conf.width        = w;
   conf.height       = h;
   conf.explicit_dim = 0; /* No need for a define, this is only for first-run. */
   conf.scalefactor  = SCALE_FACTOR_DEFAULT;
   conf.minimize     = MINIMIZE_DEFAULT;

   /* FPS. */
   conf.fps_show     = SHOW_FPS_DEFAULT;
   conf.fps_max      = FPS_MAX_DEFAULT;

   /* Pause. */
   conf.pause_show   = SHOW_PAUSE_DEFAULT;

   /* Memory. */
   conf.engineglow   = ENGINE_GLOWS_DEFAULT;
}


/*
 * Frees some memory the conf allocated.
 */
void conf_cleanup (void)
{
   if (conf.ndata != NULL)
      free(conf.ndata);
   if (conf.language != NULL)
      free(conf.language);
   if (conf.sound_backend != NULL)
      free(conf.sound_backend);
   if (conf.joystick_nam != NULL)
      free(conf.joystick_nam);

   if (conf.lastversion != NULL)
      free(conf.lastversion);

   if (conf.dev_save_sys != NULL)
      free(conf.dev_save_sys);
   if (conf.dev_save_map != NULL)
      free(conf.dev_save_map);
   if (conf.dev_save_asset != NULL)
      free(conf.dev_save_asset);

   /* Clear memory. */
   memset( &conf, 0, sizeof(conf) );
}


/*
 * @brief Parses the local conf that dictates where user data goes.
 */
void conf_loadConfigPath( void )
{
   const char *file = "datapath.lua";

   if (!nfile_fileExists(file))
      return;

   nlua_env lEnv = nlua_newEnv( 0 );
   if ( nlua_dofileenv( lEnv, file ) == 0 )
      conf_loadString( lEnv, "datapath", conf.datapath );

   nlua_freeEnv( lEnv );
}


/*
 * parses the config file
 */
int conf_loadConfig ( const char* file )
{
   int i, t;
   const char *str, *mod;
   SDL_Keycode key;
   int type;
   int w,h;
   SDL_Keymod m;

   /* Check to see if file exists. */
   if (!nfile_fileExists(file))
      return nfile_touch(file);

   /* Load the configuration. */
   nlua_env lEnv = nlua_newEnv( 0 );
   if ( nlua_dofileenv( lEnv, file ) == 0 ) {

      /* ndata. */
      conf_loadString( lEnv, "data", conf.ndata );

      /* Language. */
      conf_loadString( lEnv, "language", conf.language );

      /* OpenGL. */
      conf_loadInt( lEnv, "fsaa", conf.fsaa );
      conf_loadBool( lEnv, "vsync", conf.vsync );
      conf_loadBool( lEnv, "mipmaps", conf.mipmaps );
      conf_loadBool( lEnv, "compress", conf.compress );
      conf_loadBool( lEnv, "interpolate", conf.interpolate );
      conf_loadBool( lEnv, "npot", conf.npot );

      /* Memory. */
      conf_loadBool( lEnv, "engineglow", conf.engineglow );

      /* Window. */
      w = h = 0;
      conf_loadInt( lEnv, "width", w );
      conf_loadInt( lEnv, "height", h );
      if (w != 0) {
         conf.explicit_dim = 1;
         conf.width = w;
      }
      if (h != 0) {
         conf.explicit_dim = 1;
         conf.height = h;
      }
      conf_loadFloat( lEnv, "scalefactor", conf.scalefactor );
      conf_loadBool( lEnv, "fullscreen", conf.fullscreen );
      conf_loadBool( lEnv, "modesetting", conf.modesetting );
      conf_loadBool( lEnv, "minimize", conf.minimize );

      /* FPS */
      conf_loadBool( lEnv, "showfps", conf.fps_show );
      conf_loadInt( lEnv, "maxfps", conf.fps_max );

      /*  Pause */
      conf_loadBool( lEnv, "showpause", conf.pause_show );

      /* Sound. */
      conf_loadString( lEnv, "sound_backend", conf.sound_backend );
      conf_loadInt( lEnv, "snd_voices", conf.snd_voices );
      conf.snd_voices = MAX( VOICES_MIN, conf.snd_voices ); /* Must be at least 16. */
      conf_loadBool( lEnv, "snd_pilotrel", conf.snd_pilotrel );
      conf_loadBool( lEnv, "al_efx", conf.al_efx );
      conf_loadInt( lEnv, "al_bufsize", conf.al_bufsize );
      conf_loadBool( lEnv, "nosound", conf.nosound );
      conf_loadFloat( lEnv, "sound", conf.sound );
      conf_loadFloat( lEnv, "music", conf.music );

      /* Joystick. */
      nlua_getenv( lEnv, "joystick" );
      if (lua_isnumber(naevL, -1))
         conf.joystick_ind = (int)lua_tonumber(naevL, -1);
      else if (lua_isstring(naevL, -1))
         conf.joystick_nam = strdup(lua_tostring(naevL, -1));
      lua_pop(naevL,1);

      /* GUI. */
      conf_loadInt( lEnv, "mesg_visible", conf.mesg_visible );
      if (conf.mesg_visible <= 0)
         conf.mesg_visible = 5;

      /* Key repeat. */
      conf_loadInt( lEnv, "repeat_delay", conf.repeat_delay );
      conf_loadInt( lEnv, "repeat_freq", conf.repeat_freq );

      /* Zoom. */
      conf_loadBool( lEnv, "zoom_manual", conf.zoom_manual );
      conf_loadFloat( lEnv, "zoom_far", conf.zoom_far );
      conf_loadFloat( lEnv, "zoom_near", conf.zoom_near );
      conf_loadFloat( lEnv, "zoom_speed", conf.zoom_speed );
      conf_loadFloat( lEnv, "zoom_stars", conf.zoom_stars );

      /* Font size. */
      conf_loadInt( lEnv, "font_size_console", conf.font_size_console );
      conf_loadInt( lEnv, "font_size_intro", conf.font_size_intro );
      conf_loadInt( lEnv, "font_size_def", conf.font_size_def );
      conf_loadInt( lEnv, "font_size_small", conf.font_size_small );
      conf_loadString( lEnv, "font_name_default", conf.font_name_default );
      conf_loadString( lEnv, "font_name_monospace", conf.font_name_monospace );

      /* Misc. */
      conf_loadFloat( lEnv, "compression_velocity", conf.compression_velocity );
      conf_loadFloat( lEnv, "compression_mult", conf.compression_mult );
      conf_loadBool( lEnv, "redirect_file", conf.redirect_file );
      conf_loadBool( lEnv, "save_compress", conf.save_compress );
      conf_loadInt( lEnv, "afterburn_sensitivity", conf.afterburn_sens );
      conf_loadInt( lEnv, "mouse_thrust", conf.mouse_thrust );
      conf_loadFloat( lEnv, "mouse_doubleclick", conf.mouse_doubleclick );
      conf_loadFloat( lEnv, "autonav_abort", conf.autonav_reset_speed );
      conf_loadBool( lEnv, "devmode", conf.devmode );
      conf_loadBool( lEnv, "devautosave", conf.devautosave );
      conf_loadBool( lEnv, "conf_nosave", conf.nosave );
      conf_loadString( lEnv, "lastversion", conf.lastversion );

      /* Debugging. */
      conf_loadBool( lEnv, "fpu_except", conf.fpu_except );

      /* Editor. */
      conf_loadString( lEnv, "dev_save_sys", conf.dev_save_sys );
      conf_loadString( lEnv, "dev_save_map", conf.dev_save_map );
      conf_loadString( lEnv, "dev_save_asset", conf.dev_save_asset );

      /*
       * Keybindings.
       */
      for (i=0; keybind_info[i][0] != NULL; i++) {
         nlua_getenv( lEnv, keybind_info[ i ][ 0 ] );
         /* Handle "none". */
         if (lua_isstring(naevL,-1)) {
            str = lua_tostring(naevL,-1);
            if (strcmp(str,"none")==0) {
               input_setKeybind( keybind_info[i][0],
                     KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
            }
         }
         else if (lua_istable(naevL, -1)) { /* it's a table */
            /* gets the event type */
            lua_pushstring(naevL, "type");
            lua_gettable(naevL, -2);
            if (lua_isstring(naevL, -1))
               str = lua_tostring(naevL, -1);
            else if (lua_isnil(naevL, -1)) {
               WARN(_("Found keybind with no type field!"));
               str = "null";
            }
            else {
               WARN(_("Found keybind with invalid type field!"));
               str = "null";
            }
            lua_pop(naevL,1);

            /* gets the key */
            lua_pushstring(naevL, "key");
            lua_gettable(naevL, -2);
            t = lua_type(naevL, -1);
            if (t == LUA_TNUMBER)
               key = (int)lua_tonumber(naevL, -1);
            else if (t == LUA_TSTRING)
               key = input_keyConv( lua_tostring(naevL, -1));
            else if (t == LUA_TNIL) {
               WARN(_("Found keybind with no key field!"));
               key = SDLK_UNKNOWN;
            }
            else {
               WARN(_("Found keybind with invalid key field!"));
               key = SDLK_UNKNOWN;
            }
            lua_pop(naevL,1);

            /* Get the modifier. */
            lua_pushstring(naevL, "mod");
            lua_gettable(naevL, -2);
            if (lua_isstring(naevL, -1))
               mod = lua_tostring(naevL, -1);
            else
               mod = NULL;
            lua_pop(naevL,1);

            if (str != NULL) { /* keybind is valid */
               /* get type */
               if (strcmp(str,"null")==0)          type = KEYBIND_NULL;
               else if (strcmp(str,"keyboard")==0) type = KEYBIND_KEYBOARD;
               else if (strcmp(str,"jaxispos")==0) type = KEYBIND_JAXISPOS;
               else if (strcmp(str,"jaxisneg")==0) type = KEYBIND_JAXISNEG;
               else if (strcmp(str,"jbutton")==0)  type = KEYBIND_JBUTTON;
               else if (strcmp(str,"jhat_up")==0)  type = KEYBIND_JHAT_UP;
               else if (strcmp(str,"jhat_down")==0)  type = KEYBIND_JHAT_DOWN;
               else if (strcmp(str,"jhat_left")==0)  type = KEYBIND_JHAT_LEFT;
               else if (strcmp(str,"jhat_right")==0)  type = KEYBIND_JHAT_RIGHT;
               else {
                  WARN(_("Unknown keybinding of type %s"), str);
                  continue;
               }

               /* Check to see if it is valid. */
               if ((key == SDLK_UNKNOWN) && (type == KEYBIND_KEYBOARD)) {
                  WARN(_("Keybind for '%s' is invalid"), keybind_info[i][0]);
                  continue;
               }

               /* Set modifier, probably should be able to handle two at a time. */
               if (mod != NULL) {
                  if      (strcmp(mod,"ctrl")==0)    m = NMOD_CTRL;
                  else if (strcmp(mod,"shift")==0)   m = NMOD_SHIFT;
                  else if (strcmp(mod,"alt")==0)     m = NMOD_ALT;
                  else if (strcmp(mod,"meta")==0)    m = NMOD_META;
                  else if (strcmp(mod,"any")==0)     m = NMOD_ALL;
                  else if (strcmp(mod,"none")==0)    m = NMOD_NONE;
                  else {
                     WARN(_("Unknown keybinding mod of type %s"), mod);
                     m = NMOD_NONE;
                  }
               }
               else
                  m = NMOD_NONE;

               /* set the keybind */
               input_setKeybind( keybind_info[i][0], type, key, m );
            }
            else
               WARN(_("Malformed keybind for '%s' in '%s'."), keybind_info[i][0], file);
         }
         /* clean up after table stuff */
         lua_pop(naevL,1);
      }
   }
   else { /* failed to load the config file */
      WARN(_("Config file '%s' has invalid syntax:"), file );
      WARN("   %s", lua_tostring(naevL,-1));
      nlua_freeEnv( lEnv );
      return 1;
   }

   nlua_freeEnv( lEnv );
   return 0;
}


void conf_parseCLIPath( int argc, char** argv )
{
   static struct option long_options[] = {
      { "datapath", required_argument, 0, 'd' },
      { NULL, 0, 0, 0 }
   };

   int option_index = 1;
   int c = 0;

   /* GNU giveth, and GNU taketh away.
    * If we don't specify "-" as the first char, getopt will happily
    * mangle the initial argument order, probably causing crashes when
    * passing arguments that take values, such as -H and -W.
    */
   while ((c = getopt_long(argc, argv, "-:d:",
         long_options, &option_index)) != -1) {
      switch(c) {
         case 'd':
            conf.datapath = strdup(optarg);
            break;
      }
   }
}


/*
 * parses the CLI options
 */
void conf_parseCLI( int argc, char** argv )
{
   static struct option long_options[] = {
      { "datapath", required_argument, 0, 'd' },
      { "fullscreen", no_argument, 0, 'f' },
      { "fps", required_argument, 0, 'F' },
      { "vsync", no_argument, 0, 'V' },
      { "joystick", required_argument, 0, 'j' },
      { "Joystick", required_argument, 0, 'J' },
      { "width", required_argument, 0, 'W' },
      { "height", required_argument, 0, 'H' },
      { "mute", no_argument, 0, 'M' },
      { "sound", no_argument, 0, 'S' },
      { "mvol", required_argument, 0, 'm' },
      { "svol", required_argument, 0, 's' },
      { "generate", no_argument, 0, 'G' },
      { "scale", required_argument, 0, 'X' },
#ifdef DEBUGGING
      { "devmode", no_argument, 0, 'D' },
      { "devcsv", no_argument, 0, 'C' },
#endif /* DEBUGGING */
      { "help", no_argument, 0, 'h' },
      { "version", no_argument, 0, 'v' },
      { NULL, 0, 0, 0 } };
   int option_index = 1;
   int c = 0;

   /* man 3 getopt says optind should be initialized to 1, but that seems to
    * cause all options to get parsed, i.e. we cannot detect a trailing ndata
    * option.
    */
   optind = 0;
   while ((c = getopt_long(argc, argv,
         "fF:Vd:j:J:W:H:MSm:s:X:GNhv",
         long_options, &option_index)) != -1) {
      switch (c) {
         case 'd':
            /* Does nothing, datapath is parsed earlier. */
            break;
         case 'f':
            conf.fullscreen = 1;
            break;
         case 'F':
            conf.fps_max = atoi(optarg);
            break;
         case 'V':
            conf.vsync = 1;
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
         case 'N':
            if (conf.ndata != NULL)
               free(conf.ndata);
            conf.ndata = NULL;
            break;
         case 'X':
            conf.scalefactor = atof(optarg);
            break;
#ifdef DEBUGGING
         case 'D':
            conf.devmode = 1;
            LOG(_("Enabling developer mode."));
            break;

         case 'C':
            conf.devcsv = 1;
            LOG(_("Will generate CSV output."));
            break;
#endif /* DEBUGGING */

         case 'v':
            /* by now it has already displayed the version */
            exit(EXIT_SUCCESS);
         case 'h':
            print_usage();
            exit(EXIT_SUCCESS);
      }
   }

   /** @todo handle multiple ndata. */
   if (optind < argc) {
      if (conf.ndata != NULL)
         free(conf.ndata);
      conf.ndata = strdup( argv[ optind ] );
   }
}


/**
 * @brief nsnprintf-like function to quote and escape a string for use in Lua source code
 *
 *    @param str The destination buffer
 *    @param size The maximum amount of space in str to use
 *    @param text The string to quote and escape
 *    @return The number of characters actually written to str
 */
static size_t quoteLuaString(char *str, size_t size, const char *text)
{
   char slashescape;
   size_t count, i;
   uint32_t ch;

   if (size == 0)
      return 0;

   /* Write a Lua nil if we are given a NULL pointer */
   if (text == NULL)
      return nsnprintf(str, size, "nil");

   count = 0;

   /* Quote start */
   str[count++] = '\"';
   if (count == size)
      return count;

   /* Iterate over the characters in text */
   i = 0;
   while ((ch = u8_nextchar( text, &i ))) {
      /* Check if we can print this as a friendly backslash-escape */
      switch (ch) {
         case '\a':  slashescape = 'a';   break;
         case '\b':  slashescape = 'b';   break;
         case '\f':  slashescape = 'f';   break;
         case '\n':  slashescape = 'n';   break;
         case '\r':  slashescape = 'r';   break;
         case '\t':  slashescape = 't';   break;
         case '\v':  slashescape = 'v';   break;
         case '\\':  slashescape = '\\';  break;
         case '\"':  slashescape = '\"';  break;
         case '\'':  slashescape = '\'';  break;
         /* Technically, Lua can also represent \0, but we can't in our input */
         default:    slashescape = 0;     break;
      }
      if (slashescape != 0) {
         /* Yes, we can use a backslash-escape! */
         str[count++] = '\\';
         if (count == size)
            return count;

         str[count++] = slashescape;
         if (count == size)
            return count;

         continue;
      }

      /* Render UTF8. */
      count += u8_toutf8( &str[count], size-count, &ch, 1 );
      if (count >= size)
         return count;
   }

   /* Quote end */
   str[count++] = '\"';
   if (count == size)
      return count;

   /* zero-terminate, if possible */
   if (count != size)
      str[count] = '\0';   /* don't increase count, like nsnprintf */

   /* return the amount of characters written */
   return count;
}


#define  conf_saveComment(t)     \
pos += nsnprintf(&buf[pos], sizeof(buf)-pos, "-- %s\n", t);

#define  conf_saveEmptyLine()     \
if (sizeof(buf) != pos) \
   buf[pos++] = '\n';

#define  conf_saveInt(n,i)    \
pos += nsnprintf(&buf[pos], sizeof(buf)-pos, "%s = %d\n", n, i);

#define  conf_saveFloat(n,f)    \
pos += nsnprintf(&buf[pos], sizeof(buf)-pos, "%s = %f\n", n, f);

#define  conf_saveBool(n,b)    \
if (b) \
   pos += nsnprintf(&buf[pos], sizeof(buf)-pos, "%s = true\n", n); \
else \
   pos += nsnprintf(&buf[pos], sizeof(buf)-pos, "%s = false\n", n);

#define  conf_saveString(n,s) \
pos += nsnprintf(&buf[pos], sizeof(buf)-pos, "%s = ", n); \
pos += quoteLuaString(&buf[pos], sizeof(buf)-pos, s); \
if (sizeof(buf) != pos) \
   buf[pos++] = '\n';

#define GENERATED_START_COMMENT  "START GENERATED SECTION"
#define GENERATED_END_COMMENT    "END GENERATED SECTION"


/*
 * saves the current configuration
 */
int conf_saveConfig ( const char* file )
{
   int i;
   char *old;
   const char *oldfooter;
   size_t oldsize;
   char buf[32*1024];
   size_t pos;
   SDL_Keycode key;
   char keyname[17];
   KeybindType type;
   const char *typename;
   SDL_Keymod mod;
   const char *modname;

   pos         = 0;
   oldfooter   = NULL;

   /* User doesn't want to save the config. */
   if (conf.nosave)
      return 0;

   /* Read the old configuration, if possible */
   if (nfile_fileExists(file) && (old = nfile_readFile(&oldsize, file)) != NULL) {
      /* See if we can find the generated section and preserve
       * whatever the user wrote before it */
      const char *tmp = nstrnstr(old, "-- "GENERATED_START_COMMENT"\n", oldsize);
      if (tmp != NULL) {
         /* Copy over the user content */
         pos = MIN(sizeof(buf), (size_t)(tmp - old));
         memcpy(buf, old, pos);

         /* See if we can find the end of the section */
         tmp = nstrnstr(tmp, "-- "GENERATED_END_COMMENT"\n", oldsize-pos);
         if (tmp != NULL) {
            /* Everything after this should also be preserved */
            oldfooter = tmp + strlen("-- "GENERATED_END_COMMENT"\n");
            oldsize -= (oldfooter - old);
         }
      }
      else {
         /* Treat the contents of the old file as a footer. */
         oldfooter = old;
      }
   }
   else {
      old = NULL;

      /* Write a nice header for new configuration files */
      conf_saveComment(_("Naev configuration file"));
      conf_saveEmptyLine();
   }

   /* Back up old configuration. */
   if (nfile_backupIfExists(file) < 0) {
      WARN(_("Not saving configuration."));
      return -1;
   }

   /* Header. */
   conf_saveComment(GENERATED_START_COMMENT);
   conf_saveComment(_("The contents of this section will be rewritten by Naev!"));
   conf_saveEmptyLine();

   /* ndata. */
   conf_saveComment(_("The location of Naev's data pack, usually called 'ndata'"));
   conf_saveString("data",conf.ndata);
   conf_saveEmptyLine();

   /* Language. */
   conf_saveComment(_("Language to use. Set to the two character identifier to the language (e.g., \"en\" for English), and nil for autodetect."));
   conf_saveString("language",conf.language);
   conf_saveEmptyLine();

   /* OpenGL. */
   conf_saveComment(_("The factor to use in Full-Scene Anti-Aliasing"));
   conf_saveComment(_("Anything lower than 2 will simply disable FSAA"));
   conf_saveInt("fsaa",conf.fsaa);
   conf_saveEmptyLine();

   conf_saveComment(_("Synchronize framebuffer updates with the vertical blanking interval"));
   conf_saveBool("vsync",conf.vsync);
   conf_saveEmptyLine();

   conf_saveComment(_("Use OpenGL MipMaps"));
   conf_saveBool("mipmaps",conf.mipmaps);
   conf_saveEmptyLine();

   conf_saveComment(_("Use OpenGL Texture Compression"));
   conf_saveBool("compress",conf.compress);
   conf_saveEmptyLine();

   conf_saveComment(_("Use OpenGL Texture Interpolation"));
   conf_saveBool("interpolate",conf.interpolate);
   conf_saveEmptyLine();

   conf_saveComment(_("Use OpenGL Non-\"Power of Two\" textures if available"));
   conf_saveComment(_("Lowers memory usage by a lot, but may cause slow downs on some systems"));
   conf_saveBool("npot",conf.npot);
   conf_saveEmptyLine();

   /* Memory. */
   conf_saveComment(_("If true enables engine glow"));
   conf_saveBool("engineglow",conf.engineglow);
   conf_saveEmptyLine();

   /* Window. */
   conf_saveComment(_("The window size or screen resolution"));
   conf_saveComment(_("Set both of these to 0 to make Naev try the desktop resolution"));
   if (conf.explicit_dim) {
      conf_saveInt("width",conf.width);
      conf_saveInt("height",conf.height);
   } else {
      conf_saveInt("width",0);
      conf_saveInt("height",0);
   }
   conf_saveEmptyLine();

   conf_saveComment(_("Factor used to divide the above resolution with"));
   conf_saveComment(_("This is used to lower the rendering resolution, and scale to the above"));
   conf_saveFloat("scalefactor",conf.scalefactor);
   conf_saveEmptyLine();

   conf_saveComment(_("Run Naev in full-screen mode"));
   conf_saveBool("fullscreen",conf.fullscreen);
   conf_saveEmptyLine();

   conf_saveComment(_("Use video modesetting when fullscreen is enabled"));
   conf_saveBool("modesetting",conf.modesetting);
   conf_saveEmptyLine();

   conf_saveComment(_("Minimize on focus loss"));
   conf_saveBool("minimize",conf.minimize);
   conf_saveEmptyLine();

   /* FPS */
   conf_saveComment(_("Display a framerate counter"));
   conf_saveBool("showfps",conf.fps_show);
   conf_saveEmptyLine();

   conf_saveComment(_("Limit the rendering framerate"));
   conf_saveInt("maxfps",conf.fps_max);
   conf_saveEmptyLine();

   /* Pause */
   conf_saveComment(_("Show 'PAUSED' on screen while paused"));
   conf_saveBool("showpause",conf.pause_show);
   conf_saveEmptyLine();

   /* Sound. */
   conf_saveComment(_("Sound backend (can be \"openal\" or \"sdlmix\")"));
   conf_saveString("sound_backend",conf.sound_backend);
   conf_saveEmptyLine();

   conf_saveComment(_("Maxmimum number of simultaneous sounds to play, must be at least 16."));
   conf_saveInt("snd_voices",conf.snd_voices);
   conf_saveEmptyLine();

   conf_saveComment(_("Sets sound to be relative to pilot when camera is following a pilot instead of referenced to camera."));
   conf_saveBool("snd_pilotrel",conf.snd_pilotrel);
   conf_saveEmptyLine();

   conf_saveComment(_("Enables EFX extension for OpenAL backend."));
   conf_saveBool("al_efx",conf.al_efx);
   conf_saveEmptyLine();

   conf_saveComment(_("Size of the OpenAL music buffer (in kibibytes)."));
   conf_saveInt("al_bufsize",conf.al_bufsize);
   conf_saveEmptyLine();

   conf_saveComment(_("Disable all sound"));
   conf_saveBool("nosound",conf.nosound);
   conf_saveEmptyLine();

   conf_saveComment(_("Volume of sound effects and music, between 0.0 and 1.0"));
   conf_saveFloat("sound",(sound_disabled) ? conf.sound : sound_getVolume());
   conf_saveFloat("music",(music_disabled) ? conf.music : music_getVolume());
   conf_saveEmptyLine();

   /* Joystick. */
   conf_saveComment(_("The name or numeric index of the joystick to use"));
   conf_saveComment(_("Setting this to nil disables the joystick support"));
   if (conf.joystick_nam != NULL) {
      conf_saveString("joystick",conf.joystick_nam);
   }
   else if (conf.joystick_ind >= 0) {
      conf_saveInt("joystick",conf.joystick_ind);
   }
   else {
      conf_saveString("joystick",NULL);
   }
   conf_saveEmptyLine();

   /* GUI. */
   conf_saveComment(_("Number of lines visible in the comm window."));
   conf_saveInt("mesg_visible",conf.mesg_visible);
   conf_saveEmptyLine();

   /* Key repeat. */
   conf_saveComment(_("Delay in ms before starting to repeat (0 disables)"));
   conf_saveInt("repeat_delay",conf.repeat_delay);
   conf_saveComment(_("Delay in ms between repeats once it starts to repeat"));
   conf_saveInt("repeat_freq",conf.repeat_freq);
   conf_saveEmptyLine();

   /* Zoom. */
   conf_saveComment(_("Minimum and maximum zoom factor to use in-game"));
   conf_saveComment(_("At 1.0, no sprites are scaled"));
   conf_saveComment(_("zoom_far should be less then zoom_near"));
   conf_saveBool("zoom_manual",conf.zoom_manual);
   conf_saveFloat("zoom_far",conf.zoom_far);
   conf_saveFloat("zoom_near",conf.zoom_near);
   conf_saveEmptyLine();

   conf_saveComment(_("Zooming speed in factor increments per second"));
   conf_saveFloat("zoom_speed",conf.zoom_speed);
   conf_saveEmptyLine();

   conf_saveComment(_("Zooming modulation factor for the starry background"));
   conf_saveFloat("zoom_stars",conf.zoom_stars);
   conf_saveEmptyLine();

   /* Fonts. */
   conf_saveComment(_("Font sizes (in pixels) for Naev"));
   conf_saveComment(_("Warning, setting to other than the default can cause visual glitches!"));
   conf_saveComment(_("Console default: 10"));
   conf_saveInt("font_size_console",conf.font_size_console);
   conf_saveComment(_("Intro default: 18"));
   conf_saveInt("font_size_intro",conf.font_size_intro);
   conf_saveComment(_("Default size: 12"));
   conf_saveInt("font_size_def",conf.font_size_def);
   conf_saveComment(_("Small size: 10"));
   conf_saveInt("font_size_small",conf.font_size_small);
   conf_saveComment(_("Default font to use: unset"));
   if (conf.font_name_default) {
      conf_saveString("font_name_default",conf.font_name_default);
   }
   else {
      conf_saveComment(_("font_name_default = \"/path/to/file.ttf\""));
   }
   conf_saveComment("Default monospace font to use: unset");
   if (conf.font_name_monospace) {
      conf_saveString("font_name_monospace",conf.font_name_monospace);
   }
   else {
      conf_saveComment("font_name_monospace = \"/path/to/file.ttf\"");
   }
   conf_saveEmptyLine();

   /* Misc. */
   conf_saveComment(_("Sets the velocity (px/s) to compress up to when time compression is enabled."));
   conf_saveFloat("compression_velocity",conf.compression_velocity);
   conf_saveEmptyLine();

   conf_saveComment(_("Sets the multiplier to compress up to when time compression is enabled."));
   conf_saveFloat("compression_mult",conf.compression_mult);
   conf_saveEmptyLine();

   conf_saveComment(_("Redirects log and error output to files"));
   conf_saveBool("redirect_file",conf.redirect_file);
   conf_saveEmptyLine();

   conf_saveComment(_("Enables compression on savegames"));
   conf_saveBool("save_compress",conf.save_compress);
   conf_saveEmptyLine();

   conf_saveComment(_("Afterburner sensitivity"));
   conf_saveInt("afterburn_sensitivity",conf.afterburn_sens);
   conf_saveEmptyLine();

   conf_saveComment(_("Mouse-flying thrust control"));
   conf_saveInt("mouse_thrust",conf.mouse_thrust);
   conf_saveEmptyLine();

   conf_saveComment(_("Maximum interval to count as a double-click (0 disables)."));
   conf_saveFloat("mouse_doubleclick",conf.mouse_doubleclick);
   conf_saveEmptyLine();

   conf_saveComment(_("Condition under which the autonav aborts."));
   conf_saveFloat("autonav_abort",conf.autonav_reset_speed);
   conf_saveEmptyLine();

   conf_saveComment(_("Enables developer mode (universe editor and the likes)"));
   conf_saveBool("devmode",conf.devmode);
   conf_saveEmptyLine();

   conf_saveComment(_("Automatic saving for when using the universe editor whenever an edit is done"));
   conf_saveBool("devautosave",conf.devautosave);
   conf_saveEmptyLine();

   conf_saveComment(_("Save the config everytime game exits (rewriting this bit)"));
   conf_saveInt("conf_nosave",conf.nosave);
   conf_saveEmptyLine();

   conf_saveComment(_("Indicates the last version the game has run in before"));
   conf_saveString("lastversion", conf.lastversion);
   conf_saveEmptyLine();

   /* Debugging. */
   conf_saveComment(_("Enables FPU exceptions - only works on DEBUG builds"));
   conf_saveBool("fpu_except",conf.fpu_except);
   conf_saveEmptyLine();

   /* Editor. */
   conf_saveComment(_("Paths for saving different files from the editor"));
   conf_saveString("dev_save_sys",conf.dev_save_sys);
   conf_saveString("dev_save_map",conf.dev_save_map);
   conf_saveString("dev_save_asset",conf.dev_save_asset);
   conf_saveEmptyLine();

   /*
    * Keybindings.
    */
   conf_saveEmptyLine();
   conf_saveComment(_("Keybindings"));
   conf_saveEmptyLine();

   /* Use an extra character in keyname to make sure it's always zero-terminated */
   keyname[sizeof(keyname)-1] = '\0';

   /* Iterate over the keybinding names */
   for (i=0; keybind_info[i][0] != NULL; i++) {
      /* Save a comment line containing the description */
      conf_saveComment(input_getKeybindDescription( keybind_info[i][0] ));

      /* Get the keybind */
      key = input_getKeybind( keybind_info[i][0], &type, &mod );

      /* Determine the textual name for the keybind type */
      switch (type) {
         case KEYBIND_KEYBOARD:  typename = "keyboard";  break;
         case KEYBIND_JAXISPOS:  typename = "jaxispos";  break;
         case KEYBIND_JAXISNEG:  typename = "jaxisneg";  break;
         case KEYBIND_JBUTTON:   typename = "jbutton";   break;
         case KEYBIND_JHAT_UP:   typename = "jhat_up";   break;
         case KEYBIND_JHAT_DOWN: typename = "jhat_down"; break;
         case KEYBIND_JHAT_LEFT: typename = "jhat_left"; break;
         case KEYBIND_JHAT_RIGHT:typename = "jhat_right";break;
         default:                typename = NULL;        break;
      }
      /* Write a nil if an unknown type */
      if ((typename == NULL) || (key == SDLK_UNKNOWN && type == KEYBIND_KEYBOARD)) {
         conf_saveString( keybind_info[i][0],"none");
         continue;
      }

      /* Determine the textual name for the modifier */
      switch ((int)mod) {
         case NMOD_CTRL:  modname = "ctrl";   break;
         case NMOD_SHIFT: modname = "shift";  break;
         case NMOD_ALT:   modname = "alt";    break;
         case NMOD_META:  modname = "meta";   break;
         case NMOD_ALL:   modname = "any";     break;
         default:         modname = "none";    break;
      }

      /* Determine the textual name for the key, if a keyboard keybind */
      if (type == KEYBIND_KEYBOARD)
         quoteLuaString(keyname, sizeof(keyname)-1, SDL_GetKeyName(key));
      /* If SDL can't describe the key, store it as an integer */
      if (type != KEYBIND_KEYBOARD || strcmp(keyname, "\"unknown key\"") == 0)
         nsnprintf(keyname, sizeof(keyname)-1, "%d", key);

      /* Write out a simple Lua table containing the keybind info */
      pos += nsnprintf(&buf[pos], sizeof(buf)-pos, "%s = { type = \"%s\", mod = \"%s\", key = %s }\n",
            keybind_info[i][0], typename, modname, keyname);
   }
   conf_saveEmptyLine();

   /* Footer. */
   conf_saveComment(GENERATED_END_COMMENT);

   if (old != NULL) {
      if (oldfooter != NULL) {
         /* oldfooter and oldsize now reference the old content past the footer */
         oldsize = MIN((size_t)oldsize, sizeof(buf)-pos);
         memcpy(&buf[pos], oldfooter, oldsize);
         pos += oldsize;
      }
      free(old);
   }

   if (nfile_writeFile(buf, pos, file) < 0) {
      WARN(_("Failed to write configuration!  You'll most likely have to restore it by copying your backup configuration over your current configuration."));
      return -1;
   }

   return 0;
}

