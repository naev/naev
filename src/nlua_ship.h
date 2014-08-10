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


#ifndef NLUA_SHIP_H
#  define NLUA_SHIP_H


#include <lua.h>

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
Ship* luaL_validship( lua_State *L, int ind );
LuaShip* lua_pushship( lua_State *L, LuaShip ship );
int lua_isship( lua_State *L, int ind );


#endif /* NLUA_SHIP_H */


