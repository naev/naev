/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_music.c
 *
 * @brief Lua music playing module.
 */
/** @cond */
#include "SDL.h"

#include "naev.h"
/** @endcond */

#include "nlua_music.h"

#include "log.h"
#include "music.h"
#include "ndata.h"
#include "nluadef.h"

/* Music methods. */
static int musicL_choose( lua_State *L );
static int musicL_play( lua_State *L );
static int musicL_pause( lua_State *L );
static int musicL_stop( lua_State *L );
static int musicL_isPlaying( lua_State *L );
static int musicL_current( lua_State *L );
static int musicL_getVolume( lua_State *L );
static const luaL_Reg music_methods[] = {
   { "choose", musicL_choose },
   { "play", musicL_play },
   { "pause", musicL_pause },
   { "stop", musicL_stop },
   { "isPlaying", musicL_isPlaying },
   { "current", musicL_current },
   { "getVolume", musicL_getVolume },
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
 *    @param env Lua environment to load the music functions into.
 *    @return 0 on success.
 */
int nlua_loadMusic( nlua_env env )
{
   nlua_register(env, "music", music_methods, 0);
   return 0;
}

/**
 * @brief Delays a rechoose.
 *
 * @usage music.choose( "ambient" ) -- Rechooses ambient in 5 seconds
 *
 *    @luatparam string situation Situation to choose.
 * @luafunc choose
 */
static int musicL_choose( lua_State* L )
{
   const char *situation = luaL_checkstring(L,1);
   if (music_choose( situation ))
      return NLUA_ERROR(L,"music.choose failed!");
   return 0;
}

/**
 * @brief Plays the loaded song. Will resume paused songs.
 *
 * @luafunc play
 */
static int musicL_play( lua_State *L )
{
   const char *str = luaL_optstring(L,1,NULL);
   if (music_play( str ))
      return NLUA_ERROR(L,"music.play failed!");
   return 0;
}

/**
 * @brief Pauses the music engine.
 *
 *    @luatparam boolean disable Whether or not to disable music changes until music.play() is called. This disables all in-game music changes such as taking off.
 * @luafunc pause
 */
static int musicL_pause( lua_State* L )
{
   int disable = lua_toboolean(L,1);
   if (music_pause(disable))
      return NLUA_ERROR(L,"music.pause failed!");
   return 0;
}

/**
 * @brief Stops playing the current song.
 *
 *    @luatparam boolean nodisable Whether or not to disable music changes until music.play() is called. This disables all in-game music changes such as taking off. Disables by false, set to true to only stop the current song.
 * @luafunc stop
 */
static int musicL_stop( lua_State *L )
{
   int nodisable = lua_toboolean(L,1);
   if (music_stop(!nodisable))
      return NLUA_ERROR(L,"music.stop failed!");
   return 0;
}

/**
 * @brief Checks to see if something is playing.
 *
 *    @luatreturn boolean true if something is playing.
 * @luafunc isPlaying
 */
static int musicL_isPlaying( lua_State* L )
{
   if (music_disabled) {
      lua_pushboolean(L, 0);
   }
   else {
      MusicInfo_t *minfo = music_info();
      if (minfo==NULL)
         return NLUA_ERROR(L,"Failed to get music info!");
      lua_pushboolean(L, minfo->playing);
   }
   return 1;
}

/**
 * @brief Gets the name of the current playing song.
 *
 * @usage songname, songplayed = music.current()
 *
 *    @luatreturn string The name of the current playing song or "none" if no song is playing.
 *    @luatreturn number The current offset inside the song (-1. if music is none).
 * @luafunc current
 */
static int musicL_current( lua_State* L )
{
   if (music_disabled) {
      lua_pushstring(L, "none");
      lua_pushnumber(L, -1.);
   }
   else {
      MusicInfo_t *minfo = music_info();
      if (minfo==NULL)
         return NLUA_ERROR(L,"Failed to get music info!");
      lua_pushstring(L, minfo->name);
      lua_pushnumber(L, minfo->pos);
   }
   return 2;
}

/**
 * @brief Gets the volume level of the music.
 *
 *    @luatparam boolean log Whether or not to get the volume in log scale.
 *    @luatreturn number Volume level of the music.
 * @luafunc getVolume
 */
static int musicL_getVolume( lua_State *L )
{
   if (lua_toboolean(L,1))
      lua_pushnumber(L, music_getVolumeLog() );
   else
      lua_pushnumber(L, music_getVolume() );
   return 1;
}
