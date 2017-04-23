/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua.c
 *
 * @brief Handles creating and setting up basic Lua environments.
 */

#include "nlua.h"

#include "naev.h"

#include "nluadef.h"
#include "log.h"
#include "ndata.h"
#include "nlua_rnd.h"
#include "nlua_faction.h"
#include "nlua_var.h"
#include "nlua_naev.h"
#include "nlua_planet.h"
#include "nlua_system.h"
#include "nlua_jump.h"
#include "nlua_time.h"
#include "nlua_news.h"
#include "nlua_player.h"
#include "nlua_pilot.h"
#include "nlua_vec2.h"
#include "nlua_diff.h"
#include "nlua_outfit.h"
#include "nlua_commodity.h"
#include "nlua_cli.h"
#include "nstring.h"


lua_State *naevL = NULL;
nlua_env __NLUA_CURENV = LUA_NOREF;


/*
 * prototypes
 */
static int nlua_packfileLoader( lua_State* L );
lua_State *nlua_newState (void); /* creates a new state */
int nlua_loadBasic( lua_State* L );
int nlua_errTrace( lua_State *L );


/*
 * @brief Initializes the global Lua state.
 */
void lua_init(void) {
   naevL = nlua_newState();
   nlua_loadBasic(naevL);
}


/*
 * @brief Closes the global Lua state.
 */
void lua_exit(void) {
   lua_close(naevL);
   naevL = NULL;
}


/*
 * @brief Run code from buffer in Lua environment.
 *
 *    @param env Lua environment.
 *    @param buff Pointer to buffer.
 *    @param sz Size of buffer.
 *    @param name Name to use in error messages.
 */
int nlua_dobufenv(nlua_env env,
                  const char *buff,
                  size_t sz,
                  const char *name) {
   if (luaL_loadbuffer(naevL, buff, sz, name) != 0)
      return -1;
   nlua_pushenv(env);
   lua_setfenv(naevL, -2);
   if (nlua_pcall(env, 0, LUA_MULTRET) != 0)
      return -1;
   return 0;
}


/*
 * @brief Run code a file in Lua environment.
 *
 *    @param env Lua environment.
 *    @param filename Filename of Lua script.
 */
int nlua_dofileenv(nlua_env env, const char *filename) {
   if (luaL_loadfile(naevL, filename) != 0)
      return -1;
   nlua_pushenv(env);
   lua_setfenv(naevL, -2);
   if (nlua_pcall(env, 0, LUA_MULTRET) != 0)
      return -1;
   return 0;
}


/*
 * @brief Create an new environment in global Lua state.
 *
 * An "environment" is a table used with setfenv for sandboxing.
 *
 *    @param rw Load libraries in read/write mode.
 */
nlua_env nlua_newEnv(int rw) {
   nlua_env ref;
   lua_newtable(naevL);
   lua_pushvalue(naevL, -1);
   ref = luaL_ref(naevL, LUA_REGISTRYINDEX);

   /* Metatable */
   lua_newtable(naevL);
   lua_pushvalue(naevL, LUA_GLOBALSINDEX);
   lua_setfield(naevL, -2, "__index");
   lua_setmetatable(naevL, -2);

   /* Replace include() function with one that considers fenv */
   lua_pushvalue(naevL, -1);
   lua_pushcclosure(naevL, nlua_packfileLoader, 1);
   lua_setfield(naevL, -2, "include");

   /* Some code expect _G to be it's global state, so don't inherit it */
   lua_pushvalue(naevL, -1);
   lua_setfield(naevL, -2, "_G");

   lua_pushboolean(naevL, rw);
   lua_setfield(naevL, -2, "__RW");

   lua_pop(naevL, 1);
   return ref;
}


/*
 * @brief Frees an environment created with nlua_newEnv()
 *
 *    @param env Enviornment to free.
 */
void nlua_freeEnv(nlua_env env) {
   if (naevL != NULL)
      luaL_unref(naevL, LUA_REGISTRYINDEX, env);
}


/*
 * @brief Push environment table to stack
 * 
 *    @param env Environment.
 */
void nlua_pushenv(nlua_env env) {
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, env);
}


/*
 * @brief Gets variable from enviornment and pushes it to stack
 * 
 * This is meant to replace lua_getglobal()
 *
 *    @param env Environment.
 *    @param name Name of variable.
 */
