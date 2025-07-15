/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/** @cond */
#include <SDL3/SDL.h>

#include "naev.h"
/** @endcond */

#include "nlua.h"

#define FILE_METATABLE "file" /**< File metatable identifier. */

/**
 * @brief Wrapper to files.
 */
typedef struct LuaFile_s {
   char          path[PATH_MAX]; /**< Filename or path. */
   char          mode;
   size_t        size;
   SDL_IOStream *rw; /**< RWops. */
} LuaFile_t;

/*
 * Library loading
 */
int nlua_loadFile( nlua_env *env );
int nlua_loadFileNoEnv( lua_State *L );

/* Basic operations. */
LuaFile_t *lua_tofile( lua_State *L, int ind );
LuaFile_t *luaL_checkfile( lua_State *L, int ind );
LuaFile_t *lua_pushfile( lua_State *L, LuaFile_t file );
int        lua_isfile( lua_State *L, int ind );
