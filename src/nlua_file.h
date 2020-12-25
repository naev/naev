/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_FILE_H
#  define NLUA_FILE_H


/** @cond */
#include "SDL.h"
/** @endcond */

#include "nlua.h"


#define FILE_METATABLE      "file" /**< File metatable identifier. */


/**
 * @brief Wrapper to files.
 */
typedef struct LuaFile_s {
   char path[PATH_MAX]; /**< Filename or path. */
   char mode;
   size_t size;
   SDL_RWops *rw; /**< RWops. */
} LuaFile_t;


/*
 * Library loading
 */
int nlua_loadFile( nlua_env env );

/* Basic operations. */
LuaFile_t* lua_tofile( lua_State *L, int ind );
LuaFile_t* luaL_checkfile( lua_State *L, int ind );
LuaFile_t* lua_pushfile( lua_State *L, LuaFile_t file );
int lua_isfile( lua_State *L, int ind );


#endif /* NLUA_FILE_H */


