/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <assert.h>
/** @endcond */

#include "attributes.h"
#include "log.h"

/* Fixes clangd warning about #pragma GCC diagnostic pop
 * See: https://github.com/clangd/clangd/issues/1167 */
static_assert(1,"");
/*
 * A number of Lua error functions don't return, but aren't marked
 * as such. These redeclarations ensure that the compiler and analyzer are
 * aware that no return will take place when compiling our code.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"

NORETURN extern int lua_error( lua_State *L );
NORETURN extern int luaL_error( lua_State *L, const char *fmt, ... );
NORETURN extern int luaL_typerror( lua_State *L, int narg, const char *tname );

#pragma GCC diagnostic pop

/*
 * debug stuff
 */
#ifdef DEBUG_PARANOID
#define NLUA_DEBUG(str, ...) \
   (DEBUG("Lua: "str"\n", ## __VA_ARGS__), abort())
#else /* DEBUG_PARANOID */
#define NLUA_DEBUG(str, ...) \
   (DEBUG("Lua: "str"\n", ## __VA_ARGS__))
#endif /* DEBUG_PARANOID */
#define NLUA_INVALID_PARAMETER(L)    \
{ \
   DEBUG( "Invalid parameter for %s.", __func__ ); \
   luaL_error( L, "Invalid parameter for %s.", __func__ ); \
   return 0; \
}
#define NLUA_MIN_ARGS(n)     \
   if (lua_gettop(L) < n) { \
      DEBUG( "Too few arguments for %s.", __func__ ); \
      luaL_error( L, "Too few arguments for %s.", __func__ ); \
      return 0; \
   }

/*
 * Error stuff.
 */
#define NLUA_ERROR(L,str, ...)  (luaL_error(L,str, ## __VA_ARGS__))
