/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_TIME_H
#  define NLUA_TIME_H


#include "lua.h"

#include "ntime.h"


#define TIME_METATABLE   "time" /**< Planet metatable identifier. */


/**
 * @brief Wrapper for ntime_t.
 */
typedef struct LuaTime_s {
   ntime_t t; /**< Wrapped time. */
} LuaTime;


/*
 * Library stuff.
 */
int nlua_loadTime( lua_State *L, int readonly );

/*
 * Time operations.
 */
LuaTime* lua_totime( lua_State *L, int ind );
LuaTime* luaL_checktime( lua_State *L, int ind );
LuaTime* lua_pushtime( lua_State *L, LuaTime time );
ntime_t luaL_validtime( lua_State *L, int ind );
int lua_istime( lua_State *L, int ind );


#endif /* NLUA_TIME_H */


