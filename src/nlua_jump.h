/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_JUMP_H
#  define NLUA_JUMP_H


#include <lua.h>

#include "space.h"
#include "nlua.h"


#define JUMP_METATABLE   "jump" /**< Lua metatable identifier. */


/**
 * @brief Lua jump Wrapper.
 */
typedef struct LuaJump_s {
   int srcid;  /**< Starting star system ID. */
   int destid; /**< Destination star system ID. */
} LuaJump;


/*
 * Load the jump library.
 */
int nlua_loadJump( nlua_env env );

/*
 * Jump operations.
 */
LuaJump* lua_tojump( lua_State *L, int ind );
LuaJump* luaL_checkjump( lua_State *L, int ind );
LuaJump* lua_pushjump( lua_State *L, LuaJump jump );
JumpPoint* luaL_validjump( lua_State *L, int ind );
int lua_isjump( lua_State *L, int ind );


#endif /* NLUA_JUMP_H */

