#define NOMAIN 1
#include "naev.c"

#include "lualib.h"
#include "nlua_bkg.h"
#include "nlua_camera.h"
#include "nlua_cli.h"
#include "nlua_linopt.h"
#include "nlua_music.h"
#include "nlua_tk.h"

const char *__asan_default_options()
{
   return "detect_leaks=0";
}

int main( int argc, char **argv )
{
   char conf_file_path[PATH_MAX], **search_path;

#ifdef DEBUGGING
   /* Set Debugging flags. */
   memset( debug_flags, 0, DEBUG_FLAGS_MAX );
#endif /* DEBUGGING */

   env_detect( argc, argv );

   log_init();

   /* Set up PhysicsFS. */
   if ( PHYSFS_init( env.argv0 ) == 0 ) {
      char buf[STRMAX];
      snprintf( buf, sizeof( buf ), "PhysicsFS initialization failed: %s",
                PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
#if SDL_VERSION_ATLEAST( 3, 0, 0 )
      SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR,
                                _( "Naev Critical Error" ), buf,
                                gl_screen.window );
#endif /* SDL_VERSION_ATLEAST( 3, 0, 0 ) */
      ERR( "%s", buf );
      return -1;
   }
   PHYSFS_permitSymbolicLinks( 1 );

   /* Set up locales. */
   gettext_init();
   init_linebreak();

   /* Parse version. */
   if ( semver_parse( naev_version( 0 ), &version_binary ) )
      WARN( _( "Failed to parse version string '%s'!" ), naev_version( 0 ) );

   /* Print the version */
   LOG( " %s v%s (%s)", APPNAME, naev_version( 0 ), HOST );

#if SDL_PLATFORM_LINUX
   if ( env.isAppImage )
      LOG( "AppImage detected. Running from: %s", env.appdir );
   else
      DEBUG( "AppImage not detected." );
#endif /*SDL_PLATFORM_LINUX */

   /* Initializes SDL for possible warnings. */
   if ( SDL_Init( 0 ) ) {
      char buf[STRMAX];
      snprintf( buf, sizeof( buf ), _( "Unable to initialize SDL: %s" ),
                SDL_GetError() );
#if SDL_VERSION_ATLEAST( 3, 0, 0 )
      SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR,
                                _( "Naev Critical Error" ), buf,
                                gl_screen.window );
#endif /* SDL_VERSION_ATLEAST( 3, 0, 0 ) */
      ERR( "%s", buf );
      return -1;
   }

   /* Initialize the threadpool */
   threadpool_init();

   /* Set up debug signal handlers. */
   debug_sigInit();

#if HAS_UNIX
   /* Set window class and name. */
   SDL_setenv( "SDL_VIDEO_X11_WMCLASS", APPNAME, 0 );
#endif /* HAS_UNIX */

   /* Must be initialized before input_init is called. */
   if ( SDL_InitSubSystem( SDL_INIT_VIDEO ) < 0 ) {
      WARN( _( "Unable to initialize SDL Video: %s" ), SDL_GetError() );
      return -1;
   }

   /* We'll be parsing XML. */
   LIBXML_TEST_VERSION
   xmlInitParser();

   /* Input must be initialized for config to work. */
   input_init();

   lua_init(); /* initializes lua */
   fps_init(); /* Not actually necessary, but removes warning. */

   conf_setDefaults(); /* set the default config values */

   /*
    * Attempts to load the data path from datapath.lua
    * At this early point in the load process, the binary path
    * is the only place likely to be checked.
    */
   conf_loadConfigPath();

   /* Create the home directory if needed. */
   if ( nfile_dirMakeExist( nfile_configPath() ) )
      WARN( _( "Unable to create config directory '%s'" ), nfile_configPath() );

   /* Set the configuration. */
   snprintf( conf_file_path, sizeof( conf_file_path ), "%s" CONF_FILE,
             nfile_configPath() );

   conf_loadConfig( conf_file_path ); /* Lua to parse the configuration file */
   int opt = conf_parseCLI( argc, argv ); /* parse CLI arguments */
   if ( opt >= argc ) {
      LOG( _( "Missing Lua file!" ) );
   }
   char luafile[PATH_MAX];
   strncpy( luafile, argv[opt++], sizeof( luafile ) );

   /* Set up I/O. */
   ndata_setupWriteDir();
   log_redirect();
   ndata_setupReadDirs();
   gettext_setLanguage( conf.language ); /* now that we can find translations */
   LOG( _( "Loaded configuration: %s" ), conf_file_path );
   search_path = PHYSFS_getSearchPath();
   LOG( "%s", _( "Read locations, searched in order:" ) );
   for ( char **p = search_path; *p != NULL; p++ )
      LOG( "    %s", *p );
   PHYSFS_freeList( search_path );
   /* Logging the cache path is noisy, noisy is good at the DEBUG level. */
   DEBUG( _( "Cache location: %s" ), nfile_cachePath() );
   LOG( _( "Write location: %s\n" ), PHYSFS_getWriteDir() );

   /* Enable FPU exceptions. */
   if ( conf.fpu_except )
      debug_enableFPUExcept();

   /* Load the start info. */
   if ( start_load() ) {
      char buf[STRMAX];
      snprintf( buf, sizeof( buf ), _( "Failed to load module start data." ) );
#if SDL_VERSION_ATLEAST( 3, 0, 0 )
      SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR,
                                _( "Naev Critical Error" ), buf,
                                gl_screen.window );
