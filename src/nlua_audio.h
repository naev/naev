/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include "SDL_thread.h"
#include "al.h"
#include <vorbis/vorbisfile.h>
/** @endcond */

#include "nlua.h"

#define AUDIO_METATABLE "audio" /**< Audio metatable identifier. */

typedef enum LuaAudioType_e {
   LUA_AUDIO_NULL = 0,
   LUA_AUDIO_STATIC,
   LUA_AUDIO_STREAM,
} LuaAudioType_t;

typedef struct LuaBuffer_s {
   ALuint buffer;   /**< Buffer to use. */
   int    refcount; /**< Refcount. */
} LuaBuffer_t;

typedef struct LuaAudio_s {
#if DEBUGGING
   char *name;               /**< Filename of the audio. */
#endif                       /* DEBUGGING */
   int            ok;        /**< The source and audio is valid if ok==0. */
   LuaAudioType_t type;      /**< Type of audio. */
   int            nocleanup; /**< No need to clean up this source. */
   ALuint         source;    /**< Source to use. */
   ALuint         slot;      /**< Effects. */
   double         volume;    /**< Volume setting. */
   /* When not streaming. */
   LuaBuffer_t *buf; /**< Shared buffer. */
   /* When streaming. */
   SDL_mutex
      *lock; /**< Lock for vorbis file stream. This should be locked
       only when stream is accessed and not while soundLock() is active. */
   OggVorbis_File stream;          /**< Vorbis file stream. */
   vorbis_info   *info;            /**< Information of the tream. */
   ALenum         format;          /**< Stream format. */
   ALfloat        rg_scale_factor; /**< Replaygain scale factor. */
   ALfloat
          rg_max_scale; /**< Replaygain maximum scale factor before clipping. */
   ALuint stream_buffers[2]; /**< Double buffering for streaming. */
   int    active;            /**< Active buffer. */
   SDL_Thread *th;           /**< Buffering thread. */
   SDL_cond   *cond;         /**< For message passing. */
} LuaAudio_t;

/*
 * Library loading
 */
int nlua_loadAudio( nlua_env env );

/* Basic operations. */
LuaAudio_t *lua_toaudio( lua_State *L, int ind );
LuaAudio_t *luaL_checkaudio( lua_State *L, int ind );
LuaAudio_t *lua_pushaudio( lua_State *L, LuaAudio_t audio );
int         lua_isaudio( lua_State *L, int ind );

/* Useful stuff. */
void audio_clone( LuaAudio_t *la, const LuaAudio_t *source );
void audio_cleanup( LuaAudio_t *la );
