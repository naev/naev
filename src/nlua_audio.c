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

#include "conf.h"
#include "nlua_vec2.h"
#include "nluadef.h"
#include "nlua_file.h"
#include "nstring.h"
#include "sound.h"
#include "sound_openal.h"


/* Audio methods. */
static int audioL_gc( lua_State *L );
static int audioL_eq( lua_State *L );
static int audioL_new( lua_State *L );
static int audioL_play( lua_State *L );
static int audioL_pause( lua_State *L );
static int audioL_isPaused( lua_State *L );
static int audioL_stop( lua_State *L );
static int audioL_isStopped( lua_State *L );
static int audioL_rewind( lua_State *L );
static int audioL_seek( lua_State *L );
static int audioL_tell( lua_State *L );
static int audioL_setVolume( lua_State *L );
static int audioL_getVolume( lua_State *L );
static int audioL_setLooping( lua_State *L );
static int audioL_isLooping( lua_State *L );
static int audioL_setPitch( lua_State *L );
static int audioL_getPitch( lua_State *L );
static int audioL_soundPlay( lua_State *L );
static const luaL_Reg audioL_methods[] = {
   { "__gc", audioL_gc },
   { "__eq", audioL_eq },
   { "new", audioL_new },
   { "play", audioL_play },
   { "pause", audioL_pause },
   { "isPaused", audioL_isPaused },
   { "stop", audioL_stop },
   { "isStopped", audioL_isStopped },
   { "rewind", audioL_rewind },
   { "seek", audioL_seek },
   { "tell", audioL_tell },
   { "setVolume", audioL_setVolume },
   { "getVolume", audioL_getVolume },
   { "setLooping", audioL_setLooping },
   { "isLooping", audioL_isLooping },
   { "setPitch", audioL_setPitch },
   { "getPitch", audioL_getPitch },
   { "soundPlay", audioL_soundPlay }, /* Old API */
   {0,0}
}; /**< AudioLua methods. */


/**
 * @brief Checks to see a boolean property of a source.
 */
static int audioL_isBool( lua_State *L, ALenum param )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   int b = 1;
   if (!conf.nosound)
      alGetSourcei( la->source, param, &b );
   al_checkErr();
   lua_pushboolean(L,b);
   return 1;
}


/**
 * @brief Checks to see the state of the source.
 */
static int audioL_isState( lua_State *L, ALenum state )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   int s = AL_STOPPED;
   if (!conf.nosound)
      alGetSourcei( la->source, AL_SOURCE_STATE, &s );
   al_checkErr();
   lua_pushboolean(L, s==state );
   return 1;
}



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
 * @luafunc __gc
 */
static int audioL_gc( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   alDeleteSources( 1, &la->source );
   alDeleteBuffers( 1, &la->buffer );
   al_checkErr();
   return 0;
}


/**
 * @brief Compares two audios to see if they are the same.
 *
 *    @luatparam Audio a1 Audio 1 to compare.
 *    @luatparam Audio a2 Audio 2 to compare.
 *    @luatreturn boolean true if both audios are the same.
 * @luafunc __eq
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
 * @luafunc new
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

   if (!conf.nosound) {
      rw = PHYSFSRWOPS_openRead( name );
      if (rw==NULL)
         NLUA_ERROR(L,"Unable to open '%s'", name );

      alGenSources( 1, &la.source );
      alGenBuffers( 1, &la.buffer );

      sound_al_buffer( &la.buffer, rw, name );

      /* Attach buffer. */
      alSourcei( la.source, AL_BUFFER, la.buffer );

      /* Defaults. */
      master = sound_getVolumeLog();
      alSourcef( la.source, AL_GAIN, master );

      SDL_RWclose( rw );
   }
   else
      memset( &la, 0, sizeof(LuaAudio_t) );

   al_checkErr();
   lua_pushaudio(L, la);
   return 1;
}


/**
 * @brief Plays a source.
 *
 *    @luatparam Audio source Source to play.
 *    @luatreturn boolean True on success.
 * @luafunc play
 */
static int audioL_play( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (!conf.nosound)
      alSourcePlay( la->source );
   al_checkErr();
   lua_pushboolean(L,1);
   return 1;
}


/**
 * @brief Pauses a source.
 *
 *    @luatparam Audio source Source to pause.
 *    @luatreturn boolean True on success.
 * @luafunc pause
 */
static int audioL_pause( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (!conf.nosound)
      alSourcePause( la->source );
   al_checkErr();
   return 0;
}


/**
 * @brief Checks to see if a source is paused.
 *
 *    @luatparam Audio source Source to check to see if is paused.
 *    @luatreturn boolean Whether or not the source is paused.
 * @luafunc isPaused
 */
static int audioL_isPaused( lua_State *L )
{
   return audioL_isState( L, AL_PAUSED );
}


/**
 * @brief Stops a source.
 *
 *    @luatparam Audio source Source to stop.
 * @luafunc stop
 */
static int audioL_stop( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (!conf.nosound)
      alSourceStop( la->source );
   al_checkErr();
   return 0;
}


/**
 * @brief Checks to see if a source is stopped.
 *
 *    @luatparam Audio source Source to check to see if is stopped.
 *    @luatreturn boolean Whether or not the source is stopped.
 * @luafunc isStopped
 */
