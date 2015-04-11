/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
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
 * @file nlua_commodity.c
 *
 * @brief Handles the Lua commodity bindings.
 */


#include "nlua_commodity.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "nlua_planet.h"
#include "log.h"
#include "rng.h"


/* Commodity metatable methods. */
static int commodityL_eq( lua_State *L );
static int commodityL_get( lua_State *L );
static int commodityL_name( lua_State *L );
static int commodityL_price( lua_State *L );
static int commodityL_priceAt( lua_State *L );
static const luaL_reg commodityL_methods[] = {
   { "__tostring", commodityL_name },
   { "__eq", commodityL_eq },
   { "get", commodityL_get },
   { "name", commodityL_name },
   { "price", commodityL_price },
   { "priceAt", commodityL_priceAt },
   {0,0}
}; /**< Commodity metatable methods. */



/**
 * @brief Loads the commodity library.
 *
 *    @param L State to load commodity library into.
 *    @return 0 on success.
 */
int nlua_loadCommodity( lua_State *L, int readonly )
{
   (void) readonly; /* Everything is readonly. */

   /* Create the metatable */
   luaL_newmetatable(L, COMMODITY_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, commodityL_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, COMMODITY_METATABLE);

   return 0;
}


/**
 * @brief Lua bindings to interact with commodities.
 *
 * This will allow you to create and manipulate commodities in-game.
 *
 * An example would be:
 * @code
 * c = commodity.get( "Food" ) -- Gets the commodity by name
 * if c:price() > 500 then
 *    -- Do something with high price
 * end
 * @endcode
 *
 * @luamod commodity
 */
/**
 * @brief Gets commodity at index.
 *
 *    @param L Lua state to get commodity from.
 *    @param ind Index position to find the commodity.
 *    @return Commodity found at the index in the state.
 */
LuaCommodity* lua_tocommodity( lua_State *L, int ind )
{
   return (LuaCommodity*) lua_touserdata(L,ind);
}
/**
 * @brief Gets commodity at index or raises error if there is no commodity at index.
 *
 *    @param L Lua state to get commodity from.
 *    @param ind Index position to find commodity.
 *    @return Commodity found at the index in the state.
 */
LuaCommodity* luaL_checkcommodity( lua_State *L, int ind )
{
   if (lua_iscommodity(L,ind))
      return lua_tocommodity(L,ind);
   luaL_typerror(L, ind, COMMODITY_METATABLE);
   return NULL;
}
/**
 * @brief Makes sure the commodity is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the commodity to validate.
 *    @return The commodity (doesn't return if fails - raises Lua error ).
 */
Commodity* luaL_validcommodity( lua_State *L, int ind )
{
   LuaCommodity *lo;
   Commodity *o;

   if (lua_iscommodity(L, ind)) {
      lo = luaL_checkcommodity(L, ind);
      o  = lo->commodity;
   }
   else if (lua_isstring(L, ind))
      o = commodity_get( lua_tostring(L, ind) );
   else {
      luaL_typerror(L, ind, COMMODITY_METATABLE);
      return NULL;
   }

   if (o == NULL)
      NLUA_ERROR(L, "Commodity is invalid.");

   return o;
}
/**
 * @brief Pushes a commodity on the stack.
 *
 *    @param L Lua state to push commodity into.
 *    @param commodity Commodity to push.
 *    @return Newly pushed commodity.
 */
LuaCommodity* lua_pushcommodity( lua_State *L, LuaCommodity commodity )
{
   LuaCommodity *o;
   o = (LuaCommodity*) lua_newuserdata(L, sizeof(LuaCommodity));
   *o = commodity;
   luaL_getmetatable(L, COMMODITY_METATABLE);
   lua_setmetatable(L, -2);
   return o;
}
/**
 * @brief Checks to see if ind is a commodity.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a commodity.
 */
int lua_iscommodity( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, COMMODITY_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Checks to see if two commodities are the same.
 *
 * @usage if o1 == o2 then -- Checks to see if commodity o1 and o2 are the same
 *
 *    @luaparam o1 First commodity to compare.
 *    @luaparam o2 Second commodity to compare.
 *    @luareturn true if both commodities are the same.
 * @luafunc __eq( o1, o2 )
 */
static int commodityL_eq( lua_State *L )
{
   LuaCommodity *a, *b;
   a = luaL_checkcommodity(L,1);
   b = luaL_checkcommodity(L,2);
   if (a->commodity == b->commodity)
      lua_pushboolean(L,1);
   else
      lua_pushboolean(L,0);
   return 1;
}




/**
 * @brief Gets a commodity.
 *
 * @usage s = commodity.get( "Hyena" ) -- Gets the hyena
 *
 *    @luaparam s Name of the commodity to get.
 *    @luareturn The commodity matching name or nil if error.
 * @luafunc get( s )
 */
static int commodityL_get( lua_State *L )
{
   const char *name;
   LuaCommodity lo;

   /* Handle parameters. */
   name = luaL_checkstring(L,1);

   /* Get commodity. */
   lo.commodity = commodity_get( name );
   if (lo.commodity == NULL) {
      NLUA_ERROR(L,"Commodity '%s' not found!", name);
      return 0;
   }

   /* Push. */
   lua_pushcommodity(L, lo);
   return 1;
}
/**
 * @brief Gets the name of the commodity's commodity.
 *
 * @usage commodityname = s:name()
 *
 *    @luaparam s Commodity to get commodity name.
 *    @luareturn The name of the commodity's commodity.
 * @luafunc name( s )
 */
static int commodityL_name( lua_State *L )
{
   Commodity *c;

   /* Get the commodity. */
   c  = luaL_validcommodity(L,1);

   /** Return the commodity name. */
   lua_pushstring(L, c->name);
   return 1;
}


/**
 * @brief Gets the base price of an commodity.
 *
 * @usage print( o:price() ) -- Prints the base price of the commodity
 *
 *    @luaparam o Commodity to get information of.
 *    @luareturn The base price of the commodity.
 * @luafunc price( o )
 */
static int commodityL_price( lua_State *L )
{
   Commodity *c = luaL_validcommodity(L,1);
   lua_pushnumber(L, c->price);
   return 1;
}


/**
 * @brief Gets the base price of an commodity at a certain planet.
 *
 * @usage if o:priceAt( planet.get("Polaris Prime") ) > 100 then -- Checks price of an outfit at polaris prime
 *
 *    @luaparam o Commodity to get information of.
 *    @luaparam p Planet to get price at.
 *    @luareturn The price of the commodity at the planet.
 * @luafunc priceAt( o, p )
 */
static int commodityL_priceAt( lua_State *L )
{
   Commodity *c;
   Planet *p;
   StarSystem *sys;
   char *sysname;

   c = luaL_validcommodity(L,1);
   p = luaL_validplanet(L,2);
   sysname = planet_getSystem( p->name );
   if (sysname == NULL) {
      NLUA_ERROR( L, "Planet '%s' does not belong to a system", p->name );
      return 0;
   }
   sys = system_get( sysname );
   if (sys == NULL) {
      NLUA_ERROR( L, "Planet '%s' can not find its system '%s'", p->name, sysname );
      return 0;
   }

   lua_pushnumber( L, planet_commodityPrice( p, c ) );
   return 1;
}



