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
#include "nopenal.h"

/**
 * @brief Default pre-amp in dB.
 */
#define RG_PREAMP_DB       0.0

/**
 * @brief Handles the OpenAL effects that have been set up Lua side.
 */
typedef struct LuaAudioEfx_s {
   char *name;    /**< Effect name for look ups. */
   ALuint effect; /**< Effect ID. */
   ALuint slot;   /**< Effect slot. */
} LuaAudioEfx_t;

/**
 * @brief List of effects handled by Lua. These are persistent throughout game runtime.
 */
static LuaAudioEfx_t *lua_efx = NULL;

static int stream_thread( void *la_data );
static int stream_loadBuffer( LuaAudio_t *la, ALuint buffer );
static void rg_filter( float **pcm, long channels, long samples, void *filter_param );
static int audio_genSource( ALuint *source );

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
static int audioL_getDuration( lua_State *L );
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
static int audioL_setAttenuationDistances( lua_State *L );
static int audioL_getAttenuationDistances( lua_State *L );
static int audioL_setRolloff( lua_State *L );
static int audioL_getRolloff( lua_State *L );
static int audioL_setEffect( lua_State *L );
static int audioL_setGlobalEffect( lua_State *L );
static int audioL_setGlobalAirAbsorption( lua_State *L );
static int audioL_setGlobaDopplerFactor( lua_State *L );
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
   { "getDuration", audioL_getDuration },
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
   { "setAttenuationDistances", audioL_setAttenuationDistances },
   { "getAttenuationDistances", audioL_getAttenuationDistances },
   { "setRolloff", audioL_setRolloff },
   { "getRolloff", audioL_getRolloff },
   { "setEffect", audioL_setEffect },
   { "setGlobalEffect", audioL_setGlobalEffect },
   { "setGlobalAirAbsorption", audioL_setGlobalAirAbsorption },
   { "setGlobalDopplerFactor", audioL_setGlobaDopplerFactor },
   /* Deprecated. */
   { "soundPlay", audioL_soundPlay }, /* Old API */
   {0,0}
}; /**< AudioLua methods. */

static int stream_thread( void *la_data )
{
   LuaAudio_t *la = (LuaAudio_t*) la_data;

   while (1) {
      ALint alstate;
      ALuint removed;

      soundLock();

      /* Case finished. */
      if (la->active < 0) {
         la->th = NULL;
         SDL_CondBroadcast( la->cond );
         alSourceStop( la->source );
         soundUnlock();
         return 0;
      }

      alGetSourcei( la->source, AL_BUFFERS_PROCESSED, &alstate );
      if (alstate > 0) {
         int ret;
         /* Refill active buffer */
         alSourceUnqueueBuffers( la->source, 1, &removed );
         ret = stream_loadBuffer( la, la->stream_buffers[ la->active ] );
         if ((la->active < 0) || (ret < 0)) {
            /* stream_loadBuffer unlocks the sound lock internally, which can
             * lead to the thread being gc'd and having active = -1. We have to
             * add a check here to not mess around with stuff. */
            la->th = NULL;
            SDL_CondBroadcast( la->cond );
            alSourceStop( la->source );
            soundUnlock();
            return 0;
         }
         else {
            alSourceQueueBuffers( la->source, 1, &la->stream_buffers[ la->active ] );
            la->active = 1 - la->active;
         }
      }
      al_checkErr(); /* XXX - good or bad idea to log from the thread? */
      soundUnlock();

      SDL_Delay(10);
   }
}

/**
 * @brief Loads a buffer.
 *
 * Assumes that soundLock() is set.
 */
static int stream_loadBuffer( LuaAudio_t *la, ALuint buffer )
{
   int ret;
   size_t size;
   char buf[ 32 * 1024 ];

   soundUnlock();
   ret  = 0;
   size = 0;
   while (size < sizeof(buf)) { /* file up the entire data buffer */
      int section, result;

      SDL_mutexP( la->lock );
      result = ov_read_filter(
            &la->stream,   /* stream */
            &buf[size],             /* data */
            sizeof(buf) - size,    /* amount to read */
            (SDL_BYTEORDER == SDL_BIG_ENDIAN),
            2,                      /* 16 bit */
            1,                      /* signed */
            &section,               /* current bitstream */
            rg_filter,              /* filter function */
            la );                   /* filter parameter */
      SDL_mutexV( la->lock );

      /* End of file. */
      if (result == 0) {
         if (size == 0) {
            return -2;
         }
         ret = 1;
         break;
      }
      /* Hole error. */
      else if (result == OV_HOLE) {
         WARN(_("OGG: Vorbis hole detected in music!"));
         return 0;
      }
      /* Bad link error. */
      else if (result == OV_EBADLINK) {
         WARN(_("OGG: Invalid stream section or corrupt link in music!"));
         return -1;
      }

      size += result;
   }
   soundLock();

   /* load the buffer up */
   alBufferData( buffer, la->format, buf, size, la->info->rate );
   al_checkErr();

   return ret;
}

