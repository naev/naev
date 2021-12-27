/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"
#include "space.h"

#define SPOB_METATABLE   "planet" /**< Spob metatable identifier. */

/**
 * @brief Lua Spob Wrapper.
 */
typedef int LuaSpob;

/*
 * Load the planet library.
 */
int nlua_loadSpob( nlua_env env );

/*
 * Spob operations.
 */
LuaSpob lua_toplanet( lua_State *L, int ind );
LuaSpob luaL_checkplanet( lua_State *L, int ind );
LuaSpob* lua_pushplanet( lua_State *L, LuaSpob planet );
Spob* luaL_validplanet( lua_State *L, int ind );
int lua_isplanet( lua_State *L, int ind );
