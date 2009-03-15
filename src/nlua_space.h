/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_SPACE_H
#  define NLUA_SPACE_H


#include "lua.h"

#include "space.h"


#define PLANET_METATABLE   "Planet" /**< Planet metatable identifier. */
#define SYSTEM_METATABLE   "System" /**< System metatable identifier. */


/*
 * Lua wrappers.
 */
/**
 * @brief Lua Planet Wrapper.
 */
typedef struct LuaPlanet_s {
   Planet *p; /**< Pointer to the real Planet. */
} LuaPlanet;
/**
 * @brief Lua StarSystem Wrapper.
 */
typedef struct LuaSystem_s {
   StarSystem *s; /**< Pointer to the real StarSystem. */
} LuaSystem;


/* 
 * Load the space library.
 */
int lua_loadSpace( lua_State *L, int readonly );

/*
 * Planet operations.
 */
LuaPlanet* lua_toplanet( lua_State *L, int ind );
LuaPlanet* lua_pushplanet( lua_State *L, LuaPlanet planet );
int lua_isplanet( lua_State *L, int ind );

/*
 * System operations.
 */
LuaSystem* lua_tosystem( lua_State *L, int ind );
LuaSystem* lua_pushsystem( lua_State *L, LuaSystem sys );
int lua_issystem( lua_State *L, int ind );


#endif /* NLUA_SPACE_H */

