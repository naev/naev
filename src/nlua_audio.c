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

#include "AL/efx.h"
#include "AL/efx-presets.h"

#include "conf.h"
#include "array.h"
#include "nlua_vec2.h"
#include "nluadef.h"
#include "nlua_file.h"
#include "nstring.h"
#include "sound.h"
#include "sound_openal.h"


typedef struct LuaAudioEfx_s {
   char *name;
   ALuint effect;
   ALuint slot;
} LuaAudioEfx_t;


static LuaAudioEfx_t *lua_efx = NULL;


/* Audio methods. */
static int audioL_gc( lua_State *L );
static int audioL_eq( lua_State *L );
static int audioL_new( lua_State *L );
static int audioL_clone( lua_State *L );
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
static int audioL_setRelative( lua_State *L );
static int audioL_setPosition( lua_State *L );
static int audioL_getPosition( lua_State *L );
static int audioL_setVelocity( lua_State *L );
static int audioL_getVelocity( lua_State *L );
static int audioL_setLooping( lua_State *L );
static int audioL_isLooping( lua_State *L );
static int audioL_setPitch( lua_State *L );
static int audioL_getPitch( lua_State *L );
static int audioL_setEffect( lua_State *L );
/* Deprecated stuff. */
static int audioL_soundPlay( lua_State *L ); /* Obsolete API, to get rid of. */
static const luaL_Reg audioL_methods[] = {
   { "__gc", audioL_gc },
   { "__eq", audioL_eq },
   { "new", audioL_new },
   { "clone", audioL_clone },
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
   { "setRelative", audioL_setRelative },
   { "setPosition", audioL_setPosition },
   { "getPosition", audioL_getPosition },
   { "setVelocity", audioL_setVelocity },
   { "getVelocity", audioL_getVelocity },
   { "setLooping", audioL_setLooping },
   { "isLooping", audioL_isLooping },
   { "setPitch", audioL_setPitch },
   { "getPitch", audioL_getPitch },
   { "setEffect", audioL_setEffect },
   /* Deprecated. */
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
   if (!sound_disabled) {
      soundLock();
      alGetSourcei( la->source, param, &b );
      al_checkErr();
      soundUnlock();
   }
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
   if (!sound_disabled) {
      soundLock();
      alGetSourcei( la->source, AL_SOURCE_STATE, &s );
      al_checkErr();
      soundUnlock();
   }
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
   if (sound_disabled)
      return 0;
   soundLock();
   alDeleteSources( 1, &la->source );
   /* Check if buffers need freeing. */
   la->buf->refcount--;
   if (la->buf->refcount <= 0) {
      alDeleteBuffers( 1, &la->buf->buffer );
      free( la->buf );
   }
   /* Clean up. */
   al_checkErr();
   soundUnlock();
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

   memset( &la, 0, sizeof(LuaAudio_t) );
   if (!sound_disabled) {
      rw = PHYSFSRWOPS_openRead( name );
      if (rw==NULL)
         NLUA_ERROR(L,"Unable to open '%s'", name );

      soundLock();
      alGenSources( 1, &la.source );

      la.buf = malloc( sizeof(LuaBuffer_t) );
      la.buf->refcount = 1;
      alGenBuffers( 1, &la.buf->buffer );
      sound_al_buffer( &la.buf->buffer, rw, name );

      /* Attach buffer. */
      alSourcei( la.source, AL_BUFFER, la.buf->buffer );

      /* Defaults. */
      master = sound_getVolumeLog();
      alSourcef( la.source, AL_GAIN, master );
      /* The behaviour of sources depends on whether or not they are mono or
       * stereo. In the case they are stereo, no position stuff is actually
       * done. However, if they are mono, they are played with absolute
       * position and the sound heard depends on the listener. We can disable
       * this by setting AL_SOURCE_RELATIVE which puts the listener always at
       * the origin, and then setting the source at the same origin. It should
       * be noted that depending on the sound model this can be bad if it is
       * not bounded. */
      alSourcei( la.source, AL_SOURCE_RELATIVE, AL_TRUE );
      alSource3f( la.source, AL_POSITION, 0., 0., 0. );
      al_checkErr();
      soundUnlock();

      SDL_RWclose( rw );
   }

   lua_pushaudio(L, la);
   return 1;
}


/**
 * @brief Clones an existing audio source.
 *
 *    @luatparam Audio source Audio source to clone.
 *    @luatreturn Audio New audio corresponding to the data.
 * @luafunc clone
 */
