/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_COL_H
#  define NLUA_COL_H


#include "lua.h"

#include "colour.h"


#define COL_METATABLE      "colour" /**< COL metatable identifier. */


/**
 * @brief Lua texture wrapper.
 */
typedef struct LuaColour_s {
   glColour col;
} LuaColour; /**< Wrapper for a texture. */


/*
 * Library loading
 */
int nlua_loadCol( lua_State *L, int readonly );

/*
 * Colour operations
 */
LuaColour* lua_tocolour( lua_State *L, int ind );
LuaColour* luaL_checkcolour( lua_State *L, int ind );
LuaColour* lua_pushcolour( lua_State *L, LuaColour col );
int lua_iscolour( lua_State *L, int ind );


#endif /* NLUA_COL_H */


