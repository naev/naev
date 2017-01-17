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


static nlua_env cond_env = LUA_NOREF; /** Conditional Lua env. */


/**
 * @brief Initializes the conditional subsystem.
 */
int cond_init (void)
{
   if (cond_env != LUA_NOREF)
      return 0;

   cond_env = nlua_newEnv(0);
   if (nlua_loadStandard(cond_env)) {
      WARN("Failed to load standard Lua libraries.");
      return -1;
   }

   return 0;
}


/**
 * @brief Destroys the conditional subsystem.
 */
void cond_exit (void)
{
   if (cond_env == LUA_NOREF)
      return;

   nlua_freeEnv(cond_env);
   cond_env = LUA_NOREF;
}


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
   ret = nlua_dobufenv(cond_env, lua_tostring(naevL,-1),
                       lua_strlen(naevL,-1), "Lua Conditional");
   switch (ret) {
      case  LUA_ERRSYNTAX:
         WARN("Lua conditional syntax error: %s", lua_tostring(naevL, -1));
         goto cond_err;
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
