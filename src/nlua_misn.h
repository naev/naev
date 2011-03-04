/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_MISN
#  define NLUA_MISN

#include "lua.h"

#include "mission.h"


/* load the libraries for a lua state */
Mission* misn_getFromLua( lua_State *L );
int misn_loadLibs( lua_State *L );
int misn_loadCondLibs( lua_State *L ); /* safe read only stuff */

/* individual library stuff */
int nlua_loadMisn( lua_State *L );


#endif /* NLUA_MISN */
