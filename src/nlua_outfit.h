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


#ifndef NLUA_OUTFIT_H
#  define NLUA_OUTFIT_H


#include <lua.h>

#include "outfit.h"


#define OUTFIT_METATABLE   "outfit" /**< Outfit metatable identifier. */


/**
 * @brief Lua Outfit wrapper.
 */
typedef struct LuaOutfit_s {
   Outfit *outfit; /**< Outfit pointer. */
} LuaOutfit; /**< Wrapper for a Outfit. */


/*
 * Library loading
 */
int nlua_loadOutfit( lua_State *L, int readonly );

/*
 * Outfit operations
 */
LuaOutfit* lua_tooutfit( lua_State *L, int ind );
LuaOutfit* luaL_checkoutfit( lua_State *L, int ind );
Outfit* luaL_validoutfit( lua_State *L, int ind );
LuaOutfit* lua_pushoutfit( lua_State *L, LuaOutfit outfit );
int lua_isoutfit( lua_State *L, int ind );


#endif /* NLUA_OUTFIT_H */


