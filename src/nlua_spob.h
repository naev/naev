/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"
#include "space.h"

#define SPOB_METATABLE "spob" /**< Spob metatable identifier. */

/**
 * @brief Lua Spob Wrapper.
 */
typedef int LuaSpob;

/*
 * Load the Space Object (Spob) library.
 */
int nlua_loadSpob( nlua_env env );

/*
 * Spob operations.
 */
LuaSpob  lua_tospob( lua_State *L, int ind );
LuaSpob  luaL_checkspob( lua_State *L, int ind );
LuaSpob *lua_pushspob( lua_State *L, LuaSpob spob );
Spob    *luaL_validspob( lua_State *L, int ind );
int      lua_isspob( lua_State *L, int ind );
