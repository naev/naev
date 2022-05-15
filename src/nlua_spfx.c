/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_spfx.c
 *
 * @brief Bindings for Special effects functionality from Lua.
 */
/** @cond */
#include <lauxlib.h>
#include "physfsrwops.h"

#include "naev.h"
/** @endcond */

#include "nlua_spfx.h"

#include "conf.h"
#include "array.h"
#include "nlua_vec2.h"
#include "nluadef.h"
#include "nlua_file.h"
#include "nstring.h"

/**
 * @brief Handles the special effects Lua-side.
 */
typedef struct LuaSpfx_s {
   double ttl;    /**< Time to live. */
   vec2 pos;      /**< Position. */
   vec2 vel;      /**< Velocity. */
   int data;      /**< Reference to table of data. */
   int render_bg; /**< Reference to background render function. */
   int render_fg; /**< Reference to foreground render function. */
   int update;    /**< Reference to update function. */
} LuaSpfx_t;

/**
 * @brief List of special effects being handled.
 */
static LuaSpfx_t *lua_spfx = NULL;

/* Spfx methods. */
static int spfxL_gc( lua_State *L );
static int spfxL_eq( lua_State *L );
static int spfxL_new( lua_State *L );
static const luaL_Reg spfxL_methods[] = {
   { "__gc", spfxL_gc },
   { "__eq", spfxL_eq },
   { "new", spfxL_new },
   {0,0}
}; /**< SpfxLua methods. */

/**
 * @brief Loads the spfx library.
 *
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadSpfx( nlua_env env )
{
   nlua_register(env, SPFX_METATABLE, spfxL_methods, 1);
   return 0;
}

/**
 * @brief Gets spfx at index.
 *
 *    @param L Lua state to get spfx from.
 *    @param ind Index position to find the spfx.
 *    @return Spfx found at the index in the state.
 */
LuaSpfx_t* lua_tospfx( lua_State *L, int ind )
{
   return (LuaSpfx_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets spfx at index or raises error if there is no spfx at index.
 *
 *    @param L Lua state to get spfx from.
 *    @param ind Index position to find spfx.
 *    @return Spfx found at the index in the state.
 */
LuaSpfx_t* luaL_checkspfx( lua_State *L, int ind )
{
   if (lua_isspfx(L,ind))
      return lua_tospfx(L,ind);
   luaL_typerror(L, ind, SPFX_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a spfx on the stack.
 *
 *    @param L Lua state to push spfx into.
 *    @param spfx Spfx to push.
 *    @return Newly pushed spfx.
 */
LuaSpfx_t* lua_pushspfx( lua_State *L, LuaSpfx_t spfx )
{
   LuaSpfx_t *la = (LuaSpfx_t*) lua_newuserdata(L, sizeof(LuaSpfx_t));
   *la = spfx;
   luaL_getmetatable(L, SPFX_METATABLE);
   lua_setmetatable(L, -2);
   return la;
}
/**
 * @brief Checks to see if ind is a spfx.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a spfx.
 */
int lua_isspfx( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, SPFX_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief Lua bindings to interact with spfx.
 *
 *
 * @luamod spfx
 */
/**
 * @brief Frees a spfx.
 *
 *    @luatparam Spfx spfx Spfx to free.
 * @luafunc __gc
 */
static int spfxL_gc( lua_State *L )
{
   LuaSpfx_t *ls = luaL_checkspfx(L,1);
   return 0;
}

/**
 * @brief Compares two spfxs to see if they are the same.
 *
 *    @luatparam Spfx s1 Spfx 1 to compare.
 *    @luatparam Spfx s2 Spfx 2 to compare.
 *    @luatreturn boolean true if both spfxs are the same.
 * @luafunc __eq
 */
static int spfxL_eq( lua_State *L )
{
   LuaSpfx_t *s1, *s2;
   s1 = luaL_checkspfx(L,1);
   s2 = luaL_checkspfx(L,2);
   lua_pushboolean( L, (memcmp( s1, s2, sizeof(LuaSpfx_t) )==0) );
   return 1;
}

/**
 * @brief Creates a new spfx source.
 *
 *    @luatparam string|File data Data to load the spfx from.
 *    @luatreturn Spfx New spfx corresponding to the data.
 * @luafunc new
 */
static int spfxL_new( lua_State *L )
{
   LuaSpfx_t ls;

   memset( &ls, 0, sizeof(LuaSpfx_t) );
   lua_pushspfx(L, ls);

   return 1;
}
