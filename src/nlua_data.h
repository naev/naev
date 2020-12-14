/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_DATA_H
#  define NLUA_DATA_H


#include <lua.h>

#include "nlua.h"


#define DATA_METATABLE      "data" /**< Data metatable identifier. */


typedef enum LuaDataType_e {
   LUADATA_NUMBER,
} LuaDataType_t;


/**
 * @brief Wrapper to datas.
 */
typedef struct LuaData_s {
   size_t size; /**< Size of buffer. */
   size_t elem; /**< Size of an element. */
   char *data; /**< Actually allocated data. */
   LuaDataType_t type; /**< Type of the data. */
} LuaData_t;


/*
 * Library loading
 */
int nlua_loadData( nlua_env env );

/* Basic operations. */
LuaData_t* lua_todata( lua_State *L, int ind );
LuaData_t* luaL_checkdata( lua_State *L, int ind );
LuaData_t* lua_pushdata( lua_State *L, LuaData_t data );
int lua_isdata( lua_State *L, int ind );


#endif /* NLUA_DATA_H */


