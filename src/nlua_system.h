/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_SYSTEM_H
#  define NLUA_SYSTEM_H


#include <lua.h>

#include "space.h"


#define SYSTEM_METATABLE   "system" /**< System metatable identifier. */


/**
 * @brief Lua StarSystem Wrapper.
 */
typedef struct LuaSystem_s {
   int id; /**< Star system ID. */
} LuaSystem;


/*
 * Load the system library.
 */
int nlua_loadSystem( lua_State *L, int readonly );

/*
 * System operations.
 */
LuaSystem* lua_tosystem( lua_State *L, int ind );
LuaSystem* luaL_checksystem( lua_State *L, int ind );
LuaSystem* lua_pushsystem( lua_State *L, LuaSystem sys );
StarSystem* luaL_validsystem( lua_State *L, int ind );   /* warns if system doesn't exist */
StarSystem* luaL_getsystem( lua_State *L, int ind );  /* does not warn if system doesn't exist */
int lua_issystem( lua_State *L, int ind );


#endif /* NLUA_SYSTEM_H */

