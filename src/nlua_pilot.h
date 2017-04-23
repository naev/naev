/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_PILOT_H
#  define NLUA_PILOT_H


#include <lua.h>

#include "pilot.h"


#define PILOT_METATABLE   "pilot" /**< Pilot metatable identifier. */


/**
 * @brief Lua Pilot wrapper.
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


#endif /* NLUA_PILOT_H */


