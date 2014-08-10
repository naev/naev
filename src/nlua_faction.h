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


#ifndef NLUA_FACTION_H
#  define NLUA_FACTION_H


#include <lua.h>


#define FACTION_METATABLE  "faction" /**< Faction metatable identifier. */


/**
 * @brief Lua Faction wrapper.
 */
typedef struct LuaFaction_s {
   int f; /**< Internal use faction identifier. */
} LuaFaction;


/*
 * Load the space library.
 */
int nlua_loadFaction( lua_State *L, int readonly );

/*
 * Faction operations
 */
LuaFaction* lua_tofaction( lua_State *L, int ind );
LuaFaction* lua_pushfaction( lua_State *L, LuaFaction faction );
int luaL_validfaction( lua_State *L, int ind );
int lua_isfaction( lua_State *L, int ind );


#endif /* NLUA_FACTION_H */


