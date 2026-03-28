/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "commodity.h"
#include "nlua.h"

#define COMMODITY_METATABLE                                                    \
   "commodity" /**< Commodity metatable identifier.                            \
                */

/*
 * Library loading
 */
int nlua_loadCommodity( nlua_env *env );

/*
 * Commodity operations
 */
CommodityRef  lua_tocommodity( lua_State *L, int ind );
CommodityRef  luaL_checkcommodity( lua_State *L, int ind );
CommodityRef  luaL_validcommodity( lua_State *L, int ind );
CommodityRef *lua_pushcommodity( lua_State *L, CommodityRef commodity );
int           lua_iscommodity( lua_State *L, int ind );
