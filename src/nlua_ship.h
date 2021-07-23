/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_SHIP_H
#  define NLUA_SHIP_H


#include "nlua.h"
#include "ship.h"


#define SHIP_METATABLE   "ship" /**< Ship metatable identifier. */


/*
 * Library loading
 */
int nlua_loadShip( nlua_env env );

/*
 * Ship operations
 */
const Ship* lua_toship( lua_State *L, int ind );
const Ship* luaL_checkship( lua_State *L, int ind );
const Ship* luaL_validship( lua_State *L, int ind );
const Ship** lua_pushship( lua_State *L, const Ship *ship );
int lua_isship( lua_State *L, int ind );


#endif /* NLUA_SHIP_H */


