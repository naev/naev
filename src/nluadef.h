/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUADEF_H
# define NLUADEF_H


#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "log.h"


/*
 * A number of lua error functions don't ruturn, but arnen't marked
 * as such. These redeclarations ensure that the compiler and analizer are
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
#define NLUA_DEBUG(str, args...) \
   (DEBUG("Lua: "str"\n", ## args), abort())
#else /* DEBUG_PARANOID */
#define NLUA_DEBUG(str, args...) \
   (DEBUG("Lua: "str"\n", ## args))
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
#define NLUA_ERROR(L,str, args...)  (luaL_error(L,str, ## args))

#define NLUA_CHECKRW(L) \
{ \
   nlua_getenv(__NLUA_CURENV, "__RW"); \
   if (!lua_toboolean(L, -1)) { \
      DEBUG( "Cannot call %s in read-only environment.", __func__ ); \
      luaL_error( L, "Cannot call %s in read-only environment.", __func__ ); \
      return 0; \
   } \
   lua_pop(L, 1); \
}


#endif /* NLUADEF_H */
