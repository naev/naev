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

/**
 * @file nlua_space.c
 *
 * @brief Handles the Lua space bindings.
 *
 * These bindings control the planets and systems.
 */


#include "nlua_space.h"

#include "naev.h"

#include <lauxlib.h>

#include "log.h"
#include "nlua.h"
#include "nluadef.h"
#include "nlua_planet.h"
#include "nlua_system.h"
#include "nlua_jump.h"


/**
 * @brief Loads the space library.
 *
 *    @param L State to load space library into.
 *    @param readonly Load read only functions?
 *    @return 0 on success.
 */
int nlua_loadSpace( lua_State *L, int readonly )
{
   nlua_loadPlanet( L, readonly );
   nlua_loadSystem( L, readonly );
   nlua_loadJump( L, readonly );

   return 0;
}

