/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include "SDL.h"
/** @endcond */

#include "nlua.h"

#include "opengl.h"

#define CANVAS_METATABLE      "canvas" /**< Canvas metatable identifier. */

/**
 * @brief Wrapper to canvass.
 */
typedef struct LuaCanvas_s {
   GLuint fbo;    /**< Frame buffer object. */
   glTexture *tex;/**< Texture object. */
} LuaCanvas_t;

/*
 * Library loading
 */
int nlua_loadCanvas( nlua_env env );

/* Basic operations. */
LuaCanvas_t* lua_tocanvas( lua_State *L, int ind );
LuaCanvas_t* luaL_checkcanvas( lua_State *L, int ind );
LuaCanvas_t* lua_pushcanvas( lua_State *L, LuaCanvas_t canvas );
int lua_iscanvas( lua_State *L, int ind );

/*
 * Misc helpers.
 */
int canvas_new( LuaCanvas_t *lc, int w, int h );
void canvas_reset (void);
