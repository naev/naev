/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"

#define LINOPT_METATABLE "linopt" /**< Optim metatable identifier. */

struct LuaLinOpt_s;
typedef struct LuaLinOpt_s LuaLinOpt_t;

/*
 * Library loading
 */
int nlua_loadLinOpt( nlua_env env );

/* Basic operations. */
LuaLinOpt_t *lua_tolinopt( lua_State *L, int ind );
LuaLinOpt_t *luaL_checklinopt( lua_State *L, int ind );
LuaLinOpt_t *lua_pushlinopt( lua_State *L, LuaLinOpt_t linopt );
int          lua_islinopt( lua_State *L, int ind );
