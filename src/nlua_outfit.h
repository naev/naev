/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_OUTFIT_H
#  define NLUA_OUTFIT_H


#include <lua.h>

#include "outfit.h"
#include "nlua.h"


#define OUTFIT_METATABLE   "outfit" /**< Outfit metatable identifier. */


/*
 * Library loading
 */
int nlua_loadOutfit( nlua_env env );

/*
 * Outfit operations
 */
Outfit* lua_tooutfit( lua_State *L, int ind );
Outfit* luaL_checkoutfit( lua_State *L, int ind );
Outfit* luaL_validoutfit( lua_State *L, int ind );
Outfit** lua_pushoutfit( lua_State *L, Outfit* outfit );
int lua_isoutfit( lua_State *L, int ind );


#endif /* NLUA_OUTFIT_H */


