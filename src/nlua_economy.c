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

static int economyL_updateSolutions( lua_State *L );
static int economyL_updatePrices( lua_State *L );
static int economyL_getPrice( lua_State *L );
static int economyL_getPreferredPrice( lua_State *L);
static int economyL_setPreferredPrice( lua_State *L);
static int economyL_getSysWeight( lua_State *L );
static int economyL_setSysWeight( lua_State *L );
static int economyL_getJmpWeight( lua_State *L );
static int economyL_setJmpWeight( lua_State *L );


static const luaL_reg economy_methods[] = {
   { "updateSolutions", economyL_updateSolutions },
   { "updatePrices", economyL_updatePrices },
   { "getPreferredPrice", economyL_getPreferredPrice },
   { "setPreferredPrice", economyL_setPreferredPrice },
   { "getPrice", economyL_getPrice },
   { "getSysWeight", economyL_getSysWeight },
   { "setSysWeight", economyL_setSysWeight },
   { "getJmpWeight", economyL_getJmpWeight },
   { "setJmpWeight", economyL_setJmpWeight },
   {0,0}
}; /**< System metatable methods. */
static const luaL_reg economy_cond_methods[] = {
   { "getPreferredPrice", economyL_getPreferredPrice },
   { "getSysWeight", economyL_getSysWeight },
   { "getJmpWeight", economyL_getJmpWeight },
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
 * Prices are averages of all systems with defined prices; every system's 
 *    prices are the price of it's neighbor's and it's own price (if it has one)
 *
 * The more weighted a system it is, the more it will prefer it's own prices. 
 *    a system without any weight is the average of it's neighbor's prices, and
 *    a system with infinite weight simply has it's own preferred price.
 *
 * In addition, a jump can be weighted, so that that the price difference of the 
 *    two relevant systems is increased.
 *
 * After updating the prices of systems, you must call economy.updatePrices()
 *
 * After updating weights, you must cal economy.updateSolutions(), and then 
 *    economy.updatePrice()
 *
 * hookid = hook.enter( "penter", 5 )
 * @endcode
 *
 * @luamod hook
 */


/**
 * @brief refreshes the prices based on the solutions matrix. Use this after updating the solutions matrix or changing prices. Inexpensive
 *
 * @usage economy.updatePrices()
 */
static int economyL_updatePrices( lua_State *L ){
   (void) L;
   econ_updateprices();
   return 0;
}


/**
 * @brief regenerate the solutions matrix. Call this after changing any weights. Relatively expensive
 *
 * @usage economy.updateSolutions()
 */
static int economyL_updateSolutions( lua_State *L ){
   (void) L;
   econ_refreshsolutions();  
   return 0;
}


/**
 * @brief get the preferred price for a system (if it has one)
 *
 *    @luaparam system the system/name of system to get the preferred price of
 *    @luaparam commodity the commodity/name of commodity to get the preferred price of 
 *    @luareturn the preferred price for a system, not accounting for the commodity base price, if it has one, otherwise return false
 * @usage economy.getPreferredPrice( "Doranthex", "Food" )
 */
static int economyL_getPreferredPrice( lua_State *L)
{
   StarSystem *sys;
   Commodity *comm;

   sys = luaL_validsystem(L, 1);
   comm = luaL_validcommodity(L, 2);

   if (sys==NULL || comm==NULL){
      WARN("Invalid argument[s] to economy.getPreferredPrice()");
      return 0;
   }

   if (sys->given_prices==NULL){
      lua_pushnumber(L, 0);
      return 1;
   }

   lua_pushnumber( L, (lua_Number) sys->given_prices[comm->index] );
   return 1;
}

/**
 * @brief set the preferred price for a good in a system. If system weight is zero, will have no effect. Call economy.updatePrices() to update real prices afterwords. Default preferred good price is 0
 *
 *    @luaparam system the system/name of system to set the preferred price of
 *    @luaparam commodity the commodity/name of commodity to set the preferred price of
 *    @luaparam new the new preferred price of the system, not accounting for the commodity base price
 * @usage economy.setPreferredPrice( "Doranthex", "Food", 120 )
 */
static int economyL_setPreferredPrice( lua_State *L)
{
   StarSystem *sys;
   Commodity *comm;
   lua_Number num;

   sys = luaL_validsystem(L, 1);
   comm = luaL_validcommodity(L, 2);
   num = lua_tonumber(L, 3);

   if (sys==NULL || comm==NULL || !lua_isnumber(L, 3)){
      WARN("Invalid argument[s] to economy.setPreferredPrice()");
      return 0;
   }

   if (sys->given_prices==NULL)
      sys->given_prices = (float *) calloc(sizeof(float), econ_nprices);

   sys->given_prices[comm->index] = (float) num;

   return 0;
}


/**
 * @brief get the real price for a good in a system
 *
 *    @luaparam system the system
 *    @luaparam good the good
 *    @luareturn the real price of the good in the system
 * @usage economy.getPrice("Doranthex", "Food")
 */
static int economyL_getPrice( lua_State *L )
{
   StarSystem *sys;
   Commodity *comm;

   sys = luaL_validsystem(L, 1);
   comm = luaL_validcommodity(L, 2);

   if (sys==NULL || comm==NULL){
      WARN("Invalid argument[s] to economy.getPreferredPrice()");
      return 0;
   }

   lua_pushnumber( L, (lua_Number) sys->real_prices[comm->index] * comm->price);
   return 1;
}

/**
 * @brief get the weight for a system
 *
 *    @luaparam system the system/name of system to get the weight of
 *    @luareturn the weight of the system
 * @usage economy.getSysWeight( "Doranthex" )
 */
static int economyL_getSysWeight( lua_State *L )
{
   StarSystem *sys;

   sys = luaL_validsystem(L, 1);

   if (sys==NULL){
      WARN("Invalid argument[s] to economy.getSysWeight()");
      return 0;
   }

   lua_pushnumber( L, (lua_Number) sys->weight );
   return 1;
}

/**
 * @brief set the weight for a system. After, call economy.updateSolutions to update the solutions matrix and economy.updatePrices() to update real prices
 *
 *    @luaparam system the system/name of system to set the weight of
 *    @luaparam new the new weight of the system
 * @usage economy.setSysPrice( "Doranthex", 2.0 )
 */
static int economyL_setSysWeight( lua_State *L )
{
   StarSystem *sys;
   lua_Number num;

   sys = luaL_validsystem(L, 1);
   num = lua_tonumber(L, 2);

   if (sys==NULL || !lua_isnumber(L, 2)){
      WARN("Invalid argument[s] to economy.setSysWeight()");
      return 0;
   }

   sys->weight = (float) num;

   return 0;
}


/**
 * @brief get the weight for a jump. By default, the weight is 1
 *
 *    @luaparam jump the jump to get the weight of
 *    @luareturn the weight of the jump
 * @usage economy.getJmpWeight( jump.get( "Ogat", "Goddard" ) )
 */
static int economyL_getJmpWeight( lua_State *L )
{
   LuaJump *Ljmp;
   StarSystem *sys1;
   StarSystem *sys2;

   if (luaL_checkjump(L,1))
      Ljmp = lua_tojump(L, 1);
   else {
      WARN("Invalid argument[s] to economy.getJmpWeight()");
      return 0;
   }
   sys1 = systems_stack+Ljmp->srcid;
   sys2 = systems_stack+Ljmp->destid;

   int j;
   for (j=0; j<sys1->njumps; j++)
      if (sys1->jumps[j].target == sys2){
         lua_pushnumber(L, (lua_Number) sys1->jumps[j].trade_weight ); 
         break;}

   return 1;
}


/**
 * @brief set the weight for a jump. After, call economy.updateSolutions() and economy.updatePrices()
 *
 *    @luaparam jump the jump to set the weight of
 *    @luaparam new the new weight of the jump
 * @usage economy.setJmpWeight( jump.get( "Ogat", "Goddard" ), 0.1 )
 */
static int economyL_setJmpWeight( lua_State *L )
{
   LuaJump *Ljmp=NULL;
   StarSystem *sys1;
   StarSystem *sys2;
   lua_Number num;

   if (luaL_checkjump(L,1))
      Ljmp = lua_tojump(L, 1);
   num = lua_tonumber(L, 2);

   if (Ljmp==NULL || !lua_isnumber(L, 2)){
      WARN("Invalid argument[s] to economy.setJmpWeight()");
      return 0;
   }
   sys1 = systems_stack+Ljmp->srcid;
   sys2 = systems_stack+Ljmp->destid;

   int j;
   for (j=0; j<sys1->njumps; j++)
      if (sys1->jumps[j].target == sys2){
         sys1->jumps[j].trade_weight=num;
         break;}
   for (j=0; j<sys2->njumps; j++)   /* update both jumps */
      if (sys2->jumps[j].target == sys1){
         sys2->jumps[j].trade_weight=num;
         break;}

   return 0;
}


