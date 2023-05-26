/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"

#define SPFX_METATABLE      "spfx" /**< SPFX metatable identifier. */

typedef int LuaSpfx_t;

/*
 * Library loading
 */
int nlua_loadSpfx( nlua_env env );

/* Basic operations. */
LuaSpfx_t* lua_tospfx( lua_State *L, int ind );
LuaSpfx_t* luaL_checkspfx( lua_State *L, int ind );
LuaSpfx_t* lua_pushspfx( lua_State *L, LuaSpfx_t spfx );
int lua_isspfx( lua_State *L, int ind );

/* Global stuff. */
void spfxL_clear (void);
void spfxL_exit (void);
void spfxL_setSpeed( double s );
void spfxL_setSpeedVolume( double v );
void spfxL_update( double dt );
void spfxL_renderbg( double dt );
void spfxL_rendermg( double dt );
void spfxL_renderfg( double dt );
