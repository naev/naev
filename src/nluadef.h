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
#ifndef NODEBUG
#ifdef DEBUG_PARANOID
#define NLUA_DEBUG(str, args...) \
   (fprintf(stdout,"Lua: "str"\n", ## args), abort())
#else /* DEBUG_PARANOID */
#define NLUA_DEBUG(str, args...) \
   (fprintf(stdout,"Lua: "str"\n", ## args))
#endif /* DEBUG_PARANOID */
#define NLUA_INVALID_PARAMETER()    \
{ \
   NLUA_DEBUG("[%s] Invalid parameter (%s:%d)", __func__, __FILE__, __LINE__); \
   return 0; \
}
#define NLUA_MIN_ARGS(n)     \
   if (lua_gettop(L) < n) { \
      NLUA_DEBUG("[%s] Too few arguments (%s:%d)", __func__, __FILE__, __LINE__); \
      return 0; \
   }
#else /* NODEBUG */
#define NLUA_DEBUG(str, args...) do {;} while(0)
#define NLUA_MIN_ARGS(n)         do {;} while(0)
#define NLUA_INVALID_PARAMETER() do {;} while(0)
#endif /* NODEBUG */


/*
 * comfortability macros
 */
#define luaL_dobuffer(L, b, n, s) \
   (luaL_loadbuffer(L, b, n, s) || lua_pcall(L, 0, LUA_MULTRET, 0))


#endif /* NLUADEF_H */
