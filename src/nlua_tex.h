/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_TEX_H
#  define NLUA_TEX_H


#include <lua.h>

#include "opengl.h"


#define TEX_METATABLE      "tex" /**< Texture metatable identifier. */


/*
 * Library loading
 */
int nlua_loadTex( lua_State *L, int readonly );

/*
 * Texture operations
 */
glTexture* lua_totex( lua_State *L, int ind );
glTexture* luaL_checktex( lua_State *L, int ind );
glTexture** lua_pushtex( lua_State *L, glTexture* tex );
int lua_istex( lua_State *L, int ind );


#endif /* NLUA_TEX_H */