static int audioL_clone( lua_State *L )
{
   LuaAudio_t la;
   LuaAudio_t *source = luaL_checkaudio(L,1);
   double master;

   memset( &la, 0, sizeof(LuaAudio_t) );
   if (sound_disabled) {
      lua_pushaudio(L, la);
      return 1;
   }

   soundLock();
   alGenSources( 1, &la.source );

   /* Attach source buffer. */
   la.buf = source->buf;
   la.buf->refcount++;

   /* Attach buffer. */
   alSourcei( la.source, AL_BUFFER, la.buf->buffer );

   /* TODO this should probably set the same parameters as the original source
    * being cloned to be truly compatible with Love2D. */
   /* Defaults. */
   master = sound_getVolumeLog();
   alSourcef( la.source, AL_GAIN, master );
   /* See note in audioL_new */
   alSourcei( la.source, AL_SOURCE_RELATIVE, AL_TRUE );
   alSource3f( la.source, AL_POSITION, 0., 0., 0. );
   al_checkErr();
   soundUnlock();

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
   if (!sound_disabled) {
      soundLock();
      alSourcePlay( la->source );
      al_checkErr();
      soundUnlock();
   }
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
   if (sound_disabled)
      return 0;
   soundLock();
   alSourcePause( la->source );
   al_checkErr();
   soundUnlock();
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
   if (sound_disabled)
      return 0;

   soundLock();
   alSourceStop( la->source );
   al_checkErr();
   soundUnlock();
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
   if (!sound_disabled) {
      soundLock();
      alSourceRewind( la->source );
      al_checkErr();
      soundUnlock();
   }
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
   if (!sound_disabled) {
      soundLock();
      if (strcmp(unit,"seconds")==0)
         alSourcef( la->source, AL_SEC_OFFSET, offset );
      else if (strcmp(unit,"samples")==0)
         alSourcef( la->source, AL_SAMPLE_OFFSET, offset );
      else
         NLUA_ERROR(L, _("Unknown seek source '%s'! Should be either 'seconds' or 'samples'!"), unit );
      al_checkErr();
      soundUnlock();
   }
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
   if (!sound_disabled) {
      soundLock();
      if (strcmp(unit,"seconds")==0)
         alGetSourcef( la->source, AL_SEC_OFFSET, &offset );
      else if (strcmp(unit,"samples")==0)
         alGetSourcef( la->source, AL_SAMPLE_OFFSET, &offset );
      else
         NLUA_ERROR(L, _("Unknown seek source '%s'! Should be either 'seconds' or 'samples'!"), unit );
      al_checkErr();
      soundUnlock();
   }
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
   if (sound_disabled)
      return 0;

   soundLock();
   alSourcef( la->source, AL_GAIN, master * volume );
   al_checkErr();
   soundUnlock();
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
   if (sound_disabled)
      volume = 0.;
   else {
      soundLock();
      if (lua_gettop(L)>0) {
         la = luaL_checkaudio(L,1);
         alGetSourcef( la->source, AL_GAIN, &alvol );
         master = sound_getVolumeLog();
         volume = alvol / master;
      }
      else
         volume = sound_getVolume();
      al_checkErr();
      soundUnlock();
   }
   lua_pushnumber(L, volume);
   return 1;
}


/**
 * @brief Sets whether a source is relative or not.
 *
 *    @luatparam boolean relative Whether or not to make the source relative or not.
 * @luafunc setRelative
 */
static int audioL_setRelative( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (sound_disabled)
      return 0;

   soundLock();
   alSourcei( la->source, AL_SOURCE_RELATIVE, lua_toboolean(L,2) );
   al_checkErr();
   soundUnlock();
   return 0;
}


/**
 * @brief Sets the position of a source.
 *
 *    @luatparam Audio source Source to set position of.
 *    @luatparam number x X position.
 *    @luatparam number y Y position.
 *    @luatparam number z Z position.
 * @luafunc setPosition
 */
static int audioL_setPosition( lua_State *L )
{
   ALfloat pos[3];
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (sound_disabled)
      return 0;

   pos[0] = luaL_optnumber(L,2,0.);
   pos[1] = luaL_optnumber(L,3,0.);
   pos[2] = luaL_optnumber(L,4,0.);

   soundLock();
   alSourcefv( la->source, AL_POSITION, pos );
   al_checkErr();
   soundUnlock();
   return 0;
}


/**
 * @brief Gets the position of a source.
 *
 *    @luatparam Audio source Source to get position of.
 *    @luatreturn number X position.
 *    @luatreturn number Y position.
 *    @luatreturn number Z position.
 * @luafunc getPosition
 */
