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


#ifndef NLUA_COL_H
#  define NLUA_COL_H


#include <lua.h>

#include "colour.h"


#define COL_METATABLE      "colour" /**< COL metatable identifier. */


/**
 * @brief Lua texture wrapper.
 */
typedef struct LuaColour_s {
   glColour col; /**< Colour wrapped around. */
} LuaColour; /**< Wrapper for a texture. */


/*
 * Library loading
 */
int nlua_loadCol( lua_State *L, int readonly );

/*
 * Colour operations
 */
LuaColour* lua_tocolour( lua_State *L, int ind );
LuaColour* luaL_checkcolour( lua_State *L, int ind );
LuaColour* lua_pushcolour( lua_State *L, LuaColour col );
int lua_iscolour( lua_State *L, int ind );


#endif /* NLUA_COL_H */


