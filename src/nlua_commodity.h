/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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


