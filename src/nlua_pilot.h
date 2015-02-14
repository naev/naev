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


#ifndef NLUA_PILOT_H
#  define NLUA_PILOT_H


#include <lua.h>

#include "pilot.h"


#define PILOT_METATABLE   "pilot" /**< Pilot metatable identifier. */


/**
 * @brief Lua Pilot wrapper.
 */
typedef struct LuaPilot_s {
   unsigned int pilot; /**< ID of the pilot. */
} LuaPilot; /**< Wrapper for a Pilot. */


/*
 * Library loading
 */
int nlua_loadPilot( lua_State *L, int readonly );

/*
 * Pilot operations
 */
LuaPilot* lua_topilot( lua_State *L, int ind );
LuaPilot* luaL_checkpilot( lua_State *L, int ind );
LuaPilot* lua_pushpilot( lua_State *L, LuaPilot pilot );
Pilot* luaL_validpilot( lua_State *L, int ind );
int lua_ispilot( lua_State *L, int ind );


#endif /* NLUA_PILOT_H */


