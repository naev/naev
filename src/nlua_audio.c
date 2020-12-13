/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_audio.c
 *
 * @brief Bindings for Special effects functionality from Lua.
 */

#include "nlua_audio.h"

#include "naev.h"

#include "nstring.h"

#include <lauxlib.h>

#include "nluadef.h"
#include "sound.h"
#include "nlua_vec2.h"


/* Camera methods. */
static int audioL_gc( lua_State *L );
static int audioL_eq( lua_State *L );
static int audioL_new( lua_State *L );
static int audioL_play( lua_State *L );
static int audioL_pause( lua_State *L );
static int audioL_stop( lua_State *L );
static int audioL_setVolume( lua_State *L );
static int audioL_getVolume( lua_State *L );
static int audioL_soundPlay( lua_State *L );
static const luaL_Reg audioL_methods[] = {
   { "__gc", audioL_gc },
   { "__eq", audioL_eq },
   { "new", audioL_new },
   { "play", audioL_play },
   { "pause", audioL_pause },
   { "stop", audioL_stop },
   { "setVolume", audioL_setVolume },
   { "getVolume", audioL_getVolume },
   { "soundPlay", audioL_soundPlay }, /* Old API */
   {0,0}
}; /**< AudioLua methods. */




/**
 * @brief Loads the audio library.
 *
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadAudio( nlua_env env )
{
   nlua_register(env, AUDIO_METATABLE, audioL_methods, 1);
   return 0;
}


/**
 * @brief Gets audio at index.
 *
 *    @param L Lua state to get audio from.
 *    @param ind Index position to find the audio.
 *    @return Audio found at the index in the state.
 */
LuaAudio_t* lua_toaudio( lua_State *L, int ind )
{
   return (LuaAudio_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets audio at index or raises error if there is no audio at index.
 *
 *    @param L Lua state to get audio from.
 *    @param ind Index position to find audio.
 *    @return Audio found at the index in the state.
 */
LuaAudio_t* luaL_checkaudio( lua_State *L, int ind )
{
   if (lua_isaudio(L,ind))
      return lua_toaudio(L,ind);
   luaL_typerror(L, ind, AUDIO_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a audio on the stack.
 *
 *    @param L Lua state to push audio into.
 *    @param audio Audio to push.
 *    @return Newly pushed audio.
 */
LuaAudio_t* lua_pushaudio( lua_State *L, LuaAudio_t audio )
{
   LuaAudio_t *la = (LuaAudio_t*) lua_newuserdata(L, sizeof(LuaAudio_t));
   *la = audio;
   luaL_getmetatable(L, AUDIO_METATABLE);
   lua_setmetatable(L, -2);
   return la;
}
/**
 * @brief Checks to see if ind is a audio.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a audio.
 */
int lua_isaudio( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, AUDIO_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Lua bindings to interact with audio.
 *
 *
 * @luamod audio
 */
/**
 * @brief Frees a audio.
 *
 *    @luatparam Audio audio Audio to free.
 * @luafunc __gc( audio )
 */
static int audioL_gc( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   (void) la; /* TODO */
   return 0;
}


/**
 * @brief Compares two audios to see if they are the same.
 *
 *    @luatparam Audio a1 Audio 1 to compare.
 *    @luatparam Audio a2 Audio 2 to compare.
 *    @luatreturn boolean true if both audios are the same.
 * @luafunc __eq( a1, a2 )
 */
static int audioL_eq( lua_State *L )
{
   LuaAudio_t *a1, *a2;
   a1 = luaL_checkaudio(L,1);
   a2 = luaL_checkaudio(L,2);
   lua_pushboolean( L, (memcmp( a1, a2, sizeof(LuaAudio_t) )==0) );
   return 1;
}


static int audioL_new( lua_State *L )
{
   LuaAudio_t la;
   const char *str;
   str = luaL_checkstring(L,1);
   (void) str; /* TODO */
   lua_pushaudio(L, la);
   return 1;
}


static int audioL_play( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   (void) la; /* TODO */
   lua_pushboolean(L,1);
   return 1;
}


static int audioL_pause( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   (void) la; /* TODO */
   return 0;
}


static int audioL_stop( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   (void) la; /* TODO */
   return 0;
}


static int audioL_setVolume( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   double volume = luaL_checknumber(L,2);
   (void) la; /* TODO */
   (void) volume; /* TODO */
   return 0;
}


static int audioL_getVolume( lua_State *L )
{
   LuaAudio_t *la;
   double volume;
   if (lua_gettop(L)>0) {
      la = luaL_checkaudio(L,1);
      (void) la; /* TODO */
   }
   else {
      volume = sound_getVolume();
   }
   lua_pushnumber(L, volume);
   return 1;
}


/**
 * @brief Plays a sound.
 *
 * by default, the sound is played at player's current position
 *
 * @usage audio.soundPlay( "hail" ) -- Plays the hail sound
 * @usage audio.soundPlay( "hail", pos ) -- Plays the hail sound at position pos
 * @usage audio.soundPlay( "hail", pos, vel ) -- Plays the hail sound at position pos with velocity vel
 *
 *    @luatparam string s Name of the sound to play
 *    @luatparam[opt] Vec2 pos Position of the source
 *    @luatparam[opt] Vec2 vel Velocity of the source
 * @luafunc soundPlay( s )
 */
static int audioL_soundPlay( lua_State *L )
{
   const char *name;
   char buf[PATH_MAX];
   Vector2d *pos, *vel, vel0;
   int dopos;

   /* Flag wether to use sound_playPos or sound_play. */
   dopos = 0;

   /* Handle parameters. */
   name = luaL_checkstring(L,1);
   if (lua_gettop(L) > 1) {
      dopos = 1;
      pos = luaL_checkvector(L,2);
      if (lua_gettop(L) > 2) {
         vel = luaL_checkvector(L,3);
      }
      else {
         vectnull( &vel0 );
         vel = &vel0;
      }
   }

   nsnprintf(buf, PATH_MAX, "%s", name);

   if (dopos)
      sound_playPos( sound_get(buf), pos->x, pos->y, vel->x, vel->y );
   else
      sound_play( sound_get(buf) );

   return 0;
}

