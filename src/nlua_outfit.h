/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_OUTFIT_H
#  define NLUA_OUTFIT_H


#include "lua.h"

#include "outfit.h"


#define OUTFIT_METATABLE   "outfit" /**< Outfit metatable identifier. */


/**
 * @brief Lua Outfit wrapper.
 */
typedef struct LuaOutfit_s {
   Outfit *outfit; /**< Outfit pointer. */
} LuaOutfit; /**< Wrapper for a Outfit. */


/*
 * Library loading
 */
int nlua_loadOutfit( lua_State *L, int readonly );

/*
 * Outfit operations
 */
LuaOutfit* lua_tooutfit( lua_State *L, int ind );
LuaOutfit* luaL_checkoutfit( lua_State *L, int ind );
Outfit* luaL_validoutfit( lua_State *L, int ind );
LuaOutfit* lua_pushoutfit( lua_State *L, LuaOutfit outfit );
int lua_isoutfit( lua_State *L, int ind );


#endif /* NLUA_OUTFIT_H */


