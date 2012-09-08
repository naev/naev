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
#include "economy.h"
#include "nlua_time.h"

extern double trade_modifier;
extern double production_modifier;

static int economyL_getProductionmodifier( lua_State *L );
static int economyL_setProductionmodifier( lua_State *L );
static int economyL_update( lua_State *L );
static int economyL_refreshPrices( lua_State *L );
static const luaL_reg economy_methods[] = {
   { "update", economyL_update },
   { "refreshPrices", economyL_refreshPrices },
   { "getProductionmodifier", economyL_getProductionmodifier },
   { "setProductionmodifier", economyL_setProductionmodifier },
   {0,0}
}; /**< System metatable methods. */
static const luaL_reg economy_cond_methods[] = {
   { "update", economyL_update },
   { "refreshPrices", economyL_refreshPrices },
   { "getProductionmodifier", economyL_getProductionmodifier },
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
   ntime_t dt;

   if (lua_isnumber(L,1))
      dt = (ntime_t) lua_tointeger( L, 1 );
   else
      dt = luaL_validtime(L,1);

   if ( dt<10000000 ){
      WARN("\nEconomy will not update for values less than 1 STP");
      return 1;
   }
   economy_update(dt);
   return 0;
}

/**
 * @brief Refreshes the prices of the entire galaxy
 *
 *    @usage economy.refreshPrices()
 * @luafunc refreshPrices( time )
 */
static int economyL_refreshPrices( lua_State *L )
{
   (void) L;
   refresh_prices();
   return 0;
}



/**
 * @brief gets current production_modifier
 *
 * @usage economy.getProductionmodifier()
 */
static int economyL_getProductionmodifier( lua_State *L )
{
   (void) L;
   lua_pushnumber(L, (lua_Number) production_modifier);
   return 0;
}

/**
 * @brief sets trade modifier to value, if value<1
 *
 * @usage economy.getTrademodifier()
 *
 *   @luaparam production_mod value to set trade modifier to
 */
static int economyL_setProductionmodifier( lua_State *L )
{
   production_modifier = lua_tonumber(L,1);
   return 0;
}

