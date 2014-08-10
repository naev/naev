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
 * @file nlua_cli.c
 *
 * @brief Contains Lua bindings for the console.
 */


#include "nlua_cli.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "mission.h"


/* CLI */
static const luaL_reg cli_methods[] = {
   {0,0}
}; /**< CLI Lua methods. */


/**
 * @brief Loads the CLI Lua library.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int nlua_loadCLI( lua_State *L )
{
   luaL_register(L, "cli", cli_methods);
   return 0;
}

