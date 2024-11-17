/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_file.c
 *
 * @brief Handles files.
 */
/** @cond */
#include <lauxlib.h>
/** @endcond */

#include "physfsrwops.h"

#include "nlua_file.h"

#include "nluadef.h"
#include "physfs.h"

/* File metatable methods. */
static int fileL_gc( lua_State *L );
static int fileL_eq( lua_State *L );
static int fileL_new( lua_State *L );
static int fileL_open( lua_State *L );
static int fileL_close( lua_State *L );
static int fileL_read( lua_State *L );
static int fileL_write( lua_State *L );
static int fileL_seek( lua_State *L );
static int fileL_name( lua_State *L );
static int fileL_mode( lua_State *L );
static int fileL_size( lua_State *L );
static int fileL_isopen( lua_State *L );
static int fileL_filetype( lua_State *L );
static int fileL_mkdir( lua_State *L );
static int fileL_enumerate( lua_State *L );
static int fileL_remove( lua_State *L );

static const luaL_Reg fileL_methods[] = {
   { "__gc", fileL_gc },
   { "__eq", fileL_eq },
   { "new", fileL_new },
   { "open", fileL_open },
   { "close", fileL_close },
   { "read", fileL_read },
   { "write", fileL_write },
   { "seek", fileL_seek },
   { "getFilename", fileL_name },
   { "getMode", fileL_mode },
   { "getSize", fileL_size },
   { "isOpen", fileL_isopen },
   { "filetype", fileL_filetype },
   { "mkdir", fileL_mkdir },
   { "enumerate", fileL_enumerate },
   { "remove", fileL_remove },
   { 0, 0 } }; /**< File metatable methods. */

/**
 * @brief Loads the file library.
 *
 *    @param env Environment to load file library into.
 *    @return 0 on success.
 */
int nlua_loadFile( nlua_env env )
{
   nlua_register( env, FILE_METATABLE, fileL_methods, 1 );
   return 0;
}

/**
 * @brief Lua bindings to interact with files.
 *
 * @note The API here is designed to be compatible with that of LÖVE.
 *
 * @luamod file
 */
/**
 * @brief Gets file at index.
 *
 *    @param L Lua state to get file from.
 *    @param ind Index position to find the file.
 *    @return File found at the index in the state.
 */
LuaFile_t *lua_tofile( lua_State *L, int ind )
{
   return (LuaFile_t *)lua_touserdata( L, ind );
}
/**
 * @brief Gets file at index or raises error if there is no file at index.
 *
 *    @param L Lua state to get file from.
 *    @param ind Index position to find file.
 *    @return File found at the index in the state.
 */
LuaFile_t *luaL_checkfile( lua_State *L, int ind )
{
   if ( lua_isfile( L, ind ) )
      return lua_tofile( L, ind );
   luaL_typerror( L, ind, FILE_METATABLE );
   return NULL;
}
/**
 * @brief Pushes a file on the stack.
 *
 *    @param L Lua state to push file into.
 *    @param file File to push.
 *    @return Newly pushed file.
 */
LuaFile_t *lua_pushfile( lua_State *L, LuaFile_t file )
{
   LuaFile_t *c = (LuaFile_t *)lua_newuserdata( L, sizeof( LuaFile_t ) );
   *c           = file;
   luaL_getmetatable( L, FILE_METATABLE );
   lua_setmetatable( L, -2 );
   return c;
}
/**
 * @brief Checks to see if ind is a file.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a file.
 */
int lua_isfile( lua_State *L, int ind )
{
   int ret;

   if ( lua_getmetatable( L, ind ) == 0 )
      return 0;
   lua_getfield( L, LUA_REGISTRYINDEX, FILE_METATABLE );

   ret = 0;
   if ( lua_rawequal( L, -1, -2 ) ) /* does it have the correct mt? */
      ret = 1;

   lua_pop( L, 2 ); /* remove both metatables */
   return ret;
}

/**
 * @brief Frees a file.
 *
 *    @luatparam File file File to free.
 * @luafunc __gc
 */
static int fileL_gc( lua_State *L )
{
   LuaFile_t *lf = luaL_checkfile( L, 1 );
   if ( lf->rw != NULL ) {
      SDL_RWclose( lf->rw );
      lf->rw = NULL;
   }
   return 0;
}

/**
 * @brief Compares two files to see if they are the same.
 *
 *    @luatparam File f1 File 1 to compare.
 *    @luatparam File f2 File 2 to compare.
 *    @luatreturn boolean true if both files are the same.
 * @luafunc __eq
 */
