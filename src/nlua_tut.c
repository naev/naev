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

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "nstd.h"
#include "menu.h"


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
 * @usage tut.main_menu() -- Won't return, will kill the Lua
 * @luafunc main_menu()
 */
static int tut_mainMenu( lua_State *L )
{
   menu_main();
   lua_pushstring(L, "__done__");
   lua_error(L); /* shouldn't return */
   return 0;
}


