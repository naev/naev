/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
static int economyL_setSystemPrice( lua_State *L );
static int economyL_unsetSystemPrice( lua_State *L );
static int economyL_isSystemPriceSet( lua_State *L );
static int economyL_setPlanetPrice( lua_State *L );
static int economyL_unsetPlanetPrice( lua_State *L );
static int economyL_isPlanetPriceSet( lua_State *L );


static const luaL_reg economy_methods[] = {
   { "updatePrices", economyL_updatePrices },
   { "getPrice", economyL_getPrice },
   { "setSystemPrice", economyL_setSystemPrice },
   { "unsetSystemPrice", economyL_unsetSystemPrice },
   { "isSystemPriceSet", economyL_isSystemPriceSet },
   { "setPlanetPrice", economyL_setPlanetPrice },
   { "unsetPlanetPrice", economyL_unsetPlanetPrice },
   { "isPlanetPriceSet", economyL_isPlanetPriceSet },
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
 * econ.setSystemPrice( "Food", system.get("Arcturus"), 12000 ) --sets the price of food at Arcturus to 12000credits/ton
 * print(econ.getSystemPrice( "Food", system.get("Arcturus") )) --print the price of food at arcturus
 * print(econ.isSystemPriceSet( "Food", system.get("Arcturus") )) --prints whether the price of food was set; in this case yes
 * econ.unsetSystemPrice( "Food", system.get("Arcturus") ) --unsets the price of food at arcturus
 * econ.updatePrices() --updates all prices, including the price of food at arcturus
 * print(econ.getSystemPrice( "Food", system.get("Arcturus") )) --print the price of food at arcturus, should no longer be 12000,
 *       --and instead be the average of it's neighbors' food prices
 * @endcode
 *
 * @luamod econ
 */


/**
 * @brief refreshes the prices of any systems with unset prices, if the prices need refreshing. 
 *    Automatically done when jumping, landing, or taking off.
 *
 * @usage econ.updatePrices()
 */
static int economyL_updatePrices( lua_State *L ){
   (void) L;
   econ_updateprices();
   return 0;
}


/**
 * @brief Returns the real price for a good in a system.
 *
 *    @luaparam c Commodity to get the price of.
 *    @luaparam planet Planet to get the price on.
 *    @luareturn The real price of the commodity on the planet.
 * @usage econ.getPrice( "Ore", planet.cur() ) -- Gets the price of ore on the current planet
 */
static int economyL_getPrice( lua_State *L )
{
   Planet *p;
   StarSystem *sys;
   Commodity *comm;
   comm = luaL_validcommodity(L, 1);
   p = luaL_validplanet(L, 2);
   sys = system_get( planet_getSystem( p->name ) );
   if (comm == NULL)
      return 0;
   if (p == NULL)
      return 0;
   if (p->is_priceset[comm->index])
      lua_pushnumber( L, (lua_Number) p->prices[comm->index] * comm->price);
   else
      lua_pushnumber( L, (lua_Number) sys->prices[comm->index] * comm->price);
   return 1;
}


/**
 * @brief Sets the price of a commodity in a system.
 *
 *    @luaparam c Commodity to set the price of.
 *    @luaparam system System to set the price on.
 *    @luaparam price New price in credits.
 *
 * @usage econ.setSystemPrice( "Food", system.cur(), 12000 ) -- Sets the price of food in the current system to 12000
 */
static int economyL_setSystemPrice( lua_State *L ){
   StarSystem *sys;
   Commodity *comm;
   float price = -1.0;
   comm = luaL_validcommodity(L, 1);
   sys = luaL_validsystem(L, 2);
   price = (float) lua_tonumber(L, 3);
   if (comm == NULL)
      return 0;
   if (sys == NULL)
      return 0;
   if (price <= 0.0)
      return 0;
   sys->prices[comm->index] = price / comm->price;
   sys->is_priceset[comm->index] = 1;
   comm->changed = 1;
   return 0;
}


/**
 * @brief Unsets a price in a system. The new price in the will be the average price of it's neighbors.
 *    The price will change after jumping, landing/takeoff, and econ.updatePrices()
 *
 *    @luaparam c Commodity to unset the price of.
 *    @luaparam system System to unset the price in.
 *
 * @usage econ.unsetSystemPrice( "Food", system.cur() ) -- Unsets the price of food in the current system
 */
static int economyL_unsetSystemPrice( lua_State *L ){
   StarSystem *sys;
   Commodity *comm;
   comm = luaL_validcommodity(L, 1);
   sys = luaL_validsystem(L, 2);
   if (comm == NULL)
      return 0;
   if (sys == NULL)
      return 0;
   sys->is_priceset[comm->index] = 0;
   comm->changed = 1;
   return 0;
}


/**
 * @brief Checks to see if the price of a commodity has been set in a system.
 *
 *    @luaparam c Commodity to check for the price being set.
 *    @luaparam system System to check for the price being set.
 *    @luareturn true if the price of the commodity has been set on the system.
 *
 * @usage econ.isSystemPriceSet( "Food", system.cur() ) -- Checks whether the price of food in the current system has been set
 */
static int economyL_isSystemPriceSet( lua_State *L ){
   StarSystem *sys;
   Commodity *comm;
   comm = luaL_validcommodity(L, 1);
   sys = luaL_validsystem(L, 2);
   if (comm == NULL)
      return 0;
   if (sys == NULL)
      return 0;
   if (sys->is_priceset[comm->index])
      lua_pushboolean(L, 1);
   else
      lua_pushboolean(L, 0);
   return 1;
}


/**
 * @brief Sets the price of a commodity on a planet.
 *
 *    @luaparam c Commodity to set the price of.
 *    @luaparam p Planet to set the price on.
 *    @luaparam price New price in credits.
 *
 * @usage econ.setPlanetPrice( "Food", planet.cur(), 12000 ) -- Sets the price of food on the current planet to 12000
 */
static int economyL_setPlanetPrice( lua_State *L ){
   Planet *p;
   Commodity *comm;
   float price = -1.0;
   comm = luaL_validcommodity(L, 1);
   p = luaL_validplanet(L, 2);
   price = (float) lua_tonumber(L, 3);
   if (comm == NULL)
      return 0;
   if (p == NULL)
      return 0;
   if (price <= 0.0)
      return 0;
   p->prices[comm->index] = price / comm->price;
   p->is_priceset[comm->index] = 1;
   comm->changed = 1;
   return 0;
}


/**
 * @brief unsets a price in a system. The new price in the will be the average price of it's neighbors.
 *    The price will change after jumping, landing/takeoff, and econ.updatePrices()
 *
 *    @luaparam c Commodity to unset the price of.
 *    @luaparam p Planet to unset the price on.
 *
 * @usage econ.unsetPlanetPrice( "Food", planet.cur() ) -- Unsets the price of food on the current planet
 */
static int economyL_unsetPlanetPrice( lua_State *L ){
   Planet *p;
   Commodity *comm;
   comm = luaL_validcommodity(L, 1);
   p = luaL_validplanet(L, 2);
   if (comm == NULL)
      return 0;
   if (p == NULL)
      return 0;
   p->is_priceset[comm->index] = 0;
   comm->changed = 1;
   return 0;
}


/**
 * @brief Checks to see if the price of a commodity has been set on a planet.
 *
 *    @luaparam c Commodity to check for the price being set.
 *    @luaparam p Planet to check for the price being set.
 *    @luareturn true if the price of the commodity has been set on the planet.
 *
 * @usage econ.isPlanetPriceSet( "Food", planet.cur() ) -- Checks whether the price of food in the current planet has been set
 */
static int economyL_isPlanetPriceSet( lua_State *L ){
   Planet *p;
   Commodity *comm;
   comm = luaL_validcommodity(L, 1);
   p = luaL_validplanet(L, 2);
   if (comm == NULL)
      return 0;
   if (p == NULL)
      return 0;
   if (p->is_priceset[comm->index])
      lua_pushboolean(L, 1);
   else
      lua_pushboolean(L, 0);
   return 1;
}

