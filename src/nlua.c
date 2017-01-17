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
#include "nlua_space.h"
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


/*
 * prototypes
 */
static int nlua_packfileLoader( lua_State* L );
lua_State *nlua_newState (void); /* creates a new state */


void lua_init(void) {
   naevL = nlua_newState();
   nlua_loadBasic(naevL);
   nlua_loadStandard(naevL, 0); /* XXX read-only API */
}


void lua_exit(void) {
   lua_close(naevL);
   naevL = NULL;
}


int nlua_dobufenv(nlua_env env,
                  const char *buff,
                  size_t sz,
                  const char *name) {
   if (luaL_loadbuffer(naevL, buff, sz, name) != 0)
      return -1;
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, env);
   lua_setfenv(naevL, -2);
   if (lua_pcall(naevL, 0, LUA_MULTRET, 0) != 0)
      return -1;
   return 0;
}


int nlua_dofileenv(nlua_env env, const char *filename) {
   if (luaL_loadfile(naevL, filename) != 0)
      return -1;
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, env);
   lua_setfenv(naevL, -2);
   if (lua_pcall(naevL, 0, LUA_MULTRET, 0) != 0)
      return -1;
   return 0;
}


nlua_env nlua_newEnv(void) {
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

   lua_pop(naevL, 1);
   return ref;
}


void nlua_freeEnv(nlua_env env) {
   if (naevL != NULL)
   	luaL_unref(naevL, LUA_REGISTRYINDEX, env);
}


void nlua_getenv(nlua_env env, const char *name) {
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, env); /* env */
   lua_getfield(naevL, -1, name); /* env, value */
   lua_remove(naevL, -2); /* value */
}


void nlua_setenv(nlua_env env, const char *name) {
   /* value */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, env); /* value, env */
   lua_insert(naevL, -2); /* env, value */
   lua_setfield(naevL, -2, name); /* env */
   lua_pop(naevL, 1); /*  */
}


void nlua_register(nlua_env env, const char *libname, const luaL_Reg *l) {
   lua_newtable(naevL);
   luaL_register(naevL, NULL, l);
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
 * @brief Opens a Lua library.
 *
 *    @param L Lua state to load the library into.
 *    @param f CFunction to load.
 */
int nlua_load( lua_State* L, lua_CFunction f )
{
   lua_pushcfunction(L, f);
   if (lua_pcall(L, 0, 0, 0))
      WARN("nlua include error: %s",lua_tostring(L,1));

   return 0;
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
         "getmetatable",
         "load",
         "loadfile",
         "loadstring",
         "rawequal",
         "rawget",
         "rawset",
         "setfenv",
         /*"setmetatable",*/
         "END"
   };


   luaL_openlibs(L);

   /* replace non-safe functions */
   for (i=0; strcmp(override[i],"END")!=0; i++) {
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
 *    @return An error string on error.
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
         lua_pop(L,2); /* */
         return 0;
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
      lua_pushfstring(L, "%s not found in ndata.", filename);
      return 1;
   }

   if (luaL_loadbuffer(L, buf, bufsize, filename) != 0) {
      lua_error(L);
      return 1;
   }

   lua_pushvalue(L, envtab);
   lua_setfenv(L, -2);

   /* run the buffer */
   if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0) {
      /* will push the current error from the dobuffer */
      lua_error(L);
      return 1;
   }

   /* Mark as loaded. */
   lua_getfield(L, envtab, "_include"); /* t */
   lua_pushboolean(L, 1);           /* t b */
   lua_setfield(L, -2, filename);   /* t */
   lua_pop(L, 1);

   /* cleanup, success */
   free(buf);
   return 0;
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
 *    @param L Lua State to load modules into.
 *    @param readonly Load as readonly (good for sandboxing).
 *    @return 0 on success.
 */
int nlua_loadStandard( lua_State *L, int readonly )
{
   int r;

   r = 0;
   r |= nlua_loadBasic(L);
   r |= nlua_loadNaev(L);
   r |= nlua_loadVar(L,readonly);
   r |= nlua_loadSpace(L,readonly); /* systems, planets, jumps */
   r |= nlua_loadTime(L,readonly);
   r |= nlua_loadPlayer(L,readonly);
   r |= nlua_loadPilot(L,readonly);
   r |= nlua_loadRnd(L);
   r |= nlua_loadDiff(L,readonly);
   r |= nlua_loadFaction(L,readonly);
   r |= nlua_loadVector(L);
   r |= nlua_loadOutfit(L,readonly);
   r |= nlua_loadCommodity(L,readonly);
   r |= nlua_loadNews(L,readonly);

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


int nlua_pcall( nlua_env env, int nargs, int nresults ) {
   int errf, ret;

#if DEBUGGING
   lua_pushcfunction(naevL, nlua_errTrace);
   lua_insert(naevL, -2-nargs);
   errf = -2-nargs;
#else /* DEBUGGING */
   errf = 0;
#endif /* DEBUGGING */

   nlua_getenv(env, "__RW");
   lua_setglobal(naevL, "__RW");

   ret = lua_pcall(naevL, nargs, nresults, errf);

   lua_pushnil(naevL);
   lua_setglobal(naevL, "__RW");

#if DEBUGGING
   lua_remove(naevL, -1-nresults);
#endif /* DEBUGGING */

   return ret;
}
