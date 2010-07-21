/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_SHIP_H
#  define NLUA_SHIP_H


#include "lua.h"

#include "ship.h"


#define SHIP_METATABLE   "ship" /**< Ship metatable identifier. */


/**
 * @brief Lua Ship wrapper.
 */
typedef struct LuaShip_s {
   Ship *ship; /**< Ship pointer. */
} LuaShip; /**< Wrapper for a Ship. */


/*
 * Library loading
 */
int nlua_loadShip( lua_State *L, int readonly );

/*
 * Ship operations
 */
LuaShip* lua_toship( lua_State *L, int ind );
LuaShip* luaL_checkship( lua_State *L, int ind );
LuaShip* lua_pushship( lua_State *L, LuaShip ship );
int lua_isship( lua_State *L, int ind );


#endif /* NLUA_SHIP_H */


