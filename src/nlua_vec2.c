/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_space.c
 *
 * @brief Handles the Lua space bindings.
 *
 * These bindings control the planets and systems.
 */

#include "nlua_vec2.h"

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "naev.h"


/* Vector metatable methods */
static int vectorL_new( lua_State *L );
static int vectorL_add( lua_State *L );
static int vectorL_sub( lua_State *L );
static int vectorL_mul( lua_State *L );
static int vectorL_div( lua_State *L );
static int vectorL_get( lua_State *L );
static int vectorL_set( lua_State *L );
static int vectorL_distance( lua_State *L );
static int vectorL_mod( lua_State *L );
static const luaL_reg vector_methods[] = {
   { "new", vectorL_new },
   { "__add", vectorL_add },
   { "add", vectorL_add },
   { "__sub", vectorL_sub },
   { "sub", vectorL_sub },
   { "__mul", vectorL_mul },
   { "__div", vectorL_div },
   { "get", vectorL_get },
   { "set", vectorL_set },
   { "dist", vectorL_distance },
   { "mod", vectorL_mod },
   {0,0}
}; /**< Vector metatable methods. */


/**
 * @brief Loads the vector metatable.
 *
 *    @param L State to load the vector metatable into.
 *    @return 0 on success.
 */
int lua_loadVector( lua_State *L )
{
   vectorL_createmetatable( L );

   return 0;
}


/**
 * @brief Registers the vector metatable.
 *
 *    @param L Lua state to register metatable in.
 *    @return 0 on success.
 */
int vectorL_createmetatable( lua_State *L )
{
   /* Create the metatable */
   luaL_newmetatable(L, VECTOR_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, vector_methods);

   /* Clean up. */
   lua_pop(L,1);

   return 0; /* No error */
}


/**
 * @defgroup META_VECTOR Vector Metatable
 *
 * @brief Represents a 2d vector in Lua.
 *
 * This module allows you to manipulate 2d vectors.  Usagae is generally as follows:
 *
 * @code
 * my_vec = vec2.new( 3, 2 ) -- my_vec is now (3,2)
 * my_vec:add( 5, 3 ) -- my_vec is now (8,5)
 * @endcode
 *
 * @luamod vec2
 *
 * To call members of the metatable always use:
 * @code 
 * vector:function( param )
 * @endcode
 */
/**
 * @brief Gets vector at index.
 *
 *    @param L Lua state to get vector from.
 *    @param ind Index position of vector.
 *    @return The LuaVector at ind.
 */
LuaVector* lua_tovector( lua_State *L, int ind )
{     
   if (lua_isuserdata(L,ind)) {
      return (LuaVector*) lua_touserdata(L,ind);
   }
   luaL_typerror(L, ind, VECTOR_METATABLE);
   return NULL;
}

/**
 * @brief Pushes a vector on the stack.
 *
 *    @param L Lua state to push vector onto.
 *    @param vec Vector to push.
 *    @return Vector just pushed.
 */
LuaVector* lua_pushvector( lua_State *L, LuaVector vec )
{
   LuaVector *v;
   v = (LuaVector*) lua_newuserdata(L, sizeof(LuaVector));
   *v = vec;
   luaL_getmetatable(L, VECTOR_METATABLE);
   lua_setmetatable(L, -2);
   return v;
}

/**
 * @brief Checks to see if ind is a vector.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if there is a vector at index position.
 */
