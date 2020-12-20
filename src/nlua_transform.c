/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_transform.c
 *
 * @brief Handles transforms.
 */

#include "nlua_transform.h"

#include "naev.h"

#include <lauxlib.h>

#include "ndata.h"
#include "nluadef.h"
#include "log.h"


/* Transform metatable methods. */
static int transformL_eq( lua_State *L );
static int transformL_new( lua_State *L );
static const luaL_Reg transformL_methods[] = {
   { "__eq", transformL_eq },
   { "new", transformL_new },
   {0,0}
}; /**< Transform metatable methods. */


/**
 * @brief Loads the transform library.
 *
 *    @param env Environment to load transform library into.
 *    @return 0 on success.
 */
int nlua_loadTransform( nlua_env env )
{
   nlua_register(env, TRANSFORM_METATABLE, transformL_methods, 1);
   return 0;
}


/**
 * @brief Lua bindings to interact with transforms.
 *
 * @luamod transform
 */
/**
 * @brief Gets transform at index.
 *
 *    @param L Lua state to get transform from.
 *    @param ind Index position to find the transform.
 *    @return Transform found at the index in the state.
 */
gl_Matrix4* lua_totransform( lua_State *L, int ind )
{
   return (gl_Matrix4*) lua_touserdata(L,ind);
}
/**
 * @brief Gets transform at index or raises error if there is no transform at index.
 *
 *    @param L Lua state to get transform from.
 *    @param ind Index position to find transform.
 *    @return Transform found at the index in the state.
 */
gl_Matrix4* luaL_checktransform( lua_State *L, int ind )
{
   if (lua_istransform(L,ind))
      return lua_totransform(L,ind);
   luaL_typerror(L, ind, TRANSFORM_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a transform on the stack.
 *
 *    @param L Lua state to push transform into.
 *    @param transform Transform to push.
 *    @return Newly pushed transform.
 */
gl_Matrix4* lua_pushtransform( lua_State *L, gl_Matrix4 transform )
{
   gl_Matrix4 *t;
   t = (gl_Matrix4*) lua_newuserdata(L, sizeof(gl_Matrix4));
   *t = transform;
   luaL_getmetatable(L, TRANSFORM_METATABLE);
   lua_setmetatable(L, -2);
   return t;
}
/**
 * @brief Checks to see if ind is a transform.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a transform.
 */
int lua_istransform( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, TRANSFORM_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Compares two transforms to see if they are the same.
 *
 *    @luatparam Transform t1 Transform 1 to compare.
 *    @luatparam Transform t2 Transform 2 to compare.
 *    @luatreturn boolean true if both transforms are the same.
 * @luafunc __eq( t1, t2 )
 */
static int transformL_eq( lua_State *L )
{
   gl_Matrix4 *t1, *t2;
   t1 = luaL_checktransform(L,1);
   t2 = luaL_checktransform(L,2);
   lua_pushboolean( L, (memcmp( t1, t2, sizeof(gl_Matrix4) )==0) );
   return 1;
}


/**
 * @brief Creates a new identity transform.Gets a transform.
 *
 *    @luatreturn Transform A new transform corresponding to an identity matrix.
 * @luafunc new( )
 */
static int transformL_new( lua_State *L )
{
   lua_pushtransform( L, gl_Matrix4_Identity() );
   return 1;
}

