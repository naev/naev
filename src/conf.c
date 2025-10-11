/*
 * See Licensing and Copyright notice in naev.h
 */
/** @cond */
#include <getopt.h> /* getopt_long */
#include <stdlib.h> /* atoi */
#include <unistd.h> /* getopt */

#include "naev.h"
/** @endcond */

#include "lualib.h"
#include "physfs.h"

#include "conf.h"

#include "background.h"
#include "env.h"
#include "input.h"
#include "log.h"
#include "music.h"
#include "nfile.h"
#include "nlua.h"
#include "nstring.h"
#include "sound.h"
#include "space.h"
#include "utf8.h"

#define conf_loadInt( L, n, i )                                                \
   {                                                                           \
      lua_getglobal( L, n );                                                   \
      if ( lua_isnumber( L, -1 ) ) {                                           \
         i = lua_tointeger( L, -1 );                                           \
      }                                                                        \
      lua_pop( L, 1 );                                                         \
   }

#define conf_loadFloat( L, n, f )                                              \
   {                                                                           \
      lua_getglobal( L, n );                                                   \
      if ( lua_isnumber( L, -1 ) ) {                                           \
         f = lua_tonumber( L, -1 );                                            \
      }                                                                        \
      lua_pop( L, 1 );                                                         \
   }

#define conf_loadTime( L, n, i )                                               \
   {                                                                           \
      lua_getglobal( L, n );                                                   \
      if ( lua_isnumber( L, -1 ) ) {                                           \
         i = (time_t)lua_tonumber( L, -1 );                                    \
      }                                                                        \
      lua_pop( L, 1 );                                                         \
   }

#define conf_loadBool( L, n, b )                                               \
   {                                                                           \
      lua_getglobal( L, n );                                                   \
      if ( lua_isnumber( L, -1 ) )                                             \
         b = ( lua_tonumber( L, -1 ) != 0. );                                  \
      else if ( !lua_isnil( L, -1 ) )                                          \
         b = lua_toboolean( L, -1 );                                           \
      lua_pop( L, 1 );                                                         \
   }

#define conf_loadString( L, n, s )                                             \
   {                                                                           \
      lua_getglobal( L, n );                                                   \
      if ( lua_isstring( L, -1 ) ) {                                           \
         free( s );                                                            \
         s = strdup( lua_tostring( L, -1 ) );                                  \
      }                                                                        \
      lua_pop( L, 1 );                                                         \
   }

/* Global configuration. */
PlayerConf_t conf = {
   .loaded = 0, .ndata = NULL, .language = NULL, .joystick_nam = NULL };

/* from main.c */
extern int   show_fps;
extern int   max_fps;
extern int   indjoystick;
extern char *namjoystick;

/*
 * prototypes
 */
static void print_usage( void );

/*
 * prints usage
 */
static void print_usage( void )
{
   LOG( _( "Usage: %s [OPTIONS]" ), env.argv0 );
   LOG( _( "Options are:" ) );
   LOG( _( "   -f, --fullscreen      activate fullscreen" ) );
   LOG( _( "   -F n, --fps n         limit frames per second to n" ) );
   LOG( _( "   -V, --vsync           enable vsync" ) );
   LOG( _( "   -W n                  set width to n" ) );
   LOG( _( "   -H n                  set height to n" ) );
   LOG( _( "   -j n, --joystick n    use joystick n" ) );
   LOG( _( "   -J s, --Joystick s    use joystick whose name contains s" ) );
   LOG( _( "   -M, --mute            disables sound" ) );
   LOG( _( "   -S, --sound           forces sound" ) );
   LOG( _( "   -m f, --mvol f        sets the music volume to f" ) );
   LOG( _( "   -s f, --svol f        sets the sound volume to f" ) );
   LOG( _( "   -d, --datapath        adds a new datapath to be mounted (i.e., "
           "appends it to the search path for game assets)" ) );
   LOG( _( "   -X, --scale           defines the scale factor" ) );
   LOG(
      _( "   --devmode             enables dev mode perks like the editors" ) );
   LOG( _( "   -h, --help            display this message and exit" ) );
   LOG( _( "   -v, --version         print the version and exit" ) );
}

/**
 * @brief Sets the default configuration.
 */
void conf_setDefaults( void )
{
   conf_cleanup();

   /* Joystick. */
   conf.joystick_ind = -1;

   /* GUI. */
   conf.mesg_visible        = 5;
   conf.map_overlay_opacity = MAP_OVERLAY_OPACITY_DEFAULT;
   conf.big_icons           = BIG_ICONS_DEFAULT;
   conf.always_radar        = 0;
   conf.show_viewport       = 0;

   /* Repeat. */
   conf.repeat_delay = 500;
   conf.repeat_freq  = 30;

   /* Dynamic zoom. */
   conf.zoom_manual = MANUAL_ZOOM_DEFAULT;
   conf.zoom_far    = ZOOM_FAR_DEFAULT;
   conf.zoom_near   = ZOOM_NEAR_DEFAULT;
   conf.zoom_speed  = ZOOM_SPEED_DEFAULT;

   /* Font sizes. */
   conf.font_size_console = FONT_SIZE_CONSOLE_DEFAULT;
   conf.font_size_intro   = FONT_SIZE_INTRO_DEFAULT;
   conf.font_size_def     = FONT_SIZE_DEF_DEFAULT;
   conf.font_size_small   = FONT_SIZE_SMALL_DEFAULT;

   /* Misc. */
   conf.redirect_file = 1;
   conf.nosave        = 0;
   conf.devmode       = 0;
   conf.devautosave   = 0;
   conf.lua_enet      = 0;
   conf.lua_repl      = 0;
   free( conf.lastversion );
   conf.lastversion              = NULL;
   conf.translation_warning_seen = 0;
   memset( &conf.last_played, 0, sizeof( time_t ) );

   /* Accessibility. */
   conf.puzzle_skip = PUZZLE_SKIP_DEFAULT;

   /* Gameplay. */
   conf_setGameplayDefaults();

   /* Audio. */
   conf_setAudioDefaults();

   /* Video. */
   conf_setVideoDefaults();

   /* Input */
   input_setDefault( 1 );

   /* Debugging. */
   conf.fpu_except = 0; /* Causes many issues. */

   /* Editor. */
   if ( nfile_dirExists( "../dat/" ) )
      conf.dev_data_dir = strdup( "../dat/" );
   else
      conf.dev_data_dir = NULL;
}

