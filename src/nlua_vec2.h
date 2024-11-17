/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"
#include "physics.h"

#define VECTOR_METATABLE "vec2" /**< Vector metatable identifier. */

/* Helper. */
#define luaL_optvector( L, ind, def )                                          \
   nluaL_optarg( L, ind, def, luaL_checkvector )

/*
 * Vector library.
 */
int nlua_loadVector( nlua_env env );

/*
 * Vector operations.
 */
vec2 *lua_tovector( lua_State *L, int ind );
vec2 *luaL_checkvector( lua_State *L, int ind );
vec2 *lua_pushvector( lua_State *L, vec2 vec );
int   lua_isvector( lua_State *L, int ind );
