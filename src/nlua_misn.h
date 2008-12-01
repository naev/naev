/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_MISN
#  define NLUA_MISN

#include "lua.h"


/* checks if a flag exists on the variable stack */
int var_checkflag( char* str );
void var_cleanup (void);

/* load the libraries for a lua state */
int misn_loadLibs( lua_State *L );
int misn_loadCondLibs( lua_State *L ); /* safe read only stuff */

/* individual library stuff */
int lua_loadMisn( lua_State *L );
int lua_loadVar( lua_State *L, int readonly );
int lua_loadPlayer( lua_State *L, int readonnly );
int lua_loadHook( lua_State *L );
int lua_loadDiff( lua_State *L, int readonly );


#endif /* NLUA_MISN */
