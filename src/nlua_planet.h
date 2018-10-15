/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_PLANET_H
#  define NLUA_PLANET_H


#include <lua.h>

#include "space.h"
#include "nlua.h"


#define PLANET_METATABLE   "planet" /**< Planet metatable identifier. */


/**
 * @brief Lua Planet Wrapper.
 */
typedef int LuaPlanet;


/*
 * Load the planet library.
 */
int nlua_loadPlanet( nlua_env env );

/*
 * Planet operations.
 */
LuaPlanet* lua_toplanet( lua_State *L, int ind );
LuaPlanet* luaL_checkplanet( lua_State *L, int ind );
LuaPlanet* lua_pushplanet( lua_State *L, LuaPlanet planet );
Planet* luaL_validplanet( lua_State *L, int ind );
int lua_isplanet( lua_State *L, int ind );


#endif /* NLUA_PLANET_H */

