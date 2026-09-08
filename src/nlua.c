/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua.c
 *
 * @brief Handles creating and setting up basic Lua environments.
 */

#include "nlua.h"

#include "array.h"
#include "conf.h"
#include "console.h"
#include "debug.h"
#include "log.h"

typedef struct nlua_env nlua_env;

lua_State      *naevL         = NULL; /**< Global Naev Lua state. */
const nlua_env *__NLUA_CURENV = NULL; /**< Current environment. */
// static char *common_script; /**< Common script to run when creating
// environments. */
//  static size_t common_sz; /**< Common script size. */
// static int nlua_envs = LUA_NOREF;

/*
 * prototypes
 */
// static int        nlua_require( lua_State *L );
static lua_State *nlua_newState( void ); /* creates a new state */
// static int        nlua_loadBasic( lua_State *L );
static int nlua_errTraceInternal( lua_State *L, int idx );

/*
 * @brief Initializes the global Lua state.
 */
void lua_init( void )
{
   naevL = nlua_newState();
   // nlua_loadBasic( naevL );

   /* Environment table. */
   // lua_newtable( naevL );
   // nlua_envs = luaL_ref( naevL, LUA_REGISTRYINDEX );

   /* Better clean up. */
   // lua_atpanic( naevL, nlua_panic );
}

/*
 * @brief Closes the global Lua state.
 */
void lua_exit( void )
{
   // free( common_script );
   lua_close( naevL );
   naevL = NULL;
}

int nlua_warn( lua_State *L, int idx )
{
   const char *msg = luaL_checkstring( L, idx );
#if DEBUGGING
   nlua_errTraceInternal( L, idx );
   const char *dbgmsg = lua_tostring( L, -1 );
   LOGERR( "%s", dbgmsg );
   cli_printCoreString( dbgmsg, 1 );
   lua_pop( L, 1 );
#endif /* DEBUGGING */
   WARN( "%s", msg );
   /* Add to console. */
   cli_printCoreString( msg, 1 );
   return 0;
}

/*
 * @brief Run code from buffer in Lua environment.
 *
 *    @param env Lua environment.
 *    @param buff Pointer to buffer.
 *    @param sz Size of buffer.
 *    @param name Name to use in error messages.
 *    @return 0 on success.
 */
int nlua_dobufenv( nlua_env *env, const char *buff, size_t sz,
                   const char *name )
{
   int ret;
#if DEBUGGING
   /* We don't really want to do this, but Lua seems to trigger all sorts of
    * FPE exceptions on a daily basis.
    * TODO get rid of if possible. */
   if ( conf.fpu_except )
      debug_disableFPUExcept();
#endif /* DEBUGGING */
   ret = luaL_loadbuffer( naevL, buff, sz, name );
   if ( ret != 0 )
      return ret;
#if DEBUGGING
   if ( conf.fpu_except )
      debug_enableFPUExcept();
#endif /* DEBUGGING */

   return nlua_pcall( env, 0, LUA_MULTRET );
}

/*
 * @brief Run code a file in Lua environment.
 *
 *    @param env Lua environment.
 *    @param filename Filename of Lua script.
 */
int nlua_dofileenv( nlua_env *env, const char *filename )
{
   if ( luaL_loadfile( naevL, filename ) != 0 )
      return -1;
   if ( nlua_pcall( env, 0, LUA_MULTRET ) != 0 )
      return -1;
   return 0;
}

int nlua_loadbuffer( lua_State *L, const char *buff, size_t sz,
                     const char *name )
{
#if DEBUGGING
   /* We don't really want to do this, but Lua seems to trigger all sorts of
    * FPE exceptions on a daily basis.
    * TODO get rid of if possible. */
   if ( conf.fpu_except )
      debug_disableFPUExcept();
#endif /* DEBUGGING */
   int ret = luaL_loadbuffer( L, buff, sz, name );
#if DEBUGGING
   if ( conf.fpu_except )
      debug_enableFPUExcept();
#endif /* DEBUGGING */
   return ret;
}

/*
 * @brief Run code from chunk in Lua environment.
 *
 *    @param env Lua environment.
 *    @param chunk Chunk to run.
 *    @return 0 on success.
 */
