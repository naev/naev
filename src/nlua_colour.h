/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "colour.h"
#include "nlua.h"

#define COL_METATABLE "colour" /**< COL metatable identifier. */

/* Helper. */
#define luaL_optcolour( L, ind, def )                                          \
   nluaL_optarg( L, ind, def, luaL_checkcolour )

/*
 * Library loading
 */
// int nlua_loadCol( nlua_env *env );

/*
 * Colour operations
 */
glColour *lua_tocolour( lua_State *L, int ind );
glColour *luaL_checkcolour( lua_State *L, int ind );
glColour *lua_pushcolour( lua_State *L, glColour col );
int       lua_iscolour( lua_State *L, int ind );