/**
 * @brief This is the filter function for the decoded Ogg Vorbis stream.
 *
 * base on:
 * vgfilter.c (c) 2007,2008 William Poetra Yoga Hadisoeseno
 * based on:
 * vgplay.c 1.0 (c) 2003 John Morton
 */
static void rg_filter( float **pcm, long channels, long samples, void *filter_param )
{
   LuaAudio_t *param    = filter_param;
   float scale_factor   = param->rg_scale_factor;
   float max_scale      = param->rg_max_scale;

   /* Apply the gain, and any limiting necessary */
   if (scale_factor > max_scale) {
      for (int i=0; i < channels; i++)
         for (int j=0; j < samples; j++) {
            float cur_sample = pcm[i][j] * scale_factor;
            /*
             * This is essentially the scaled hard-limiting algorithm
             * It looks like the soft-knee to me
             * I haven't found a better limiting algorithm yet...
             */
            if (cur_sample < -0.5)
               cur_sample = tanh((cur_sample + 0.5) / (1-0.5)) * (1-0.5) - 0.5;
            else if (cur_sample > 0.5)
               cur_sample = tanh((cur_sample - 0.5) / (1-0.5)) * (1-0.5) + 0.5;
            pcm[i][j] = cur_sample;
         }
   }
   else if (scale_factor > 0.0)
      for (int i=0; i < channels; i++)
         for (int j=0; j < samples; j++)
            pcm[i][j] *= scale_factor;
}

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

void audio_cleanup( LuaAudio_t *la )
{
   if ((la==NULL) || (la->nocleanup))
      return;

   switch (la->type) {
      case LUA_AUDIO_NULL:
         break;
      case LUA_AUDIO_STATIC:
         soundLock();
         if (alIsSource( la->source )==AL_TRUE)
            alDeleteSources( 1, &la->source );
         /* Check if buffers need freeing. */
         if (la->buf != NULL) {
            la->buf->refcount--;
            if (la->buf->refcount <= 0) {
               alDeleteBuffers( 1, &la->buf->buffer );
               free( la->buf );
            }
         }
         al_checkErr();
         soundUnlock();
         break;

      case LUA_AUDIO_STREAM:
         soundLock();
         if (la->th != NULL) {
            la->active = -1;
            if (SDL_CondWaitTimeout( la->cond, sound_lock, 3000 ) == SDL_MUTEX_TIMEDOUT)
#if DEBUGGING
               WARN(_("Timed out while waiting for audio thread of '%s' to finish!"), la->name);
#else /* DEBUGGING */
               WARN(_("Timed out while waiting for audio thread to finish!"));
#endif /* DEBUGGING */
         }
         if (alIsSource( la->source )==AL_TRUE)
            alDeleteSources( 1, &la->source );
         if (alIsBuffer( la->stream_buffers[0] )==AL_TRUE)
            alDeleteBuffers( 2, la->stream_buffers );
         if (la->cond != NULL)
            SDL_DestroyCond( la->cond );
         if (la->lock != NULL)
            SDL_DestroyMutex( la->lock );
         ov_clear( &la->stream );
         al_checkErr();
         soundUnlock();
         break;
   }

#if DEBUGGING
   free(la->name);
#endif /* DEBUGGING */
}

/**
 * @brief Lua bindings to interact with audio.
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
   audio_cleanup( la );
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
 * @brief Tries to generate a single openal source, running GC if necessary.
 */
static int audio_genSource( ALuint *source )
{
   ALenum err;
   alGenSources( 1, source );
   if (alIsSource(*source)==AL_TRUE)
      return 0;
   err = alGetError();
   switch (err) {
      case AL_NO_ERROR:
         break;
      case AL_OUT_OF_MEMORY:
         /* Assume that we need to collect audio stuff. */
         soundUnlock();
         lua_gc( naevL, LUA_GCCOLLECT, 0 );
         soundLock();
         /* Try to create source again. */
         alGenSources( 1, source );
         if (alIsSource(*source)==AL_TRUE)
            return 0;
         al_checkErr();
         break;

      default:
#if DEBUGGING
         al_checkHandleError( err, __func__, __LINE__ );
#endif /* DEBUGGING */
         break;
   }
   return -1;
}

/**
 * @brief Creates a new audio source.
 *
 *    @luatparam string|File data Data to load the audio from.
 *    @luatparam[opt="static"] string  Either "static" to load the entire source at the start, or "stream" to load it in real time.
 *    @luatreturn Audio New audio corresponding to the data.
 * @luafunc new
 */
