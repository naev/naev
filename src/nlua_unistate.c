/**
 * @file nlua_unistate.c
 *
 * @brief Handles lua bindings to the state of the universe.
 *
 * This controls asset ownership and presense modifications, for example.
 */


#include "nlua_unistate.h"

#include "naev.h"

#include <lauxlib.h>

#include "console.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_planet.h"
#include "nlua_faction.h"
#include "nlua_vec2.h"
#include "nlua_system.h"
#include "nlua_tex.h"
#include "nlua_ship.h"
#include "nlua_outfit.h"
#include "nlua_commodity.h"
#include "nlua_col.h"
#include "log.h"
#include "rng.h"
#include "land.h"
#include "map.h"
#include "nmath.h"
#include "nstring.h"
#include "nfile.h"



static int unistateL_changeowner(lua_State *L);
static int unistateL_changepresence(lua_State *L);
static int unistateL_getpresence(lua_State *L);
static const luaL_reg unistate_methods[] = {
   { "changeAssetOwner", unistateL_changeowner },
   { "changeSysPresence", unistateL_changepresence },
   { "getSysPresence", unistateL_getpresence },
   {0,0}
};
static const luaL_reg unistate_cond_methods[] = {
   { "changeAssetOwner", unistateL_changeowner },
   { "changeSysPresence", unistateL_changepresence },
   { "getSysPresence", unistateL_getpresence },
   {0,0}
};

/**
 * @brief Loads the unistate library.
 * 
 *   @param L State to load the library into.
 *   @param readonly Whether to use readonly functions or not
 *   @return 0 on success.
 */
int nlua_loadUnistate( lua_State *L, int readonly )
{
   /* Create the metatable */
   luaL_newmetatable(L, UNISTATE_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   if (readonly)
      luaL_register(L, NULL, unistate_cond_methods);
   else
      luaL_register(L, NULL, unistate_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, UNISTATE_METATABLE);

   return 0; /* No error */
}

/**
 * @brief Changes the faction ownership of a planet.
 * 
 */
int unistateL_changeowner(lua_State *L)
{
   NLUA_ERROR(L, "Not implemented yet :(");
   return 0;
}


/**
 * @brief Changes the faction presence in a system.
 * 
 */
int unistateL_changepresence(lua_State *L)
{
   NLUA_ERROR(L, "Not implemented yet :(");
   return 0;
}

/**
 * @brief Gets the net faction presence in a system.
 * 
 */
int unistateL_getpresence(lua_State *L)
{
   NLUA_ERROR(L, "Not implemented yet :(");
   return 0;
}

