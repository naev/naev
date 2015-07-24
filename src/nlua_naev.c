/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_naev.c
 *
 * @brief Contains Naev generic Lua bindings.
 */

#include "nlua_naev.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "nlua_evt.h"
#include "nlua_misn.h"
#include "log.h"
#include "nstd.h"
#include "input.h"
#include "land.h"
#include "nstring.h"


/* Naev methods. */
static int naev_lang( lua_State *L );
static int naev_ticks( lua_State *L );
static int naev_keyGet( lua_State *L );
static int naev_keyEnable( lua_State *L );
static int naev_keyEnableAll( lua_State *L );
static int naev_keyDisableAll( lua_State *L );
static int naev_eventStart( lua_State *L );
static int naev_missionStart( lua_State *L );
static const luaL_reg naev_methods[] = {
   { "lang", naev_lang },
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
 *    @param L Lua state.
 *    @return 0 on success.
 */
int nlua_loadNaev( lua_State *L )
{
   luaL_register(L, "naev", naev_methods);
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
 * @brief Gets the language Naev is currently using.
 *
 * @usage if naev.lang() == "en" then -- Language is english
 *
 *    @luareturn Two character identifier of the language.
 * @luafunc lang()
 */
static int naev_lang( lua_State *L )
{
   /** @todo multilanguage stuff */
   lua_pushstring(L,"en");
   return 1;
}

/**
 * @brief Gets the SDL ticks.
 *
 * Useful for doing timing on Lua functions.
 *
 *    @luareturn The SDL ticks since the application started running.
 * @luafunc ticks()
 */
static int naev_ticks( lua_State *L )
{
   lua_pushinteger(L, SDL_GetTicks());
   return 1;
}


/**
 * @brief Gets the keybinding value by name.
 *
 * @usage bindname = naev.keyGet( "accel" )
 *
 *    @luaparam keyname Name of the keybinding to get value of.
 * @luafunc keyGet( keyname )
 */
static int naev_keyGet( lua_State *L )
{
   int p;
   const char *keyname;
   SDLKey key;
   KeybindType type;
   SDLMod mod;
   char buf[128];

   /* Get parameters. */
   keyname = luaL_checkstring( L, 1 );

   /* Get the keybinding. */
   key = input_getKeybind( keyname, &type, &mod );

   /* Handle type. */
   switch (type) {
      case KEYBIND_NULL:
         lua_pushstring( L, "Not bound" );
         break;

      case KEYBIND_KEYBOARD:
         p = 0;
         /* Handle mod. */
         if ((mod != NMOD_NONE) && (mod != NMOD_ALL))
            p += nsnprintf( &buf[p], sizeof(buf)-p, "%s + ", input_modToText(mod) );
         /* Print key. */
         if (nstd_isalpha(key))
            p += nsnprintf( &buf[p], sizeof(buf)-p, "%c", nstd_toupper(key) );
         else
            p += nsnprintf( &buf[p], sizeof(buf)-p, "%s", SDL_GetKeyName(key) );
         lua_pushstring( L, buf );
         break;

      case KEYBIND_JBUTTON:
         nsnprintf( buf, sizeof(buf), "joy button %d", key );
         lua_pushstring( L, buf );
         break;

      case KEYBIND_JAXISPOS:
         nsnprintf( buf, sizeof(buf), "joy axis %d-", key );
         lua_pushstring( L, buf );
         break;

      case KEYBIND_JAXISNEG:
         nsnprintf( buf, sizeof(buf), "joy axis %d+", key );
         lua_pushstring( L, buf );
         break;
   }

   return 1;
}


/**
 * @brief Disables or enables a specific keybinding.
 *
 * Use with caution, this can make the player get stuck.
 *
 * @usage naev.keyEnable( "accel", false ) -- Disables the acceleration key
 *    @luaparam keyname Name of the key to disable (for example "accel").
 *    @luaparam enable Whether to enable or disable (if omitted disables).
 * @luafunc keyEnable( keyname, enable )
 */
static int naev_keyEnable( lua_State *L )
{
   const char *key;
   int enable;

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
 * @luafunc keyEnableAll()
 */
static int naev_keyEnableAll( lua_State *L )
{
   (void) L;
   input_enableAll();
   return 0;
}


/**
 * @brief Disables all inputs.
 *
 * @usage naev.keyDisableAll() -- Disables all inputs
 * @luafunc keyDisableAll()
 */
static int naev_keyDisableAll( lua_State *L )
{
   (void) L;
   input_disableAll();
   return 0;
}


/**
 * @brief Starts an event, does not start check conditions.
 *
 * @usage naev.eventStart( "Some Event" )
 *    @luaparam evtname Name of the event to start.
 *    @luareturn true on success.
 * @luafunc eventStart( evtname )
 */
static int naev_eventStart( lua_State *L )
{
   int ret;
   const char *str;

   str = luaL_checkstring(L, 1);
   ret = event_start( str, NULL );

   /* Get if console. */
   lua_getglobal(L, "__cli");
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
 *    @luaparam misnname Name of the mission to start.
 *    @luareturn true on success.
 * @luafunc missionStart( misnname )
 */
static int naev_missionStart( lua_State *L )
{
   int ret;
   const char *str;

   str = luaL_checkstring(L, 1);
   ret = mission_start( str, NULL );

   /* Get if console. */
   lua_getglobal(L, "__cli");
   if (lua_toboolean(L,-1) && landed)
      bar_regen();
   lua_pop(L,1);

   lua_pushboolean( L, !ret );
   return 1;
}