static int audioL_new( lua_State *L )
{
   LuaAudio_t la;
   LuaFile_t *lf;
   double master;
   int stream;
   const char *name;
   SDL_RWops *rw;

   /* First parameter. */
   if (lua_isstring(L,1))
      name = lua_tostring(L,1);
   else if (lua_isfile(L,1)) {
      lf = lua_tofile(L,1);
      name = lf->path;
   }
   else
      NLUA_INVALID_PARAMETER(L);

   /* Second parameter. */
   if (lua_isnoneornil(L,2)) {
      stream = 0;
   }
   else {
      const char *type = luaL_optstring(L,2,"static");
      if (strcmp(type,"static")==0)
         stream = 0;
      else if (strcmp(type,"stream")==0)
         stream = 1;
      else
         NLUA_INVALID_PARAMETER(L);
   }

   memset( &la, 0, sizeof(LuaAudio_t) );
   if (sound_disabled) {
      la.nocleanup = 1; /* Not initialized so no need to clean up. */
      lua_pushaudio(L, la);
      return 1;
   }
   rw = PHYSFSRWOPS_openRead( name );
   if (rw==NULL)
      NLUA_ERROR(L,"Unable to open '%s'", name );
#if DEBUGGING
   la.name = strdup( name );
#endif /* DEBUGGING */

   soundLock();
   la.ok = audio_genSource( &la.source );
   if (la.ok) {
      la.nocleanup = 1; /* Not initialized so no need to clean up. */
      lua_pushaudio(L, la);
      return 1;
   }

   /* Deal with stream. */
   if (!stream) {
      la.type = LUA_AUDIO_STATIC;
      la.buf = malloc( sizeof(LuaBuffer_t) );
      la.buf->refcount = 1;
      sound_al_buffer( &la.buf->buffer, rw, name );

      /* Attach buffer. */
      alSourcei( la.source, AL_BUFFER, la.buf->buffer );

      /* Clean up. */
      SDL_RWclose( rw );
   }
   else {
      vorbis_comment *vc;
      ALfloat track_gain_db, track_peak;
      char *tag;

      la.type = LUA_AUDIO_STREAM;
      /* ov_clear will close rw for us. */
      if (ov_open_callbacks( rw, &la.stream, NULL, 0, sound_al_ovcall ) < 0) {
         SDL_RWclose( rw );
         NLUA_ERROR(L,_("Audio '%s' does not appear to be a Vorbis bitstream."), name );
      }
      la.info = ov_info( &la.stream, -1 );

      /* Replaygain information. */
      vc             = ov_comment( &la.stream, -1 );
      track_gain_db  = 0.;
      track_peak     = 1.;
      if ((tag = vorbis_comment_query(vc, "replaygain_track_gain", 0)))
         track_gain_db  = atof(tag);
      if ((tag = vorbis_comment_query(vc, "replaygain_track_peak", 0)))
         track_peak     = atof(tag);
      la.rg_scale_factor = pow(10.0, (track_gain_db + RG_PREAMP_DB)/20.0);
      la.rg_max_scale  = 1.0 / track_peak;

      /* Set the format */
      if (la.info->channels == 1)
         la.format = AL_FORMAT_MONO16;
      else
         la.format = AL_FORMAT_STEREO16;

      la.active = 0;
      la.lock = SDL_CreateMutex();
      la.cond = SDL_CreateCond();
      alGenBuffers( 2, la.stream_buffers );
      /* Buffers get queued later. */
   }

   /* Defaults. */
   la.volume = 1.;
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

   lua_pushaudio(L, la);
   return 1;
}

