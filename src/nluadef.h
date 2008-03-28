/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUADEF_H
# define NLUADEF_H


#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


/*
 * debug stuff
 */
#define NLUA_DEBUG(str, args...) \
   (fprintf(stdout,"Lua: "str"\n", ## args))
#define NLUA_INVALID_PARAMETER()    \
{ \
   NLUA_DEBUG("[%s] Invalid parameter", __func__); \
   return 0; \
}
#define NLUA_MIN_ARGS(n)     \
   if (lua_gettop(L) < n) { \
      NLUA_DEBUG("[%s] Too few arguments", __func__); \
      return 0; \
   }


/*
 * comfortability macros
 */
#define luaL_dobuffer(L, b, n, s) \
   (luaL_loadbuffer(L, b, n, s) || lua_pcall(L, 0, LUA_MULTRET, 0))


#endif /* NLUADEF_H */
