/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file cond.c
 *
 * @brief Handles lua conditionals.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "cond.h"

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

   cond_env = nlua_newEnv();
   if (nlua_loadStandard(cond_env)) {
      WARN(_("Failed to load standard Lua libraries."));
      return -1;
   }

   return 0;
}

/**
 * @brief Destroys the conditional subsystem.
 */
void cond_exit (void)
{
   nlua_freeEnv(cond_env);
   cond_env = LUA_NOREF;
}

/**
 * @brief Compiles a conditional statement that can then be used as a reference.
 *
 *    @param cond Conditional string to compile.
 *    @return LUA_NOREF on failure, a valid reference otherwise.
 */
int cond_compile( const char *cond )
{
   int ret, ref;
   char buf[STRMAX_SHORT];

   /* Load the string directly. */
   if (strstr( cond, "return" ) != NULL) {
      lua_pushstring(naevL, cond);
   }
   else {
      /* Append "return" first. */
      lua_pushstring(naevL, "return ");
      lua_pushstring(naevL, cond);
      lua_concat(naevL, 2);
   }
   ret = luaL_loadbuffer( naevL, lua_tostring(naevL,-1), lua_strlen(naevL,-1), "Lua Conditional" );
   switch (ret) {
      case  LUA_ERRSYNTAX:
         snprintf( buf, sizeof(buf), _("Lua conditional syntax error: %s"), lua_tostring(naevL, -1));
         goto cond_err;
      case LUA_ERRMEM:
         snprintf( buf, sizeof(buf), _("Lua Conditional ran out of memory: %s"), lua_tostring(naevL, -1));
         goto cond_err;
      default:
         break;
   }
   ref = luaL_ref( naevL, LUA_REGISTRYINDEX ); /* pops */
   lua_pop( naevL, 1 );
   return ref;

cond_err:
   print_with_line_numbers( cond );
   WARN( "%s", buf );

   /* Clear the stack. */
   lua_settop(naevL, 0);
   return LUA_NOREF;
}

/**
 * @brief Checks to see if a condition is true.
 *
 *    @param cond Condition to check.
 *    @return 0 if is false, 1 if is true, -1 on error.
 */
int cond_check( const char *cond )
{
   int ret;
   char buf[STRMAX_SHORT];

   /* Load the string directly. */
   if (strstr( cond, "return" ) != NULL) {
      lua_pushstring(naevL, cond);
   }
   else {
      /* Append "return" first. */
      lua_pushstring(naevL, "return ");
      lua_pushstring(naevL, cond);
      lua_concat(naevL, 2);
   }
   ret = nlua_dobufenv(cond_env, lua_tostring(naevL,-1),
                       lua_strlen(naevL,-1), "Lua Conditional");
   switch (ret) {
      case  LUA_ERRSYNTAX:
         snprintf( buf, sizeof(buf), _("Lua conditional syntax error: %s"), lua_tostring(naevL, -1));
         goto cond_err;
      case LUA_ERRRUN:
         snprintf( buf, sizeof(buf), _("Lua Conditional had a runtime error: %s"), lua_tostring(naevL, -1));
         goto cond_err;
      case LUA_ERRMEM:
         snprintf( buf, sizeof(buf), _("Lua Conditional ran out of memory: %s"), lua_tostring(naevL, -1));
         goto cond_err;
      case LUA_ERRERR:
         snprintf( buf, sizeof(buf), _("Lua Conditional had an error while handling error function: %s"), lua_tostring(naevL, -1));
         goto cond_err;
      default:
         break;
   }

   /* Check the result. */
   if (lua_isboolean(naevL, -1)) {
      ret = !!lua_toboolean(naevL, -1);
      lua_pop(naevL, 1);

      /* Clear the stack. */
      lua_settop(naevL, 0);

      return ret;
   }
   snprintf( buf, sizeof(buf), _("Lua Conditional didn't return a boolean"));

cond_err:
   print_with_line_numbers( cond );
   WARN( "%s", buf );

   /* Clear the stack. */
   lua_settop(naevL, 0);
   return -1;
}

int cond_checkChunk( int chunk, const char *cond )
{
   char buf[STRMAX_SHORT];
   int ret;

   if (chunk==LUA_NOREF) {
      WARN(_("Trying to run Lua Conditional chunk that is not referenced!"));
      return 0;
   }

   ret = nlua_dochunkenv( cond_env, chunk, "Lua Conditional" );
   switch (ret) {
      case LUA_ERRRUN:
         snprintf( buf, sizeof(buf), _("Lua Conditional had a runtime error: %s"), lua_tostring(naevL, -1));
         goto cond_err;
      case LUA_ERRMEM:
         snprintf( buf, sizeof(buf), _("Lua Conditional ran out of memory: %s"), lua_tostring(naevL, -1));
         goto cond_err;
      case LUA_ERRERR:
         snprintf( buf, sizeof(buf), _("Lua Conditional had an error while handling error function: %s"), lua_tostring(naevL, -1));
         goto cond_err;
      default:
         break;
   }

   /* Check the result. */
   if (lua_isboolean(naevL, -1)) {
      ret = !!lua_toboolean(naevL, -1);
      lua_pop(naevL, 1);

      /* Clear the stack. */
      lua_settop(naevL, 0);

      return ret;
   }
   snprintf( buf, sizeof(buf), _("Lua Conditional didn't return a boolean"));

cond_err:
   print_with_line_numbers( cond );
   WARN( "%s", buf );

   /* Clear the stack. */
   lua_settop(naevL, 0);
   return -1;
}