static int audioL_getPosition( lua_State *L )
{
   ALfloat pos[3];
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (sound_disabled) {
      lua_pushnumber(L,0.);
      lua_pushnumber(L,0.);
      lua_pushnumber(L,0.);
      return 0;
   }

   soundLock();
   alGetSource3f( la->source, AL_POSITION, &pos[0], &pos[1], &pos[2] );
   al_checkErr();
   soundUnlock();

   lua_pushnumber(L,pos[0]);
   lua_pushnumber(L,pos[1]);
   lua_pushnumber(L,pos[2]);
   return 3;
}


/**
 * @brief Sets the velocity of a source.
 *
 *    @luatparam Audio source Source to set velocity of.
 *    @luatparam number x X velocity.
 *    @luatparam number y Y velocity.
 *    @luatparam number z Z velocity.
 * @luafunc setVelocity
 */
static int audioL_setVelocity( lua_State *L )
{
   ALfloat vel[3];
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (sound_disabled)
      return 0;

   vel[0] = luaL_optnumber(L,2,0.);
   vel[1] = luaL_optnumber(L,3,0.);
   vel[2] = luaL_optnumber(L,4,0.);

   soundLock();
   alSourcefv( la->source, AL_VELOCITY, vel );
   al_checkErr();
   soundUnlock();
   return 0;
}


/**
 * @brief Gets the velocity of a source.
 *
 *    @luatparam Audio source Source to get velocity of.
 *    @luatreturn number X velocity.
 *    @luatreturn number Y velocity.
 *    @luatreturn number Z velocity.
 * @luafunc getVelocity
 */
