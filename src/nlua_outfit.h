/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"
#include "outfit.h"

#define OUTFIT_METATABLE   "outfit" /**< Outfit metatable identifier. */

/* Helper. */
#define luaL_optoutfit(L,ind,def)   nluaL_optarg(L,ind,def,luaL_checkoutfit)

/*
 * Library loading
 */
int nlua_loadOutfit( nlua_env env );

/*
 * Outfit operations
 */
const Outfit* lua_tooutfit( lua_State *L, int ind );
const Outfit* luaL_checkoutfit( lua_State *L, int ind );
const Outfit* luaL_validoutfit( lua_State *L, int ind );
const Outfit** lua_pushoutfit( lua_State *L, const Outfit* outfit );
int lua_isoutfit( lua_State *L, int ind );