/**
 * @brief Sets the gameplay defaults.
 */
void conf_setGameplayDefaults( void )
{
   conf.difficulty        = DIFFICULTY_DEFAULT;
   conf.doubletap_sens    = DOUBLETAP_SENSITIVITY_DEFAULT;
   conf.mouse_hide        = MOUSE_HIDE_DEFAULT;
   conf.mouse_accel       = MOUSE_ACCEL_DEFAULT;
   conf.mouse_doubleclick = MOUSE_DOUBLECLICK_TIME;
   conf.mouse_fly         = MOUSE_FLY_DEFAULT;
   conf.zoom_manual       = MANUAL_ZOOM_DEFAULT;
}

/**
 * @brief Sets the audio defaults.
 */
void conf_setAudioDefaults( void )
{
   /* Sound. */
   conf.al_efx     = USE_EFX_DEFAULT;
   conf.nosound    = MUTE_SOUND_DEFAULT;
   conf.sound      = SOUND_VOLUME_DEFAULT;
   conf.music      = MUSIC_VOLUME_DEFAULT;
   conf.engine_vol = ENGINE_VOLUME_DEFAULT;
}

/**
 * @brief Sets the video defaults.
 */
void conf_setVideoDefaults( void )
{
   int w, h, f;

   conf.num_backups = NUM_BACKUPS_DEFAULT;

   /* More complex resolution handling. */
   f                                 = 0;
   const SDL_DisplayMode *resolution = SDL_GetCurrentDisplayMode( 0 );
   if ( resolution != NULL ) {
      /* Try higher resolution. */
      w = RESOLUTION_W_DEFAULT;
      h = RESOLUTION_H_DEFAULT;

      /* Fullscreen and fit everything onscreen. */
      if ( ( resolution->w <= w ) || ( resolution->h <= h ) ) {
         w = resolution->w;
         h = resolution->h;
         f = FULLSCREEN_DEFAULT;
      }
   } else {
      w = 800;
      h = 600;
   }

   /* OpenGL. */
   conf.fsaa  = FSAA_DEFAULT;
   conf.vsync = VSYNC_DEFAULT;

   /* Window. */
   conf.fullscreen          = f;
   conf.width               = w;
   conf.height              = h;
   conf.scalefactor         = SCALE_FACTOR_DEFAULT;
   conf.nebu_scale          = NEBULA_SCALE_FACTOR_DEFAULT;
   conf.minimize            = MINIMIZE_DEFAULT;
   conf.colourblind_sim     = COLOURBLIND_SIM_DEFAULT;
   conf.colourblind_correct = COLOURBLIND_CORRECT_DEFAULT;
   conf.colourblind_type    = COLOURBLIND_TYPE_DEFAULT;
   conf.game_speed          = GAME_SPEED_DEFAULT;
   conf.healthbars          = HEALTHBARS_DEFAULT;
   conf.bg_brightness       = BG_BRIGHTNESS_DEFAULT;
   conf.nebu_nonuniformity  = NEBU_NONUNIFORMITY_DEFAULT;
   conf.nebu_saturation     = NEBU_SATURATION_DEFAULT;
   conf.jump_brightness     = JUMP_BRIGHTNESS_DEFAULT;
   conf.gamma_correction    = GAMMA_CORRECTION_DEFAULT;
   conf.low_memory          = LOW_MEMORY_DEFAULT;
   conf.max_3d_tex_size     = MAX_3D_TEX_SIZE;

   if ( cur_system )
      background_load( cur_system->background );

   /* FPS. */
   conf.fps_show = SHOW_FPS_DEFAULT;
   conf.fps_max  = FPS_MAX_DEFAULT;

   /* Pause. */
   conf.pause_show = SHOW_PAUSE_DEFAULT;
}

/*
 * Frees some memory the conf allocated.
 */
void conf_cleanup( void )
{
   conf_free( &conf );
}

/*
 * @brief Parses the local configuration that dictates where user data goes.
 */
void conf_loadConfigPath( void )
{
   const char *file = "datapath.lua";

   if ( !nfile_fileExists( file ) )
      return;

   lua_State *L = luaL_newstate();

   if ( luaL_dofile( L, file ) != 0 ) {
      lua_close( L );
      return;
   }
   conf_loadString( L, "datapath", conf.datapath );

   lua_close( L );
}

/*
 * parses the config file
 */
