/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"
#include "opengl.h"

#define TEX_METATABLE "tex" /**< Texture metatable identifier. */

/*
 * Library loading
 */
int nlua_loadTex( nlua_env );

/*
 * Texture operations
 */
glTexture  *lua_totex( lua_State *L, int ind );
glTexture  *luaL_checktex( lua_State *L, int ind );
glTexture **lua_pushtex( lua_State *L, glTexture *tex );
int         lua_istex( lua_State *L, int ind );
glTexture  *luaL_validtex( lua_State *L, int ind, const char *searchpath );
