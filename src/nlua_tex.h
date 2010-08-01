/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_TEX_H
#  define NLUA_TEX_H


#include "lua.h"

#include "opengl.h"


#define TEX_METATABLE      "tex" /**< GFX metatable identifier. */


/**
 * @brief Lua texture wrapper.
 */
typedef struct LuaTex_s {
   glTexture *tex;
} LuaTex; /**< Wrapper for a texture. */


/*
 * Library loading
 */
int nlua_loadTex( lua_State *L, int readonly );

/*
 * Texture operations
 */
LuaTex* lua_totex( lua_State *L, int ind );
LuaTex* luaL_checktex( lua_State *L, int ind );
LuaTex* lua_pushtex( lua_State *L, LuaTex tex );
int lua_istex( lua_State *L, int ind );


#endif /* NLUA_TEX_H */


