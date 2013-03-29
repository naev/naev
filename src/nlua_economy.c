/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_economy.c
 *
 * @brief Lua economy module.
 */

#include "nlua_economy.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "nlua_time.h"
#include "space.h"
#include "economy.h"
#include "nlua_planet.h"
#include "nlua_system.h"
#include "nlua_commodity.h"

static int economyL_update( lua_State *L );
static int economyL_getIsActive( lua_State *L );

static const luaL_reg economy_methods[] = {
   { "update", economyL_update },
   { "isActive", economyL_getIsActive },
   {0,0}
}; /**< System metatable methods. */
static const luaL_reg economy_cond_methods[] = {
   {0,0}
}; /**< Read only economy metatable methods. */

/**
 * @brief Loads the economy library.
 *
 *    @param L State to load economy library into.
 *    @param readonly Load read only functions?
 *    @return 0 on success.
 */
int nlua_loadEconomy( lua_State *L, int readonly )
{

   /* Create the metatable */
   luaL_newmetatable(L, ECONOMY_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   if (readonly)
      luaL_register(L, NULL, economy_cond_methods);
   else
      luaL_register(L, NULL, economy_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, ECONOMY_METATABLE);

   return 0; /* No error */
}

/**
 * @brief Updates economy by a certain amount of time, in STP
 *
 * @usage economy.update(time.create(0,1,0))
 *
 *    @luaparam time length of time to update
 * @luafunc update( time )
 */
static int economyL_update( lua_State *L )
{
   (void) L;
   /* dummy for now */

   return 0;
}

/**
 * @brief check if a planet participates in the economy
 *
 *    @luaparam planet or planet name
 *  
 * @luareturn bool
 */
static int economyL_getIsActive( lua_State *L )
{
   /* dummy for now */
   (void) L;

   return 0;
}