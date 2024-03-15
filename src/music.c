/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file music.c
 *
 * @brief Controls all the music playing.
 */

/** @cond */
#include "SDL.h"
#include "physfsrwops.h"

#include "naev.h"
/** @endcond */

#include "music.h"

#include "conf.h"
#include "log.h"
#include "ndata.h"
#include "nlua.h"
#include "nlua_audio.h"
#include "nlua_tk.h"
#include "nlua_var.h"
#include "nluadef.h"
#include "nstring.h"
#include "ntracing.h"
#include "sound.h"

#define MUSIC_SUFFIX ".ogg" /**< Suffix of musics. */

int           music_disabled = 0; /**< Whether or not music is disabled. */
static double music_vol      = 0.;
static double music_vol_lin  = 0.;

/*
 * Handle if music should run Lua script.  Must be locked to ensure same
 *  behaviour always.
 */
static int music_runchoose =
   0; /**< Whether or not music should run the choose function. */

/*
 * global music lua
 */
static nlua_env music_env        = LUA_NOREF; /**< The Lua music control env. */
static int      music_lua_update = LUA_NOREF;
static int      music_lua_choose = LUA_NOREF;
static int      music_lua_play   = LUA_NOREF;
static int      music_lua_stop   = LUA_NOREF;
static int      music_lua_pause  = LUA_NOREF;
static int      music_lua_resume = LUA_NOREF;
static int      music_lua_info   = LUA_NOREF;
static int      music_lua_volume = LUA_NOREF;

/* functions */
static int music_runLua( const char *situation );

/*
 * prototypes
 */
/* music stuff */
static int music_find( void );
/* Lua stuff */
static int  music_luaInit( void );
static void music_luaQuit( void );

/**
 * @brief Updates the music.
 */
void music_update( double dt )
{
   if ( music_disabled )
      return;

   NTracingZone( _ctx, 1 );

   /* Run the choose function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_update );
   lua_pushnumber( naevL, dt );
   if ( nlua_pcall( music_env, 1, 0 ) ) { /* error has occurred */
      WARN( _( "Error while running music function '%s': %s" ), "update",
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
   }

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Runs the Lua music choose function.
 *
 *    @param situation Situation in to choose music for.
 *    @return 0 on success.
 */
static int music_runLua( const char *situation )
{
   if ( music_disabled )
      return 0;

   /* Run the choose function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_choose );
   if ( situation != NULL )
      lua_pushstring( naevL, situation );
   else
      lua_pushnil( naevL );
   if ( nlua_pcall( music_env, 1, 0 ) ) { /* error has occurred */
      WARN( _( "Error while running music function '%s': %s" ), "choose",
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
      return -1;
   }

   return 0;
}

/**
 * @brief Initializes the music subsystem.
 *
 *    @return 0 on success.
 */
int music_init( void )
{
   if ( music_disabled )
      return 0;

   /* Load the music. */
   music_find();

   /* Start up Lua. */
   if ( music_luaInit() < 0 )
      return -1;

   /* Set the volume. */
   if ( ( conf.music > 1. ) || ( conf.music < 0. ) )
      WARN( _( "Music has invalid value, clamping to [0:1]." ) );
   music_volume( conf.music );

   return 0;
}

/**
 * @brief Exits the music subsystem.
 */
void music_exit( void )
{
   if ( music_disabled )
      return;

   /* Clean up Lua. */
   music_luaQuit();
}

/**
 * @brief Internal music loading routines.
 *
 *    @return 0 on success.
 */
static int music_find( void )
{
   char **files;
   int    suflen;
   int    nmusic;

   if ( music_disabled )
      return 0;

   /* get the file list */
   files = PHYSFS_enumerateFiles( MUSIC_PATH );

   /* load the profiles */
   nmusic = 0;
   suflen = strlen( MUSIC_SUFFIX );
   for ( size_t i = 0; files[i] != NULL; i++ ) {
      int flen = strlen( files[i] );
      if ( ( flen > suflen ) &&
           strncmp( &files[i][flen - suflen], MUSIC_SUFFIX, suflen ) == 0 ) {

         /* grow the selection size */
         nmusic++;
      }
   }

   DEBUG( n_( "Loaded %d Song", "Loaded %d Songs", nmusic ), nmusic );

   /* More clean up. */
   PHYSFS_freeList( files );

   return 0;
}

/**
 * @brief Sets the music volume from a linear value.
 *
 *    @param vol Volume to set to (between 0 and 1).
 *    @return 0 on success.
 */
int music_volume( double vol )
{
   if ( music_disabled )
      return 0;

   music_vol_lin = vol;
   if ( vol > 0. )
      music_vol = 1. / pow( 2., ( 1. - vol ) * 8. );
   else
      music_vol = 0.;

   /* Run the choose function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_volume );
   lua_pushnumber( naevL, music_vol );
   if ( nlua_pcall( music_env, 1, 0 ) ) { /* error has occurred */
      WARN( _( "Error while running music function '%s': %s" ), "volume",
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
      return -1;
   }
   return 0;
}

/**
 * @brief Gets the current music volume (linear).
 *
 *    @return The current music volume.
 */
double music_getVolume( void )
{
   return music_vol_lin;
}

/**
 * @brief Gets the current music volume (logarithmic).
 *
 *    @return The current music volume.
 */
double music_getVolumeLog( void )
{
   return music_vol;
}

/**
 * @brief Plays the loaded music.
 */
int music_play( const char *filename )
{
   if ( music_disabled )
      return 0;

   /* Run the play function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_play );
   if ( filename != NULL )
      lua_pushstring( naevL, filename );
   else
      lua_pushnil( naevL );
   if ( nlua_pcall( music_env, 1, 0 ) ) { /* error has occurred */
      WARN( _( "Error while running music function '%s': %s" ), "play",
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
      return -1;
   }
   return 0;
}

/**
 * @brief Stops the loaded music.
 *
 *    @param disable Whether or not to temporarily disable the music.
 *    @return 0 on success
 */
int music_stop( int disable )
{
   if ( music_disabled )
      return 0;

   /* Run the stop function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_stop );
   lua_pushboolean( naevL, disable );
   if ( nlua_pcall( music_env, 1, 0 ) ) { /* error has occurred */
      WARN( _( "Error while running music function '%s': %s" ), "stop",
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
      return -1;
   }
   return 0;
}

/**
 * @brief Pauses the music.
 */
int music_pause( int disable )
{
   if ( music_disabled )
      return 0;

   /* Run the pause function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_pause );
   lua_pushboolean( naevL, disable );
   if ( nlua_pcall( music_env, 1, 0 ) ) { /* error has occurred */
      WARN( _( "Error while running music function '%s': %s" ), "pause",
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
      return -1;
   }
   return 0;
}

/**
 * @brief Resumes the music.
 */
int music_resume( void )
{
   if ( music_disabled )
      return 0;

   /* Run the resume function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_resume );
   if ( nlua_pcall( music_env, 0, 0 ) ) { /* error has occurred */
      WARN( _( "Error while running music function '%s': %s" ), "resume",
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
      return -1;
   }
   return 0;
}

static MusicInfo_t minfo;
/**
 * @brief Gets information about the current music state.
 */
MusicInfo_t *music_info( void )
{
   if ( minfo.name ) {
      free( minfo.name );
      minfo.name = NULL;
   }

   /* Run the info function in Lua. */
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, music_lua_info );
   if ( nlua_pcall( music_env, 0, 3 ) ) { /* error has occurred */
      WARN( _( "Error while running music function '%s': %s" ), "info",
            lua_tostring( naevL, -1 ) );
      lua_pop( naevL, 1 );
      return NULL;
   }

   minfo.playing = lua_toboolean( naevL, -3 );
   minfo.name    = strdup( luaL_optstring( naevL, -2, "" ) );
   minfo.pos     = luaL_optnumber( naevL, -1, -1 );

   lua_pop( naevL, 3 );

   return &minfo;
}

