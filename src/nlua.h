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


#ifndef NLUA_H
#  define NLUA_H


#include <lua.h>


#define NLUA_DONE       "__done__"


/*
 * standard Lua stuff wrappers
 */
lua_State *nlua_newState (void); /* creates a new state */
int nlua_load( lua_State* L, lua_CFunction f );
int nlua_loadBasic( lua_State* L );
int nlua_loadStandard( lua_State *L, int readonly );
int nlua_errTrace( lua_State *L );

#endif /* NLUA_H */


