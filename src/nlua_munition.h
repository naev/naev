/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <lua.h>
/** @endcond */

#include "weapon.h"

#define MUNITION_METATABLE "munition" /**< Munition metatable identifier. */

/**
 * @brief Lua Munition wrapper.
 */
typedef struct LuaMunition_ {
   unsigned int id;  /**< Munition ID. */
   size_t       idx; /**< Munition location in stack. */
} LuaMunition;

/*
 * Library loading
 */
int nlua_loadMunition( nlua_env env );

/*
 * Munition operations
 */
LuaMunition *lua_tomunition( lua_State *L, int ind );
LuaMunition *luaL_checkmunition( lua_State *L, int ind );
LuaMunition *lua_pushmunition( lua_State *L, const Weapon *w );
Weapon      *luaL_validmunition( lua_State *L, int ind );
int          lua_ismunition( lua_State *L, int ind );
