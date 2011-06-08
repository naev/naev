/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_outfit.c
 *
 * @brief Handles the Lua outfit bindings.
 */

#include "nlua_outfit.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "nlua_tex.h"
#include "log.h"
#include "rng.h"


/* Outfit metatable methods. */
static int outfitL_eq( lua_State *L );
static int outfitL_get( lua_State *L );
static int outfitL_name( lua_State *L );
static int outfitL_type( lua_State *L );
static int outfitL_typeBroad( lua_State *L );
static int outfitL_cpu( lua_State *L );
static int outfitL_slot( lua_State *L );
static const luaL_reg outfitL_methods[] = {
   { "__tostring", outfitL_name },
   { "__eq", outfitL_eq },
   { "get", outfitL_get },
   { "name", outfitL_name },
   { "type", outfitL_type },
   { "typeBroad", outfitL_typeBroad },
   { "cpu", outfitL_cpu },
   { "slot", outfitL_slot },
   {0,0}
}; /**< Outfit metatable methods. */



/**
 * @brief Loads the outfit library.
 *
 *    @param L State to load outfit library into.
 *    @return 0 on success.
 */
int nlua_loadOutfit( lua_State *L, int readonly )
{
   (void) readonly; /* Everything is readonly. */

   /* Create the metatable */
   luaL_newmetatable(L, OUTFIT_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, outfitL_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, OUTFIT_METATABLE);

   return 0;
}


/**
 * @brief Lua bindings to interact with outfits.
 *
 * This will allow you to create and manipulate outfits in-game.
 *
 * An example would be:
 * @code
 * o = outfit.get( "Heavy Laser" ) -- Gets the outfit by name
 * cpu_usage = o:cpu() -- Gets the cpu usage of the outfit
 * slot_name, slot_size = o:slot() -- Gets slot information about the outfit
 * @endcode
 *
 * @luamod outfit
 */
/**
 * @brief Gets outfit at index.
 *
 *    @param L Lua state to get outfit from.
 *    @param ind Index position to find the outfit.
 *    @return Outfit found at the index in the state.
 */
LuaOutfit* lua_tooutfit( lua_State *L, int ind )
{
   return (LuaOutfit*) lua_touserdata(L,ind);
}
/**
 * @brief Gets outfit at index or raises error if there is no outfit at index.
 *
 *    @param L Lua state to get outfit from.
 *    @param ind Index position to find outfit.
 *    @return Outfit found at the index in the state.
 */
LuaOutfit* luaL_checkoutfit( lua_State *L, int ind )
{
   if (lua_isoutfit(L,ind))
      return lua_tooutfit(L,ind);
   luaL_typerror(L, ind, OUTFIT_METATABLE);
   return NULL;
}
/**
 * @brief Makes sure the outfit is valid or raises a Lua error.
 *
 *    @param L State currently running.
 *    @param ind Index of the outfit to validate.
 *    @return The outfit (doesn't return if fails - raises Lua error ).
 */
Outfit* luaL_validoutfit( lua_State *L, int ind )
{
   LuaOutfit *lo;
   Outfit *o;

   /* Get the outfit. */
   lo = luaL_checkoutfit(L,ind);
   o  = lo->outfit;
   if (o==NULL) {
      NLUA_ERROR(L,"Outfit is invalid.");
      return NULL;
   }

   return o;
}
/**
 * @brief Pushes a outfit on the stack.
 *
 *    @param L Lua state to push outfit into.
 *    @param outfit Outfit to push.
 *    @return Newly pushed outfit.
 */
LuaOutfit* lua_pushoutfit( lua_State *L, LuaOutfit outfit )
{
   LuaOutfit *o;
   o = (LuaOutfit*) lua_newuserdata(L, sizeof(LuaOutfit));
   *o = outfit;
   luaL_getmetatable(L, OUTFIT_METATABLE);
   lua_setmetatable(L, -2);
   return o;
}
/**
 * @brief Checks to see if ind is a outfit.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a outfit.
 */
