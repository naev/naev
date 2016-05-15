/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_FACTION_H
#  define NLUA_FACTION_H


#include <lua.h>


#define FACTION_METATABLE  "faction" /**< Faction metatable identifier. */


/**
 * @brief Lua Faction wrapper.
 */
typedef int LuaFaction;


/*
 * Load the space library.
 */
int nlua_loadFaction( lua_State *L, int readonly );

/*
 * Faction operations
 */
LuaFaction lua_tofaction( lua_State *L, int ind );
LuaFaction* lua_pushfaction( lua_State *L, LuaFaction faction );
int luaL_validfaction( lua_State *L, int ind );
int lua_isfaction( lua_State *L, int ind );


#endif /* NLUA_FACTION_H */


