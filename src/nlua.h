/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_H
#  define NLUA_H


#include <lua.h>
#include <lauxlib.h>


#define NLUA_DONE       "__done__"

typedef int nlua_env;
extern lua_State *naevL;

/*
 * standard Lua stuff wrappers
 */
void lua_init(void);
void lua_exit(void);
nlua_env nlua_newEnv(void);
void nlua_freeEnv(nlua_env env);
void nlua_setenv(nlua_env env, const char *name);
void nlua_getenv(nlua_env env, const char *name);
void nlua_register(nlua_env env, const char *libname, const luaL_Reg *l);
int nlua_dobufenv(nlua_env env,
                  const char *buff,
                  size_t sz,
                  const char *name);
int nlua_dofileenv(nlua_env env, const char *filename);
lua_State *nlua_newState (void); /* creates a new state */
int nlua_load( lua_State* L, lua_CFunction f );
int nlua_loadBasic( lua_State* L );
int nlua_loadStandard( lua_State *L, int readonly );
int nlua_errTrace( lua_State *L );

#endif /* NLUA_H */