int nlua_dochunkenv( nlua_env *env, int chunk, const char *name )
{
   (void)name;
   int ret;
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, chunk );
   ret = nlua_pcall( env, 0, LUA_MULTRET );
   if ( ret != 0 )
      return ret;
#if DEBUGGING
   lua_pushstring( naevL, name );
   nlua_setenv( naevL, env, "__name" );
#endif /* DEBUGGING */
   return 0;
}

/*
 * @brief Gets variable from environment and pushes it to stack
 *
 * This is meant to replace lua_getglobal()
 *
 *    @param env Environment.
 *    @param name Name of variable.
 */
void nlua_getenv( lua_State *L, const nlua_env *env, const char *name )
{
   nlua_pushenv( L, env );      /* env */
   lua_getfield( L, -1, name ); /* env, value */
   lua_remove( L, -2 );         /* value */
}

/*
 * @brief Pops a value from the stack and sets it in the environment.
 *
 * This is meant to replace lua_setglobal()
 *
 *    @param env Environment.
 *    @param name Name of variable.
 */
void nlua_setenv( lua_State *L, const nlua_env *env, const char *name )
{
   /* value */
   nlua_pushenv( L, env );      /* value, env */
   lua_insert( L, -2 );         /* env, value */
   lua_setfield( L, -2, name ); /* env */
   lua_pop( L, 1 );             /*  */
}

/*
 * @brief Registers C functions as Lua library in environment
 *
 * This is meant to replace luaL_register()
 *
 *    @param env Environment.
 *    @param libname Name of library table.
 *    @param l Array of functions to register.
 *    @param metatable Library will be used as metatable (so register __index).
 */
void nlua_register( nlua_env *env, const char *libname, const luaL_Reg *l,
                    int metatable )
{
   if ( luaL_newmetatable( naevL, libname ) ) {
      if ( metatable ) {
         lua_pushvalue( naevL, -1 );
         lua_setfield( naevL, -2, "__index" );
         lua_pushstring( naevL, libname );
         lua_setfield( naevL, -2, "__name" );
      }
      luaL_register( naevL, NULL, l );
   } /* lib */
   nlua_getenv( naevL, env, "naev" );  /* lib, naev */
   lua_pushvalue( naevL, -2 );         /* lib, naev, lib */
   lua_setfield( naevL, -2, libname ); /* lib, naev */
   lua_pop( naevL, 1 );                /* lib  */
   nlua_setenv( naevL, env, libname ); /* */
}

/**
 * @brief Wrapper around luaL_newstate.
 *
 *    @return A newly created `lua_State`.
 */
static lua_State *nlua_newState( void )
{
   /* Try to create the new state */
   lua_State *L = luaL_newstate();
   if ( L == NULL ) {
      WARN( _( "Failed to create new Lua state." ) );
      return NULL;
   }
   return L;
}

/**
 * @brief Gets a trace from Lua.
 */
int nlua_errTrace( lua_State *L )
{
   return nlua_errTraceInternal( L, 1 );
}

static int nlua_errTraceInternal( lua_State *L, int idx )
{
   /* Handle special done case. */
   const char *msg = luaL_tolstring( L, idx, NULL );
   if ( ( msg != NULL ) && ( strcmp( msg, NLUA_DONE ) == 0 ) )
      return 1;

   /* Otherwise obtain traceback. */
   luaL_traceback( L, L, msg, 1 );
   lua_remove( L, idx ); /* ret */
   return 1;
}

#if 0
/*
 * @brief Wrapper around `lua_pcall()` that handles errors and environments
 *
 *    @param env Environment.
 *    @param nargs Number of arguments to pass.
 *    @param nresults Number of return values to take.
 *    @return 0 on success, or -1 on error with message pushed on stack.
 */
