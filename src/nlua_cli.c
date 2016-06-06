/*
 * See Licensing and Copyright notice in naev.h
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

