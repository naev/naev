/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_SHIP_H
#  define NLUA_SHIP_H


#include <lua.h>

#include "ship.h"


#define SHIP_METATABLE   "ship" /**< Ship metatable identifier. */


/*
 * Library loading
 */
int nlua_loadShip( lua_State *L, int readonly );

/*
 * Ship operations
 */
Ship* lua_toship( lua_State *L, int ind );
Ship* luaL_checkship( lua_State *L, int ind );
Ship* luaL_validship( lua_State *L, int ind );
Ship** lua_pushship( lua_State *L, Ship *ship );
int lua_isship( lua_State *L, int ind );


#endif /* NLUA_SHIP_H */