int nlua_pcall( const nlua_env *env, int nargs, int nresults )
{
   int             errf, ret;
   const nlua_env *prev_env;

#if DEBUGGING
   errf = lua_gettop( naevL ) - nargs;
   lua_pushcfunction( naevL, nlua_errTrace );
   lua_insert( naevL, errf );

   /* We don't really want to do this, but Lua seems to trigger all sorts of
    * FPE exceptions on a daily basis.
    * TODO get rid of if possible. */
   if ( conf.fpu_except )
      debug_disableFPUExcept();
#else  /* DEBUGGING */
   errf = 0;
#endif /* DEBUGGING */

   prev_env      = __NLUA_CURENV;
   __NLUA_CURENV = env;

   /* Have to bypass metamethods. */
   lua_pushstring( naevL, "_ENV" );
   nlua_pushenv( naevL, env );
   lua_rawset( naevL, LUA_GLOBALSINDEX );
   ret = lua_pcall( naevL, nargs, nresults, errf );
   lua_pushstring( naevL, "_ENV" );
   if ( prev_env == NULL )
      lua_pushnil( naevL );
   else
      nlua_pushenv( naevL, prev_env );
   lua_rawset( naevL, LUA_GLOBALSINDEX );

   __NLUA_CURENV = prev_env;

#if DEBUGGING
   lua_remove( naevL, errf );

   if ( conf.fpu_except )
      debug_enableFPUExcept();
#endif /* DEBUGGING */

   return ret;
}
#endif

/**
 * @brief Gets the reference of a global in a Lua environment.
 *
 *    @param env Environment.
 *    @param name Name of the global to get.
 *    @return LUA_NOREF if no global found, reference otherwise.
 */
int nlua_refenv( nlua_env *env, const char *name )
{
   nlua_getenv( naevL, env, name );
   if ( !lua_isnil( naevL, -1 ) )
      return luaL_ref( naevL, LUA_REGISTRYINDEX );
   lua_pop( naevL, 1 );
   return LUA_NOREF;
}

/**
 * @brief Gets the reference of a global in a Lua environment if it matches a
 * type.
 *
 *    @param env Environment.
 *    @param name Name of the global to get.
 *    @param type Type to match, e.g., `LUA_TFUNCTION`.
 *    @return LUA_NOREF if no global found, reference otherwise.
 */
int nlua_refenvtype( nlua_env *env, const char *name, int type )
{
   nlua_getenv( naevL, env, name );
   if ( lua_type( naevL, -1 ) == type )
      return luaL_ref( naevL, LUA_REGISTRYINDEX );
   lua_pop( naevL, 1 );
   return LUA_NOREF;
}

/**
 * @brief Gets the reference to the specified field from an object reference.
 *
 *    @param objref Reference to the object to be indexed.
 *    @param name Name of the field to get.
 *    @return LUA_NOREF if no field found, reference otherwise.
 */
int nlua_reffield( int objref, const char *name )
{
   if ( objref == LUA_NOREF )
      return LUA_NOREF;
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, objref );
   lua_getfield( naevL, -1, name );
   lua_remove( naevL, -2 );
   if ( !lua_isnil( naevL, -1 ) )
      return luaL_ref( naevL, LUA_REGISTRYINDEX );
   lua_pop( naevL, 1 );
   return LUA_NOREF;
}

/**
 * @brief Creates a new reference to a Lua structure at a position.
 */
int nlua_ref( lua_State *L, int idx )
{
   lua_pushvalue( L, idx );
   return luaL_ref( L, LUA_REGISTRYINDEX );
}

/**
 * @brief Removes a reference set with nlua_ref.
 */
void nlua_unref( lua_State *L, int idx )
{
   if ( idx != LUA_NOREF )
      luaL_unref( L, LUA_REGISTRYINDEX, idx );
}

/**
 * @brief Helper function to deal with tags.
 */
int nlua_helperTags( lua_State *L, int idx, char *const *tags )
{
   if ( lua_isnoneornil( L, idx ) ) {
      lua_newtable( L );
      for ( int i = 0; i < array_size( tags ); i++ ) {
         lua_pushstring( L, tags[i] );
         lua_pushboolean( L, 1 );
         lua_rawset( L, -3 );
      }
      return 1;
   } else {
      const char *s = luaL_checkstring( L, idx );
      for ( int i = 0; i < array_size( tags ); i++ ) {
         if ( strcmp( s, tags[i] ) == 0 ) {
            lua_pushboolean( L, 1 );
            return 1;
         }
      }
      lua_pushboolean( L, 0 );
      return 1;
   }
}