/*
 * music Lua stuff
 */
/**
 * @brief Initialize the music Lua control system.
 *
 *    @return 0 on success.
 */
static int music_luaInit( void )
{
   char  *buf;
   size_t bufsize;

   if ( music_disabled )
      return 0;

   if ( music_env != LUA_NOREF )
      music_luaQuit();

   /* Reset the environment. */
   music_env = nlua_newEnv();
   nlua_loadStandard( music_env );
   nlua_loadTk( music_env );

   /* load the actual Lua music code */
   buf = ndata_read( MUSIC_LUA_PATH, &bufsize );
   if ( nlua_dobufenv( music_env, buf, bufsize, MUSIC_LUA_PATH ) != 0 ) {
      ERR( _( "Error loading music file: %s\n"
              "%s\n"
              "Most likely Lua file has improper syntax, please check" ),
           MUSIC_LUA_PATH, lua_tostring( naevL, -1 ) );
      return -1;
   }
   free( buf );

   /* Set up comfort functions. */
   music_lua_choose = nlua_refenvtype( music_env, "choose", LUA_TFUNCTION );
   music_lua_update = nlua_refenvtype( music_env, "update", LUA_TFUNCTION );
   music_lua_play   = nlua_refenvtype( music_env, "play", LUA_TFUNCTION );
   music_lua_stop   = nlua_refenvtype( music_env, "stop", LUA_TFUNCTION );
   music_lua_pause  = nlua_refenvtype( music_env, "pause", LUA_TFUNCTION );
   music_lua_resume = nlua_refenvtype( music_env, "resume", LUA_TFUNCTION );
   music_lua_info   = nlua_refenvtype( music_env, "info", LUA_TFUNCTION );
   music_lua_volume = nlua_refenvtype( music_env, "volume", LUA_TFUNCTION );

   return 0;
}

/**
 * @brief Quits the music Lua control system.
 */
static void music_luaQuit( void )
{
   if ( music_disabled )
      return;

   nlua_freeEnv( music_env );
   music_env        = LUA_NOREF;
   music_lua_choose = LUA_NOREF;
   music_lua_update = LUA_NOREF;
   music_lua_play   = LUA_NOREF;
   music_lua_stop   = LUA_NOREF;
   music_lua_pause  = LUA_NOREF;
   music_lua_resume = LUA_NOREF;
   music_lua_info   = LUA_NOREF;
   music_lua_volume = LUA_NOREF;
}

/**
 * @brief Actually runs the music stuff, based on situation.
 *
 *    @param situation Choose a new music to play.
 *    @return 0 on success.
 */
int music_choose( const char *situation )
{
   if ( music_disabled || sound_disabled )
      return 0;

   if ( music_runLua( situation ) )
      return -1;

   return 0;
}

/**
 * @brief Attempts to rechoose the music.
 */
void music_rechoose( void )
{
   if ( music_disabled )
      return;

   /* Lock so it doesn't run in between an update. */
   music_runchoose = 1;
}
