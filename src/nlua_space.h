/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_SPACE_H
#  define NLUA_SPACE_H


#include "lua.h"


#define PLANET_METATABLE   "Planet"
#define SYSTEM_METATABLE   "System"


/* 
 * Load the space library.
 */
int lua_loadSpace( lua_State *L, int readonly );


#endif /* NLUA_SPACE_H */


