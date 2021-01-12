/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_naev.c
 *
 * @brief Contains Naev generic Lua bindings.
 */

/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_naev.h"

#include "input.h"
#include "land.h"
#include "log.h"
#include "nlua_evt.h"
#include "nlua_misn.h"
#include "nluadef.h"
#include "nstring.h"
#include "player.h"


/* Naev methods. */
static int naev_Lversion( lua_State *L );
static int naev_ticks( lua_State *L );
static int naev_keyGet( lua_State *L );
static int naev_keyEnable( lua_State *L );
static int naev_keyEnableAll( lua_State *L );
static int naev_keyDisableAll( lua_State *L );
static int naev_eventStart( lua_State *L );
static int naev_missionStart( lua_State *L );
static const luaL_Reg naev_methods[] = {
   { "version", naev_Lversion },
   { "ticks", naev_ticks },
   { "keyGet", naev_keyGet },
   { "keyEnable", naev_keyEnable },
   { "keyEnableAll", naev_keyEnableAll },
   { "keyDisableAll", naev_keyDisableAll },
   { "eventStart", naev_eventStart },
   { "missionStart", naev_missionStart },
   {0,0}
}; /**< Naev Lua methods. */


/**
 * @brief Loads the Naev Lua library.
 *
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadNaev( nlua_env env )
{
   nlua_register(env, "naev", naev_methods, 0);
   return 0;
}

/**
 * @brief Naev generic Lua bindings.
 *
 * An example would be:
 * @code
 * if naev.lang() == "en" then
 *    --Language is English.
 * end
 * @endcode
 *
 * @luamod naev
 */

/**
 * @brief Gets the version of Naev and the save game.
 *
 * @usage game_version, save_version = naev.version()
 *
 *    @luatreturn game_version The version of the game.
 *    @luatreturn save_version Version of current loaded save or nil if not loaded.
 * @luafunc version
 */
static int naev_Lversion( lua_State *L )
{
   lua_pushstring( L, naev_version(0) );
   if (player.loaded_version==NULL)
      lua_pushnil( L );
   else
      lua_pushstring( L, player.loaded_version );
   return 2;
}

/**
 * @brief Gets the SDL ticks.
 *
 * Useful for doing timing on Lua functions.
 *
 *    @luatreturn number The SDL ticks since the application started running.
 * @luafunc ticks
 */
static int naev_ticks( lua_State *L )
{
   lua_pushinteger(L, SDL_GetTicks());
   return 1;
}


/**
 * @brief Gets a human-readable name for the key bound to a function.
 *
 * @usage bindname = naev.keyGet( "accel" )
 *
 *    @luatparam string keyname Name of the keybinding to get value of. Valid values are listed in src/input.c: keybind_info.
 * @luafunc keyGet
 */
static int naev_keyGet( lua_State *L )
{
   const char *keyname;
   char buf[128];

   /* Get parameters. */
   keyname = luaL_checkstring( L, 1 );

   input_getKeybindDisplay( keyname, buf, sizeof(buf) );
   lua_pushstring( L, buf );

   return 1;
}


/**
 * @brief Disables or enables a specific keybinding.
 *
 * Use with caution, this can make the player get stuck.
 *
 * @usage naev.keyEnable( "accel", false ) -- Disables the acceleration key
 *    @luatparam string keyname Name of the key to disable (for example "accel").
 *    @luatparam[opt=false] boolean enable Whether to enable or disable.
 * @luafunc keyEnable
 */
static int naev_keyEnable( lua_State *L )
{
   const char *key;
   int enable;

   NLUA_CHECKRW(L);

   /* Parameters. */
   key = luaL_checkstring(L,1);
   enable = lua_toboolean(L,2);

   input_toggleEnable( key, enable );
   return 0;
}


/**
 * @brief Enables all inputs.
 *
 * @usage naev.keyEnableAll() -- Enables all inputs
 * @luafunc keyEnableAll
 */
static int naev_keyEnableAll( lua_State *L )
{
   NLUA_CHECKRW(L);
   input_enableAll();
   return 0;
}


/**
 * @brief Disables all inputs.
 *
 * @usage naev.keyDisableAll() -- Disables all inputs
 * @luafunc keyDisableAll
 */
static int naev_keyDisableAll( lua_State *L )
{
   NLUA_CHECKRW(L);
   input_disableAll();
   return 0;
}


/**
 * @brief Starts an event, does not start check conditions.
 *
 * @usage naev.eventStart( "Some Event" )
 *    @luatparam string evtname Name of the event to start.
 *    @luatreturn boolean true on success.
 * @luafunc eventStart
 */
static int naev_eventStart( lua_State *L )
{
   int ret;
   const char *str;

   NLUA_CHECKRW(L);

   str = luaL_checkstring(L, 1);
   ret = event_start( str, NULL );

   /* Get if console. */
   nlua_getenv(__NLUA_CURENV, "__cli");
   if (lua_toboolean(L,-1) && landed)
      bar_regen();
   lua_pop(L,1);

   lua_pushboolean( L, !ret );
   return 1;
}


/**
 * @brief Starts a mission, does no check start conditions.
 *
 * @usage naev.missionStart( "Some Mission" )
 *    @luatparam string misnname Name of the mission to start.
 *    @luatreturn boolean true on success.
 * @luafunc missionStart
 */
static int naev_missionStart( lua_State *L )
{
   int ret;
   const char *str;

   NLUA_CHECKRW(L);

   str = luaL_checkstring(L, 1);
   ret = mission_start( str, NULL );

   /* Get if console. */
   nlua_getenv(__NLUA_CURENV, "__cli");
   if (lua_toboolean(L,-1) && landed)
      bar_regen();
   lua_pop(L,1);

   lua_pushboolean( L, !ret );
   return 1;
}




