/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_FONT_H
#  define NLUA_FONT_H


#include "font.h"
#include "nlua.h"


#define FONT_METATABLE      "font" /**< Font metatable identifier. */


/*
 * Library loading
 */
int nlua_loadFont( nlua_env env );

/*
 * Font operations
 */
glFont* lua_tofont( lua_State *L, int ind );
glFont* luaL_checkfont( lua_State *L, int ind );
glFont* lua_pushfont( lua_State *L, glFont font );
int lua_isfont( lua_State *L, int ind );


#endif /* NLUA_FONT_H */


