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


#ifndef NLUA_SPACE_H
#  define NLUA_SPACE_H


#include <lua.h>

#include "nlua_planet.h"
#include "nlua_system.h"
#include "nlua_jump.h"


/*
 * Load the space library.
 */
int nlua_loadSpace( lua_State *L, int readonly );


#endif /* NLUA_SPACE_H */