void audio_clone( LuaAudio_t *la, const LuaAudio_t *source )
{
   double master;

   memset( la, 0, sizeof(LuaAudio_t) );
   if (sound_disabled || source->ok) {
      la->nocleanup = 1;
      return;
   }

   soundLock();
   la->ok = audio_genSource( &la->source );
   if (!la->ok) {
      la->nocleanup = 1;
      return;
   }

   switch (source->type) {
      case LUA_AUDIO_STATIC:
         /* Attach source buffer. */
         la->buf = source->buf;
         la->buf->refcount++;

         /* Attach buffer. */
         alSourcei( la->source, AL_BUFFER, la->buf->buffer );
         break;

      case LUA_AUDIO_STREAM:
         WARN(_("Unimplemented"));
         break;

      case LUA_AUDIO_NULL:
         break;
   }
   la->type = source->type;

   /* TODO this should probably set the same parameters as the original source
    * being cloned to be truly compatible with Love2D. */
   /* Defaults. */
   master = sound_getVolumeLog();
   alSourcef( la->source, AL_GAIN, master * source->volume );
   la->volume = source->volume;
   /* See note in audioL_new */
   alSourcei( la->source, AL_SOURCE_RELATIVE, AL_TRUE );
   alSource3f( la->source, AL_POSITION, 0., 0., 0. );
   al_checkErr();
   soundUnlock();
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
   audio_clone( &la, source );
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
   if (sound_disabled || la->ok)
      return 0;

   if ((la->type == LUA_AUDIO_STREAM) && (la->th == NULL)) {
      int ret = 0;
      ALint alstate;
      soundLock();
      alGetSourcei( la->source, AL_BUFFERS_QUEUED, &alstate );
      while (alstate < 2) {
         ret = stream_loadBuffer( la, la->stream_buffers[ la->active ] );
         if (ret < 0)
            break;
         alSourceQueueBuffers( la->source, 1, &la->stream_buffers[ la->active ] );
         la->active = 1-la->active;
         alGetSourcei( la->source, AL_BUFFERS_QUEUED, &alstate );
      }
      if (ret == 0)
         la->th = SDL_CreateThread( stream_thread, "stream_thread", la );
   }
   else
      soundLock();
   alSourcePlay( la->source );
   al_checkErr();
   soundUnlock();

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
   if (sound_disabled || la->ok)
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
   ALint alstate;
   ALuint removed[2];
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (sound_disabled || la->ok)
      return 0;

   soundLock();
   switch (la->type) {
      case LUA_AUDIO_NULL:
         break;
      case LUA_AUDIO_STATIC:
         alSourceStop( la->source );
         break;

      case LUA_AUDIO_STREAM:
         /* Kill the thread first. */
         if (la->th != NULL) {
            la->active = -1;
            if (SDL_CondWaitTimeout( la->cond, sound_lock, 3000 ) == SDL_MUTEX_TIMEDOUT)
#if DEBUGGING
               WARN(_("Timed out while waiting for audio thread of '%s' to finish!"), la->name);
#else /* DEBUGGING */
               WARN(_("Timed out while waiting for audio thread to finish!"));
#endif /* DEBUGGING */
         }
         la->th = NULL;

         /* Stopping a source will make all buffers become processed. */
         alSourceStop( la->source );

         /* Unqueue the buffers. */
         alGetSourcei( la->source, AL_BUFFERS_PROCESSED, &alstate );
         alSourceUnqueueBuffers( la->source, alstate, removed );

         /* Seek the stream to the beginning. */
         SDL_mutexP( la->lock );
         ov_pcm_seek( &la->stream, 0 );
         SDL_mutexV( la->lock );
         break;
   }
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
   if (sound_disabled || la->ok)
      return 0;

   switch (la->source) {
      case LUA_AUDIO_STATIC:
         soundLock();
         alSourceRewind( la->source );
         al_checkErr();
         soundUnlock();
         break;
      case LUA_AUDIO_STREAM:
         SDL_mutexP( la->lock );
         ov_raw_seek( &la->stream, 0 );
         SDL_mutexV( la->lock );
         break;
      case LUA_AUDIO_NULL:
         break;
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
   int seconds = 1;

   if (strcmp(unit,"samples")==0)
      seconds = 0;
   else if (strcmp(unit,"seconds")!=0)
      NLUA_ERROR(L, _("Unknown seek source '%s'! Should be either 'seconds' or 'samples'!"), unit );

   if (sound_disabled || la->ok)
      return 0;

   switch (la->type) {
      case LUA_AUDIO_STATIC:
         soundLock();
         if (seconds)
            alSourcef( la->source, AL_SEC_OFFSET, offset );
         else
            alSourcef( la->source, AL_SAMPLE_OFFSET, offset );
         al_checkErr();
         soundUnlock();
         break;

      case LUA_AUDIO_STREAM:
         SDL_mutexP( la->lock );
         if (seconds)
            ov_time_seek( &la->stream, offset );
         else
            ov_pcm_seek( &la->stream, offset );
         SDL_mutexV( la->lock );
         /* TODO force a reset of the buffers. */
         break;

      case LUA_AUDIO_NULL:
         break;
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
   double offset = -1.;
   float aloffset;
   int seconds = 1;

   if (strcmp(unit,"samples")==0)
      seconds = 0;
   else if (strcmp(unit,"seconds")!=0)
      NLUA_ERROR(L, _("Unknown seek source '%s'! Should be either 'seconds' or 'samples'!"), unit );

   if (sound_disabled || la->ok) {
      lua_pushnumber(L, -1.);
      return 1;
   }

   switch (la->type) {
      case LUA_AUDIO_STATIC:
         soundLock();
         if (seconds)
            alGetSourcef( la->source, AL_SEC_OFFSET, &aloffset );
         else
            alGetSourcef( la->source, AL_SAMPLE_OFFSET, &aloffset );
         offset = aloffset;
         al_checkErr();
         soundUnlock();
         break;

      case LUA_AUDIO_STREAM:
         SDL_mutexP( la->lock );
         if (seconds)
            offset = ov_time_tell( &la->stream );
         else
            offset = ov_pcm_tell( &la->stream );
         SDL_mutexV( la->lock );
         break;

      case LUA_AUDIO_NULL:
         break;
   }

   lua_pushnumber(L, offset);
   return 1;
}

/**
 * @brief Gets the length of a source.
 *
 *    @luatparam Audio source Source to get duration of.
 *    @luatparam[opt="seconds"] string unit Either "seconds" or "samples" indicating the type to report.
 *    @luatreturn number Duration of the source or -1 on error.
 * @luafunc getDuration
 */
static int audioL_getDuration( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   const char *unit = luaL_optstring(L,2,"seconds");
   float duration = -1.;
   int seconds = 1;
   ALint bytes, channels, bits, samples;
   ALuint buffer;

   if (strcmp(unit,"samples")==0)
      seconds = 0;
   else if (strcmp(unit,"seconds")!=0)
      NLUA_ERROR(L, _("Unknown duration source '%s'! Should be either 'seconds' or 'samples'!"), unit );

   if (sound_disabled || la->ok) {
      lua_pushnumber(L, -1.);
      return 1;
   }

   switch (la->type) {
      case LUA_AUDIO_STATIC:
         soundLock();
         buffer = la->buf->buffer;
         alGetBufferi( buffer, AL_SIZE, &bytes );
         alGetBufferi( buffer, AL_CHANNELS, &channels );
         alGetBufferi( buffer, AL_BITS, &bits );

         samples = bytes * 8 / (channels * bits);

         if (seconds) {
            ALint freq;
            alGetBufferi( buffer, AL_FREQUENCY, &freq );
            duration = (float) samples / (float) freq;
         }
         else
            duration = samples;
         al_checkErr();
         soundUnlock();
         break;

      case LUA_AUDIO_STREAM:
         SDL_mutexP( la->lock );
         if (seconds)
            duration = ov_time_total( &la->stream, -1 );
         else
            duration = ov_pcm_total( &la->stream, -1 );
         SDL_mutexV( la->lock );
         break;

      case LUA_AUDIO_NULL:
         break;
   }

   lua_pushnumber(L, duration);
   return 1;
}

/**
 * @brief Sets the volume of a source.
 *
 *    @luatparam Audio source Source to set volume of.
 *    @luatparam number vol Volume to set the source to with 0.0 being silent and 1.0 being full volume.
 *    @luatparam boolean ignorevol Don't modify volume based on master.
 * @luafunc setVolume
 */
static int audioL_setVolume( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   double volume = CLAMP( 0.0, 1.0, luaL_checknumber(L,2) );
   int ignorevol = lua_toboolean(L,3);
   if (sound_disabled || la->ok)
      return 0;

   soundLock();
   if (ignorevol)
      alSourcef( la->source, AL_GAIN, volume );
   else {
      double master = sound_getVolumeLog();
      alSourcef( la->source, AL_GAIN, master * volume );
   }
   al_checkErr();
   soundUnlock();
   la->volume = volume;
   return 0;
}

/**
 * @brief Gets the volume of a source.
 *
 *    @luatparam[opt] Audio source Source to get volume of.
 *    @luatreturn number Volume the source is set to.
 * @luafunc getVolume
 */
static int audioL_getVolume( lua_State *L )
{
   double volume;
   if (sound_disabled)
      volume = 0.;
   else if (lua_gettop(L) > 0)
      volume = luaL_checkaudio(L,1)->volume;
   else
      volume = sound_getVolume();
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
   if (sound_disabled || la->ok)
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
   if (sound_disabled || la->ok)
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
   if (sound_disabled || la->ok) {
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
   if (sound_disabled || la->ok)
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
   if (sound_disabled || la->ok) {
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
   if (sound_disabled || la->ok)
      return 0;
   soundLock();
   alSourcei( la->source, AL_LOOPING, b );
   al_checkErr();
   soundUnlock();
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
   if (sound_disabled || la->ok)
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
   if (!sound_disabled && !la->ok) {
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
   vec2 *pos, *vel, vel0;
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

/**
 * @brief Sets the attenuation distances for the audio source.
 *
 *    @luatparam number ref Reference distance.
 *    @luatparam number max Maximum distance.
 * @luafunc setAttenuationDistances
 */
static int audioL_setAttenuationDistances( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   double ref = luaL_checknumber(L,2);
   double max = luaL_checknumber(L,3);
   if (sound_disabled || la->ok)
      return 0;
   soundLock();
   alSourcef( la->source, AL_REFERENCE_DISTANCE, ref );
   alSourcef( la->source, AL_MAX_DISTANCE, max );
   al_checkErr();
   soundUnlock();
   return 0;
}

/**
 * @brief Gets the attenuation distances for the audio source. Set to 0. if audio is disabled.
 *
 *    @luatreturn number Reference distance.
 *    @luatreturn number Maximum distance.
 * @luafunc getAttenuationDistances
 */
static int audioL_getAttenuationDistances( lua_State *L )
{
   ALfloat ref, max;
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (sound_disabled || la->ok) {
      lua_pushnumber(L,0.);
      lua_pushnumber(L,0.);
      return 2;
   }
   soundLock();
   alGetSourcef( la->source, AL_REFERENCE_DISTANCE, &ref );
   alGetSourcef( la->source, AL_MAX_DISTANCE, &max );
   al_checkErr();
   soundUnlock();
   lua_pushnumber( L, ref );
   lua_pushnumber( L, max );
   return 2;
}

/**
 * @brief Sets the rollof factor.
 *
 *    @luatparam number rolloff New rolloff factor.
 * @luafunc setRolloff
 */
static int audioL_setRolloff( lua_State *L )
{
   LuaAudio_t *la = luaL_checkaudio(L,1);
   double rolloff = luaL_checknumber(L,2);
   if (sound_disabled || la->ok)
      return 0;
   soundLock();
   alSourcef( la->source, AL_ROLLOFF_FACTOR, rolloff );
   al_checkErr();
   soundUnlock();
   return 0;
}

/**
 * @brief Gets the rolloff factor.
 *
 *    @luatreturn number Rolloff factor or 0. if sound is disabled.
 * @luafunc getRolloff
 */
static int audioL_getRolloff( lua_State *L )
{
   ALfloat rolloff;
   LuaAudio_t *la = luaL_checkaudio(L,1);
   if (sound_disabled || la->ok) {
      lua_pushnumber(L,0.);
      return 1;
   }
   soundLock();
   alGetSourcef( la->source, AL_ROLLOFF_FACTOR, &rolloff);
   al_checkErr();
   soundUnlock();
   lua_pushnumber( L, rolloff );
   return 1;
}

static void efx_setnum( lua_State *L, int pos, ALuint effect, const char *name, ALuint param ) {
   lua_getfield(L,pos,name);
   if (!lua_isnil(L,-1))
      nalEffectf( effect, param, luaL_checknumber(L,-1) );
   lua_pop(L,1);
}
static void efx_setint( lua_State *L, int pos, ALuint effect, const char *name, ALuint param ) {
   lua_getfield(L,pos,name);
   if (!lua_isnil(L,-1))
      nalEffecti( effect, param, luaL_checkinteger(L,-1) );
   lua_pop(L,1);
}
static void efx_setbool( lua_State *L, int pos, ALuint effect, const char *name, ALuint param ) {
   lua_getfield(L,pos,name);
   if (!lua_isnil(L,-1))
      nalEffecti( effect, param, lua_toboolean(L,-1) ? AL_TRUE : AL_FALSE );
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
   if (lae == NULL) {
      lae = &array_grow( &lua_efx );
      nalGenEffects(1, &effect);
      nalGenAuxiliaryEffectSlots( 1, &slot );
      lae->name   = strdup( name );
      lae->effect = effect;
      lae->slot   = slot;
   }
   else {
      effect   = lae->effect;
      slot     = lae->slot;
   }

   /* Handle types. */
   if (strcmp(type,"reverb")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);

      efx_setnum( L, p, effect, "density", AL_REVERB_DENSITY ); /* 0.0 to 1.0 (1.0) */
      efx_setnum( L, p, effect, "diffusion", AL_REVERB_DIFFUSION ); /* 0.0 to 1.0 (1.0) */
      efx_setnum( L, p, effect, "gain", AL_REVERB_GAIN ); /* 0.0 to 1.0 (0.32) */
      efx_setnum( L, p, effect, "highgain", AL_REVERB_GAINHF ); /* 0.0 to 1.0 (0.89) */
      efx_setnum( L, p, effect, "decaytime", AL_REVERB_DECAY_TIME ); /* 0.1 to 20.0 (1.49) */
      efx_setnum( L, p, effect, "decayhighratio", AL_REVERB_DECAY_HFRATIO ); /* 0.1 to 2.0 (0.83) */
      efx_setnum( L, p, effect, "earlygain", AL_REVERB_REFLECTIONS_GAIN ); /* 0.0 to 3.16 (0.05) */
      efx_setnum( L, p, effect, "earlydelay", AL_REVERB_REFLECTIONS_DELAY ); /* 0.0 to 0.3 (0.007) */
      efx_setnum( L, p, effect, "lategain", AL_REVERB_LATE_REVERB_GAIN ); /* 0.0 to 10.0 (1.26) */
      efx_setnum( L, p, effect, "latedelay", AL_REVERB_LATE_REVERB_DELAY ); /* 0.0 to 0.1 (0.011) */
      efx_setnum( L, p, effect, "roomrolloff", AL_REVERB_ROOM_ROLLOFF_FACTOR ); /* 0.0 to 10.0 (0.0) */
      efx_setnum( L, p, effect, "airabsorption", AL_REVERB_AIR_ABSORPTION_GAINHF ); /* 0.892 to 1.0 (0.994) */
      efx_setbool( L, p, effect, "highlimit", AL_REVERB_DECAY_HFLIMIT ); /* AL_FALSE or AL_TRUE (AL_TRUE) */
   }
   else if (strcmp(type,"distortion")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_DISTORTION);

      efx_setnum( L, p, effect, "gain", AL_DISTORTION_GAIN ); /* 0.01 to 1.0 (0.2) */
      efx_setnum( L, p, effect, "edge", AL_DISTORTION_EDGE ); /* 0.0 to 1.0 (0.05) */
      efx_setnum( L, p, effect, "lowcut", AL_DISTORTION_LOWPASS_CUTOFF ); /* 80.0 to 24000.0 (8000.0) */
      efx_setnum( L, p, effect, "center", AL_DISTORTION_EQCENTER ); /* 80.0 to 24000.0 (3600.0) */
      efx_setnum( L, p, effect, "bandwidth", AL_DISTORTION_EQBANDWIDTH ); /* 80.0 to 24000.0 (3600.0) */
   }
   else if (strcmp(type,"chorus")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_CHORUS);

      efx_setint( L, p, effect, "waveform", AL_CHORUS_WAVEFORM ); /* 0=sin, 1=triangle (1) */
      efx_setint( L, p, effect, "phase", AL_CHORUS_PHASE ); /* -180 to 180 (90) */
      efx_setnum( L, p, effect, "rate",  AL_CHORUS_RATE ); /* 0.0 to 10.0 (1.1) */
      efx_setnum( L, p, effect, "depth",  AL_CHORUS_DEPTH ); /* 0.0 to 1.0 (0.1) */
      efx_setnum( L, p, effect, "feedback",  AL_CHORUS_FEEDBACK ); /* -1.0 to 1.0 (0.25) */
      efx_setnum( L, p, effect, "delay", AL_CHORUS_DELAY ); /* 0.0 to 0.016 (0.016) */
   }
   else if (strcmp(type,"compressor")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_COMPRESSOR);

      efx_setbool( L, p, effect, "enable", AL_COMPRESSOR_ONOFF ); /* AL_FALSE or AL_TRUE (AL_TRUE) */
   }
   else if (strcmp(type,"echo")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_ECHO);

      efx_setnum( L, p, effect, "delay", AL_ECHO_DELAY ); /* 0.0 to 0.207 (0.1) */
      efx_setnum( L, p, effect, "tapdelay", AL_ECHO_LRDELAY ); /* 0.0 to 0.404 (0.1) */
      efx_setnum( L, p, effect, "damping", AL_ECHO_DAMPING ); /* 0.0 to 0.99 (0.5) */
      efx_setnum( L, p, effect, "feedback", AL_ECHO_FEEDBACK ); /* 0.0 to 1.0 (0.5) */
      efx_setnum( L, p, effect, "spread", AL_ECHO_SPREAD ); /* -1.0 to 1.0 (-1.0) */
   }
   else if (strcmp(type,"ringmodulator")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_RING_MODULATOR);

      efx_setnum( L, p, effect, "frequency", AL_RING_MODULATOR_FREQUENCY ); /* 0.0 to 8000.0 (440.0) */
      efx_setnum( L, p, effect, "highcut", AL_RING_MODULATOR_HIGHPASS_CUTOFF ); /* 0.0 to 24000.0 (800.0) */
      efx_setint( L, p, effect, "waveform", AL_RING_MODULATOR_WAVEFORM ); /* 0 (sin), 1 (saw), 2 (square) (0 (sin)) */
   }
   else if (strcmp(type,"equalizer")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_EQUALIZER);

      efx_setnum( L, p, effect, "lowgain", AL_EQUALIZER_LOW_GAIN ); /* 0.126 to 7.943 (1.0) */
      efx_setnum( L, p, effect, "lowcut", AL_EQUALIZER_LOW_CUTOFF ); /* 50.0 to 800.0 (200.0) */
      efx_setnum( L, p, effect, "lowmidgain", AL_EQUALIZER_MID1_GAIN ); /* 0.126 to 7.943 (1.0) */
      efx_setnum( L, p, effect, "lowmidfrequency", AL_EQUALIZER_MID1_CENTER ); /* 200.0 to 3000.0 (500.0) */
      efx_setnum( L, p, effect, "lowmidbandwidth", AL_EQUALIZER_MID1_WIDTH ); /* 0.01 to 1.0 (1.0) */
      efx_setnum( L, p, effect, "highmidgain", AL_EQUALIZER_MID2_GAIN ); /* 0.126 to 7.943 (1.0) */
      efx_setnum( L, p, effect, "highmidfrequency", AL_EQUALIZER_MID2_CENTER ); /* 1000.0 to 8000.0 (3000.0) */
      efx_setnum( L, p, effect, "highmidbandwidth", AL_EQUALIZER_MID2_WIDTH ); /* 0.01 to 1.0 (1.0) */
      efx_setnum( L, p, effect, "highgain", AL_EQUALIZER_HIGH_GAIN ); /* 0.126 to 7.943 (1.0) */
      efx_setnum( L, p, effect, "highcut", AL_EQUALIZER_HIGH_CUTOFF ); /* 4000.0 to 16000.0 (6000.0) */
   }
   else if (strcmp(type,"pitchshifter")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_PITCH_SHIFTER);

      efx_setint( L, p, effect, "tunecoarse", AL_PITCH_SHIFTER_COARSE_TUNE ); /* -12 to 12 (12) */
      efx_setint( L, p, effect, "tunefine'", AL_PITCH_SHIFTER_FINE_TUNE ); /* -50 to 50  (0) */
   }
   else if (strcmp(type,"vocalmorpher")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_VOCAL_MORPHER);

      efx_setint( L, p, effect, "phonemea", AL_VOCAL_MORPHER_PHONEMEA ); /* 0 to 29 (0 ("A")) */
      efx_setint( L, p, effect, "phonemeb", AL_VOCAL_MORPHER_PHONEMEB ); /* 0 to 29 (10 ("ER")) */
      efx_setint( L, p, effect, "tunecoarsea", AL_VOCAL_MORPHER_PHONEMEA_COARSE_TUNING ); /* -24 to 24 (0) */
      efx_setint( L, p, effect, "tunecoarseb", AL_VOCAL_MORPHER_PHONEMEB_COARSE_TUNING ); /* -24 to 24 (0) */
      efx_setint( L, p, effect, "waveform", AL_VOCAL_MORPHER_WAVEFORM ); /* 0 (sin), 1 (saw), 2 (square) (0 (sin)) */
      efx_setnum( L, p, effect, "rate", AL_VOCAL_MORPHER_RATE ); /* 0.0 to 10.0 (1.41) */
   }
   else if (strcmp(type,"flanger")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_FLANGER);

      efx_setint( L, p, effect, "waveform", AL_FLANGER_WAVEFORM ); /*  0 (sin), 1 (triangle)  (1 (triangle)) */
      efx_setnum( L, p, effect, "phase", AL_FLANGER_PHASE ); /* -180 to 180 (0) */
      efx_setnum( L, p, effect, "rate", AL_FLANGER_RATE ); /* 0.0 to 10.0 (0.27) */
      efx_setnum( L, p, effect, "depth", AL_FLANGER_DEPTH ); /* 0.0 to 1.0 (1.0) */
      efx_setnum( L, p, effect, "feedback", AL_FLANGER_FEEDBACK ); /* -1.0 to 1.0 (-0.5) */
      efx_setnum( L, p, effect, "delay", AL_FLANGER_DELAY ); /* 0.0 to 0.004 (0.002) */
   }
   else if (strcmp(type,"frequencyshifter")==0) {
      nalEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_FREQUENCY_SHIFTER);

      efx_setnum( L, p, effect, "frequency", AL_FREQUENCY_SHIFTER_FREQUENCY ); /* 0.0 to 24000.0 (0.0) */
      efx_setint( L, p, effect, "leftdirection", AL_FREQUENCY_SHIFTER_LEFT_DIRECTION ); /* 0 (down), 1 (up), 2 (off) (0 (down)) */
      efx_setint( L, p, effect, "rightdirection", AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION ); /* 0 (down), 1 (up), 2 (off) (0 (down)) */
   }
   else {
      soundUnlock();
      NLUA_ERROR(L, _("Usupported audio effect type '%s'!"), type);
   }

   if (volume > 0.)
      nalAuxiliaryEffectSlotf( slot, AL_EFFECTSLOT_GAIN, volume );
   nalAuxiliaryEffectSloti( slot, AL_EFFECTSLOT_EFFECT, effect );

   al_checkErr();
   soundUnlock();

   return 0;
}

