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
extern void refresh_sys_prodmods(StarSystem *sys);

static int economyL_getProdMod( lua_State *L );
static int economyL_setProdMod( lua_State *L );
static int economyL_update( lua_State *L );
static int economyL_refreshValues( lua_State *L );
static const luaL_reg economy_methods[] = {
   { "update", economyL_update },
   { "refreshValues", economyL_refreshValues },
   { "getProdMod", economyL_getProdMod },
   { "setProdMod", economyL_setProdMod },
   {0,0}
}; /**< System metatable methods. */
static const luaL_reg economy_cond_methods[] = {
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
static int economyL_refreshValues( lua_State *L )
{
   (void) L;
   refresh_prices();
   return 0;
}



/**
 * @brief gets current production_modifier from either a system or a planet
 *
 * @usage economy.getProdMod("Arcturus Gamma", "Food")
 */
static int economyL_getProdMod( lua_State *L )
{

   const char *pl_name;
   const char *comm_name;
   Planet *pl;
   StarSystem *sys;
   Commodity *comm;
   int comm_ind;

   if (lua_isstring(L, 2)){

      comm_name = lua_tolstring(L, 2, NULL);
      comm = commodity_get(comm_name);

      if (comm==NULL){
         WARN("Commodity %s could not be found");
         return 0;
      }

      comm_ind = comm->index;
   }
   else if (lua_iscommodity(L, 2))
      comm = luaL_validcommodity(L, 2);
   else {
      WARN("Argument 2 is not a commodity");
      return 0;
   }


   if (lua_isstring(L, 1)){

      pl_name = lua_tolstring(L, 1, NULL);

      if (planet_exists(pl_name))
         pl = planet_get(pl_name);
      else if (system_exists(pl_name))
         sys = system_get(pl_name);
      else{
         WARN("planet or system %s does not exist", pl_name);
         return 0;
      }
   }
   else if (lua_isplanet(L, 1))
      pl = luaL_validplanet(L, 1);
   else if (lua_issystem(L, 1))
      sys = luaL_validsystem(L, 1);
   else {
      WARN("Argument 1 is not a planet or system name");
      return 0;
   }

   if (pl){
      lua_pushnumber(L, (lua_Number) pl->prod_mods[comm_ind]);}
   else if (sys)
      lua_pushnumber(L, (lua_Number) sys->prod_mods[comm_ind]);

   return 1;
}

/**
 * @brief sets current production_modifier for a planet
 *
 * @usage economy.getProdMod("Arcturus Gamma", "Food", amount)
 */
static int economyL_setProdMod( lua_State *L )
{

   const char *pl_name;
   const char *comm_name;
   Planet *pl;
   Commodity *comm;
   int comm_ind;
   double amount;

   if (lua_isnumber(L, 3))
      amount = lua_tonumber(L, 3);
   else{
      WARN("Amount to set production modifier to is not a valid number");
      return 0;
   }

   if (lua_isstring(L, 2)){

      comm_name = lua_tolstring(L, 2, NULL);
      comm = commodity_get(comm_name);

      if (comm==NULL){
         WARN("Commodity %s could not be found");
         return 0;
      }

      comm_ind = comm->index;
   }
   else if (lua_iscommodity(L, 2))
      comm = luaL_validcommodity(L, 2);
   else {
      WARN("Argument 2 is not a commodity");
      return 0;
   }


   if (lua_isstring(L, 1)){

      pl_name = lua_tolstring(L, 1, NULL);

      if (planet_exists(pl_name))
         pl = planet_get(pl_name);
      else{
         WARN("planet %s does not exist", pl_name);
         return 0;
      }
   }
   else if (lua_isplanet(L, 1))
      pl = luaL_validplanet(L, 1);
   else {
      WARN("Argument 1 is not a planet name");
      return 0;
   }

   pl->prod_mods[comm_ind] = amount;

   char *sys_name = planet_getSystem(pl->name);
   refresh_sys_prodmods( system_get(sys_name) );

   return 1;
}

