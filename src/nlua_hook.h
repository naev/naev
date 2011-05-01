/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_HOOK
#  define NLUA_HOOK

#include <lua.h>


/* individual library stuff */
int nlua_loadHook( lua_State *L );


/* Misc. */
int hookL_getarg( lua_State *L, unsigned int hook );


#endif /* NLUA_HOOK */
