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


#ifndef NLUA_EVT
#  define NLUA_EVT


#include "event.h"
#include <lua.h>


/* Run Lua for an event. */
Event_t *event_getFromLua( lua_State *L );
lua_State *event_setupLua( Event_t *ev, const char *func );
int event_runLuaFunc( Event_t *ev, const char *func, int nargs );
int event_runLua( Event_t *ev, const char *func );

/* individual library stuff */
int nlua_loadEvt( lua_State *L );


#endif /* NLUA_EVT */
