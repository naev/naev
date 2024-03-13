/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "asteroid.h"
#include "nlua.h"

#define ASTEROID_METATABLE "asteroid" /**< Astroid metatable identifier. */

typedef struct LuaAsteroid_s {
   int parent;
   int id;
} LuaAsteroid_t;

/*
 * Asteroid library.
 */
int nlua_loadAsteroid( nlua_env env );

/*
 * Asteroid operations.
 */
LuaAsteroid_t *lua_toasteroid( lua_State *L, int ind );
LuaAsteroid_t *luaL_checkasteroid( lua_State *L, int ind );
LuaAsteroid_t *lua_pushasteroid( lua_State *L, LuaAsteroid_t vec );
Asteroid      *luaL_validasteroid( lua_State *L, int ind );
int            lua_isasteroid( lua_State *L, int ind );
