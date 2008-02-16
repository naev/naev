/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef MISN_LUA
#  define MISN_LUA

#include "lua.h"


/* checks if a flag exists on the variable stack */
int var_checkflag( char* str );
void var_cleanup (void);

/* load the libraries for a lua state */
int misn_loadLibs( lua_State *L );


#endif /* MISN_LUA */