void nlua_getenv(nlua_env env, const char *name) {
   nlua_pushenv(env); /* env */
   lua_getfield(naevL, -1, name); /* env, value */
   lua_remove(naevL, -2); /* value */
}


/*
 * @brief Pops a value from the stack and sets it in the environment.
 * 
 * This is meant to replace lua_setglobal()
 *
 *    @param env Environment.
 *    @param name Name of variable.
 */
void nlua_setenv(nlua_env env, const char *name) {
   /* value */
   nlua_pushenv(env); /* value, env */
   lua_insert(naevL, -2); /* env, value */
   lua_setfield(naevL, -2, name); /* env */
   lua_pop(naevL, 1); /*  */
}


/*
 * @brief Registers C functions as lua library in environment
 * 
 * This is meant to replace luaL_register()
 *
 *    @param env Environment.
 *    @param libname Name of library table.
 *    @param l Array of functions to register.
 *    @param metatable Library will be used as metatable (so register __index).
 */
void nlua_register(nlua_env env, const char *libname,
                   const luaL_Reg *l, int metatable) {
   if (luaL_newmetatable(naevL, libname)) {
      if (metatable) {
         lua_pushvalue(naevL,-1);
         lua_setfield(naevL,-2,"__index");
      }
      luaL_register(naevL, NULL, l);
   }
   nlua_setenv(env, libname);
}


/**
 * @brief Wrapper around luaL_newstate.
 *
 *    @return A newly created lua_State.
 */
lua_State *nlua_newState (void)
{
   lua_State *L;

   /* try to create the new state */
   L = luaL_newstate();
   if (L == NULL) {
      WARN("Failed to create new Lua state.");
      return NULL;
   }

   return L;
}


/**
 * @brief Loads specially modified basic stuff.
 *
 *    @param L Lua State to load the basic stuff into.
 *    @return 0 on success.
 */
int nlua_loadBasic( lua_State* L )
{
   int i;
   const char *override[] = { /* unsafe functions */
         "collectgarbage",
         "dofile",
         "getfenv",
         "load",
         "loadfile",
         "loadstring",
         "setfenv",
	 NULL
   };


   luaL_openlibs(L);

   /* replace non-safe functions */
   for (i=0; override[i]!=NULL; i++) {
      lua_pushnil(L);
      lua_setglobal(L, override[i]);
   }

   /* Override print to print in the console. */
   lua_register(L, "print", cli_print);
   lua_register(L, "warn",  cli_warn);

   /* add our own */
   lua_pushvalue(L, LUA_GLOBALSINDEX);
   lua_pushcclosure(L, nlua_packfileLoader, 1);
   lua_setglobal(L, "include");

   return 0;
}


/**
 * @brief include( string module )
 *
 * Loads a module into the current Lua state from inside the data file.
 *
 *    @param module Name of the module to load.
 *    @return The return value of the chunk, or true.
 */
static int nlua_packfileLoader( lua_State* L )
{
   const char *filename;
   char *path_filename;
   char *buf;
   int len;
   uint32_t bufsize;
   int envtab;

   /* Environment table to load module into */
   envtab = lua_upvalueindex(1);

   /* Get parameters. */
   filename = luaL_checkstring(L,1);

   /* Check to see if already included. */
   lua_getfield( L, envtab, "_include" ); /* t */
   if (!lua_isnil(L,-1)) {
      lua_getfield(L,-1,filename); /* t, f */
      /* Already included. */
      if (!lua_isnil(L,-1)) {
         lua_remove(L, -2); /* val */
         return 1;
      }
      lua_pop(L,2); /* */
   }
   /* Must create new _include table. */
   else {
      lua_newtable(L);              /* t */
      lua_setfield(L, envtab, "_include"); /* */
   }

   /* Try to locate the data directly */
   buf = NULL;
   if (ndata_exists( filename ))
      buf = ndata_read( filename, &bufsize );
   /* If failed to load or doesn't exist try again with INCLUDE_PATH prefix. */
   if (buf == NULL) {
      /* Try to locate the data in the data path */
      len           = strlen(LUA_INCLUDE_PATH)+strlen(filename)+2;
      path_filename = malloc( len );
      nsnprintf( path_filename, len, "%s%s", LUA_INCLUDE_PATH, filename );
      if (ndata_exists( path_filename ))
         buf = ndata_read( path_filename, &bufsize );
      free( path_filename );
   }

   /* Must have buf by now. */
   if (buf == NULL) {
      DEBUG("include(): %s not found in ndata.", filename);
      luaL_error(L, "include(): %s not found in ndata.", filename);
      return 1;
   }

   if (luaL_loadbuffer(L, buf, bufsize, filename) != 0) {
      lua_error(L);
      return 1;
   }

   lua_pushvalue(L, envtab);
   lua_setfenv(L, -2);

   /* run the buffer */
   if (lua_pcall(L, 0, 1, 0) != 0) {
      /* will push the current error from the dobuffer */
      lua_error(L);
      return 1;
   }

   /* Mark as loaded. */
   /* val */
   if (lua_isnil(L,-1)) {
      lua_pop(L, 1);
      lua_pushboolean(L, 1);
   }
   lua_getfield(L, envtab, "_include"); /* val, t */
   lua_pushvalue(L, -2); /* val, t, val */
   lua_setfield(L, -2, filename);   /* val, t */
   lua_pop(L, 1); /* val */

   /* cleanup, success */
   free(buf);
   return 1;
}


