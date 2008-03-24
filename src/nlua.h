/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_H
#  define NLUA_H


#include "lua.h"


/* individual libraries */
int lua_loadNaev( lua_State *L ); /* always read only */
int lua_loadSpace( lua_State *L, int readonly );
int lua_loadTime( lua_State *L, int readonly );
int lua_loadRnd( lua_State *L ); /* always read only */
int lua_loadTk( lua_State *L ); /* always read only */


#endif /* NLUA_H */


