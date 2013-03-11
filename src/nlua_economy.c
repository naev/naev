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

extern double trade_modifier;
extern double production_modifier;

extern void refresh_sys_prices(StarSystem *sys);   /* refresh the prices of a system */
extern void refresh_pl_prices(Planet *pl); /* refresh the prices of a planet */

static int economyL_update( lua_State *L );

static int economyL_getProducing( lua_State *L );
static int economyL_getProdMod( lua_State *L );
static int economyL_setProdMod( lua_State *L );
static int economyL_getCredits( lua_State *L );
static int economyL_setCredits( lua_State *L );
static int economyL_getPrice( lua_State *L );
static int economyL_getStockpile( lua_State *L );
static int economyL_setStockpile( lua_State *L );
static int economyL_getIsActive( lua_State *L );

static const luaL_reg economy_methods[] = {
   { "update", economyL_update },
   { "getProducing", economyL_getProducing },
   { "isActive", economyL_getIsActive },
   { "getProdMod", economyL_getProdMod },
   { "setProdMod", economyL_setProdMod },
   { "getCredits", economyL_getCredits },
   { "setCredits", economyL_setCredits },
   { "getPrice", economyL_getPrice },
   { "getStockpile", economyL_getStockpile },
   { "setStockpile", economyL_setStockpile },
   { "getProdMod", economyL_getProdMod },
   { "setProdMod", economyL_setProdMod },
   {0,0}
}; /**< System metatable methods. */
static const luaL_reg economy_cond_methods[] = {
   { "getCredits", economyL_getCredits },
   { "getPrice", economyL_getPrice },
   { "getStockpile", economyL_getStockpile },
   { "getProdMod", economyL_getProdMod },
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
static int economyL_update( lua_State *L )   //overflows too soon
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
 * @brief gets amount a plant will produce on it's next turn
 *
 * @usage economy.getAmountProducing("Em 1", "Food")
 */
static int economyL_getProducing( lua_State *L )
{
   Planet *pl;
   Commodity *comm;

   if ((comm=luaL_validcommodity(L,2)) == NULL){
      WARN("Invalid commodity\n");
      return 0;
   }
   if ((pl=luaL_validplanet(L,1)) == NULL){
      WARN("Invalid planet\n");
      return 0;
   }
   if (!planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE)){
      WARN("Planet does not participate in the economy!\n");
      return 0;
   }
   lua_pushnumber(L, production( pl->prod_mods[comm->index], pl->stockpiles[comm->index]) );

   return 1;
}

/**
 * @brief gets current production_modifier from either a system or a planet
 *
 * @usage economy.getProdMod("Arcturus Gamma", "Food")
 */
static int economyL_getProdMod( lua_State *L )
{
   Planet *pl;
   Commodity *comm;

   if ((comm=luaL_validcommodity(L,2)) == NULL){
      WARN("Invalid commodity\n");
      return 0;
   }
   if ((pl=luaL_validplanet(L,1)) == NULL){
      WARN("Invalid planet\n");
      return 0;
   }
   if (!planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE)){
      WARN("Planet does not participate in the economy!\n");
      return 0;
   }
   lua_pushnumber(L, pl->prod_mods[comm->index]);

   return 1;
}

/**
 * @brief sets current production_modifier for a planet
 *
 * @usage economy.getProdMod("Arcturus Gamma", "Food", amount)
 */
static int economyL_setProdMod( lua_State *L )
{
   Planet *pl;
   Commodity *comm;
   double amount;

   if (lua_isnumber(L, 3))
      amount = lua_tonumber(L, 3);
   else{
      WARN("Amount to set production modifier to is not a valid number");
      return 0;
   }

   if ((comm=luaL_validcommodity(L,2)) == NULL){
      WARN("Invalid commodity\n");
      return 0;
   }

   if ((pl=luaL_validplanet(L,1)) == NULL){
      WARN("Invalid planet\n");
      return 0;
   }
   if (!planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE)){
      WARN("Planet does not participate in the economy\n");
      return 0;
   }

   pl->prod_mods[comm->index] = amount;

   return 0;
}

/**
 * @brief Gets system or planet credits
 *
 *    @luaparam s System name
 *    @luareturn The credits in the system.
 * @luafunc getCredits( sys )
 */
