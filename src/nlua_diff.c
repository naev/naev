/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_diff.c
 *
 * @brief Unidiff Lua module.
 */


#include "nlua_diff.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include "nstring.h"
#include <math.h>

#include <lua.h>
#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "unidiff.h"


/* diffs */
static int diff_applyL( lua_State *L );
static int diff_removeL( lua_State *L );
static int diff_isappliedL( lua_State *L );
static const luaL_reg diff_methods[] = {
   { "apply", diff_applyL },
   { "remove", diff_removeL },
   { "isApplied", diff_isappliedL },
   {0,0}
}; /**< Unidiff Lua methods. */
static const luaL_reg diff_cond_methods[] = {
   { "isApplied", diff_isappliedL },
   {0,0}
}; /**< Unidiff Lua read only methods. */


/**
 * @brief Loads the diff Lua library.
 *    @param L Lua state.
 *    @param readonly Load read only functions?
 *    @return 0 on success.
 */
int nlua_loadDiff( lua_State *L, int readonly )
{
   if (readonly == 0)
      luaL_register(L, "diff", diff_methods);
   else
      luaL_register(L, "diff", diff_cond_methods);
   return 0;
}


/**
 * @brief Lua bindings to apply/remove Universe Diffs.
 *
 * Universe Diffs are patches you can apply to the universe to make permanent
 *  changes. They are defined in dat/unidiff.xml.
 *
 * Typical usage would be:
 * @code
 * diff.apply( "collective_dead" )
 * @endcode
 *
 * @luamod diff
 */
/**
 * @brief Applies a diff by name.
 *
 *    @luaparam name Name of the diff to apply.
 * @luafunc apply( name )
 */
static int diff_applyL( lua_State *L )
{
   const char *name;

   name = luaL_checkstring(L,1);

   diff_apply( name );
   return 0;
}
/**
 * @brief Removes a diff by name.
 *
 *    @luaparam name Name of the diff to remove.
 * @luafunc remove( name )
 */
static int diff_removeL( lua_State *L )
{
   const char *name;

   name = luaL_checkstring(L,1);

   diff_remove( name );
   return 0;
}
/**
 * @brief Checks to see if a diff is currently applied.
 *
 *    @luaparam name Name of the diff to check.
 *    @luareturn true if diff is applied, false if it isn't.
 * @luafunc isApplied( name )
 */
static int diff_isappliedL( lua_State *L )
{
   const char *name;

   name = luaL_checkstring(L,1);

   lua_pushboolean(L,diff_isApplied(name));
   return 1;
}
