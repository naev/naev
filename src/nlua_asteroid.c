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

#include "array.h"
#include "space.h"
#include "nluadef.h"
#include "nlua_vec2.h"
#include "nlua_commodity.h"

/* Asteroid methods. */
static int asteroidL_eq( lua_State *L );
static int asteroidL_pos( lua_State *L );
static int asteroidL_vel( lua_State *L );
static int asteroidL_scanned( lua_State *L );
static int asteroidL_timer( lua_State *L );
static int asteroidL_setTimer( lua_State *L );
static int asteroidL_armour( lua_State *L );
static int asteroidL_setArmour( lua_State *L );
static int asteroidL_materials( lua_State *L );
static const luaL_Reg asteroidL_methods[] = {
   { "__eq", asteroidL_eq },
   { "pos", asteroidL_pos },
   { "vel", asteroidL_vel },
   { "scanned", asteroidL_scanned },
   { "timer", asteroidL_timer },
   { "setTimer", asteroidL_setTimer },
   { "armour", asteroidL_armour },
   { "setArmour", asteroidL_setArmour },
   { "materials", asteroidL_materials },
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
 * @brief Gets asteroid at index raising an error if type doesn't match.
 *
 *    @param L Lua state to get system from.
 *    @param ind Index position of system.
 *    @return The Asteroid at ind.
 */
Asteroid* luaL_validasteroid( lua_State *L, int ind )
{
   Asteroid *a;
   if (lua_isasteroid(L, ind)) {
      LuaAsteroid_t *la = luaL_checkasteroid(L, ind);
      AsteroidAnchor *field = &cur_system->asteroids[ la->parent ];
      a = &field->asteroids[ la->id ];
   }
   else {
      luaL_typerror(L, ind, ASTEROID_METATABLE);
      return NULL;
   }

   if (a == NULL)
      NLUA_ERROR(L, _("Asteroid is invalid"));

   return a;
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

/**
 * @brief Gets the position of an asteroid.
 *
 *    @luatparam Asteroid a Asteroid to get position of.
 *    @luatreturn Vector Position of the asteroid.
 * @luafunc pos
 */
static int asteroidL_pos( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   lua_pushvector(L,a->pos);
   return 1;
}

/**
 * @brief Gets the velocity of an asteroid.
 *
 *    @luatparam Asteroid a Asteroid to get velocity of.
 *    @luatreturn Vector Position of the asteroid.
 * @luafunc vel
 */
static int asteroidL_vel( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   lua_pushvector(L,a->vel);
   return 1;
}

/**
 * @brief Gets whether or not an asteroid got scanned.
 *
 *    @luatparam Asteroid a Asteroid to get scanned state of.
 *    @luatreturn boolean Whether or not the asteroid was scanned.
 * @luafunc scanned
 */
static int asteroidL_scanned( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   lua_pushboolean(L,a->scanned);
   return 1;
}

/**
 * @brief Gets the time left on the asteroid.
 *
 *    @luatparam Asteroid a Asteroid to get time left on.
 *    @luatreturn number Seconds left before the asteroid is unavailable.
 *    @luatreturn number Total number of seconds the asteroid had to live.
 * @luafunc timer
 */
static int asteroidL_timer( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   lua_pushnumber(L,a->timer);
   lua_pushnumber(L,a->timer_max);
   return 2;
}

/**
 * @brief Sets the time left on the asteroid.
 *
 *    @luatparam Asteroid a Asteroid to set time left of.
 *    @luatparam number Time left to set to in seconds.
 * @luafunc setTimer
 */
static int asteroidL_setTimer( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   a->timer = luaL_checknumber(L,2);
   a->timer_max = MAX( a->timer_max, a->timer );
   return 0;
}

/**
 * @brief Gets the armour (health) left on the asteroid.
 *
 *    @luatparam Asteroid a Asteroid to get armour of.
 *    @luatreturn number Armour left before the asteroid is destroyed.
 * @luafunc armour
 */
static int asteroidL_armour( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   lua_pushboolean(L,a->armour);
   return 1;
}

/**
 * @brief Sets the armour of the asteroid.
 *
 *    @luatparam Asteroid a Asteroid to set armour of.
 *    @luatparam number Amount to set the armour to (negative values will explode it).
 * @luafunc setTimer
 */
static int asteroidL_setArmour( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   a->armour = luaL_checknumber(L,2);
   if (a->armour <= 0.)
      asteroid_explode( a, 0 );
   return 0;
}

/**
 * @brief Gets the materials the asteroid can potentially drop.
 *
 *    @luatparam Asteroid a Asteroid to get materials of.
 *    @luatreturn table Table containing the materials.
 * @luafunc materials
 */
static int asteroidL_materials( lua_State *L )
{
   Asteroid *a = luaL_validasteroid(L,1);
   const AsteroidType *at = asttype_get( a->type );

   lua_newtable(L);
   for (int i=0; i<array_size(at->material); i++) {
      const AsteroidReward *mat = &at->material[i];

      lua_pushcommodity( L, mat->material );
      lua_setfield(L,-2,"commodity");

      lua_pushnumber( L, mat->quantity );
      lua_setfield(L,-2,"quantity");

      lua_pushnumber( L, mat->rarity );
      lua_setfield(L,-2,"rarity");

      lua_rawseti( L, -2, i+1 );
   }
   lua_setfield(L,-2,"material");

   return 1;
}
