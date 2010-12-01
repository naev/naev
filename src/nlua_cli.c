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

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "mission.h"


/* CLI */
static int cli_missionStart( lua_State *L );
static int cli_missionTest( lua_State *L );
static const luaL_reg cli_methods[] = {
   { "missionStart", cli_missionStart },
   { "missionTest", cli_missionTest },
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

/**
 * @brief CLI generic Lua bindings.
 *
 * An example would be:
 * @code
 * cli.mission("Pirate Bounty") -- Triggers the Pirate Bounty mission.
 * @endcode
 *
 * @luamod cli
 */
/**
 * @brief Starts a mission without testing conditionals.
 *
 * @usage cli.missionStart( "Pirate Bounty" )
 *
 *    @luaparam misn Name of the mission to start.
 * @luafunc missionStart( misn )
 */
static int cli_missionStart( lua_State *L )
{
   const char *str;
   int ret;

   str = luaL_checkstring(L, 1);
   ret = mission_start( str, NULL );
   if (ret < 0) {
      NLUA_ERROR(L,"Failed to start mission.");
      return 0;
   }

   return 0;
}

/**
 * @brief Starts a mission by testing all the conditionals.
 *
 * @usage cli.missionTest( "Pirate Bounty" )
 *
 *    @luaparam misn Name of the mission to start.
 * @luafunc missionTest( misn )
 */
static int cli_missionTest( lua_State *L )
{
   const char *str;
   int ret;

   str = luaL_checkstring(L, 1);
   ret = mission_start( str, NULL );
   if (ret < 0) {
      NLUA_ERROR(L,"Failed to start mission.");
      return 0;
   }

   return 0;
}

