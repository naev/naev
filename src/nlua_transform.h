/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "mat3.h"
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
int nlua_loadTransform( nlua_env *env );

/*
 * Transform operations
 */
mat3 *lua_totransform( lua_State *L, int ind );
mat3 *luaL_checktransform( lua_State *L, int ind );
mat3 *lua_pushtransform( lua_State *L, mat3 Transform );
int   lua_istransform( lua_State *L, int ind );
