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


#ifndef NLUA_JUMP_H
#  define NLUA_JUMP_H


#include <lua.h>

#include "space.h"


#define JUMP_METATABLE   "jump" /**< Lua metatable identifier. */


/**
 * @brief Lua jump Wrapper.
 */
typedef struct LuaJump_s {
   int srcid;  /**< Starting star system ID. */
   int destid; /**< Destination star system ID. */
} LuaJump;


/*
 * Load the jump library.
 */
int nlua_loadJump( lua_State *L, int readonly );

/*
 * Jump operations.
 */
LuaJump* lua_tojump( lua_State *L, int ind );
LuaJump* luaL_checkjump( lua_State *L, int ind );
LuaJump* lua_pushjump( lua_State *L, LuaJump jump );
JumpPoint* luaL_validjump( lua_State *L, int ind );
int lua_isjump( lua_State *L, int ind );


#endif /* NLUA_JUMP_H */

