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
 * @file nlua_tut.c
 *
 * @brief Contains tutorial-specific Lua bindings.
 */


#include "nlua_tut.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "nstd.h"
#include "menu.h"
#include "land.h"


/* Tutorial methods. */
static int tut_mainMenu( lua_State *L );
static const luaL_reg tut_methods[] = {
   { "main_menu", tut_mainMenu },
   {0,0}
}; /**< Tutorial Lua methods. */


/**
 * @brief Loads the Tutorial Lua library.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int nlua_loadTut( lua_State *L )
{
   luaL_register(L, "tut", tut_methods);
   return 0;
}

/**
 * @brief Tutorial generic Lua bindings.
 *
 * An example would be:
 * @code
 * if naev.lang() == "en" then
 *    --Language is English.
 * end
 * @endcode
 *
 * @luamod tut
 */
/**
 * @brief Opens the main menu.
 *
 * @note You can not exit while landed.
 *
 * @usage tut.main_menu() -- Won't return, will kill the Lua
 * @luafunc main_menu()
 */
static int tut_mainMenu( lua_State *L )
{
   if (landed) {
      NLUA_ERROR(L,"Can not go to main menu while landed.");
      return 0;
   }
   menu_main();
   lua_pushstring(L, "__done__");
   lua_error(L); /* shouldn't return */
   return 0;
}


