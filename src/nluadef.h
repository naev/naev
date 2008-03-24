/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUADEF_H
# define NLUADEF_H


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


#endif /* NLUADEF_H */
