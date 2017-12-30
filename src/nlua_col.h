/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_COL_H
#  define NLUA_COL_H


#include <lua.h>

#include "nlua.h"
#include "colour.h"


#define COL_METATABLE      "colour" /**< COL metatable identifier. */


/*
 * Library loading
 */
int nlua_loadCol( nlua_env env );

/*
 * Colour operations
 */
glColour* lua_tocolour( lua_State *L, int ind );
glColour* luaL_checkcolour( lua_State *L, int ind );
glColour* lua_pushcolour( lua_State *L, glColour col );
int lua_iscolour( lua_State *L, int ind );


#endif /* NLUA_COL_H */


