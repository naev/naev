/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_data.c
 *
 * @brief Handles datas.
 */

#include "nlua_data.h"

#include "naev.h"

#include <lauxlib.h>

#include "nluadef.h"
#include "log.h"


/* Helper functions. */
static size_t dataL_checkpos( lua_State *L, LuaData_t *ld, long pos );


/* Data metatable methods. */
static int dataL_gc( lua_State *L );
static int dataL_eq( lua_State *L );
static int dataL_new( lua_State *L );
static int dataL_get( lua_State *L );
static int dataL_set( lua_State *L );
static int dataL_getSize( lua_State *L );
static int dataL_getString( lua_State *L );
static const luaL_Reg dataL_methods[] = {
   { "__gc", dataL_gc },
   { "__eq", dataL_eq },
   { "new", dataL_new },
   { "get", dataL_get },
   { "set", dataL_set },
   { "getSize", dataL_getSize },
   { "getString", dataL_getString },
   {0,0}
}; /**< Data metatable methods. */




/**
 * @brief Loads the data library.
 *
 *    @param env Environment to load data library into.
 *    @return 0 on success.
 */
int nlua_loadData( nlua_env env )
{
   nlua_register(env, DATA_METATABLE, dataL_methods, 1);
   return 0;
}


/**
 * @brief Lua bindings to interact with datas.
 *
 * @luamod data
 */
/**
 * @brief Gets data at index.
 *
 *    @param L Lua state to get data from.
 *    @param ind Index position to find the data.
 *    @return Data found at the index in the state.
 */
LuaData_t* lua_todata( lua_State *L, int ind )
{
   return (LuaData_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets data at index or raises error if there is no data at index.
 *
 *    @param L Lua state to get data from.
 *    @param ind Index position to find data.
 *    @return Data found at the index in the state.
 */
LuaData_t* luaL_checkdata( lua_State *L, int ind )
{
   if (lua_isdata(L,ind))
      return lua_todata(L,ind);
   luaL_typerror(L, ind, DATA_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a data on the stack.
 *
 *    @param L Lua state to push data into.
 *    @param data Data to push.
 *    @return Newly pushed data.
 */
LuaData_t* lua_pushdata( lua_State *L, LuaData_t data )
{
   LuaData_t *c;
   c = (LuaData_t*) lua_newuserdata(L, sizeof(LuaData_t));
   *c = data;
   luaL_getmetatable(L, DATA_METATABLE);
   lua_setmetatable(L, -2);
   return c;
}
/**
 * @brief Checks to see if ind is a data.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a data.
 */
int lua_isdata( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, DATA_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Frees a data.
 *
 *    @luatparam Data data Data to free.
 * @luafunc __gc( data )
 */
static int dataL_gc( lua_State *L )
{
   LuaData_t *ld = luaL_checkdata(L,1);
   free(ld->data);
   return 0;
}


/**
 * @brief Compares two datas to see if they are the same.
 *
 *    @luatparam Data d1 Data 1 to compare.
 *    @luatparam Data d2 Data 2 to compare.
 *    @luatreturn boolean true if both datas are the same.
 * @luafunc __eq( d1, d2 )
 */
static int dataL_eq( lua_State *L )
{
   LuaData_t *d1, *d2;
   d1 = luaL_checkdata(L,1);
   d2 = luaL_checkdata(L,2);
   if (d1->size != d2->size) {
      lua_pushboolean( L, 0 );
      return 1;
   }
   lua_pushboolean( L, (memcmp( d1->data, d2->data, d1->size)==0) );
   return 1;
}


/**
 * @brief Opens a new data.
 *
 *    @luatparam number size Size to allocate for data.
 *    @luatparam string type Type of the data to create ("number")
 *    @luatreturn Data New data object.
 * @luafunc new( size, type )
 */
static int dataL_new( lua_State *L )
{
   LuaData_t ld;
   size_t size = luaL_checklong(L,1);
   const char *type = luaL_checkstring(L,2);
   NLUA_CHECKRW(L);
   if (strcmp(type,"number")==0) {
      ld.type = LUADATA_NUMBER;
      ld.elem = sizeof(double);
      ld.size = size*ld.elem;
      ld.data = calloc( ld.elem, size );
   }
   else
      NLUA_ERROR(L, _("unknown data type '%s'"), type);
   lua_pushdata( L, ld );
   return 1;
}



static size_t dataL_checkpos( lua_State *L, LuaData_t *ld, long pos )
{
   size_t mpos;
   if (pos < 0)
      NLUA_ERROR(L, _("position argument must be positive!"));
   mpos = pos * ld->elem;
   if (mpos > ld->size)
      NLUA_ERROR(L, _("position argument out of bounds: %d of %d elements"), pos, ld->size/ld->elem);
   return mpos;
}



/**
 * @brief Gets the value of an element.
 *
 * @luafunc get( data, pos )
 */
static int dataL_get( lua_State *L )
{
   LuaData_t *ld = luaL_checkdata(L,1);
   long pos = luaL_checklong(L,2);
   size_t mpos = dataL_checkpos( L, ld, pos );
   switch (ld->type) {
      case LUADATA_NUMBER:
         lua_pushnumber(L, *((double*)&ld->data[mpos]));
         break;
   }
   return 1;
}


/**
 * @brief Sets the value of an element.
 *
 * @luafunc get( data, pos, value )
 */
static int dataL_set( lua_State *L )
{
   LuaData_t *ld = luaL_checkdata(L,1);
   long pos = luaL_checklong(L,2);
   size_t mpos = dataL_checkpos( L, ld, pos );
   double value;
   switch (ld->type) {
      case LUADATA_NUMBER:
         value = luaL_checknumber(L,3);
         *((double*)&ld->data[mpos]) = value;
         break;
   }
   return 0;
}


/**
 * @luafunc getSize( data )
 */
static int dataL_getSize( lua_State *L )
{
   LuaData_t *ld = luaL_checkdata(L,1);
   lua_pushnumber(L, ld->size);
   return 1;
}


/**
 * @luafunc getString( data )
 */
static int dataL_getString( lua_State *L )
{
   LuaData_t *ld = luaL_checkdata(L,1);
   lua_pushlstring(L, ld->data, ld->size);
   return 1;
}