int conf_loadConfig( const char *file )
{
   int         t, cb;
   SDL_Keycode key;
   int         type;
   int         w, h;
   SDL_Keymod  m;
   lua_State  *L;

   /* Check to see if file exists. */
   if ( !nfile_fileExists( file ) ) {
      conf.loaded = 1;
      return nfile_touch( file );
   }

   /* Load the configuration. */
   L = luaL_newstate();
   // TODO sandbox
   luaL_openlibs( L );
   if ( luaL_dofile( L, file ) != 0 ) {
      WARN( _( "Config file '%s' has invalid syntax:" ), file );
      WARN( "   %s", lua_tostring( L, -1 ) );
      lua_close( L );
      return -1;
   }

   /* ndata. */
   conf_loadString( L, "data", conf.ndata );

   /* Language. */
   conf_loadString( L, "language", conf.language );

   /* OpenGL. */
   conf_loadInt( L, "fsaa", conf.fsaa );
   conf_loadBool( L, "vsync", conf.vsync );

   /* Window. */
   w = h = 0;
   conf_loadInt( L, "width", w );
   conf_loadInt( L, "height", h );
   if ( w != 0 ) {
      conf.width = w;
   }
   if ( h != 0 ) {
      conf.height = h;
   }
   conf_loadFloat( L, "scalefactor", conf.scalefactor );
   conf_loadFloat( L, "nebu_scale", conf.nebu_scale );
   conf_loadBool( L, "fullscreen", conf.fullscreen );
   conf_loadBool( L, "notresizable", conf.notresizable );
   conf_loadBool( L, "minimize", conf.minimize );
   cb = 0;
   conf_loadBool( L, "colourblind", cb ); /* TODO remove in 0.13.0 or so. */
   if ( cb ) {
      /* Old colourblind used Rod Monochromancy, so we'll restore that in
       * this case. TODO Remove in 0.13.0 or so. */
      conf.colourblind_type = 3;
      conf.colourblind_sim  = 1.; /* Turn on at max. */
   }
   conf_loadFloat( L, "colourblind_sim", conf.colourblind_sim );
   conf_loadFloat( L, "colourblind_correct", conf.colourblind_correct );
   conf_loadInt( L, "colourblind_type", conf.colourblind_type );
   conf_loadFloat( L, "game_speed", conf.game_speed );
   conf_loadBool( L, "healthbars", conf.healthbars );
   conf_loadFloat( L, "bg_brightness", conf.bg_brightness );
   conf_loadBool( L, "puzzle_skip", conf.puzzle_skip );
   /* TODO leave only nebu_nonuniformity for 0.13.0 */
   conf_loadFloat( L, "nebu_uniformity", conf.nebu_nonuniformity );
   conf_loadFloat( L, "nebu_nonuniformity", conf.nebu_nonuniformity );
   /* end todo */
   conf_loadFloat( L, "nebu_saturation", conf.nebu_saturation );
   conf_loadFloat( L, "jump_brightness", conf.jump_brightness );
   conf_loadFloat( L, "gamma_correction", conf.gamma_correction );
   conf_loadBool( L, "low_memory", conf.low_memory );
   conf_loadInt( L, "max_3d_tex_size", conf.max_3d_tex_size );

   /* FPS */
   conf_loadBool( L, "showfps", conf.fps_show );
   conf_loadInt( L, "maxfps", conf.fps_max );

   /*  Pause */
   conf_loadBool( L, "showpause", conf.pause_show );

   /* Sound. */
   conf_loadBool( L, "al_efx", conf.al_efx );
   conf_loadBool( L, "nosound", conf.nosound );
   conf_loadFloat( L, "sound", conf.sound );
   conf_loadFloat( L, "music", conf.music );
   conf_loadFloat( L, "engine_vol", conf.engine_vol );

   /* Joystick. */
   lua_getglobal( L, "joystick" );
   if ( lua_isnumber( L, -1 ) )
      conf.joystick_ind = (int)lua_tonumber( L, -1 );
   else if ( lua_isstring( L, -1 ) )
      conf.joystick_nam = strdup( lua_tostring( L, -1 ) );
   lua_pop( L, 1 );

   /* GUI. */
   conf_loadInt( L, "mesg_visible", conf.mesg_visible );
   if ( conf.mesg_visible <= 0 )
      conf.mesg_visible = 5;
   conf_loadFloat( L, "map_overlay_opacity", conf.map_overlay_opacity );
   conf.map_overlay_opacity = CLAMP( 0, 1, conf.map_overlay_opacity );
   conf_loadBool( L, "big_icons", conf.big_icons );
   conf_loadBool( L, "always_radar", conf.always_radar );
   conf_loadBool( L, "show_viewport", conf.show_viewport );

   /* Key repeat. */
   conf_loadInt( L, "repeat_delay", conf.repeat_delay );
   conf_loadInt( L, "repeat_freq", conf.repeat_freq );

   /* Zoom. */
   conf_loadBool( L, "zoom_manual", conf.zoom_manual );
   conf_loadFloat( L, "zoom_far", conf.zoom_far );
   conf_loadFloat( L, "zoom_near", conf.zoom_near );
   conf_loadFloat( L, "zoom_speed", conf.zoom_speed );

   /* Font size. */
   conf_loadInt( L, "font_size_console", conf.font_size_console );
   conf_loadInt( L, "font_size_intro", conf.font_size_intro );
   conf_loadInt( L, "font_size_def", conf.font_size_def );
   conf_loadInt( L, "font_size_small", conf.font_size_small );

   /* Misc. */
   conf_loadString( L, "difficulty", conf.difficulty );
   conf_loadFloat( L, "compression_velocity", conf.compression_velocity );
   conf_loadFloat( L, "compression_mult", conf.compression_mult );
   conf_loadBool( L, "redirect_file", conf.redirect_file );
   conf_loadInt( L, "doubletap_sensitivity", conf.doubletap_sens );
   conf_loadFloat( L, "mouse_hide", conf.mouse_hide );
   conf_loadBool( L, "mouse_fly", conf.mouse_fly );
   conf_loadInt( L, "mouse_accel", conf.mouse_accel );
   conf_loadFloat( L, "mouse_doubleclick", conf.mouse_doubleclick );
   conf_loadFloat( L, "autonav_reset_dist", conf.autonav_reset_dist );
   conf_loadFloat( L, "autonav_reset_shield", conf.autonav_reset_shield );
   conf_loadBool( L, "devmode", conf.devmode );
   conf_loadBool( L, "devautosave", conf.devautosave );
   conf_loadBool( L, "lua_enet", conf.lua_enet );
   conf_loadBool( L, "lua_repl", conf.lua_repl );
   conf_loadBool( L, "conf_nosave", conf.nosave );
   conf_loadString( L, "lastversion", conf.lastversion );
   conf_loadBool( L, "translation_warning_seen",
                  conf.translation_warning_seen );
   conf_loadTime( L, "last_played", conf.last_played );

   /* Debugging. */
   conf_loadBool( L, "fpu_except", conf.fpu_except );

   /* Editor. */
   conf_loadString( L, "dev_data_dir", conf.dev_data_dir );

   /*
    * Keybindings.
    */
   for ( int i = 0; i <= KST_PASTE; i++ ) {
      lua_getglobal( L, input_getKeybindBrief( i ) );

      /* Use 'none' to differentiate between not instantiated and disabled
       * bindings. */
      if ( lua_isstring( L, -1 ) ) {
         const char *str = lua_tostring( L, -1 );
         if ( strcmp( str, "none" ) == 0 ) {
            input_setKeybind( i, KEYBIND_NULL, SDLK_UNKNOWN, NMOD_NONE );
         }
      } else if ( lua_istable( L, -1 ) ) { /* it's a table */
         const char *str, *mod;
         /* gets the event type */
         lua_getfield( L, -1, "type" );
         if ( lua_isstring( L, -1 ) )
            str = lua_tostring( L, -1 );
         else if ( lua_isnil( L, -1 ) ) {
            WARN( _( "Found keybind with no type field!" ) );
            str = "null";
         } else {
            WARN( _( "Found keybind with invalid type field!" ) );
            str = "null";
         }
         lua_pop( L, 1 );

         /* gets the key */
         lua_getfield( L, -1, "key" );
         t = lua_type( L, -1 );
         if ( t == LUA_TNUMBER )
            key = (int)lua_tonumber( L, -1 );
         else if ( t == LUA_TSTRING ) {
            const char *name = lua_tostring( L, -1 );
            key              = input_keyFromStr( name );
            if ( key == SDLK_UNKNOWN )
               WARN( _( "Keyname '%s' doesn't match any key." ), name );
         } else if ( t == LUA_TNIL ) {
            WARN( _( "Found keybind with no key field!" ) );
            key = SDLK_UNKNOWN;
         } else {
            WARN( _( "Found keybind with invalid key field!" ) );
            key = SDLK_UNKNOWN;
         }
         lua_pop( L, 1 );

         /* Get the modifier. */
         lua_getfield( L, -1, "mod" );
         if ( lua_isstring( L, -1 ) )
            mod = lua_tostring( L, -1 );
         else
            mod = NULL;
         lua_pop( L, 1 );

         if ( str != NULL ) { /* keybind is valid */
            /* get type */
            if ( strcmp( str, "null" ) == 0 )
               type = KEYBIND_NULL;
            else if ( strcmp( str, "keyboard" ) == 0 )
               type = KEYBIND_KEYBOARD;
            else if ( strcmp( str, "jaxispos" ) == 0 )
               type = KEYBIND_JAXISPOS;
            else if ( strcmp( str, "jaxisneg" ) == 0 )
               type = KEYBIND_JAXISNEG;
            else if ( strcmp( str, "jbutton" ) == 0 )
               type = KEYBIND_JBUTTON;
            else if ( strcmp( str, "jhat_up" ) == 0 )
               type = KEYBIND_JHAT_UP;
            else if ( strcmp( str, "jhat_down" ) == 0 )
               type = KEYBIND_JHAT_DOWN;
            else if ( strcmp( str, "jhat_left" ) == 0 )
               type = KEYBIND_JHAT_LEFT;
            else if ( strcmp( str, "jhat_right" ) == 0 )
               type = KEYBIND_JHAT_RIGHT;
            else {
               WARN( _( "Unknown keybinding of type %s" ), str );
               continue;
            }

            /* Check to see if it is valid. */
            if ( ( key == SDLK_UNKNOWN ) && ( type == KEYBIND_KEYBOARD ) ) {
               WARN( _( "Keybind for '%s' is invalid" ),
                     input_getKeybindName( i ) );
               continue;
            }

            /* Set modifier, probably should be able to handle two at a time.
             */
            if ( mod != NULL ) {
               if ( strcmp( mod, "ctrl" ) == 0 )
                  m = NMOD_CTRL;
               else if ( strcmp( mod, "shift" ) == 0 )
                  m = NMOD_SHIFT;
               else if ( strcmp( mod, "alt" ) == 0 )
                  m = NMOD_ALT;
               else if ( strcmp( mod, "meta" ) == 0 )
                  m = NMOD_META;
               else if ( strcmp( mod, "any" ) == 0 )
                  m = NMOD_ANY;
               else if ( strcmp( mod, "none" ) == 0 )
                  m = NMOD_NONE;
               else {
                  WARN( _( "Unknown keybinding mod of type %s" ), mod );
                  m = NMOD_NONE;
               }
            } else
               m = NMOD_NONE;

            /* set the keybind */
            input_setKeybind( i, type, key, m );
         } else
            WARN( _( "Malformed keybind for '%s' in '%s'." ),
                  input_getKeybindName( i ), file );
      }
      /* clean up after table stuff */
      lua_pop( L, 1 );
   }
   lua_pop( L, 1 );

   conf.loaded = 1;

   lua_close( L );
   return 0;
}

