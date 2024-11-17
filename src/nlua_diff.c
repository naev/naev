/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_diff.c
 *
 * @brief Unidiff Lua module.
 */
/** @cond */
#include <lauxlib.h>
#include <lua.h>
/** @endcond */

#include "nlua_diff.h"

#include "unidiff.h"

/* diffs */
static int diffL_apply( lua_State *L );
static int diffL_remove( lua_State *L );
static int diffL_isapplied( lua_State *L );

static const luaL_Reg diffL_methods[] = {
   { "apply", diffL_apply },
   { "remove", diffL_remove },
   { "isApplied", diffL_isapplied },
   { 0, 0 } }; /**< Unidiff Lua methods. */

/**
 * @brief Loads the diff Lua library.
 *    @param env Lua enviornment.
 *    @return 0 on success.
 */
int nlua_loadDiff( nlua_env env )
{
   nlua_register( env, "diff", diffL_methods, 0 );
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
 * If your diff modifies a Spob or a system, the safe lanes will be re-computed
 * just after it is applied, causing the game to freeze for a short time.
 * As a result, prefer not to apply a diff when the player is in space.
 *
 *    @luatparam string name Name of the diff to apply.
 * @luafunc apply
 */
static int diffL_apply( lua_State *L )
{
   const char *name = luaL_checkstring( L, 1 );
   diff_apply( name );
   return 0;
}

/**
 * @brief Removes a diff by name.
 *
 *    @luatparam string name Name of the diff to remove.
 * @luafunc remove
 */
static int diffL_remove( lua_State *L )
{
   const char *name = luaL_checkstring( L, 1 );
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
static int diffL_isapplied( lua_State *L )
{
   const char *name = luaL_checkstring( L, 1 );
   lua_pushboolean( L, diff_isApplied( name ) );
   return 1;
}
