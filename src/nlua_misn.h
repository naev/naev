/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_MISN
#  define NLUA_MISN

#include <lua.h>

#include "mission.h"


/* load the libraries for a Lua state */
Mission* misn_getFromLua( lua_State *L );
int misn_loadLibs( nlua_env env );
int misn_loadCondLibs( lua_State *L ); /* safe read only stuff */

/* individual library stuff */
int nlua_loadMisn( nlua_env env );


#endif /* NLUA_MISN */
