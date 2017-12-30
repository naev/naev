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
extern nlua_env __NLUA_CURENV;

/*
 * standard Lua stuff wrappers
 */
void lua_init(void);
void lua_exit(void);
nlua_env nlua_newEnv(int rw);
void nlua_freeEnv(nlua_env env);
void nlua_pushenv(nlua_env env);
void nlua_setenv(nlua_env env, const char *name);
void nlua_getenv(nlua_env env, const char *name);
void nlua_register(nlua_env env, const char *libname,
                   const luaL_Reg *l, int metatable);
int nlua_dobufenv(nlua_env env,
                  const char *buff,
                  size_t sz,
                  const char *name);
int nlua_dofileenv(nlua_env env, const char *filename);
int nlua_loadStandard( nlua_env env );
int nlua_pcall( nlua_env env, int nargs, int nresults );

#endif /* NLUA_H */
