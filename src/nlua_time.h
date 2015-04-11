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


#ifndef NLUA_TIME_H
#  define NLUA_TIME_H


#include <lua.h>

#include "ntime.h"


#define TIME_METATABLE   "time" /**< Planet metatable identifier. */


/**
 * @brief Wrapper for ntime_t.
 */
typedef struct LuaTime_s {
   ntime_t t; /**< Wrapped time. */
} LuaTime;


/*
 * Library stuff.
 */
int nlua_loadTime( lua_State *L, int readonly );

/*
 * Time operations.
 */
LuaTime* lua_totime( lua_State *L, int ind );
LuaTime* luaL_checktime( lua_State *L, int ind );
LuaTime* lua_pushtime( lua_State *L, LuaTime time );
ntime_t luaL_validtime( lua_State *L, int ind );
int lua_istime( lua_State *L, int ind );


#endif /* NLUA_TIME_H */


