/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"

#define FACTION_METATABLE "faction" /**< Faction metatable identifier. */

/* Helper. */
#define luaL_optfaction( L, ind, def )                                         \
   nluaL_optarg( L, ind, def, lua_checkfaction )

/**
 * @brief Lua Faction wrapper.
 */
typedef int LuaFaction;

/*
 * Load the space library.
 */
int nlua_loadFaction( nlua_env env );

/*
 * Faction operations
 */
LuaFaction  lua_tofaction( lua_State *L, int ind );
LuaFaction  lua_checkfaction( lua_State *L, int ind );
LuaFaction *lua_pushfaction( lua_State *L, LuaFaction faction );
int         luaL_validfaction( lua_State *L, int ind );
int         lua_isfaction( lua_State *L, int ind );
