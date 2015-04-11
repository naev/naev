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


#ifndef NLUA_VEC2_H
#  define NLUA_VEC2_H


#include <lua.h>
#include "physics.h"


#define VECTOR_METATABLE   "vec2"   /**< Vector metatable identifier. */


/**
 * @brief Lua Vector2d Wrapper.
 */
typedef struct LuaVector_s {
   Vector2d vec; /**< The actual Vector2d. */
} LuaVector;


/*
 * Vector library.
 */
int nlua_loadVector( lua_State *L );

/*
 * Vector operations.
 */
LuaVector* lua_tovector( lua_State *L, int ind );
LuaVector* luaL_checkvector( lua_State *L, int ind );
LuaVector* lua_pushvector( lua_State *L, LuaVector vec );
int lua_isvector( lua_State *L, int ind );


#endif /* NLUA_VEC2_H */


