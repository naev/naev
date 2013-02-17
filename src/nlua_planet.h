/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_PLANET_H
#  define NLUA_PLANET_H


#include <lua.h>

#include "space.h"


#define PLANET_METATABLE   "planet" /**< Planet metatable identifier. */


/**
 * @brief Lua Planet Wrapper.
 */
typedef struct LuaPlanet_s {
   int id; /**< ID to the planet. */
} LuaPlanet;


/*
 * Load the planet library.
 */
int nlua_loadPlanet( lua_State *L, int readonly );

/*
 * Planet operations.
 */
LuaPlanet* lua_toplanet( lua_State *L, int ind );
LuaPlanet* luaL_checkplanet( lua_State *L, int ind );
LuaPlanet* lua_pushplanet( lua_State *L, LuaPlanet planet );
Planet* luaL_validplanet( lua_State *L, int ind );    /* gets the planet and warns if it doesn't exist */
Planet* luaL_getplanet( lua_State *L, int ind );   /* gets the planet but doesn't warn if it doesn't exist */
int lua_isplanet( lua_State *L, int ind );


#endif /* NLUA_PLANET_H */

