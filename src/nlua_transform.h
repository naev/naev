/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "mat4.h"
#include "nlua.h"
#include "opengl.h"

#define TRANSFORM_METATABLE                                                    \
   "transform" /**< Transform metatable identifier.                            \
                */

#define luaL_opttransform( L, ind, def )                                       \
   nluaL_optarg( L, ind, def, luaL_checktransform )

/*
 * Library loading
 */
int nlua_loadTransform( nlua_env env );

/*
 * Transform operations
 */
mat4 *lua_totransform( lua_State *L, int ind );
mat4 *luaL_checktransform( lua_State *L, int ind );
mat4 *lua_pushtransform( lua_State *L, mat4 Transform );
int   lua_istransform( lua_State *L, int ind );
