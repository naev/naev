/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_diff.c
 *
 * @brief Unidiff Lua module.
 */


/** @cond */
#include "nstring.h"
#include <lauxlib.h>
#include <lua.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
/** @endcond */

#include "naev.h"

#include "nlua_diff.h"

#include "log.h"
#include "nluadef.h"
#include "unidiff.h"


/* diffs */
static int diff_applyL( lua_State *L );
static int diff_removeL( lua_State *L );
static int diff_isappliedL( lua_State *L );
static const luaL_Reg diff_methods[] = {
   { "apply", diff_applyL },
   { "remove", diff_removeL },
   { "isApplied", diff_isappliedL },
   {0,0}
}; /**< Unidiff Lua methods. */


/**
 * @brief Loads the diff Lua library.
 *    @param env Lua enviornment.
 *    @return 0 on success.
 */
int nlua_loadDiff( nlua_env env )
{
   nlua_register(env, "diff", diff_methods, 0);
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
 *    @luatparam string name Name of the diff to apply.
 * @luafunc apply
 */
static int diff_applyL( lua_State *L )
{
   const char *name;

   NLUA_CHECKRW(L);

   name = luaL_checkstring(L,1);

   diff_apply( name );
   return 0;
}
/**
 * @brief Removes a diff by name.
 *
 *    @luatparam string name Name of the diff to remove.
 * @luafunc remove
 */
static int diff_removeL( lua_State *L )
{
   const char *name;

   NLUA_CHECKRW(L);

   name = luaL_checkstring(L,1);

   diff_remove( name );
   return 0;
}
/**
 * @brief Checks to see if a diff is currently applied.
 *
 *    @luatparam string name Name of the diff to check.
 *    @luatreturn boolean true if diff is applied, false if it isn't.
 * @luafunc isApplied
 */
static int diff_isappliedL( lua_State *L )
{
   const char *name;

   name = luaL_checkstring(L,1);

   lua_pushboolean(L,diff_isApplied(name));
   return 1;
}