/*
 * parses the CLI options
 */
int conf_parseCLI( int argc, char **argv )
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
      { "scale", required_argument, 0, 'X' },
      { "devmode", no_argument, 0, 'D' },
      { "help", no_argument, 0, 'h' },
      { "version", no_argument, 0, 'v' },
      { "exitmainmenu", no_argument, 0, '\e' },
      { NULL, 0, 0, 0 } };
   int option_index = 1;
   int c            = 0;

   /* man 3 getopt says optind should be initialized to 1, but that seems to
    * cause all options to get parsed, i.e. we cannot detect a trailing ndata
    * option.
    */
   optind = 0;
   while ( ( c = getopt_long( argc, argv, "fF:Vd:j:J:W:H:MSm:s:X:Nhv",
                              long_options, &option_index ) ) != -1 ) {
      switch ( c ) {
      case 'd':
         PHYSFS_mount( optarg, NULL, 1 );
         break;
      case 'f':
         conf.fullscreen = 1;
         break;
      case 'F':
         conf.fps_max = atoi( optarg );
         break;
      case 'V':
         conf.vsync = 1;
         break;
      case 'j':
         conf.joystick_ind = atoi( optarg );
         break;
      case 'J':
         conf.joystick_nam = strdup( optarg );
         break;
      case 'W':
         conf.width = atoi( optarg );
         break;
      case 'H':
         conf.height = atoi( optarg );
         break;
      case 'M':
         conf.nosound = 1;
         break;
      case 'S':
         conf.nosound = 0;
         break;
      case 'm':
         conf.music = atof( optarg );
         break;
      case 's':
         conf.sound = atof( optarg );
         break;
      case 'N':
         free( conf.ndata );
         conf.ndata = NULL;
         break;
      case 'X':
         conf.scalefactor = atof( optarg );
         break;
      case 'D':
         conf.devmode = 1;
         LOG( _( "Enabling developer mode." ) );
         break;
      case '\e':
         conf.exit_main_menu = 1;
         break;

      case 'v':
         /* by now it has already displayed the version */
         exit( EXIT_SUCCESS );
      case 'h':
         print_usage();
         exit( EXIT_SUCCESS );
      }
   }

   return optind;
}

/**
 * @brief snprintf-like function to quote and escape a string for use in Lua
 * source code
 *
 *    @param str The destination buffer
 *    @param size The maximum amount of space in string to use
 *    @param text The string to quote and escape
 *    @return The number of characters actually written to string
 */