#endif /* SDL_VERSION_ATLEAST( 3, 0, 0 ) */
      ERR( "%s", buf );
   }
   LOG( " %s", start_name() );
   DEBUG_BLANK();

   /* Display the SDL Version. */
   print_SDLversion();
   DEBUG_BLANK();

   /* random numbers */
   rng_init();

   /*
    * OpenGL
    */
   if ( gl_init( SDL_WINDOW_HIDDEN ) ) { /* initializes video output */
      char buf[STRMAX];
      snprintf( buf, sizeof( buf ),
                _( "Initializing video output failed, exiting…" ) );
#if SDL_VERSION_ATLEAST( 3, 0, 0 )
      SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR,
                                _( "Naev Critical Error" ), buf,
                                gl_screen.window );
#endif /* SDL_VERSION_ATLEAST( 3, 0, 0 ) */
      ERR( "%s", buf );
      // SDL_Quit();
      exit( EXIT_FAILURE );
   }
   window_caption();

   /* Have to set up fonts before rendering anything. */
   // DEBUG("Using '%s' as main font and '%s' as monospace font.",
   // _(FONT_DEFAULT_PATH), _(FONT_MONOSPACE_PATH));
   gl_fontInit( &gl_defFont, _( FONT_DEFAULT_PATH ), conf.font_size_def,
                FONT_PATH_PREFIX, 0 ); /* initializes default font to size */
   gl_fontInit( &gl_smallFont, _( FONT_DEFAULT_PATH ), conf.font_size_small,
                FONT_PATH_PREFIX, 0 ); /* small font */
   gl_fontInit( &gl_defFontMono, _( FONT_MONOSPACE_PATH ), conf.font_size_def,
                FONT_PATH_PREFIX, 0 );

   /* Detect size changes that occurred after window creation. */
   naev_resize();

   /* Display the load screen. */
   loadscreen_load();
   loadscreen_update( 0., _( "Initializing subsystems…" ) );
   last_t = SDL_GetPerformanceCounter();

   /*
    * Input
    */
   if ( ( conf.joystick_ind >= 0 ) || ( conf.joystick_nam != NULL ) ) {
      if ( joystick_init() )
         WARN( _( "Error initializing joystick input" ) );
      if ( conf.joystick_nam !=
           NULL ) { /* use the joystick name to find a joystick */
         if ( joystick_use( joystick_get( conf.joystick_nam ) ) ) {
            WARN( _( "Failure to open any joystick, falling back to default "
                     "keybinds" ) );
            input_setDefault( 1 );
         }
         free( conf.joystick_nam );
      } else if ( conf.joystick_ind >= 0 ) /* use a joystick id instead */
         if ( joystick_use( conf.joystick_ind ) ) {
            WARN( _( "Failure to open any joystick, falling back to default "
                     "keybinds" ) );
            input_setDefault( 1 );
         }
   }

   /*
    * OpenAL - Sound
    */
   if ( conf.nosound ) {
      LOG( _( "Sound is disabled!" ) );
      sound_disabled = 1;
      music_disabled = 1;
   }
   if ( sound_init() )
      WARN( _( "Problem setting up sound!" ) );
   music_choose( "load" );

   /* FPS stuff. */
   fps_setPos( 15., (double)( gl_screen.h - 15 - gl_defFontMono.h ) );

   /* Misc graphics init */
   render_init();
   nebu_init();       /* Initializes the nebula */
   gui_init();        /* initializes the GUI graphics */
   toolkit_init();    /* initializes the toolkit */
   map_init();        /* initializes the map. */
   map_system_init(); /* Initialise the solar system map */
   cond_init();       /* Initialize conditional subsystem. */
   cli_init();        /* Initialize console. */

   /* Data loading */
   load_all();

   /* Detect size changes that occurred during load. */
   naev_resize();

   /* Unload load screen. */
   loadscreen_unload();

   nlua_env nenv = nlua_newEnv( "naevlua" );
   nlua_loadStandard( nenv );
   nlua_loadTex( nenv );
   nlua_loadCol( nenv );
   nlua_loadBackground( nenv );
   nlua_loadCLI( nenv );
   nlua_loadCamera( nenv );
   nlua_loadMusic( nenv );
   nlua_loadTk( nenv );
   nlua_loadLinOpt( nenv );
   /* Reload IO library that was sandboxed out. */
   lua_pushcfunction( naevL, luaopen_io );
   nlua_pcall( nenv, 0, 1 );
   nlua_setenv( naevL, nenv, "io" );
   /* Finally run the file. */
   if ( luaL_loadfile( naevL, luafile ) != 0 ) {
      WARN( _( "Script '%s' Lua error:\n%s" ), luafile,
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
      return -1;
   }
   LOG( _( "Processing '%s'..." ), luafile );
   nlua_pushenv( naevL, nenv );
   lua_setfenv( naevL, -2 );
   /* Add argument table. */
   lua_newtable( naevL );
   int nargs = 0;
   lua_pushstring( naevL, luafile );
   lua_rawseti( naevL, -2, 0 );
   while ( opt < argc ) {
      lua_pushstring( naevL, argv[opt++] );
      lua_rawseti( naevL, -2, ++nargs );
   }
   nlua_setenv( naevL, nenv, "arg" );
   /* Run the code. */
   if ( nlua_pcall( nenv, 0, LUA_MULTRET ) != 0 ) {
      WARN( _( "Script '%s' Lua error:\n%s" ), luafile,
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
      return -1;
   }
   unload_all();
   return 0;
}