static int audioL_isStopped( lua_State *L )
{
   return audioL_isState( L, AL_STOPPED );
}


/**
 * @brief Rewinds a source.
 *
 *    @luatparam Audio source Source to rewind.
 * @luafunc rewind
 */
static int audioL_rewind( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (!conf.nosound)
      alSourceRewind( la->source );
   al_checkErr();
   return 0;
}


/**
 * @brief Seeks a source.
 *
 *    @luatparam Audio source Source to seek.
 *    @luatparam number offset Offset to seek to.
 *    @luatparam[opt="seconds"] string unit Either "seconds" or "samples" indicating the type to seek to.
 * @luafunc seek
 */
static int audioL_seek( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   double offset = luaL_checknumber(L,2);
   const char *unit = luaL_optstring(L,3,"seconds");
   if (!conf.nosound) {
      if (strcmp(unit,"seconds")==0)
         alSourcef( la->source, AL_SEC_OFFSET, offset );
      else if (strcmp(unit,"samples")==0)
         alSourcef( la->source, AL_SAMPLE_OFFSET, offset );
      else
         NLUA_ERROR(L, _("Unknown seek source '%s'! Should be either 'seconds' or 'samples'!"), unit );
   }
   al_checkErr();
   return 0;
}


/**
 * @brief Gets the position of a source.
 *
 *    @luatparam Audio source Source to get position of.
 *    @luatparam[opt="seconds"] string unit Either "seconds" or "samples" indicating the type to report.
 *    @luatreturn number Offset of the source or -1 on error.
 * @luafunc tell
 */
static int audioL_tell( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   const char *unit = luaL_optstring(L,2,"seconds");
   float offset = -1.0;
   if (!conf.nosound) {
      if (strcmp(unit,"seconds")==0)
         alGetSourcef( la->source, AL_SEC_OFFSET, &offset );
      else if (strcmp(unit,"samples")==0)
         alGetSourcef( la->source, AL_SAMPLE_OFFSET, &offset );
      else
         NLUA_ERROR(L, _("Unknown seek source '%s'! Should be either 'seconds' or 'samples'!"), unit );
   }
   al_checkErr();
   lua_pushnumber(L, offset);
   return 1;
}


/**
 * @brief Sets the volume of a source.
 *
 *    @luatparam Audio source Source to set volume of.
 *    @luatparam number vol Volume to set the source to.
 * @luafunc setVolume
 */
static int audioL_setVolume( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   double volume = CLAMP( 0.0, 1.0,  luaL_checknumber(L,2) );
   double master = sound_getVolumeLog();
   if (!conf.nosound)
      alSourcef( la->source, AL_GAIN, master * volume );
   al_checkErr();
   return 0;
}


/**
 * @brief Gets the volume of a source.
 *
 *    @luatparam Audio source Source to get volume of.
 *    @luatreturn number Volume the source is set to.
 * @luafunc getVolume
 */
static int audioL_getVolume( lua_State *L )
{
   LuaAudio_t *la;
   double volume, master;
   ALfloat alvol;
   if (conf.nosound)
      volume = 0.;
   else if (lua_gettop(L)>0) {
      la = luaL_checkaudio(L,1);
      alGetSourcef( la->source, AL_GAIN, &alvol );
      master = sound_getVolumeLog();
      volume = alvol / master;
   }
   else
      volume = sound_getVolume();
   al_checkErr();
   lua_pushnumber(L, volume);
   return 1;
}


/**
 * @brief Sets a source to be looping or not.
 *
 *    @luatparam Audio source Source to set looping state of.
 *    @luatparam boolean enable Whether or not the source should be set to looping.
 * @luafunc setLooping
 */
static int audioL_setLooping( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   int b = lua_toboolean(L,2);
   if (!conf.nosound)
      alSourcei( la->source, AL_LOOPING, b );
   al_checkErr();
   return 0;
}


/**
 * @brief Gets the looping state of a source.
 *
 *    @luatparam Audio source Source to get looping state of.
 *    @luatreturn boolean Whether or not the source is looping.
 * @luafunc isLooping
 */
static int audioL_isLooping( lua_State *L )
{
   return audioL_isBool( L, AL_LOOPING );
}


/**
 * @brief Sets the pitch of a source.
 *
 *    @luatparam Audio source Source to set pitch of.
 *    @luatparam number pitch Pitch to set the source to.
 * @luafunc setPitch
 */
static int audioL_setPitch( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   double pitch = luaL_checknumber(L,2);
   if (!conf.nosound)
      alSourcef( la->source, AL_PITCH, pitch );
   al_checkErr();
   return 0;
}


/**
 * @brief Gets the pitch of a source.
 *
 *    @luatparam Audio source Source to get pitch of.
 *    @luatreturn number Pitch of the source.
 * @luafunc getPitch
 */
static int audioL_getPitch( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   float p = 1.0;
   if (!conf.nosound)
      alGetSourcef( la->source, AL_PITCH, &p );
   al_checkErr();
   lua_pushnumber(L,p);
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
 * @luafunc soundPlay
 */
static int audioL_soundPlay( lua_State *L )
{
   const char *name;
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

   if (dopos)
      sound_playPos( sound_get(name), pos->x, pos->y, vel->x, vel->y );
   else
      sound_play( sound_get(name) );

   return 0;
}