static int economyL_getCredits( lua_State *L ) 
{
   StarSystem *sys=NULL;
   Planet *pl=NULL;


   if ((pl=luaL_getplanet(L,1)) != NULL){
      if (!planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE)) {
         WARN("Planet is not economically active!\n");
         return 0;
      }
      lua_pushnumber(L, pl->credits);
   }
   else if ((sys=luaL_getsystem(L,1)) != NULL){
      lua_pushnumber(L, sys->credits);
   }
   else {
      WARN("No matching planet or system\n");
      return 0;
   }

   return 1;
}

/**
 * @brief Sets system credits
 *
 *    @luaparam s System name
 *    @luaparam amount credits to set system credits to
 * @luafunc setCredits( sys, amount )
 */
static int economyL_setCredits( lua_State *L )
{
   StarSystem *sys; 
   Planet *pl;
   double amount=-1.0;
   amount = lua_tonumber(L,2);

   if (amount<=0.0){
      WARN("Invalid number of credits\n");
      return 0;
   }

   if ((pl=luaL_getplanet(L,1)) != NULL)
      pl->credits = amount;
   else if ((sys=luaL_getsystem(L,1)) != NULL)
      sys->credits = amount;
   else
      WARN("No matching planet or system\n");
   return 0;
}


/**
 * @brief gets stockpile size in a planet or system
 *
 *    @luaparam s System name
 *    @luaparam commodity commodity name
 * @luafunc getPrice( sys, commodity )
 */
static int economyL_getStockpile( lua_State *L )
{
   StarSystem *sys;
   Planet *pl;
   Commodity *com;
   com = luaL_validcommodity(L,2);

   if (com==NULL){
      WARN("Invalid commodity\n");
      return 0;
   }

   if ((pl=luaL_getplanet(L,1)) != NULL){
      lua_pushnumber(L, pl->stockpiles[com->index]);
   }
   else if ((sys=luaL_getsystem(L,1)) != NULL){
      lua_pushnumber(L, sys->stockpiles[com->index]);
   }
   else {
      WARN("No matching planet or system\n");
      return 0;
   }
   return 1;
}

/**
 * @brief sets stockpile size in a system
 *
 *    @luaparam s System name
 *    @luaparam commodity commodity name
 *    @luaparam amount amount to set the stockpile size to
 * @luafunc setPrice( sys, commodity, amount )
 */
static int economyL_setStockpile( lua_State *L )
{
   StarSystem *sys;
   Planet *pl;
   Commodity *com;
   double amount=-1.0;
   com = luaL_validcommodity(L,2);
   amount = lua_tonumber(L,3);

   if (com==NULL || amount<= 0.0){
      WARN("Provide a valid commodity and amount");
      return 0;
   }

   if ((pl=luaL_getplanet(L,1)) != NULL)
      pl->stockpiles[com->index]=amount;
   else if ((sys=luaL_getsystem(L,1)) != NULL)
      sys->stockpiles[com->index]=amount;
   else 
      WARN("No matching planet or system\n");
   return 0;
}

/**
 * @brief Gets commodity price in a system or planet
 *
 *    @luaparam s System name
 *    @luaparam commodity commodity name
 * @luafunc getPrice( sys, commodity )
 */
static int economyL_getPrice( lua_State *L )
{
   StarSystem *sys;
   Planet *pl;
   Commodity *com;
   
   if ( (com=luaL_validcommodity(L,2)) == NULL ){
      WARN("Invalid commodity or commodity name");
      return 0;
   }

   if ((pl=luaL_getplanet(L,1)) != NULL ){
      if (!planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE)){
         WARN("Planet does not participate in the economy!\n");
         return 0;
      }
      lua_pushnumber(L, PRICE_OF(com, pl));
   }
   else if ((sys=luaL_getsystem(L,1)) != NULL){
      lua_pushnumber(L, PRICE_OF(com, pl));
   }
   else {
      WARN("No matching planet or system\n");
      return 0;
   }
   
   return 1;
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
   Planet *pl;
   if ((pl=luaL_getplanet(L,1)) != NULL ){
      if (planet_isFlag(pl, PL_ECONOMICALLY_ACTIVE))
         lua_pushboolean(L,1);
      else
         lua_pushboolean(L, 0);
   }
   else WARN("is not a valid a planet");
   return 1;
}