static int fileL_eq( lua_State *L )
{
   LuaFile_t *f1, *f2;
   f1 = luaL_checkfile( L, 1 );
   f2 = luaL_checkfile( L, 2 );
   lua_pushboolean( L, ( memcmp( f1, f2, sizeof( LuaFile_t ) ) == 0 ) );
   return 1;
}

/**
 * @brief Opens a new file.
 *
 *    @luatparam string path Path to open.
 *    @luatreturn File New file object.
 * @luafunc new
 */
static int fileL_new( lua_State *L )
{
   LuaFile_t   lf;
   const char *str = luaL_checkstring( L, 1 );
   memset( &lf, 0, sizeof( lf ) );
   strncpy( lf.path, str, sizeof( lf.path ) - 1 );
   lf.mode = 'c';
   lf.rw   = NULL;
   lua_pushfile( L, lf );
   return 1;
}

/**
 * @brief Opens a File object.
 *
 *    @luatparam File File object to open.
 *    @luatparam[opt="r"] mode Mode to open the file in (should be 'r', 'w', or
 * 'a').
 *    @luatreturn boolean true on success, false and an error string on failure.
 * @luafunc open
 */
static int fileL_open( lua_State *L )
{
   LuaFile_t  *lf   = luaL_checkfile( L, 1 );
   const char *mode = luaL_optstring( L, 2, "r" );

   /* TODO handle mode. */
   if ( strcmp( mode, "w" ) == 0 )
      lf->rw = PHYSFSRWOPS_openWrite( lf->path );
   else if ( strcmp( mode, "a" ) == 0 )
      lf->rw = PHYSFSRWOPS_openAppend( lf->path );
   else
      lf->rw = PHYSFSRWOPS_openRead( lf->path );
   if ( lf->rw == NULL ) {
      lua_pushboolean( L, 0 );
      lua_pushstring( L, SDL_GetError() );
      return 2;
   }
   lf->mode = mode[0];
   lf->size = (size_t)SDL_RWsize( lf->rw );

   lua_pushboolean( L, 1 );
   return 1;
}

/**
 * @brief Closes a file.
 *
 *    @luatparam File file File to close.
 *    @luatreturn true on success.
 * @luafunc close
 */
static int fileL_close( lua_State *L )
{
   LuaFile_t *lf = luaL_checkfile( L, 1 );
   if ( lf->rw != NULL ) {
      SDL_RWclose( lf->rw );
      lf->rw = NULL;
   }
   lf->mode = 'c';
   lua_pushboolean( L, 1 );
   return 1;
}

/**
 * @brief Reads from an open file.
 *
 *    @luatparam File file File to read from.
 *    @luatparam[opt] number bytes Number of bytes to read or all if ommitted.
 *    @luatreturn string Read data.
 *    @luatreturn number Number of bytes actually read.
 * @luafunc read
 */
static int fileL_read( lua_State *L )
{
   LuaFile_t *lf = luaL_checkfile( L, 1 );
   size_t     readlen, len;
   char      *buf;

   if ( lf->rw == NULL )
      return NLUA_ERROR( L, _( "file not open!" ) );

   /* Figure out how much to read. */
   readlen = luaL_optinteger( L, 2, SDL_RWsize( lf->rw ) );

   /* Create buffer and read into it. */
   buf = malloc( readlen );
   len = SDL_RWread( lf->rw, buf, 1, readlen );

   lua_pushlstring( L, buf, len );
   lua_pushinteger( L, len );
   free( buf );
   return 2;
}

/**
 * @brief Reads from an open file.
 *
 *    @luatparam File file File to write to.
 *    @luatparam string data Data to write.
 *    @luatparam[opt] number bytes Number of bytes to write.
 * @luafunc write
 */
static int fileL_write( lua_State *L )
{
   LuaFile_t  *lf = luaL_checkfile( L, 1 );
   size_t      write, wrote, len;
   const char *buf;

   if ( lf->rw == NULL )
      return NLUA_ERROR( L, _( "file not open!" ) );

   buf   = luaL_checklstring( L, 2, &len );
   write = luaL_optlong( L, 3, len );

   wrote = SDL_RWwrite( lf->rw, buf, 1, write );
   if ( wrote != write ) {
      lua_pushboolean( L, 0 );
      lua_pushstring( L, SDL_GetError() );
      return 2;
   }

   lua_pushboolean( L, 1 );
   return 1;
}

/**
 * @brief Seeks in an open file.
 *
 *    @luatparam File file File to seek in.
 *    @luatparam number pos Position to seek to (from start of file).
 *    @luatreturn boolean true on success.
 * @luafunc seek
 */
