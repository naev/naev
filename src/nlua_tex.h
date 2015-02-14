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


#ifndef NLUA_TEX_H
#  define NLUA_TEX_H


#include <lua.h>

#include "opengl.h"


#define TEX_METATABLE      "tex" /**< Texture metatable identifier. */


/**
 * @brief Lua texture wrapper.
 */
typedef struct LuaTex_s {
   glTexture *tex; /**< Texture wrapped around. */
} LuaTex; /**< Wrapper for a texture. */


/*
 * Library loading
 */
int nlua_loadTex( lua_State *L, int readonly );

/*
 * Texture operations
 */
LuaTex* lua_totex( lua_State *L, int ind );
LuaTex* luaL_checktex( lua_State *L, int ind );
LuaTex* lua_pushtex( lua_State *L, LuaTex tex );
int lua_istex( lua_State *L, int ind );


#endif /* NLUA_TEX_H */


