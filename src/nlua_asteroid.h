/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"
#include "asteroid.h"

#define ASTEROID_METATABLE   "asteroid"   /**< Astroid metatable identifier. */

typedef struct LuaAsteroid_s {
   int field;
   int asteroid;
} LuaAsteroid_t;

/*
 * Asteroid library.
 */
int nlua_loadAsteroid( nlua_env env );

/*
 * Asteroid operations.
 */
LuaAsteroid_t* lua_toasteroid( lua_State *L, int ind );
LuaAsteroid_t* luaL_checkasteroid( lua_State *L, int ind );
LuaAsteroid_t* lua_pushasteroid( lua_State *L, LuaAsteroid_t vec );
int lua_isasteroid( lua_State *L, int ind );
