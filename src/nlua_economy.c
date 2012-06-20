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

extern double trade_modifier;
extern double production_modifier;

static int economyL_getTrademodifier( lua_State *L );
static int economyL_setTrademodifier( lua_State *L );
static int economyL_getProductionmodifier( lua_State *L );
static int economyL_setProductionmodifier( lua_State *L );
static int economyL_update( lua_State *L );
static int economyL_refreshPrices( lua_State *L );
static const luaL_reg economy_methods[] = {
   { "update", economyL_update },
   { "getTrademodifier", economyL_getTrademodifier },
   { "setTrademodifier", economyL_setTrademodifier },
   { "refreshPrices", economyL_refreshPrices },
   { "getProductionmodifier", economyL_getProductionmodifier },
   { "setProductionmodifier", economyL_setProductionmodifier },
   {0,0}
}; /**< System metatable methods. */
static const luaL_reg economy_cond_methods[] = {
   { "update", economyL_update },
   { "getTrademodifier", economyL_getTrademodifier },
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
 * @brief gets current trade_modifier
 *
 * @usage economy.getTrademodifier()
 */
static int economyL_getTrademodifier( lua_State *L )
{
   lua_pushnumber(L, (lua_Number) trade_modifier);
   return 0;
}

/**
 * @brief sets trade modifier to value, if value<1
 *
 * @usage economy.getTrademodifier()
 *
 *   @luaparam trade_mod value to set trade modifier to
 *
 * @luafunc getTrademodifier()
 */
static int economyL_setTrademodifier( lua_State *L )
{
   double trade_mod;
   trade_mod = lua_tonumber(L,1);
   if (trade_mod>=1.){
      WARN("trade_modifier should not be set to a value>=1");
      return 0;
   }
   trade_modifier = trade_mod;
   return 0;
}


/**
 * @brief Updates economy by a certain amount of time, in STP
 *
 * @usage economy.update(10000000)
 *
 *    @luaparam time length of time to update
 * @luafunc update( time )
 */
static int economyL_update( lua_State *L )
{
   unsigned int dt;
   dt = (unsigned int) lua_tointeger( L, 1 );

   if ( dt<10000000 ){
      WARN("\nInvalid argument to economy.update, must be integer>=10000000");
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