static LuaAudioEfx_t *audio_getEffectByName( const char *name )
{
   for (int i=0; i<array_size(lua_efx); i++)
      if (strcmp(name,lua_efx[i].name)==0)
         return &lua_efx[i];
   WARN(_("Unknown audio effect '%s'!"), name);
   return NULL;
}

/**
 * @brief Sets effect stuff, behaves different if the first parameter is a source or not.
 *
 * @usage audio.setEffect( "reverb", { type="reverb" } )
 * @usage source:setEffect( "reverb" )
 *
 *    @luatparam string name Name of the effect.
 *    @luatparam table|boolean params Parameter table of the effect if not applied to the source, or whether or not to enable it on the source otherwise.
 *    @luatreturn boolean true on success.
 * @luafunc setEffect
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

   soundLock();
   if (enable) {
      LuaAudioEfx_t *lae = audio_getEffectByName( name );
      if (lae == NULL) {
         soundUnlock();
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

/**
 * @brief Sets a global effect. Will overwrite whatever was set. Does not affect sources created in Lua.
 *
 *    @luatparam[opt] string name Name of the effect to set or nil to disable.
 * @luafunc setGlobalEffect
 */
static int audioL_setGlobalEffect( lua_State *L )
{
   LuaAudioEfx_t *lae;
   const char *name = luaL_optstring(L,1,NULL);

   if (sound_disabled)
      return 0;

   if (al_info.efx == AL_FALSE)
      return 0;

   /* Disable. */
   if (name==NULL) {
      soundLock();
      nalAuxiliaryEffectSloti( sound_efx_directSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL );
      al_checkErr();
      soundUnlock();
      return 0;
   }

   /* Try to set it. */
   lae = audio_getEffectByName( name );
   if (lae == NULL)
      return 0;

   /* Set the effect. */
   soundLock();
   nalAuxiliaryEffectSloti( sound_efx_directSlot, AL_EFFECTSLOT_EFFECT, lae->effect );
   al_checkErr();
   soundUnlock();
   return 0;
}