/**
 * @brief Loads the standard Naev Lua API.
 *
 * Loads the modules:
 *  - naev
 *  - var
 *  - space
 *    - planet
 *    - system
 *    - jumps
 *  - time
 *  - player
 *  - pilot
 *  - rnd
 *  - diff
 *  - faction
 *  - vec2
 *  - outfit
 *  - commodity
 *
 * Only is missing:
 *  - misn
 *  - tk
 *  - hook
 *  - music
 *  - ai
 *
 *    @param L Lua Environment to load modules into.
 *    @return 0 on success.
 */
int nlua_loadStandard( nlua_env env )
{
   int r;

   r = 0;
   r |= nlua_loadNaev(env);
   r |= nlua_loadVar(env);
   r |= nlua_loadPlanet(env);
   r |= nlua_loadSystem(env);
   r |= nlua_loadJump(env);
   r |= nlua_loadTime(env);
   r |= nlua_loadPlayer(env);
   r |= nlua_loadPilot(env);
   r |= nlua_loadRnd(env);
   r |= nlua_loadDiff(env);
   r |= nlua_loadFaction(env);
   r |= nlua_loadVector(env);
   r |= nlua_loadOutfit(env);
   r |= nlua_loadCommodity(env);
   r |= nlua_loadNews(env);

   return r;
}



/**
 * @brief Gets a trace from Lua.
 */
int nlua_errTrace( lua_State *L )
{
   /* Handle special done case. */
   const char *str = luaL_checkstring(L,1);
   if (strcmp(str,NLUA_DONE)==0)
      return 1;

   /* Otherwise execute "debug.traceback( str, int )". */
   lua_getglobal(L, "debug");
   if (!lua_istable(L, -1)) {
      lua_pop(L, 1);
      return 1;
   }
   lua_getfield(L, -1, "traceback");
   if (!lua_isfunction(L, -1)) {
      lua_pop(L, 2);
      return 1;
   }
   lua_pushvalue(L, 1);
   lua_pushinteger(L, 2);
   lua_call(L, 2, 1);
   return 1;
}


/*
 * @brief Wrapper around lua_pcall() that handles errors and enviornments
 * 
 *    @param env Environment.
 *    @param nargs Number of arguments to pass.
 *    @param nresults Number of return values to take.
 */
int nlua_pcall( nlua_env env, int nargs, int nresults ) {
   int errf, ret, top, prev_env;

#if DEBUGGING
   top = lua_gettop(naevL);
   lua_pushcfunction(naevL, nlua_errTrace);
   lua_insert(naevL, -2-nargs);
   errf = -2-nargs;
#else /* DEBUGGING */
   errf = 0;
#endif /* DEBUGGING */

   prev_env = __NLUA_CURENV;
   __NLUA_CURENV = env;

   ret = lua_pcall(naevL, nargs, nresults, errf);

   __NLUA_CURENV = prev_env;

#if DEBUGGING
   lua_remove(naevL, top-nargs);
#endif /* DEBUGGING */

   return ret;
}
