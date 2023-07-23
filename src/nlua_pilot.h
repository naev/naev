/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <lua.h>
/** @endcond */

#include "pilot.h"

#define PILOT_METATABLE   "pilot" /**< Pilot metatable identifier. */

/* Helper. */
#define luaL_optpilot(L,ind,def)   nluaL_optarg(L,ind,def,luaL_validpilot)

/**
 * @brief Lua Pilot wrapper.
 *
 * TODO probably wrap pilots with id + index, so we can try the index look-up
 * first and fall back to bsearch otherwise.
 */
typedef unsigned int LuaPilot; /**< Wrapper for a Pilot. */

/*
 * Library loading
 */
int nlua_loadPilot( nlua_env env );

/*
 * Pilot operations
 */
LuaPilot lua_topilot( lua_State *L, int ind );
LuaPilot luaL_checkpilot( lua_State *L, int ind );
LuaPilot* lua_pushpilot( lua_State *L, LuaPilot pilot );
Pilot* luaL_validpilot( lua_State *L, int ind );
int lua_ispilot( lua_State *L, int ind );