static size_t quoteLuaString( char *str, size_t size, const char *text )
{
   char     slashescape;
   size_t   count, i;
   uint32_t ch;

   if ( size == 0 )
      return 0;

   /* Write a Lua nil if we are given a NULL pointer */
   if ( text == NULL )
      return scnprintf( str, size, "nil" );

   count = 0;

   /* Quote start */
   str[count++] = '\"';
   if ( count == size )
      return count;

   /* Iterate over the characters in text */
   i = 0;
   while ( ( ch = u8_nextchar( text, &i ) ) ) {
      /* Check if we can print this as a friendly backslash-escape */
      switch ( ch ) {
      case '#':
         slashescape = 'a';
         break;
      case '\b':
         slashescape = 'b';
         break;
      case '\f':
         slashescape = 'f';
         break;
      case '\n':
         slashescape = 'n';
         break;
      case '\r':
         slashescape = 'r';
         break;
      case '\t':
         slashescape = 't';
         break;
      case '\v':
         slashescape = 'v';
         break;
      case '\\':
         slashescape = '\\';
         break;
      case '\"':
         slashescape = '\"';
         break;
      case '\'':
         slashescape = '\'';
         break;
      /* Technically, Lua can also represent \0, but we can't in our input */
      default:
         slashescape = 0;
         break;
      }
      if ( slashescape != 0 ) {
         /* Yes, we can use a backslash-escape! */
         str[count++] = '\\';
         if ( count == size )
            return count;

         str[count++] = slashescape;
         if ( count == size )
            return count;

         continue;
      }

      /* Render UTF8. */
      count += u8_toutf8( &str[count], size - count, &ch, 1 );
      if ( count >= size )
         return count;
   }

   /* Quote end */
   str[count++] = '\"';
   if ( count == size )
      return count;

   /* zero-terminate, if possible */
   str[count] = '\0'; /* don't increase count, like snprintf */

   /* return the amount of characters written */
   return count;
}

#define conf_saveComment( t )                                                  \
   pos += scnprintf( &buf[pos], sizeof( buf ) - pos, "-- %s\n", t );