static int audioL_getVelocity( lua_State *L )
{
   ALfloat vel[3];
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (sound_disabled) {
      lua_pushnumber(L,0.);
      lua_pushnumber(L,0.);
      lua_pushnumber(L,0.);
      return 0;
   }

   soundLock();
   alGetSource3f( la->source, AL_VELOCITY, &vel[0], &vel[1], &vel[2] );
   al_checkErr();
   soundUnlock();

   lua_pushnumber(L,vel[0]);
   lua_pushnumber(L,vel[1]);
   lua_pushnumber(L,vel[2]);
   return 3;
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
   if (!sound_disabled) {
      soundLock();
      alSourcei( la->source, AL_LOOPING, b );
      al_checkErr();
      soundUnlock();
   }
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
   if (sound_disabled)
      return 0;
   soundLock();
   alSourcef( la->source, AL_PITCH, pitch );
   al_checkErr();
   soundUnlock();
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
   if (!sound_disabled) {
      soundLock();
      alGetSourcef( la->source, AL_PITCH, &p );
      al_checkErr();
      soundUnlock();
   }
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


static void efx_setnum( lua_State *L, int pos, ALuint effect, const char *name, ALuint param, ALfloat def ) {
   lua_getfield(L,pos,name);
   if (lua_isnil(L,-1))
      nalEffectf( effect, param, def );
   else
      nalEffectf( effect, param, luaL_checknumber(L,-1) );
   lua_pop(L,1);
}
static void efx_setint( lua_State *L, int pos, ALuint effect, const char *name, ALuint param, ALint def ) {
   lua_getfield(L,pos,name);
   if (lua_isnil(L,-1))
      nalEffecti( effect, param, def );
   else
      nalEffecti( effect, param, luaL_checkinteger(L,-1) );
   lua_pop(L,1);
}
static int audioL_setEffectGlobal( lua_State *L )
{
   const char *name = luaL_checkstring(L,1);
   ALuint effect, slot;
   const char *type;
   double volume;
   LuaAudioEfx_t *lae;
   const int p = 2;

   /* Get the type. */
   lua_getfield(L,p,"type");
   type = luaL_checkstring(L,-1);
   lua_pop(L,1);

   /* Get the volume. */
   lua_getfield(L,p,"volume");
   if (lua_isnil(L,-1))
      volume = -1.;
   else
      volume = luaL_checknumber(L,-1);
   lua_pop(L,1);

   soundLock();
   nalGenEffects(1, &effect);

   /* Handle types. */
   if (strcmp(type,"reverb")==0) {
      const EFXEAXREVERBPROPERTIES reverb = EFX_REVERB_PRESET_GENERIC;

      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);

      efx_setnum( L, p, effect, "gain", AL_REVERB_GAIN, reverb.flGain );
      efx_setnum( L, p, effect, "highgain", AL_REVERB_GAINHF, reverb.flGainHF );
      efx_setnum( L, p, effect, "density", AL_REVERB_DENSITY, reverb.flDensity );
      efx_setnum( L, p, effect, "diffusion", AL_REVERB_DIFFUSION, reverb.flDiffusion );
      efx_setnum( L, p, effect, "decaytime", AL_REVERB_DECAY_TIME, reverb.flDecayTime );
      efx_setnum( L, p, effect, "decayhighratio", AL_REVERB_DECAY_HFRATIO, reverb.flDecayHFRatio );
      efx_setnum( L, p, effect, "earlygain", AL_REVERB_REFLECTIONS_GAIN, reverb.flReflectionsGain );
      efx_setnum( L, p, effect, "earlydelay", AL_REVERB_REFLECTIONS_DELAY, reverb.flReflectionsDelay );
      efx_setnum( L, p, effect, "lategain", AL_REVERB_LATE_REVERB_GAIN, reverb.flLateReverbGain );
      efx_setnum( L, p, effect, "latedelay", AL_REVERB_LATE_REVERB_DELAY, reverb.flLateReverbDelay );
      efx_setnum( L, p, effect, "roomrolloff", AL_REVERB_ROOM_ROLLOFF_FACTOR, reverb.flRoomRolloffFactor );
      efx_setnum( L, p, effect, "airabsorption", AL_REVERB_AIR_ABSORPTION_GAINHF, reverb.flAirAbsorptionGainHF );
      efx_setint( L, p, effect, "highlimit", AL_REVERB_DECAY_HFLIMIT, reverb.iDecayHFLimit );
   }
   else if (strcmp(type,"distortion")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_DISTORTION);

      efx_setnum( L, p, effect, "gain", AL_DISTORTION_GAIN, 0.2 ); /* 0.01 to 1.0 */
      efx_setnum( L, p, effect, "edge", AL_DISTORTION_EDGE, 0.05 ); /* 0.0 to 1.0 */
      efx_setnum( L, p, effect, "lowcut", AL_DISTORTION_LOWPASS_CUTOFF, 8000. ); /* 80.0 to 24000.0 */
      efx_setnum( L, p, effect, "center", AL_DISTORTION_EQCENTER, 3600. ); /* 80.0 to 24000.0 */
      efx_setnum( L, p, effect, "bandwidth", AL_DISTORTION_EQBANDWIDTH, 3600. ); /* 80.0 to 24000.0 */
   }
   else {
      soundUnlock();
      NLUA_ERROR(L, _("Usupported audio effect type '%s'!"), type);
   }

   al_checkErr();

   nalGenAuxiliaryEffectSlots( 1, &slot );
   if (volume > 0.)
      nalAuxiliaryEffectSlotf( slot, AL_EFFECTSLOT_GAIN, volume );
   nalAuxiliaryEffectSloti( slot, AL_EFFECTSLOT_EFFECT, effect );

   al_checkErr();
   soundUnlock();

   /* Find or add to array as necessary. */
   if (lua_efx == NULL)
      lua_efx = array_create( LuaAudioEfx_t );
   lae = NULL;
   for (int i=0; i<array_size(lua_efx); i++) {
      if (strcmp(name,lua_efx[i].name)==0) {
         lae = &lua_efx[i];
         break;
      }
   }
   if (lae == NULL)
      lae = &array_grow( &lua_efx );
   lae->name   = strdup( name );
   lae->effect = effect;
   lae->slot   = slot;

   return 0;
}


/**
 * @brief Sets effect stuff.
 */
static int audioL_setEffect( lua_State *L )
{
   if (al_info.efx == AL_FALSE) {
      lua_pushboolean(L,1);
      return 1;
   }

   /* Creating new effect. */
   if (!lua_isaudio(L,1))
      return audioL_setEffectGlobal(L);

   LuaAudio_t *la = luaL_checkaudio(L,1);
   const char *name = luaL_checkstring(L,2);
   int enable = (lua_isnoneornil(L,3)) ? 1 : lua_toboolean(L,3);
   LuaAudioEfx_t *lae;

   soundLock();
   if (enable) {
      lae = NULL;
      for (int i=0; i<array_size(lua_efx); i++) {
         if (strcmp(name,lua_efx[i].name)==0) {
            lae = &lua_efx[i];
            break;
         }
      }
      if (lae == NULL) {
         WARN(_("Unknown audio effect '%s'!"), name);
         return 0;
      }
      /* TODO allow more effect slots. */
      alSource3i( la->source, AL_AUXILIARY_SEND_FILTER, lae->slot, 0, AL_FILTER_NULL );
   }
   else
      alSource3i( la->source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL );

   al_checkErr();
   soundUnlock();

   lua_pushboolean(L,1);
   return 1;
}

