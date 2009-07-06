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


/* naev */
static int naev_lang( lua_State *L );
static const luaL_reg naev_methods[] = {
   { "lang", naev_lang },
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