int lua_isvector( lua_State *L, int ind )
{  
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, VECTOR_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */ 
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @ingroup META_VECTOR
 *
 * @brief Creates a new vector.
 *
 * @usage vec2.new( 5, 3 ) -- creates a vector at (5,3)
 * @usage vec2.new() -- creates a vector at (0,0)
 *
 *    @luaparam x If set, the X value for the new vector.
 *    @luaparam y If set, the Y value for the new vector.
 *    @luareturn The new vector.
 * @luafunc new( x, y )
 */
static int vectorL_new( lua_State *L )
{
   LuaVector v;
   double x, y;

   if ((lua_gettop(L) > 1) && lua_isnumber(L,1) && lua_isnumber(L,2)) {
      x = lua_tonumber(L,1);
      y = lua_tonumber(L,2);
   }
   else {
      x = 0.;
      y = 0.;
   }

   vect_cset( &v.vec, x, y );
   lua_pushvector(L, v);
   return 1;
}

/**
 * @ingroup META_VECTOR
 *
 * @brief __add( Vec2 vector )
 *
 * __add( number x, number y )
 *
 * Adds two vectors or a vector and some cartesian coordinates.
 */
static int vectorL_add( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   LuaVector *v1, *v2;
   double x, y;

   /* Get self. */
   v1 = lua_tovector(L,1);

   /* Get rest of parameters. */
   v2 = NULL;
   if (lua_isvector(L,2)) {
      v2 = lua_tovector(L,2);
      x = v2->vec.x;
      y = v2->vec.y;
   }
   else if ((lua_gettop(L) > 2) && lua_isnumber(L,2) && lua_isnumber(L,3)) {
      x = lua_tonumber(L,2);
      y = lua_tonumber(L,3);
   }
   else NLUA_INVALID_PARAMETER();

   /* Actually add it */
   vect_cadd( &v1->vec, x, y );
   return 0;
}

/**
 * @ingroup META_VECTOR
 *
 * @brief Subtracts a vector and some cartesian coordinates.
 *    @luaparam x X coordinate to subtract.
 *    @luaparam y Y cooridinate to subtract.
 * @luafunc __sub( vector )
 * @brief Subtracts two vectors.
 *    @luaparam x
 * @luafunc __sub( x, y )
 */
static int vectorL_sub( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   LuaVector *v1, *v2;
   double x, y;

   /* Get self. */
   v1 = lua_tovector(L,1);

   /* Get rest of parameters. */
   v2 = NULL;
   if (lua_isvector(L,2)) {
      v2 = lua_tovector(L,2);
      x = v2->vec.x;
      y = v2->vec.y;
   }
   else if ((lua_gettop(L) > 2) && lua_isnumber(L,2) && lua_isnumber(L,3)) {
      x = lua_tonumber(L,2);
      y = lua_tonumber(L,3);
   }
   else NLUA_INVALID_PARAMETER();

   /* Actually add it */
   vect_cadd( &v1->vec, -x, -y );
   return 0;
}

/**
 * @ingroup META_VECTOR
 *
 * @brief Multiplies a vector by a number.
 *    @luaparam mod Amount to multiply by.
 * @luafunc __mul( mod )
 */
static int vectorL_mul( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   LuaVector *v1;
   double mod;

   /* Get self. */
   v1 = lua_tovector(L,1);

   /* Get rest of parameters. */
   if (lua_isnumber(L,2))
      mod = lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();

   /* Actually add it */
   vect_cadd( &v1->vec, v1->vec.x * mod, v1->vec.x * mod );
   return 0;
}

/**
 * @ingroup META_VECTOR
 *
 * @brief Divides a vector by a number.
 *    @luaparam mod Amount to divide by.
 * @luafunc __div( mod )
 */
static int vectorL_div( lua_State *L )
{
   NLUA_MIN_ARGS(2);
   LuaVector *v1;
   double mod;

   /* Get self. */
   v1 = lua_tovector(L,1);

   /* Get rest of parameters. */
   if (lua_isnumber(L,2))
      mod = lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();

   /* Actually add it */
   vect_cadd( &v1->vec, v1->vec.x / mod, v1->vec.x / mod );
   return 0;
}


/**
 * @ingroup META_VECTOR
 *
 * @brief Gets the cartesian positions of the vector.
 *    @luareturn X and Y position of the vector.
 * @luafunc get()
 */
static int vectorL_get( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   LuaVector *v1;

   /* Get self. */
   v1 = lua_tovector(L,1);

   /* Push the vector. */
   lua_pushnumber(L, v1->vec.x);
   lua_pushnumber(L, v1->vec.y);
   return 2;
}

/**
 * @ingroup META_VECTOR
 *
 * @brief Sets the vector by cartesian coordinates.
 *    @luaparam x X coordinate to set.
 *    @luaparam y Y coordinate to set.
 * @luafunc set( x, y )
 */
static int vectorL_set( lua_State *L )
{
   NLUA_MIN_ARGS(3);
   LuaVector *v1;
   double x, y;

   /* Get self. */
   v1 = lua_tovector(L,1);

   /* Get parameters. */
   if (lua_isnumber(L,2))
      x = lua_tonumber(L,2);
   else NLUA_INVALID_PARAMETER();
   if (lua_isnumber(L,3))
      y = lua_tonumber(L,3);
   else NLUA_INVALID_PARAMETER();

   vect_cset( &v1->vec, x, y );
   return 0;
}

/**
 * @ingroup META_VECTOR
 *
 * @brief Gets the distance from the Vec2.
 *    @param vector Vector to get distance from, uses origin (0,0) if not set.
 *    @luareturn The distance calculated.
 * @luafunc dist( vector )
 */
static int vectorL_distance( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   LuaVector *v1, *v2;
   double dist;

   /* Get self. */
   v1 = lua_tovector(L,1);

   /* Get rest of parameters. */
   v2 = NULL;
   if (lua_gettop(L) > 1) {
      if (lua_isvector(L,2))
         v2 = lua_tovector(L,2);
      else NLUA_INVALID_PARAMETER();
   }

   /* Get distance. */
   if (v2 == NULL)
      dist = vect_odist(&v1->vec);
   else
      dist = vect_dist(&v1->vec, &v2->vec);

   /* Return the distance. */
   lua_pushnumber(L, dist);
   return 1;
}

/**
 * @ingroup META_VECTOR
 *
 * @brief Gets the modulus of the vector.
 *    @luareturn The modulus of the vector.
 * @luafunc mod()
 */
static int vectorL_mod( lua_State *L )
{
   NLUA_MIN_ARGS(1);
   LuaVector *v;

   v = lua_tovector(L,1);
   lua_pushnumber(L, VMOD(v->vec));
   return 1;
}

