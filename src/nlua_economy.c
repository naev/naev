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
#include "nlua_jump.h"

static int economyL_updatePrices( lua_State *L );
static int economyL_getPrice( lua_State *L );
static int economyL_setPrice( lua_State *L );
static int economyL_unsetPrice( lua_State *L );
static int economyL_isPriceSet( lua_State *L );


static const luaL_reg economy_methods[] = {
   { "updatePrices", economyL_updatePrices },
   { "getPrice", economyL_getPrice },
   { "setPrice", economyL_setPrice },
   { "unsetPrice", economyL_unsetPrice },
   { "isPriceSet", economyL_isPriceSet },
   {0,0}
}; /**< System metatable methods. */
static const luaL_reg economy_cond_methods[] = {
   {0,0}
}; /**< Read only economy metatable methods. */

extern int econ_nprices;
extern StarSystem *systems_stack;

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
 * @brief Manipulates prices in the galaxy
 *
 * If a system doesn't have a price for a good set in the XML, it will default to the 
 *    average price of it's neighbors.
 *
 * After changing the prices of systems, the prices of any systems that do NOT have 
 *    any set prices will be updated after landing, jumping, or econ.resfreshPrices()
 *
 * @code
 * econ.setPrice("Arcturus", "Food", 12000) --sets the price of food at Arcturus to 12000credits/ton
 * print(econ.getPrice("Arcturus", "Food")) --print the price of food at arcturus
 * print(econ.isPriceSet("Arcturus", "Food")) --prints whether the price of food was set; in this case yes
 * econ.unsetPrice("Arcturus", "Food") --unsets the price of food at arcturus
 * econ.updatePrices() --updates all prices, including the price of food at arcturus
 * print(econ.getPrice("Arcturus", "Food")) --print the price of food at arcturus, should no longer be 12000,
 *       --and instead be the average of it's neighbors' food prices
 * econ.update
 * @endcode
 *
 * @luamod econ
 */


/**
 * @brief refreshes the prices of any systems with unset prices, if the prices need refreshing. 
 *    Automatically done when jumping, landing, or taking off
 *
 * @usage economy.updatePrices()
 */
static int economyL_updatePrices( lua_State *L ){
   (void) L;
   econ_updateprices();
   return 0;
}


/**
 * @brief get the real price for a good in a system
 *
 *    @luaparam good the good/commodity
 *    @luaparam system the system
 *    @luareturn the real price of the good in the system
 * @usage economy.getPrice("Doranthex", "Food")
 */
static int economyL_getPrice( lua_State *L )
{
   StarSystem *sys;
   Commodity *comm;

   comm = luaL_validcommodity(L, 1);
   sys = luaL_validsystem(L, 2);

   if (comm==NULL){
      WARN("Invalid commodity for arg 1");
      return 0;
   }
   if (sys==NULL){
      WARN("Invalid system for arg 2");
      return 0;
   }

   lua_pushnumber( L, (lua_Number) sys->prices[comm->index] * comm->price);
   return 1;
}

/**
 * @brief set the price of a good at a system
 *
 *    @luaparam good the good/commodity
 *    @luaparam system the systemy
 *    @luaparam price the price in credits/ton
 *
 * @usage econ.setPrice("Arcturus", "Food", 12000)
 */
static int economyL_setPrice( lua_State *L ){
   StarSystem *sys;
   Commodity *comm;
   float price=-1.0;

   comm = luaL_validcommodity(L, 1);
   sys = luaL_validsystem(L, 2);
   price = (float) lua_tonumber(L, 3);

   if (comm==NULL){
      WARN("Invalid commodity for arg 1");
      return 0;
   }
   if (sys==NULL){
      WARN("Invalid system for arg 2");
      return 0;
   }
   if (price<=0.0){
      WARN("Invalid price for arg 3");
      return 0;
   }

   sys->prices[comm->index]=price/comm->price;
   sys->is_priceset[comm->index]=1;
   comm->changed=1;

   return 0;
}

/**
 * @brief unsets a price in a system. The new price in the will be the average price of it's neighbors.
 *    The price will change after jumping, landing/takeoff, and econ.updatePrices()
 *
 *    @luaparam good the good/commodity
 *    @luaparam system the system
 *
 * @usage econ.unsetPrice("Arcturus", "Food") --unsets the price of food in arcturus
 */
static int economyL_unsetPrice( lua_State *L ){
   StarSystem *sys;
   Commodity *comm;

   comm = luaL_validcommodity(L, 1);
   sys = luaL_validsystem(L, 2);

   if (comm==NULL){
      WARN("Invalid commodity for arg 1");
      return 0;
   }
   if (sys==NULL){
      WARN("Invalid system for arg 2");
      return 0;
   }

   sys->is_priceset[comm->index]=0;
   comm->changed=1;

   return 0;
}


static int economyL_isPriceSet( lua_State *L ){
   StarSystem *sys;
   Commodity *comm;

   comm = luaL_validcommodity(L, 1);
   sys = luaL_validsystem(L, 2);

   if (comm==NULL){
      WARN("Invalid commodity for arg 1");
      return 0;
   }
   if (sys==NULL){
      WARN("Invalid system for arg 2");
      return 0;
   }

   if (sys->is_priceset[comm->index])
      lua_pushboolean(L, 1);
   else
      lua_pushboolean(L, 0);

   return 1;
}




