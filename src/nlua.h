/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <lauxlib.h> // IWYU pragma: export
#include <limits.h>  // IWYU pragma: export
#include <lua.h>     // IWYU pragma: export

#include "log.h" // IWYU pragma: export
/** @endcond */

#define NLUA_LOAD_TABLE                                                        \
   "_LOADED" /**< Table to use to store the status of required libraries. */

#define NLUA_DONE "__done__"
#define NLUA_DEPRECATED( L, f )                                                \
   do {                                                                        \
      lua_pushfstring( L, _( "Deprecated function call: %s" ), f );            \
      nlua_warn( L, -1 );                                                      \
   } while ( 0 )
#define NLUA_WARN( L, str, ... )                                               \
   do {                                                                        \
      lua_pushfstring( L, str, ##__VA_ARGS__ );                                \
      nlua_warn( L, -1 );                                                      \
   } while ( 0 )

#define nluaL_optarg( L, ind, def, checkfunc )                                 \
   ( lua_isnoneornil( L, ind ) ? ( def ) : checkfunc( L, ind ) )

typedef struct nlua_env nlua_env;
// typedef int       nlua_env;
extern lua_State *naevL;
extern nlua_env  *__NLUA_CURENV;

/*
 * standard Lua stuff wrappers
 */
void      lua_init( void );
void      lua_exit( void );
int       nlua_warn( lua_State *L, int idx );
void      lua_clearCache( void );
nlua_env *nlua_newEnv( const char *name );
nlua_env *nlua_dupEnv( nlua_env *env );
void      nlua_freeEnv( nlua_env *env );
void      nlua_pushenv( lua_State *L, nlua_env *env );
void      nlua_setenv( lua_State *L, nlua_env *env, const char *name );
void      nlua_getenv( lua_State *L, nlua_env *env, const char *name );
void      nlua_register( nlua_env *env, const char *libname, const luaL_Reg *l,
                         int metatable );
int       nlua_dobufenv( nlua_env *env, const char *buff, size_t sz,
                         const char *name );
int       nlua_dofileenv( nlua_env *env, const char *filename );
int       nlua_dochunkenv( nlua_env *env, int chunk, const char *name );
int       nlua_loadStandard( nlua_env *env );
int       nlua_errTrace( lua_State *L );
int       nlua_pcall( nlua_env *env, int nargs, int nresults );
int       nlua_refenv( nlua_env *env, const char *name );
int       nlua_refenvtype( nlua_env *env, const char *name, int type );
int       nlua_reffield( int objref, const char *name );
void      nlua_dumpstack( lua_State *L );

/* Reference stuff. */
int  nlua_ref( lua_State *L, int idx );
void nlua_unref( lua_State *L, int idx );

/* Hack to handle resizes. */
void nlua_resize( void );

/* Useful stuff that we want to reuse. */
int nlua_helperTags( lua_State *L, int idx, char *const *tags );

/* Loaders. */
int nlua_package_preload( lua_State *L );
int nlua_package_loader_lua( lua_State *L );
int nlua_package_loader_c( lua_State *L );
int nlua_package_loader_croot( lua_State *L );

#if DEBUGGING
void nlua_pushEnvTable( lua_State *L );
#endif /* DEBUGGING */

// Extra functionality exposed by mlua from newer versions of Lua via rust
const char *luaL_tolstring( lua_State *L, int idx, size_t *len );
void luaL_traceback( lua_State *L, lua_State *L1, const char *msg, int level );
