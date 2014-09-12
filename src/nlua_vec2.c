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

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "log.h"


/* Vector metatable methods */
static int vectorL_new( lua_State *L );
static int vectorL_add__( lua_State *L );
static int vectorL_add( lua_State *L );
static int vectorL_sub__( lua_State *L );
static int vectorL_sub( lua_State *L );
static int vectorL_mul__( lua_State *L );
static int vectorL_mul( lua_State *L );
static int vectorL_div__( lua_State *L );
static int vectorL_div( lua_State *L );
static int vectorL_get( lua_State *L );
static int vectorL_polar( lua_State *L );
static int vectorL_set( lua_State *L );
static int vectorL_setP( lua_State *L );
static int vectorL_distance( lua_State *L );
static int vectorL_distance2( lua_State *L );
static int vectorL_mod( lua_State *L );
static const luaL_reg vector_methods[] = {
   { "new", vectorL_new },
   { "__add", vectorL_add },
   { "add", vectorL_add__ },
   { "__sub", vectorL_sub },
   { "sub", vectorL_sub__ },
   { "__mul", vectorL_mul },
   { "mul", vectorL_mul__ },
   { "__div", vectorL_div },
   { "div", vectorL_div__ },
   { "get", vectorL_get },
   { "polar", vectorL_polar },
   { "set", vectorL_set },
   { "setP", vectorL_setP },
   { "dist", vectorL_distance },
   { "dist2", vectorL_distance2 },
   { "mod", vectorL_mod },
   {0,0}
}; /**< Vector metatable methods. */


/**
 * @brief Loads the vector metatable.
 *
 *    @param L State to load the vector metatable into.
 *    @return 0 on success.
 */
int nlua_loadVector( lua_State *L )
{
   /* Create the metatable */
   luaL_newmetatable(L, VECTOR_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, vector_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, VECTOR_METATABLE);

   return 0;
}


/**
 * @brief Represents a 2D vector in Lua.
 *
 * This module allows you to manipulate 2D vectors.  Usage is generally as follows:
 *
 * @code
 * my_vec = vec2.new( 3, 2 ) -- my_vec is now (3,2)
 * my_vec:add( 5, 3 ) -- my_vec is now (8,5)
 * my_vec = my_vec * 3 -- my_vec is now (24,15)
 * your_vec = vec2.new( 5, 2 ) -- your_vec is now (5,2)
 * my_vec = my_vec - your_vec -- my_vec is now (19,13)
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
   return (LuaVector*) lua_touserdata(L,ind);
}
/**
 * @brief Gets vector at index making sure type is valid.
 *
 *    @param L Lua state to get vector from.
 *    @param ind Index position of vector.
 *    @return The LuaVector at ind.
 */
