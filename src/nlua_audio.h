/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_AUDIO_H
#  define NLUA_AUDIO_H

/** @cond */
#include "al.h"
/** @endcond */

#include "nlua.h"


#define AUDIO_METATABLE      "audio" /**< Audio metatable identifier. */


typedef struct LuaAudio_s {
   ALuint source;
   ALuint buffer;
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


#endif /* NLUA_AUDIO_H */


