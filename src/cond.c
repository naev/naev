/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file cond.c
 *
 * @brief Handles lua conditionals.
 */


#include "cond.h"

#include "naev.h"

#include "log.h"
#include "nlua.h"
#include "nluadef.h"


/**
 * @brief Checks to see if a condition is true.
 *
 *    @param cond Condition to check.
 *    @return 0 if is false, 1 if is true, -1 on error.
 */
int cond_check( const char* cond )
{
   int b;
   int ret;

   /* Load the string. */
   lua_pushstring(naevL, "return ");
   lua_pushstring(naevL, cond);
   lua_concat(naevL, 2);
   ret = luaL_loadbuffer(naevL, lua_tostring(naevL,-1),
         lua_strlen(naevL,-1), "Lua Conditional");
   switch (ret) {
      case  LUA_ERRSYNTAX:
         WARN("Lua conditional syntax error: %s", lua_tostring(naevL, -1));
         goto cond_err;
      case LUA_ERRMEM:
         WARN("Lua Conditional ran out of memory: %s", lua_tostring(naevL, -1));
         goto cond_err;
      default:
         break;
   }

   /* Run the string. */
   ret = lua_pcall( naevL, 0, 1, 0 );
   switch (ret) {
      case LUA_ERRRUN:
         WARN("Lua Conditional had a runtime error: %s", lua_tostring(naevL, -1));
         goto cond_err;
      case LUA_ERRMEM:
         WARN("Lua Conditional ran out of memory: %s", lua_tostring(naevL, -1));
         goto cond_err;
      case LUA_ERRERR:
         WARN("Lua Conditional had an error while handling error function: %s", lua_tostring(naevL, -1));
         goto cond_err;
      default:
         break;
   }

   /* Check the result. */
   if (lua_isboolean(naevL, -1)) {
      b = lua_toboolean(naevL, -1);
      lua_pop(naevL, 1);
      if (b)
         ret = 1;
      else
         ret = 0;

      /* Clear the stack. */
      lua_settop(naevL, 0);

      return ret;
   }
   WARN("Lua Conditional didn't return a boolean");

cond_err:
   /* Clear the stack. */
   lua_settop(naevL, 0);
   return -1;
}
