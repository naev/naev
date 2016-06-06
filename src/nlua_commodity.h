/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_COMMODITY_H
#  define NLUA_COMMODITY_H


#include <lua.h>

#include "economy.h"


#define COMMODITY_METATABLE   "commodity" /**< Commodity metatable identifier. */


/**
 * @brief Lua Commodity wrapper.
 */
typedef struct LuaCommodity_s {
   Commodity *commodity; /**< Commodity pointer. */
} LuaCommodity; /**< Wrapper for a Commodity. */


/*
 * Library loading
 */
int nlua_loadCommodity( lua_State *L, int readonly );

/*
 * Commodity operations
 */
LuaCommodity* lua_tocommodity( lua_State *L, int ind );
LuaCommodity* luaL_checkcommodity( lua_State *L, int ind );
Commodity* luaL_validcommodity( lua_State *L, int ind );
LuaCommodity* lua_pushcommodity( lua_State *L, LuaCommodity commodity );
int lua_iscommodity( lua_State *L, int ind );


#endif /* NLUA_COMMODITY_H */


