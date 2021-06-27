/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_linopt.c
 *
 * @brief Handles Linear linoptization in Lua.
 */

#include <glpk.h>

/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_linopt.h"

#include "log.h"
#include "nluadef.h"


typedef struct LuaLinOpt_s {
   glp_prob *prob;
} LuaLinOpt_t;


/* Optim metatable methods. */
static int linoptL_gc( lua_State *L );
static int linoptL_eq( lua_State *L );
static int linoptL_new( lua_State *L );
static const luaL_Reg linoptL_methods[] = {
   { "__gc", linoptL_gc },
   { "__eq", linoptL_eq },
   { "new", linoptL_new },
   {0,0}
}; /**< Optim metatable methods. */




/**
 * @brief Loads the linopt library.
 *
 *    @param env Environment to load linopt library into.
 *    @return 0 on success.
 */
int nlua_loadOptim( nlua_env env )
{
   nlua_register(env, LINOPT_METATABLE, linoptL_methods, 1);
   return 0;
}


/**
 * @brief Lua bindings to interact with linopts.
 *
 * @luamod linopt
 */
/**
 * @brief Gets linopt at index.
 *
 *    @param L Lua state to get linopt from.
 *    @param ind Index position to find the linopt.
 *    @return Optim found at the index in the state.
 */
LuaLinOpt_t* lua_tolinopt( lua_State *L, int ind )
{
   return (LuaLinOpt_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets linopt at index or raises error if there is no linopt at index.
 *
 *    @param L Lua state to get linopt from.
 *    @param ind Index position to find linopt.
 *    @return Optim found at the index in the state.
 */
LuaLinOpt_t* luaL_checklinopt( lua_State *L, int ind )
{
   if (lua_islinopt(L,ind))
      return lua_tolinopt(L,ind);
   luaL_typerror(L, ind, LINOPT_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a linopt on the stack.
 *
 *    @param L Lua state to push linopt into.
 *    @param linopt Optim to push.
 *    @return Newly pushed linopt.
 */
LuaLinOpt_t* lua_pushlinopt( lua_State *L, LuaLinOpt_t linopt )
{
   LuaLinOpt_t *c;
   c = (LuaLinOpt_t*) lua_newuserdata(L, sizeof(LuaLinOpt_t));
   *c = linopt;
   luaL_getmetatable(L, LINOPT_METATABLE);
   lua_setmetatable(L, -2);
   return c;
}
/**
 * @brief Checks to see if ind is a linopt.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a linopt.
 */
int lua_islinopt( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, LINOPT_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Frees a linopt.
 *
 *    @luatparam Optim linopt Optim to free.
 * @luafunc __gc
 */
static int linoptL_gc( lua_State *L )
{
   LuaLinOpt_t *lp = luaL_checklinopt(L,1);
   glp_delete_prob(lp->prob);
   return 0;
}


/**
 * @brief Compares two linopts to see if they are the same.
 *
 *    @luatparam Optim d1 Optim 1 to compare.
 *    @luatparam Optim d2 Optim 2 to compare.
 *    @luatreturn boolean true if both linopts are the same.
 * @luafunc __eq
 */
static int linoptL_eq( lua_State *L )
{
   LuaLinOpt_t *lp1, *lp2;
   lp1 = luaL_checklinopt(L,1);
   lp2 = luaL_checklinopt(L,2);
   lua_pushboolean( L, (memcmp( lp1, lp2, sizeof(LuaLinOpt_t))==0) );
   return 1;
}


/**
 * @brief Opens a new linopt.
 *
 *    @luatparam string name Name of the optimization program.
 *    @luatreturn Optim New linopt object.
 * @luafunc new
 */
static int linoptL_new( lua_State *L )
{
   LuaLinOpt_t lp;
   const char *name = luaL_optstring(L,1,NULL);

   lp.prob = glp_create_prob();
   if (name)
      glp_set_prob_name( lp.prob, name );
   
   lua_pushlinopt( L, lp );
   return 1;
}
