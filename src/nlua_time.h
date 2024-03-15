/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"
#include "ntime.h"

#define TIME_METATABLE "time" /**< Spob metatable identifier. */

/*
 * Library stuff.
 */
int nlua_loadTime( nlua_env env );

/*
 * Time operations.
 */
ntime_t *lua_totime( lua_State *L, int ind );
ntime_t *luaL_checktime( lua_State *L, int ind );
ntime_t *lua_pushtime( lua_State *L, ntime_t time );
ntime_t  luaL_validtime( lua_State *L, int ind );
int      lua_istime( lua_State *L, int ind );
