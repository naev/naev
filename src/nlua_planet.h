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


#ifndef NLUA_PLANET_H
#  define NLUA_PLANET_H


#include <lua.h>

#include "space.h"


#define PLANET_METATABLE   "planet" /**< Planet metatable identifier. */


/**
 * @brief Lua Planet Wrapper.
 */
typedef struct LuaPlanet_s {
   int id; /**< ID to the planet. */
} LuaPlanet;


/*
 * Load the planet library.
 */
int nlua_loadPlanet( lua_State *L, int readonly );

/*
 * Planet operations.
 */
LuaPlanet* lua_toplanet( lua_State *L, int ind );
LuaPlanet* luaL_checkplanet( lua_State *L, int ind );
LuaPlanet* lua_pushplanet( lua_State *L, LuaPlanet planet );
Planet* luaL_validplanet( lua_State *L, int ind );
int lua_isplanet( lua_State *L, int ind );


#endif /* NLUA_PLANET_H */