/**
 * @brief Allows setting the speed of sound and air absorption.
 *
 *    @luatparam[opt=3443] number speed Air speed.
 *    @luatparam[opt=-1] number absorption Air absorptuion for all sources. Has to be a value between 0 and 10. If negative, value is ignored.
 * @luafunc setGlobalAirAbsorption
 */
static int audioL_setGlobalAirAbsorption( lua_State *L )
{
   double speed = luaL_optnumber( L, 1, 3433. );
   double absorption = luaL_optnumber( L, 2, -1. );

   if (sound_disabled)
      return 0;

   soundLock();
   alSpeedOfSound( speed );
   if (absorption > 0.)
      sound_setAbsorption( absorption );
   al_checkErr();
   soundUnlock();
   return 0;
}

/**
 * @brief Sets the doppler effect factor.
 *
 * Defaults to 0.3 outside of the nebula and 1.0 in the nebula.
 *
 *    @luatparam number factor Factor to set doppler effect to. Must be positive.
 * @luafunc setGlobalDopplerFactor
 */
static int audioL_setGlobaDopplerFactor( lua_State *L )
{
   if (sound_disabled)
      return 0;

   soundLock();
   alDopplerFactor( luaL_checknumber(L,1) );
   al_checkErr();
   soundUnlock();
   return 0;
}
