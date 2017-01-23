/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_tut.c
 *
 * @brief Contains tutorial-specific Lua bindings.
 */

#include "nlua_tut.h"

#include "naev.h"

#include <lauxlib.h>

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
int nlua_loadTut( nlua_env env )
{
   nlua_register(env, "tut", tut_methods, 0);
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


