/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file mission.c
 *
 * @brief Handles missions.
 */


#include "cond.h"

#include "naev.h"

#include "log.h"
#include "nlua.h"
#include "nluadef.h"


static lua_State *cond_L = NULL; /** Conditional Lua state. */


/**
 * @brief Initializes the conditional subsystem.
 */
int cond_init (void)
{
   if (cond_L != NULL)
      return 0;

   cond_L = nlua_newState();
   if (nlua_loadStandard(cond_L,1)) {
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
   if (cond_L == NULL)
      return;

   lua_close(cond_L);
   cond_L = NULL;
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
   lua_pushstring(cond_L, "return ");
   lua_pushstring(cond_L, cond);
   lua_concat(cond_L, 2);
   ret = luaL_loadbuffer(cond_L, lua_tostring(cond_L,-1),
         lua_strlen(cond_L,-1), "Lua Conditional");
   switch (ret) {
      case  LUA_ERRSYNTAX:
         WARN("Lua conditional syntax error: %s", lua_tostring(cond_L, -1));
         goto cond_err;
      case LUA_ERRMEM:
         WARN("Lua Conditional ran out of memory: %s", lua_tostring(cond_L, -1));
         goto cond_err;
      default:
         break;
   }

   /* Run the string. */
   ret = lua_pcall( cond_L, 0, 1, 0 );
   switch (ret) {
      case LUA_ERRRUN:
         WARN("Lua Conditional had a runtime error: %s", lua_tostring(cond_L, -1));
         goto cond_err;
      case LUA_ERRMEM:
         WARN("Lua Conditional ran out of memory: %s", lua_tostring(cond_L, -1));
         goto cond_err;
      case LUA_ERRERR:
         WARN("Lua Conditional had an error while handling error function: %s", lua_tostring(cond_L, -1));
         goto cond_err;
      default:
         break;
   }

   /* Check the result. */
   if (lua_isboolean(cond_L, -1)) {
      b = lua_toboolean(cond_L, -1);
      lua_pop(cond_L, 1);
      if (b)
         ret = 1;
      else
         ret = 0;

      /* Clear the stack. */
      lua_settop(cond_L, 0);

      return ret;
   }
   WARN("Lua Conditional didn't return a boolean");

cond_err:
   /* Clear the stack. */
   lua_settop(cond_L, 0);
   return -1;
}