#define conf_saveCommentVar( str, ... )                                        \
   do {                                                                        \
      char _BUF[STRMAX_SHORT];                                                 \
      scnprintf( _BUF, sizeof( _BUF ), str, ##__VA_ARGS__ );                   \
      pos += scnprintf( &buf[pos], sizeof( buf ) - pos, "-- %s\n", _BUF );     \
   } while ( 0 )

#define conf_saveEmptyLine()                                                   \
   if ( sizeof( buf ) != pos )                                                 \
      buf[pos++] = '\n';

#define conf_saveInt( n, i )                                                   \
   pos += scnprintf( &buf[pos], sizeof( buf ) - pos, "%s = %d\n", n, i );

#define conf_saveULong( n, i )                                                 \
   pos += scnprintf( &buf[pos], sizeof( buf ) - pos, "%s = %lu\n", n, i );

#define conf_saveTime( n, i )                                                  \
   pos += scnprintf( &buf[pos], sizeof( buf ) - pos, "%s = %llu\n", n,         \
                     (unsigned long long)i );

#define conf_saveFloat( n, f )                                                 \
   pos += scnprintf( &buf[pos], sizeof( buf ) - pos, "%s = %f\n", n, f );

#define conf_saveBool( n, b )                                                  \
   if ( b )                                                                    \
      pos += scnprintf( &buf[pos], sizeof( buf ) - pos, "%s = true\n", n );    \
   else                                                                        \
      pos += scnprintf( &buf[pos], sizeof( buf ) - pos, "%s = false\n", n );

#define conf_saveString( n, s )                                                \
   pos += scnprintf( &buf[pos], sizeof( buf ) - pos, "%s = ", n );             \
   pos += quoteLuaString( &buf[pos], sizeof( buf ) - pos, s );                 \
   if ( sizeof( buf ) != pos )                                                 \
      buf[pos++] = '\n';

#define GENERATED_START_COMMENT "START GENERATED SECTION"
#define GENERATED_END_COMMENT "END GENERATED SECTION"

/*
 * saves the current configuration
 */
int conf_saveConfig( const char *file )
{
   char       *old;
   const char *oldfooter;
   size_t      oldsize;
   char        buf[32 * 1024];
   size_t      pos;

   pos       = 0;
   oldfooter = NULL;

   /* User doesn't want to save the config. */
   if ( conf.nosave )
      return 0;

   /* Read the old configuration, if possible */
   if ( nfile_fileExists( file ) &&
        ( old = nfile_readFile( &oldsize, file ) ) != NULL ) {
      /* See if we can find the generated section and preserve
       * whatever the user wrote before it */
      const char *tmp =
         SDL_strnstr( old, "-- " GENERATED_START_COMMENT "\n", oldsize );
      if ( tmp != NULL ) {
         /* Copy over the user content */
         pos = MIN( sizeof( buf ), (size_t)( tmp - old ) );
         memcpy( buf, old, pos );

         /* See if we can find the end of the section */
         tmp =
            SDL_strnstr( tmp, "-- " GENERATED_END_COMMENT "\n", oldsize - pos );
         if ( tmp != NULL ) {
            /* Everything after this should also be preserved */
            oldfooter = tmp + strlen( "-- " GENERATED_END_COMMENT "\n" );
            oldsize -= ( oldfooter - old );
         }
      } else {
         /* Treat the contents of the old file as a footer. */
         oldfooter = old;
      }
   } else {
      old = NULL;

      /* Write a nice header for new configuration files */
      conf_saveComment( _( "Naev configuration file" ) );
      conf_saveEmptyLine();
   }

   /* Back up old configuration. */
   if ( nfile_backupIfExists( file ) < 0 ) {
      WARN( _( "Not saving configuration." ) );
      return -1;
   }

   /* Header. */
   conf_saveComment( GENERATED_START_COMMENT );
   conf_saveComment(
      _( "The contents of this section will be rewritten by Naev!" ) );
   conf_saveEmptyLine();

   /* ndata. */
   conf_saveComment(
      _( "The location of Naev's data pack, usually called 'ndata'" ) );
   conf_saveString( "data", conf.ndata );
   conf_saveEmptyLine();

   /* OpenGL. */
   conf_saveComment( _( "Number of save game backups" ) );
   conf_saveInt( "num_backups", conf.num_backups );
   conf_saveEmptyLine();

   /* Language. */
   conf_saveComment(
      _( "Language to use. Set to the two character identifier to the language "
         "(e.g., \"en\" for English), and nil for autodetect." ) );
   conf_saveString( "language", conf.language );
   conf_saveEmptyLine();

   /* Difficulty. */
   conf_saveComment(
      _( "Global difficulty to set the game to. Can be overwritten by saved "
         "game settings. Has to match one of the difficulties defined in "
         "\"difficulty.xml\" in the data files." ) );
   if ( conf.difficulty == NULL ) {
      conf_saveComment( "difficulty = nil" );
   } else {
      conf_saveString( "difficulty", conf.difficulty );
   }
   conf_saveEmptyLine();

   /* OpenGL. */
   conf_saveComment( _( "The factor to use in Full-Scene Anti-Aliasing" ) );
   conf_saveComment( _( "Anything lower than 2 will simply disable FSAA" ) );
   conf_saveInt( "fsaa", conf.fsaa );
   conf_saveEmptyLine();

   conf_saveComment( _(
      "Synchronize framebuffer updates with the vertical blanking interval" ) );
   conf_saveBool( "vsync", conf.vsync );
   conf_saveEmptyLine();

   /* Window. */
   conf_saveComment( _( "The window size or screen resolution" ) );
   conf_saveComment(
      _( "Set both of these to 0 to make Naev try the desktop resolution" ) );
   conf_saveInt( "width", conf.width );
   conf_saveInt( "height", conf.height );
   conf_saveEmptyLine();

   conf_saveComment( _( "Factor used to divide the above resolution with" ) );
   conf_saveComment( _( "This is used to lower the rendering resolution, and "
                        "scale to the above" ) );
   conf_saveFloat( "scalefactor", conf.scalefactor );
   conf_saveEmptyLine();

   conf_saveComment( _( "Scale factor for rendered nebula backgrounds." ) );
   conf_saveComment(
      _( "Larger values can save time but lead to a blurrier appearance." ) );
   conf_saveFloat( "nebu_scale", conf.nebu_scale );
   conf_saveEmptyLine();

   conf_saveComment( _( "Run Naev in full-screen mode" ) );
   conf_saveBool( "fullscreen", conf.fullscreen );
   conf_saveEmptyLine();

   conf_saveComment( _( "Disable allowing resizing the window." ) );
   conf_saveBool( "notresizable", conf.notresizable );
   conf_saveEmptyLine();

   conf_saveComment( _( "Minimize the game on focus loss." ) );
   conf_saveBool( "minimize", conf.minimize );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Enables colourblind simulation. A value of 0. disables." ) );
   conf_saveFloat( "colourblind_sim", conf.colourblind_sim );
   conf_saveEmptyLine();

   conf_saveComment( _( "Type of colourblindness to simulate or correct." ) );
   conf_saveComment( _( "0 is Protanopia" ) );
   conf_saveComment( _( "1 is Deuteranopia" ) );
   conf_saveComment( _( "2 is Tritanapia" ) );
   conf_saveComment( _( "3 is Rod Monochromacy" ) );
   conf_saveComment( _( "4 is Cone Monochromacy" ) );
   conf_saveInt( "colourblind_type", conf.colourblind_type );
   conf_saveEmptyLine();

   conf_saveComment( _( "Intensity of the colour blindness correction. A value "
                        "of 0. disables." ) );
   conf_saveFloat( "colourblind_correct", conf.colourblind_correct );
   conf_saveEmptyLine();

   conf_saveComment( _( "Slows down the game to improve accessibility." ) );
   conf_saveFloat( "game_speed", conf.game_speed );
   conf_saveEmptyLine();

   conf_saveComment( _( "Enable health bars. These show hostility/friendliness "
                        "and health of pilots on screen." ) );
   conf_saveBool( "healthbars", conf.healthbars );
   conf_saveEmptyLine();

   conf_saveCommentVar(
      _( "Background brightness. 1 is full brightness while setting it to 0 "
         "would make the backgrounds pitch black. Defaults to %.1f." ),
      BG_BRIGHTNESS_DEFAULT );
   conf_saveFloat( "bg_brightness", conf.bg_brightness );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Nebula non-uniformity. 1 is normal nebula while setting it to 0 "
         "would make the nebula a solid colour." ) );
   conf_saveFloat( "nebu_nonuniformity", conf.nebu_nonuniformity );
   conf_saveEmptyLine();

   conf_saveComment( _(
      "Nebula saturation. Modifies the base saturation of the nebula colour. "
      "Lower values desaturate the nebulas to make them easier to view." ) );
   conf_saveFloat( "nebu_saturation", conf.nebu_saturation );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Controls the intensity to which the screen fades when jumping. 1.0 "
         "would be pure white, while 0.0 would be pure black." ) );
   conf_saveFloat( "jump_brightness", conf.jump_brightness );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Gamma correction parameter. A value of 1 disables it (no curve)." ) );
   conf_saveFloat( "gamma_correction", conf.gamma_correction );
   conf_saveEmptyLine();

   conf_saveComment( _( "Enables low memory mode which foregoes using normal "
                        "textures and ambient occlusion. Useful when you want "
                        "to run Naev or more limited hardware." ) );
   conf_saveBool( "low_memory", conf.low_memory );
   conf_saveEmptyLine();

   conf_saveComment( _( "Provide an in-game option to skip puzzles that appear "
                        "throughout the game." ) );
   conf_saveBool( "puzzle_skip", conf.puzzle_skip );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Maximum texture size to use for 3D models when in low memory mode. A "
         "value of less than or equal to 0 disables texture resizing." ) );
   conf_saveInt( "max_3d_tex_size", conf.max_3d_tex_size );
   conf_saveEmptyLine();

   /* FPS */
   conf_saveComment( _( "Display a frame rate counter" ) );
   conf_saveBool( "showfps", conf.fps_show );
   conf_saveEmptyLine();

   conf_saveComment( _( "Limit the rendering frame rate" ) );
   conf_saveInt( "maxfps", conf.fps_max );
   conf_saveEmptyLine();

   /* Pause */
   conf_saveComment( _( "Show 'PAUSED' on screen while paused" ) );
   conf_saveBool( "showpause", conf.pause_show );
   conf_saveEmptyLine();

   /* Sound. */
   conf_saveComment( _( "Enables EFX extension for OpenAL backend." ) );
   conf_saveBool( "al_efx", conf.al_efx );
   conf_saveEmptyLine();

   conf_saveComment( _( "Disable all sound" ) );
   conf_saveBool( "nosound", conf.nosound );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Volume of sound effects and music, between 0.0 and 1.0" ) );
   conf_saveFloat( "sound",
                   ( sound_disabled ) ? conf.sound : sound_getVolume() );
   conf_saveFloat( "music",
                   ( music_disabled ) ? conf.music : music_getVolume() );
   conf_saveComment(
      _( "Relative engine sound volume. Should be between 0.0 and 1.0" ) );
   conf_saveFloat( "engine_vol", conf.engine_vol );
   conf_saveEmptyLine();

   /* Joystick. */
   conf_saveComment( _( "The name or numeric index of the joystick to use" ) );
   conf_saveComment( _( "Setting this to nil disables the joystick support" ) );
   if ( conf.joystick_nam != NULL ) {
      conf_saveString( "joystick", conf.joystick_nam );
   } else if ( conf.joystick_ind >= 0 ) {
      conf_saveInt( "joystick", conf.joystick_ind );
   } else {
      conf_saveString( "joystick", NULL );
   }
   conf_saveEmptyLine();

   /* GUI. */
   conf_saveComment( _( "Number of lines visible in the comm window." ) );
   conf_saveInt( "mesg_visible", conf.mesg_visible );
   conf_saveComment( _( "Opacity fraction (0-1) for the overlay map." ) );
   conf_saveFloat( "map_overlay_opacity", conf.map_overlay_opacity );
   conf_saveComment(
      _( "Use bigger icons in the outfit, shipyard, and other lists." ) );
   conf_saveBool( "big_icons", conf.big_icons );
   conf_saveComment( _(
      "Always show the radar and don't hide it when the overlay is active." ) );
   conf_saveBool( "always_radar", conf.always_radar );
   conf_saveComment( _( "Show the viewport in the radar/overlay." ) );
   conf_saveBool( "show_viewport", conf.show_viewport );
   conf_saveEmptyLine();

   /* Key repeat. */
   conf_saveComment(
      _( "Delay in ms before starting to repeat (0 disables)" ) );
   conf_saveInt( "repeat_delay", conf.repeat_delay );
   conf_saveComment(
      _( "Delay in ms between repeats once it starts to repeat" ) );
   conf_saveInt( "repeat_freq", conf.repeat_freq );
   conf_saveEmptyLine();

   /* Zoom. */
   conf_saveComment( _( "Minimum and maximum zoom factor to use in-game" ) );
   conf_saveComment( _( "At 1.0, no sprites are scaled" ) );
   conf_saveComment( _( "zoom_far should be less then zoom_near" ) );
   conf_saveBool( "zoom_manual", conf.zoom_manual );
   conf_saveFloat( "zoom_far", conf.zoom_far );
   conf_saveFloat( "zoom_near", conf.zoom_near );
   conf_saveEmptyLine();

   conf_saveComment( _( "Zooming speed in factor increments per second" ) );
   conf_saveFloat( "zoom_speed", conf.zoom_speed );
   conf_saveEmptyLine();

   /* Fonts. */
   conf_saveComment( _( "Font sizes (in pixels) for Naev" ) );
   conf_saveComment( _( "Warning, setting to other than the default can cause "
                        "visual glitches!" ) );
   pos +=
      scnprintf( &buf[pos], sizeof( buf ) - pos,
                 _( "-- Console default: %d\n" ), FONT_SIZE_CONSOLE_DEFAULT );
   conf_saveInt( "font_size_console", conf.font_size_console );
   pos += scnprintf( &buf[pos], sizeof( buf ) - pos,
                     _( "-- Intro default: %d\n" ), FONT_SIZE_INTRO_DEFAULT );
   conf_saveInt( "font_size_intro", conf.font_size_intro );
   pos += scnprintf( &buf[pos], sizeof( buf ) - pos,
                     _( "-- Default size: %d\n" ), FONT_SIZE_DEF_DEFAULT );
   conf_saveInt( "font_size_def", conf.font_size_def );
   pos += scnprintf( &buf[pos], sizeof( buf ) - pos, _( "-- Small size: %d\n" ),
                     FONT_SIZE_SMALL_DEFAULT );
   conf_saveInt( "font_size_small", conf.font_size_small );

   /* Misc. */
   conf_saveComment( _( "Redirects log and error output to files" ) );
   conf_saveBool( "redirect_file", conf.redirect_file );
   conf_saveEmptyLine();

   conf_saveComment( _( "Doubletap sensitivity (used for double tap accel for "
                        "afterburner or double tap reverse for cooldown)" ) );
   conf_saveInt( "doubletap_sensitivity", conf.doubletap_sens );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Time (in seconds) to wait until hiding mouse when not used." ) );
   conf_saveBool( "mouse_hide", conf.mouse_hide );
   conf_saveEmptyLine();

   conf_saveComment( _( "Whether or not clicking the middle mouse button "
                        "toggles mouse flying mode." ) );
   conf_saveBool( "mouse_fly", conf.mouse_fly );
   conf_saveEmptyLine();

   conf_saveComment( _( "Mouse-flying accel control" ) );
   conf_saveInt( "mouse_accel", conf.mouse_accel );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Maximum interval to count as a double-click (0 disables)." ) );
   conf_saveFloat( "mouse_doubleclick", conf.mouse_doubleclick );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Enables developer mode (universe editor and the likes)" ) );
   conf_saveBool( "devmode", conf.devmode );
   conf_saveEmptyLine();

   conf_saveComment( _( "Automatic saving for when using the universe editor "
                        "whenever an edit is done" ) );
   conf_saveBool( "devautosave", conf.devautosave );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Enable the lua-enet library, for use by online/multiplayer mods "
         "(CAUTION: online Lua scripts may have security vulnerabilities!)" ) );
   conf_saveBool( "lua_enet", conf.lua_enet );
   conf_saveComment( _( "Enable the experimental CLI based on lua-repl." ) );
   conf_saveBool( "lua_repl", conf.lua_repl );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Save the config every time game exits (rewriting this bit)" ) );
   conf_saveInt( "conf_nosave", conf.nosave );
   conf_saveEmptyLine();

   conf_saveComment(
      _( "Indicates the last version the game has run in before" ) );
   conf_saveString( "lastversion", conf.lastversion );
   conf_saveEmptyLine();

   conf_saveComment( _( "Indicates whether we've already warned about "
                        "incomplete game translations." ) );
   conf_saveBool( "translation_warning_seen", conf.translation_warning_seen );
   conf_saveEmptyLine();

   conf_saveComment( _( "Time Naev was last played. This gets refreshed each "
                        "time you exit Naev." ) );
   conf_saveTime( "last_played", time( NULL ) );
   conf_saveEmptyLine();

   /* Debugging. */
   conf_saveComment(
      _( "Enables FPU exceptions - only works on DEBUG builds" ) );
   conf_saveBool( "fpu_except", conf.fpu_except );
   conf_saveEmptyLine();

   /* Editor. */
   conf_saveComment( _( "Path where the main data is stored at" ) );
   conf_saveString( "dev_data_dir", conf.dev_data_dir );
   conf_saveEmptyLine();

   /*
    * Keybindings.
    */
   conf_saveEmptyLine();
   conf_saveComment( _( "Keybindings" ) );
   conf_saveEmptyLine();

   /* Iterate over the keybinding. */
   for ( int i = 0; i <= KST_PASTE; i++ ) {
      SDL_Keycode key;
      KeybindType type;
      const char *typename;
      SDL_Keymod  mod;
      const char *modname;
      char        keyname[17];

      /* Use an extra character in keyname to make sure it's always
       * zero-terminated */
      keyname[sizeof( keyname ) - 1] = '\0';

      /* Save a comment line containing the description */
      conf_saveComment( input_getKeybindDescription( i ) );

      /* Get the keybind */
      key = input_getKeybind( i, &type, &mod );

      /* Determine the textual name for the keybind type */
      switch ( type ) {
      case KEYBIND_KEYBOARD:
         typename = "keyboard";
         break;
      case KEYBIND_JAXISPOS:
         typename = "jaxispos";
         break;
      case KEYBIND_JAXISNEG:
         typename = "jaxisneg";
         break;
      case KEYBIND_JBUTTON:
         typename = "jbutton";
         break;
      case KEYBIND_JHAT_UP:
         typename = "jhat_up";
         break;
      case KEYBIND_JHAT_DOWN:
         typename = "jhat_down";
         break;
      case KEYBIND_JHAT_LEFT:
         typename = "jhat_left";
         break;
      case KEYBIND_JHAT_RIGHT:
         typename = "jhat_right";
         break;
      default:
         typename = NULL;
         break;
      }
      /* Write a nil if an unknown type */
      if ( ( typename == NULL ) ||
           ( key == SDLK_UNKNOWN && type == KEYBIND_KEYBOARD ) ) {
         pos += scnprintf( &buf[pos], sizeof( buf ) - pos, "%s = \"none\"\n",
                           input_getKeybindBrief( i ) );
         continue;
      }

      /* Determine the textual name for the modifier */
      switch ( (int)mod ) {
      case NMOD_CTRL:
         modname = "ctrl";
         break;
      case NMOD_SHIFT:
         modname = "shift";
         break;
      case NMOD_ALT:
         modname = "alt";
         break;
      case NMOD_META:
         modname = "meta";
         break;
      case NMOD_ANY:
         modname = "any";
         break;
      default:
         modname = "none";
         break;
      }

      /* Determine the textual name for the key, if a keyboard keybind */
      if ( type == KEYBIND_KEYBOARD )
         quoteLuaString( keyname, sizeof( keyname ) - 1,
                         input_keyToStr( key ) );
      /* If SDL can't describe the key, store it as an integer */
      if ( type != KEYBIND_KEYBOARD ||
           strcmp( keyname, "\"unknown key\"" ) == 0 )
         scnprintf( keyname, sizeof( keyname ) - 1, "%d", key );

      /* Write out a simple Lua table containing the keybind info */
      pos +=
         scnprintf( &buf[pos], sizeof( buf ) - pos,
                    "%s = { type = \"%s\", mod = \"%s\", key = %s }\n",
                    input_getKeybindBrief( i ), typename, modname, keyname );
   }
   conf_saveEmptyLine();

   /* Footer. */
   conf_saveComment( GENERATED_END_COMMENT );

   if ( old != NULL ) {
      if ( oldfooter != NULL ) {
         /* oldfooter and oldsize now reference the old content past the footer
          */
         oldsize = MIN( (size_t)oldsize, sizeof( buf ) - pos );
         memcpy( &buf[pos], oldfooter, oldsize );
         pos += oldsize;
      }
      free( old );
   }

   if ( nfile_writeFile( buf, pos, file ) < 0 ) {
      WARN( _( "Failed to write configuration!  You'll most likely have to "
               "restore it by copying your backup configuration over your "
               "current configuration." ) );
      return -1;
   }

   return 0;
}

/**
 * @brief Copies a configuration over another.
 */
void conf_copy( PlayerConf_t *dest, const PlayerConf_t *src )
{
   conf_free( dest );
   memcpy( dest, src, sizeof( PlayerConf_t ) );
#define STRDUP( s ) dest->s = ( ( src->s == NULL ) ? NULL : strdup( src->s ) )
   STRDUP( ndata );
   STRDUP( datapath );
   STRDUP( language );
   STRDUP( joystick_nam );
   STRDUP( lastversion );
   STRDUP( dev_data_dir );
   if ( src->difficulty != NULL )
      STRDUP( difficulty );
#undef STRDUP
}

/**
 * @brief Frees a configuration.
 */
void conf_free( PlayerConf_t *config )
{
   free( config->ndata );
   free( config->datapath );
   free( config->language );
   free( config->joystick_nam );
   free( config->lastversion );
   free( config->dev_data_dir );
   free( config->difficulty );

   /* Clear memory. */
   memset( config, 0, sizeof( PlayerConf_t ) );
}
