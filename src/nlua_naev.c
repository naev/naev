/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_naev.c
 *
 * @brief Contains NAEV generic Lua bindings.
 */

#include "nlua_naev.h"

#include "naev.h"

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "nstd.h"
#include "input.h"


/* NAEV methods. */
static int naev_lang( lua_State *L );
static int naev_keyGet( lua_State *L );
static int naev_keyEnable( lua_State *L );
static int naev_keyEnableAll( lua_State *L );
static int naev_keyDisableAll( lua_State *L );
static const luaL_reg naev_methods[] = {
   { "lang", naev_lang },
   { "keyGet", naev_keyGet },
   { "keyEnable", naev_keyEnable },
   { "keyEnableAll", naev_keyEnableAll },
   { "keyDisableAll", naev_keyDisableAll },
   {0,0}
}; /**< NAEV Lua methods. */


/**
 * @brief Loads the NAEV Lua library.
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
 * @brief NAEV generic Lua bindings.
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
 * @brief Gets the language NAEV is currently using.
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
            p += snprintf( &buf[p], sizeof(buf)-p, "%s + ", input_modToText(mod) );
         /* Print key. */
         if (nstd_isalpha(key))
            p += snprintf( &buf[p], sizeof(buf)-p, "%c", nstd_toupper(key) );
         else
            p += snprintf( &buf[p], sizeof(buf)-p, "%s", SDL_GetKeyName(key) );
         lua_pushstring( L, buf );
         break;

      case KEYBIND_JBUTTON:
         snprintf( buf, sizeof(buf), "joy button %d", key );
         lua_pushstring( L, buf );
         break;

      case KEYBIND_JAXISPOS:
         snprintf( buf, sizeof(buf), "joy axis %d-", key );
         lua_pushstring( L, buf );
         break;

      case KEYBIND_JAXISNEG:
         snprintf( buf, sizeof(buf), "joy axis %d+", key );
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
 *    @luaparam enable Whether to enable or disable (if ommitted disables).
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



