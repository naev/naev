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


#ifndef NLUADEF_H
# define NLUADEF_H


#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "log.h"


/*
 * debug stuff
 */
#ifdef DEBUGGING
#ifdef DEBUG_PARANOID
#define NLUA_DEBUG(str, args...) \
   (DEBUG("Lua: "str"\n", ## args), abort())
#else /* DEBUG_PARANOID */
#define NLUA_DEBUG(str, args...) \
   (DEBUG("Lua: "str"\n", ## args))
#endif /* DEBUG_PARANOID */
#define NLUA_INVALID_PARAMETER(L)    \
{ \
   DEBUG( "Invalid parameter for %s.", __func__ ); \
   luaL_error( L, "Invalid parameter for %s.", __func__ ); \
   return 0; \
}
#define NLUA_MIN_ARGS(n)     \
   if (lua_gettop(L) < n) { \
      DEBUG( "Too few arguments for %s.", __func__ ); \
      luaL_error( L, "Too few arguments for %s.", __func__ ); \
      return 0; \
   }
#else /* DEBUGGING */
#define NLUA_DEBUG(str, args...) do {;} while(0)
#define NLUA_MIN_ARGS(n)         do {;} while(0)
#define NLUA_INVALID_PARAMETER(L) do {;} while(0)
#endif /* DEBUGGING */


/*
 * Error stuff.
 */
#define NLUA_ERROR(L,str, args...)  (luaL_error(L,str, ## args))


/*
 * comfortability macros
 */
#define luaL_dobuffer(L, b, n, s) \
   (luaL_loadbuffer(L, b, n, s) || lua_pcall(L, 0, LUA_MULTRET, 0))


#endif /* NLUADEF_H */