int lua_isoutfit( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, OUTFIT_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Checks to see if two outfits are the same.
 *
 * @usage if o1 == o2 then -- Checks to see if outfit o1 and o2 are the same
 *
 *    @luaparam o1 First outfit to compare.
 *    @luaparam o2 Second outfit to compare.
 *    @luareturn true if both outfits are the same.
 * @luafunc __eq( o1, o2 )
 */
static int outfitL_eq( lua_State *L )
{
   LuaOutfit *a, *b;
   a = luaL_checkoutfit(L,1);
   b = luaL_checkoutfit(L,2);
   if (a->outfit == b->outfit)
      lua_pushboolean(L,1);
   else
      lua_pushboolean(L,0);
   return 1;
}




/**
 * @brief Gets a outfit.
 *
 * @usage s = outfit.get( "Hyena" ) -- Gets the hyena
 *
 *    @luaparam s Name of the outfit to get.
 *    @luareturn The outfit matching name or nil if error.
 * @luafunc get( s )
 */
static int outfitL_get( lua_State *L )
{
   const char *name;
   LuaOutfit lo;

   /* Handle parameters. */
   name = luaL_checkstring(L,1);

   /* Get outfit. */
   lo.outfit = outfit_get( name );
   if (lo.outfit == NULL) {
      NLUA_ERROR(L,"Outfit '%s' not found!", name);
      return 0;
   }

   /* Push. */
   lua_pushoutfit(L, lo);
   return 1;
}
/**
 * @brief Gets the name of the outfit's outfit.
 *
 * @usage outfitname = s:name()
 *
 *    @luaparam s Outfit to get outfit name.
 *    @luareturn The name of the outfit's outfit.
 * @luafunc name( s )
 */
static int outfitL_name( lua_State *L )
{
   Outfit *o;

   /* Get the outfit. */
   o  = luaL_validoutfit(L,1);

   /** Return the outfit name. */
   lua_pushstring(L, o->name);
   return 1;
}


/**
 * @brief Gets the type of an outfit.
 *
 * @usage print( o:type() ) -- Prints the type of the outfit
 *
 *    @luaparam o Outfit to get information of.
 *    @luareturn The name of the outfit type.
 * @luafunc type( o )
 */
static int outfitL_type( lua_State *L )
{
   Outfit *o = luaL_validoutfit(L,1);
   lua_pushstring(L, outfit_getType(o));
   return 1;
}


/**
 * @brief Gets the broad type of an outfit.
 *
 * This name is more generic and vague than type().
 *
 * @usage print( o:typeBroad() ) -- Prints the broad type of the outfit
 *
 *    @luaparam o Outfit to get information of.
 *    @luareturn The name of the outfit broad type.
 * @luafunc typeBroad( o )
 */
static int outfitL_typeBroad( lua_State *L )
{
   Outfit *o = luaL_validoutfit(L,1);
   lua_pushstring(L, outfit_getTypeBroad(o));
   return 1;
}


/**
 * @brief Gets the cpu usage of an outfit.
 *
 * @usage print( o:cpu() ) -- Prints the cpu usage of an outfit
 *
 *    @luaparam o Outfit to get information of.
 *    @luareturn The amount of cpu the outfit uses.
 * @luafunc cpu( o )
 */
static int outfitL_cpu( lua_State *L )
{
   Outfit *o = luaL_validoutfit(L,1);
   lua_pushnumber(L, outfit_cpu(o));
   return 1;
}


/**
 * @brief Gets the slot name and size of an outfit.
 *
 * @usage slot_name, slot_size = o:slot() -- Gets the slot information of an outfit
 *
 *    @luaparam o Outfit to get information of.
 *    @luareturn The name and the size in human readable strings.
 * @luafunc slot( o )
 */
static int outfitL_slot( lua_State *L )
{
   Outfit *o = luaL_validoutfit(L,1);
   lua_pushstring(L, outfit_slotName(o));
   lua_pushstring(L, outfit_slotSize(o));
   return 2;
}


