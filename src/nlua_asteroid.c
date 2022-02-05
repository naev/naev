/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_asteroid.c
 *
 * @brief Bindings for asteroids from Lua.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_asteroid.h"

/* Asteroid methods. */
static int asteroidL_eq( lua_State *L );
static const luaL_Reg asteroidL_methods[] = {
   { "__eq", asteroidL_eq },
   {0,0}
}; /**< AsteroidLua methods. */

/**
 * @brief Loads the asteroid library.
 *
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadAsteroid( nlua_env env )
{
   nlua_register(env, ASTEROID_METATABLE, asteroidL_methods, 1);
   return 0;
}

/**
 * @brief Gets asteroid at index.
 *
 *    @param L Lua state to get asteroid from.
 *    @param ind Index position to find the asteroid.
 *    @return Asteroid found at the index in the state.
 */
LuaAsteroid_t* lua_toasteroid( lua_State *L, int ind )
{
   return (LuaAsteroid_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets asteroid at index or raises error if there is no asteroid at index.
 *
 *    @param L Lua state to get asteroid from.
 *    @param ind Index position to find asteroid.
 *    @return Asteroid found at the index in the state.
 */
LuaAsteroid_t* luaL_checkasteroid( lua_State *L, int ind )
{
   if (lua_isasteroid(L,ind))
      return lua_toasteroid(L,ind);
   luaL_typerror(L, ind, ASTEROID_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a asteroid on the stack.
 *
 *    @param L Lua state to push asteroid into.
 *    @param asteroid Asteroid to push.
 *    @return Newly pushed asteroid.
 */
LuaAsteroid_t* lua_pushasteroid( lua_State *L, LuaAsteroid_t asteroid )
{
   LuaAsteroid_t *la = (LuaAsteroid_t*) lua_newuserdata(L, sizeof(LuaAsteroid_t));
   *la = asteroid;
   luaL_getmetatable(L, ASTEROID_METATABLE);
   lua_setmetatable(L, -2);
   return la;
}
/**
 * @brief Checks to see if ind is a asteroid.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a asteroid.
 */
int lua_isasteroid( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, ASTEROID_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief Lua bindings to interact with asteroid.
 *
 *
 * @luamod asteroid
 */
/**
 * @brief Compares two asteroids to see if they are the same.
 *
 *    @luatparam Asteroid a1 Asteroid 1 to compare.
 *    @luatparam Asteroid a2 Asteroid 2 to compare.
 *    @luatreturn boolean true if both asteroids are the same.
 * @luafunc __eq
 */
static int asteroidL_eq( lua_State *L )
{
   LuaAsteroid_t *a1, *a2;
   a1 = luaL_checkasteroid(L,1);
   a2 = luaL_checkasteroid(L,2);
   lua_pushboolean( L, (memcmp( a1, a2, sizeof(LuaAsteroid_t) )==0) );
   return 1;
}
