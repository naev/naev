/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_TIME_H
#  define NLUA_TIME_H


#include <lua.h>

#include "ntime.h"
#include "nlua.h"


#define TIME_METATABLE   "time" /**< Planet metatable identifier. */


/*
 * Library stuff.
 */
int nlua_loadTime( nlua_env env );

/*
 * Time operations.
 */
ntime_t* lua_totime( lua_State *L, int ind );
ntime_t* luaL_checktime( lua_State *L, int ind );
ntime_t* lua_pushtime( lua_State *L, ntime_t time );
ntime_t luaL_validtime( lua_State *L, int ind );
int lua_istime( lua_State *L, int ind );


#endif /* NLUA_TIME_H */


