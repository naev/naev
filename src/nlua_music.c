/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_music.c
 *
 * @brief Lua music playing module.
 */


#include "nlua_music.h"

#include "SDL.h"

#include "nlua.h"
#include "nluadef.h"
#include "music.h"
#include "naev.h"
#include "log.h"
#include "ndata.h"


/* functions */
static int musicL_load( lua_State* L );
static int musicL_play( lua_State* L );
static int musicL_stop( lua_State* L );
static int musicL_isPlaying( lua_State* L );
static int musicL_current( lua_State* L );
static const luaL_reg music_methods[] = {
   { "load", musicL_load },
   { "play", musicL_play },
   { "stop", musicL_stop },
   { "isPlaying", musicL_isPlaying },
   { "current", musicL_current },
   {0,0}
}; /**< Music specific methods. */


/**
 * @brief Music Lua module.
 *
 * Typical usage would be something like:
 * @code
 * music.load( "intro" ) -- Load the song
 * music.play() -- Play it
 * @endcode
 *
 * @luamod music
 */


/**
 * @brief Loads the music functions into a lua_State.
 *
 *    @param L Lua State to load the music functions into.
 *    @param read_only Load the write functions?
 *    @return 0 on success.
 */
int lua_loadMusic( lua_State *L, int read_only )
{
   (void)read_only; /* future proof */
   luaL_register(L, "music", music_methods);
   return 0;
}


/**
 * @brief Loads a song.
 *
 *    @luaparam name Name of the song to load.
 * @luafunc load( name )
 */
static int musicL_load( lua_State *L )
{
   const char* str;

   /* check parameters */
   NLUA_MIN_ARGS(1);
   str = luaL_checkstring(L,1);

   music_load( str );
   return 0;
}


/**
 * @brief Plays the loaded song.
 *
 * @luafunc play()
 */
static int musicL_play( lua_State *L )
{
   (void)L;
   music_play();
   return 0;
}


/**
 * @brief Stops playing the current song.
 *
 * @luafunc stop()
 */
static int musicL_stop( lua_State *L )
{
   (void)L;
   music_stop();
   return 0;
}


/**
 * @brief Checks to see if something is playing.
 *
 *    @luareturn true if something is playing.
 * @luafunc isPlaying()
 */
static int musicL_isPlaying( lua_State* L )
{
   lua_pushboolean(L, music_isPlaying());
   return 1;
}


/**
 * @brief Gets the name of the current playing song.
 *
 *    @luareturn The name of the current playing song or "none" if no song is playing.
 * @luafunc current()
 */
static int musicL_current( lua_State* L )
{
   const char *music_name;
   
   music_name = music_playingName();

   lua_pushstring(L, (music_name != NULL) ? music_name : "none" );
   return 1;
}



