/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include "al.h"
/** @endcond */

#include "nlua.h"

#define AUDIO_METATABLE      "audio" /**< Audio metatable identifier. */

typedef struct LuaBuffer_s {
   ALuint buffer;    /**< Buffer to use. */
   int refcount;     /**< Refcount. */
} LuaBuffer_t;

typedef struct LuaAudio_s {
   int nocleanup;    /**< No need to clean up this source. */
   ALuint source;    /**< Source to use. */
   LuaBuffer_t *buf; /**< Shared buffer. */
   ALuint slot;      /**< Effects. */
   double volume;    /**< Volume setting. */
} LuaAudio_t;

/*
 * Library loading
 */
int nlua_loadAudio( nlua_env env );

/* Basic operations. */
LuaAudio_t* lua_toaudio( lua_State *L, int ind );
LuaAudio_t* luaL_checkaudio( lua_State *L, int ind );
LuaAudio_t* lua_pushaudio( lua_State *L, LuaAudio_t audio );
int lua_isaudio( lua_State *L, int ind );

/* Useful stuff. */
void audio_clone( LuaAudio_t *la, const LuaAudio_t *source );
void audio_cleanup( LuaAudio_t *la );
