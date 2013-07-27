/*
 * See Licensing and Copyright notice in naev.h
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
static int commodityL_getprice( lua_State *L );
static int commodityL_setprice( lua_State *L );
static const luaL_reg commodityL_methods[] = {
   { "__tostring", commodityL_name },
   { "__eq", commodityL_eq },
   { "get", commodityL_get },
   { "name", commodityL_name },
   { "getprice", commodityL_getprice },
   { "setprice", commodityL_setprice },
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
 * @usage print( o:getprice() ) -- Prints the base price of the commodity
 *
 *    @luaparam c Commodity to get information of.
 *    @luareturn The base price of the commodity.
 * @luafunc getprice( o )
 */
static int commodityL_getprice( lua_State *L )
{
   Commodity *c = luaL_validcommodity(L,1);
   lua_pushnumber(L, c->price);
   return 1;
}


/**
 * @brief Sets the base price of an commodity.
 *
 * @usage o:setprice(1.5) -- sets the base price of the good
 *
 *    @luaparam c Commodity to get information of.
 *    @luaparam new_base The new base price of the commodity.
 * @luafunc setprice( o )
 */
static int commodityL_setprice( lua_State *L )
{
   Commodity *c;
   float price=-1.0;

   c = luaL_validcommodity(L,1);
   price = lua_tonumber(L, 2);


   if (c==NULL){
      WARN("Invalid commodity for argument 1\n");
      return 0;
   }
   if (price<=0.0){
      WARN("Invalid base price for argument 2\n");
      return 0;
   }

   c->price = price;

   return 0;
}

