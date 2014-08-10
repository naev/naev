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
StarSystem* luaL_validsystem( lua_State *L, int ind );
int lua_issystem( lua_State *L, int ind );


#endif /* NLUA_SYSTEM_H */

