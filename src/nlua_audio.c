/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_audio.c
 *
 * @brief Bindings for Special effects functionality from Lua.
 */


/** @cond */
#include <lauxlib.h>
#include "physfsrwops.h"

#include "naev.h"
/** @endcond */

#include "nlua_audio.h"

#include "nlua_vec2.h"
#include "nluadef.h"
#include "nlua_file.h"
#include "nstring.h"
#include "sound.h"
#include "sound_openal.h"


/* Camera methods. */
static int audioL_gc( lua_State *L );
static int audioL_eq( lua_State *L );
static int audioL_new( lua_State *L );
static int audioL_play( lua_State *L );
static int audioL_pause( lua_State *L );
static int audioL_stop( lua_State *L );
static int audioL_setVolume( lua_State *L );
static int audioL_getVolume( lua_State *L );
static int audioL_setLooping( lua_State *L );
static int audioL_setPitch( lua_State *L );
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
   { "setLooping", audioL_setLooping },
   { "setPitch", audioL_setPitch },
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
   alDeleteSources( 1, &la->source );
   alDeleteBuffers( 1, &la->buffer );
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


/**
 * @brief Creates a new audio source.
 *
 *    @luatparam string|File data Data to load the audio from.
 *    @luatreturn Audio New audio corresponding to the data.
 * @luafunc new( data )
 */
static int audioL_new( lua_State *L )
{
   LuaAudio_t la;
   LuaFile_t *lf;
   const char *name;
   SDL_RWops *rw;
   double master;

   name = NULL;
   rw = NULL;

   if (lua_isstring(L,1))
      name = lua_tostring(L,1);
   else if (lua_isfile(L,1)) {
      lf = lua_tofile(L,1);
      name = lf->path;
   }
   else
      NLUA_INVALID_PARAMETER(L);
   rw = PHYSFSRWOPS_openRead( name );

   alGenSources( 1, &la.source );
   alGenBuffers( 1, &la.buffer );

   sound_al_buffer( &la.buffer, rw, name );

   /* Attach buffer. */
   alSourcei( la.source, AL_BUFFER, la.buffer );

   /* Defaults. */
   master = sound_getVolumeLog();
   alSourcef( la.source, AL_GAIN, master );

   SDL_RWclose( rw );

   lua_pushaudio(L, la);
   return 1;
}


/**
 * @brief Plays a source.
 *
 *    @luatparam Audio source Source to play.
 *    @luatreturn boolean True on success.
 * @luafunc play( source )
 */
static int audioL_play( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   alSourcePlay( la->source );
   lua_pushboolean(L,1);
   return 1;
}


/**
 * @brief Pauses a source.
 *
 *    @luatparam Audio source Source to pause.
 *    @luatreturn boolean True on success.
 * @luafunc pause( source )
 */
static int audioL_pause( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   alSourcePause( la->source );
   return 0;
}


/**
 * @brief Stops a source.
 *
 *    @luatparam Audio source Source to stop.
 *    @luatreturn boolean True on success.
 * @luafunc stop( source )
 */
static int audioL_stop( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   alSourceStop( la->source );
   return 0;
}


/**
 * @brief Sets the volume of a source.
 *
 *    @luatparam Audio source Source to set volume of.
 *    @luatparam number vol Volume to set the source to.
 * @luafunc setVolume( source, vol )
 */
static int audioL_setVolume( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   double volume = luaL_checknumber(L,2);
   double master = sound_getVolumeLog();
   alSourcef( la->source, AL_GAIN, master * volume );
   return 0;
}


/**
 * @brief Gets the volume of a source.
 *
 *    @luatparam Audio source Source to get volume of.
 *    @luatreturn number Volume the source is set to.
 * @luafunc getVolume( source, vol )
 */
static int audioL_getVolume( lua_State *L )
{
   LuaAudio_t *la;
   double volume, master;
   ALfloat alvol;
   if (lua_gettop(L)>0) {
      la = luaL_checkaudio(L,1);
      alGetSourcef( la->source, AL_GAIN, &alvol );
      master = sound_getVolumeLog();
      volume = alvol / master;
   }
   else {
      volume = sound_getVolume();
   }
   lua_pushnumber(L, volume);
   return 1;
}


/**
 * @brief Sets a source to be looping or not.
 *
 *    @luatparam Audio source Source to set looping state of.
 *    @luatparam boolean enable Whether or not the source should be set to looping.
 * @luafunc getVolume( source, vol )
 */
static int audioL_setLooping( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   int b = lua_toboolean(L,2);
   alSourcei( la->source, AL_LOOPING, b );
   return 0;
}


/**
 * @brief Sets the pitch of a source.
 *
 *    @luatparam Audio source Source to set pitch of.
 *    @luatparam number pitch Pitch to set the source to.
 * @luafunc setPitch( source, pitch )
 */
static int audioL_setPitch( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   double pitch = luaL_checknumber(L,2);
   alSourcei( la->source, AL_PITCH, pitch );
   return 0;
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