static int fileL_seek( lua_State *L )
{
   LuaFile_t *lf  = luaL_checkfile( L, 1 );
   size_t     pos = luaL_checkinteger( L, 2 );
   Sint64     ret;

   if ( lf->rw == NULL ) {
      lua_pushboolean( L, 1 );
      return 1;
   }

   ret = SDL_RWseek( lf->rw, pos, RW_SEEK_SET );

   lua_pushboolean( L, ret >= 0 );
   return 1;
}

/**
 * @brief Gets the name of a file object.
 *
 *    @luatparam File file File object to get name of.
 *    @luatreturn string Name of the file object.
 * @luafunc getFilename
 */
static int fileL_name( lua_State *L )
{
   LuaFile_t *lf = luaL_checkfile( L, 1 );
   lua_pushstring( L, lf->path );
   return 1;
}

/**
 * @brief Gets the mode a file is currently in.
 *
 *    @luatparam File file File to get mode of.
 *    @luatreturn string Mode of the file (either 'c', 'w', 'r', or 'a')
 * @luafunc getMode
 */
static int fileL_mode( lua_State *L )
{
   LuaFile_t *lf = luaL_checkfile( L, 1 );
   lua_pushlstring( L, &lf->mode, 1 );
   return 1;
}

/**
 * @brief Gets the size of a file (must be open).
 *
 *    @luatparam File file File to get the size of.
 *    @luatreturn number Size of the file.
 * @luafunc getSize
 */
static int fileL_size( lua_State *L )
{
   LuaFile_t *lf = luaL_checkfile( L, 1 );
   lua_pushinteger( L, lf->size );
   return 1;
}

/**
 * @brief Checks to see if a file is open.
 *
 *    @luatparam File file File to check to see if is open.
 *    @luatreturn boolean true if the file is open, false otherwise.
 * @luafunc isOpen
 */
static int fileL_isopen( lua_State *L )
{
   const LuaFile_t *lf = luaL_checkfile( L, 1 );
   lua_pushboolean( L, lf->rw != NULL );
   return 1;
}

/**
 * @brief Checks to see the filetype of a path.
 *
 *    @luatparam string path Path to check to see what type it is.
 *    @luatreturn string What type of file it is or nil if doesn't exist.
 * @luafunc filetype
 */
static int fileL_filetype( lua_State *L )
{
   const char *path = luaL_checkstring( L, 1 );
   PHYSFS_Stat path_stat;

   if ( !PHYSFS_stat( path, &path_stat ) ) {
      /* No need for warning, might not exist. */
      lua_pushnil( L );
      return 1;
   }
   if ( path_stat.filetype == PHYSFS_FILETYPE_REGULAR )
      lua_pushstring( L, "file" );
   else if ( path_stat.filetype == PHYSFS_FILETYPE_DIRECTORY )
      lua_pushstring( L, "directory" );
   else
      lua_pushnil( L );
   return 1;
}

/**
 * @brief Makes a directory.
 *
 *    @luatparam string dir Name of the directory to make.
 *    @luatreturn boolean True on success.
 * @luafunc mkdir
 */
static int fileL_mkdir( lua_State *L )
{
   const char *path = luaL_checkstring( L, 1 );
   int         ret  = PHYSFS_mkdir( path );
   lua_pushboolean( L, ret );
   if ( ret == 0 ) {
      lua_pushstring( L, PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
      return 2;
   }
   return 1;
}

/**
 * @brief Returns a list of files and subdirectories of a directory.
 *
 *    @luatparam string dir Name of the directory to check.
 *    @luatreturn table Table containing all the names (strings) of the
 * subdirectories and files in the directory.
 * @luafunc enumerate
 */
static int fileL_enumerate( lua_State *L )
{
   char      **items;
   const char *path = luaL_checkstring( L, 1 );

   lua_newtable( L );
   items = PHYSFS_enumerateFiles( path );
   if ( items == NULL )
      return NLUA_ERROR(
         L, _( "Directory '%s' enumerate error: %s" ), path,
         _( PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
   for ( int i = 0; items[i] != NULL; i++ ) {
      lua_pushstring( L, items[i] );
      lua_rawseti( L, -2, i + 1 );
   }
   PHYSFS_freeList( items );
   return 1;
}

/**
 * @brief Removes a file or directory.
 *
 *    @luatparam string path Name of the path to remove.
 *    @luatreturn boolean True on success.
 * @luafunc remove
 */
static int fileL_remove( lua_State *L )
{
   const char *path = luaL_checkstring( L, 1 );
   int         ret  = PHYSFS_delete( path );
   lua_pushboolean( L, ret );
   if ( ret == 0 ) {
      lua_pushstring( L, PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
      return 2;
   }
   return 1;
}
