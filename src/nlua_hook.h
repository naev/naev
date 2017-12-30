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
int hookL_getarg( unsigned int hook );
void hookL_unsetarg( unsigned int hook );


#endif /* NLUA_HOOK */
