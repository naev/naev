/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_COMMODITY_H
#  define NLUA_COMMODITY_H


#include <lua.h>

#include "economy.h"


#define COMMODITY_METATABLE   "commodity" /**< Commodity metatable identifier. */


/*
 * Library loading
 */
int nlua_loadCommodity( lua_State *L, int readonly );

/*
 * Commodity operations
 */
Commodity* lua_tocommodity( lua_State *L, int ind );
Commodity* luaL_checkcommodity( lua_State *L, int ind );
Commodity* luaL_validcommodity( lua_State *L, int ind );
Commodity** lua_pushcommodity( lua_State *L, Commodity* commodity );
int lua_iscommodity( lua_State *L, int ind );


#endif /* NLUA_COMMODITY_H */


