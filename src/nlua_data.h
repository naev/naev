/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nlua.h"

#define DATA_METATABLE "data" /**< Data metatable identifier. */

typedef enum LuaDataType_e {
   LUADATA_NUMBER,
} LuaDataType_t;

/**
 * @brief Wrapper to datas.
 */
typedef struct LuaData_s {
   size_t        size; /**< Size of buffer (already multiplied by elem). */
   size_t        elem; /**< Size of an element. */
   void         *data; /**< Actually allocated data. */
   LuaDataType_t type; /**< Type of the data. */
} LuaData_t;

/*
 * Library loading
 */
int nlua_loadData( nlua_env env );

/* Basic operations. */
LuaData_t *lua_todata( lua_State *L, int ind );
LuaData_t *luaL_checkdata( lua_State *L, int ind );
LuaData_t *lua_pushdata( lua_State *L, LuaData_t data );
int        lua_isdata( lua_State *L, int ind );
