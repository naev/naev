/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_HOOK
#  define NLUA_HOOK

#include <lua.h>

#include "nlua.h"


/* individual library stuff */
int nlua_loadHook( nlua_env env );


/* Misc. */
int hookL_getarg( lua_State *L, unsigned int hook );
void hookL_unsetarg( lua_State *L, unsigned int hook );


#endif /* NLUA_HOOK */