LuaVector* luaL_checkvector( lua_State *L, int ind )
{
   if (lua_isvector(L,ind))
      return (LuaVector*) lua_touserdata(L,ind);
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
 * @brief Adds two vectors or a vector and some cartesian coordinates.
 *
 * If x is a vector it adds both vectors, otherwise it adds cartesian coordinates
 * to the vector.
 *
 * @usage my_vec = my_vec + your_vec
 * @usage my_vec:add( your_vec )
 * @usage my_vec:add( 5, 3 )
 *
 *    @luaparam v Vector getting stuff subtracted from.
 *    @luaparam x X coordinate or vector to add to.
 *    @luaparam y Y coordinate or nil to add to.
 *    @luareturn The result of the vector operation.
 * @luafunc add( v, x, y )
 */
static int vectorL_add( lua_State *L )
{
   LuaVector vout, *v1, *v2;
   double x, y;

   /* Get self. */
   v1    = luaL_checkvector(L,1);

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
   else {
      NLUA_INVALID_PARAMETER(L);
      return 0;
   }

   /* Actually add it */
   vect_cset( &vout.vec, v1->vec.x + x, v1->vec.y + y );
   lua_pushvector( L, vout );

   return 1;
}
static int vectorL_add__( lua_State *L )
{
   LuaVector *v1, *v2;
   double x, y;

   /* Get self. */
   v1    = luaL_checkvector(L,1);

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
   else {
      NLUA_INVALID_PARAMETER(L);
      return 0;
   }

   /* Actually add it */
   vect_cset( &v1->vec, v1->vec.x + x, v1->vec.y + y );
   lua_pushvector( L, *v1 );

   return 1;
}

/**
 * @brief Subtracts two vectors or a vector and some cartesian coordinates.
 *
 * If x is a vector it subtracts both vectors, otherwise it subtracts cartesian
 * coordinates to the vector.
 *
 * @usage my_vec = my_vec - your_vec
 * @usage my_vec:sub( your_vec )
 * @usage my_vec:sub( 5, 3 )
 *
 *    @luaparam v Vector getting stuff subtracted from.
 *    @luaparam x X coordinate or vector to subtract.
 *    @luaparam y Y coordinate or nil to subtract.
 *    @luareturn The result of the vector operation.
 * @luafunc sub( v, x, y )
 */
static int vectorL_sub( lua_State *L )
{
   LuaVector vout, *v1, *v2;
   double x, y;

   /* Get self. */
   v1    = luaL_checkvector(L,1);

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
   else {
      NLUA_INVALID_PARAMETER(L);
      return 0;
   }

   /* Actually add it */
   vect_cset( &vout.vec, v1->vec.x - x, v1->vec.y - y );
   lua_pushvector( L, vout );
   return 1;
}
static int vectorL_sub__( lua_State *L )
{
   LuaVector *v1, *v2;
   double x, y;

   /* Get self. */
   v1    = luaL_checkvector(L,1);

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
   else {
      NLUA_INVALID_PARAMETER(L);
      return 0;
   }

   /* Actually add it */
   vect_cset( &v1->vec, v1->vec.x - x, v1->vec.y - y );
   lua_pushvector( L, *v1 );
   return 1;
}

/**
 * @brief Multiplies a vector by a number.
 *
 * @usage my_vec = my_vec * 3
 * @usage my_vec:mul( 3 )
 *
 *    @luaparam v Vector to multiply.
 *    @luaparam mod Amount to multiply by.
 *    @luareturn The result of the vector operation.
 * @luafunc mul( v, mod )
 */
static int vectorL_mul( lua_State *L )
{
   LuaVector vout, *v1;
   double mod;

   /* Get parameters. */
   v1    = luaL_checkvector(L,1);
   mod   = luaL_checknumber(L,2);

   /* Actually add it */
   vect_cset( &vout.vec, v1->vec.x * mod, v1->vec.y * mod );
   lua_pushvector( L, vout );
   return 1;
}
static int vectorL_mul__( lua_State *L )
{
   LuaVector *v1;
   double mod;

   /* Get parameters. */
   v1    = luaL_checkvector(L,1);
   mod   = luaL_checknumber(L,2);

   /* Actually add it */
   vect_cset( &v1->vec, v1->vec.x * mod, v1->vec.y * mod );
   lua_pushvector( L, *v1 );
   return 1;
}

/**
 * @brief Divides a vector by a number.
 *
 * @usage my_vec = my_vec / 3
 * @usage my_vec:div(3)
 *
 *    @luaparam v Vector to divide.
 *    @luaparam mod Amount to divide by.
 *    @luareturn The result of the vector operation.
 * @luafunc div( v, mod )
 */
static int vectorL_div( lua_State *L )
{
   LuaVector vout, *v1;
   double mod;

   /* Get parameters. */
   v1    = luaL_checkvector(L,1);
   mod   = luaL_checknumber(L,2);

   /* Actually add it */
   vect_cset( &vout.vec, v1->vec.x / mod, v1->vec.y / mod );
   lua_pushvector( L, vout );
   return 1;
}
static int vectorL_div__( lua_State *L )
{
   LuaVector *v1;
   double mod;

   /* Get parameters. */
   v1    = luaL_checkvector(L,1);
   mod   = luaL_checknumber(L,2);

   /* Actually add it */
   vect_cset( &v1->vec, v1->vec.x / mod, v1->vec.y / mod );
   lua_pushvector( L, *v1 );
   return 1;
}


/**
 * @brief Gets the cartesian positions of the vector.
 *
 * @usage x,y = my_vec:get()
 *
 *    @luaparam v Vector to get position of.
 *    @luareturn X and Y position of the vector.
 * @luafunc get(v)
 */
static int vectorL_get( lua_State *L )
{
   LuaVector *v1;

   /* Get self. */
   v1 = luaL_checkvector(L,1);

   /* Push the vector. */
   lua_pushnumber(L, v1->vec.x);
   lua_pushnumber(L, v1->vec.y);
   return 2;
}

/**
 * @brief Gets polar coordinates of a vector.
 *
 * The angle is in degrees, not radians.
 *
 * @usage modulus, angle = my_vec:polar()
 *
 *    @luaparam v Vector to get polar coordinates of.
 *    @luareturn The modulus and angle of the vector.
 * @luafunc polar(v)
 */
static int vectorL_polar( lua_State *L )
{
   LuaVector *v1;

   /* Get self. */
   v1 = luaL_checkvector(L,1);

   lua_pushnumber(L, VMOD(v1->vec));
   lua_pushnumber(L, VANGLE(v1->vec)*180./M_PI);
   return 2;
}

/**
 * @brief Sets the vector by cartesian coordinates.
 *
 * @usage my_vec:set(5, 3) -- my_vec is now (5,3)
 *
 *    @luaparam v Vector to set coordinates of.
 *    @luaparam x X coordinate to set.
 *    @luaparam y Y coordinate to set.
 * @luafunc set( v, x, y )
 */
static int vectorL_set( lua_State *L )
{
   LuaVector *v1;
   double x, y;

   /* Get parameters. */
   v1 = luaL_checkvector(L,1);
   x  = luaL_checknumber(L,2);
   y  = luaL_checknumber(L,3);

   vect_cset( &v1->vec, x, y );
   return 0;
}

/**
 * @brief Sets the vector by cartesian coordinates.
 *
 * @usage my_vec:setP( 1, math.pi ) -- my_vec is now (0,1)
 *
 *    @luaparam v Vector to set coordinates of.
 *    @luaparam m Modulus to set.
 *    @luaparam a Angle to set.
 * @luafunc setP( v, m, a )
 */
static int vectorL_setP( lua_State *L )
{
   LuaVector *v1;
   double m, a;

   /* Get parameters. */
   v1 = luaL_checkvector(L,1);
   m  = luaL_checknumber(L,2);
   a  = luaL_checknumber(L,3)/180.*M_PI;

   vect_pset( &v1->vec, m, a );
   return 0;
}

/**
 * @brief Gets the distance from the Vec2.
 *
 * @usage my_vec:dist() -- Gets length of the vector (distance from origin).
 * @usage my_vec:dist( your_vec ) -- Gets distance from both vectors (your_vec - my_vec).
 *
 *    @luaparam v Vector to act as origin.
 *    @luaparam v2 Vector to get distance from, uses origin (0,0) if not set.
 *    @luareturn The distance calculated.
 * @luafunc dist( v, v2 )
 */
static int vectorL_distance( lua_State *L )
{
   LuaVector *v1, *v2;
   double dist;

   /* Get self. */
   v1 = luaL_checkvector(L,1);

   /* Get rest of parameters. */
   v2 = NULL;
   if (lua_gettop(L) > 1) {
      v2 = luaL_checkvector(L,2);
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
 * @brief Gets the squared distance from the Vec2 (saves a sqrt())
 *
 * @usage my_vec:dist2() -- Gets squared length of the vector (distance squared from origin).
 * @usage my_vec:dist2( your_vec ) -- Gets squared distance from both vectors (your_vec - my_vec)^2.
 *
 *    @luaparam v Vector to act as origin.
 *    @luaparam v2 Vector to get squared distance from, uses origin (0,0) if not set.
 *    @luareturn The distance calculated.
 * @luafunc dist2( v, v2 )
 */
static int vectorL_distance2( lua_State *L )
{
   LuaVector *v1, *v2;
   double dist2;

   /* Get self. */
   v1 = luaL_checkvector(L,1);

   /* Get rest of parameters. */
   v2 = NULL;
   if (lua_gettop(L) > 1) {
      v2 = luaL_checkvector(L,2);
   }

   /* Get distance. */
   if (v2 == NULL)
      dist2 = vect_odist2(&v1->vec);
   else
      dist2 = vect_dist2(&v1->vec, &v2->vec);

   /* Return the distance. */
   lua_pushnumber(L, dist2);
   return 1;
}

/**
 * @brief Gets the modulus of the vector.
 *    @luaparam v Vector to get modulus of.
 *    @luareturn The modulus of the vector.
 * @luafunc mod(v)
 */
static int vectorL_mod( lua_State *L )
{
   LuaVector *v;

   v = luaL_checkvector(L,1);
   lua_pushnumber(L, VMOD(v->vec));
   return 1;
}

