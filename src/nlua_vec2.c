/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_vec2.c
 *
 * @brief Handles the Lua vector handling bindings.
 *
 * These bindings control the planets and systems.
 */

#include "nlua_vec2.h"

#include "naev.h"

#include <lauxlib.h>

#include "nluadef.h"
#include "log.h"


/* Vector metatable methods */
static int vectorL_new( lua_State *L );
static int vectorL_newP( lua_State *L );
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
   { "newP", vectorL_newP },
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
 *    @param env Environment to load the vector metatable into.
 *    @return 0 on success.
 */
int nlua_loadVector( nlua_env env )
{
   nlua_register(env, VECTOR_METATABLE, vector_methods, 1);
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
 * To call members of the metatable always use:
 * @code
 * vector:function( param )
 * @endcode
 *
 * @luamod vec2
 */
/**
 * @brief Gets vector at index.
 *
 *    @param L Lua state to get vector from.
 *    @param ind Index position of vector.
 *    @return The Vector2d at ind.
 */
Vector2d* lua_tovector( lua_State *L, int ind )
{
   return (Vector2d*) lua_touserdata(L,ind);
}
/**
 * @brief Gets vector at index making sure type is valid.
 *
 *    @param L Lua state to get vector from.
 *    @param ind Index position of vector.
 *    @return The Vector2d at ind.
 */
Vector2d* luaL_checkvector( lua_State *L, int ind )
{
   if (lua_isvector(L,ind))
      return (Vector2d*) lua_touserdata(L,ind);
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
Vector2d* lua_pushvector( lua_State *L, Vector2d vec )
{
   Vector2d *v;
   v = (Vector2d*) lua_newuserdata(L, sizeof(Vector2d));
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
 *    @luatparam number x If set, the X value for the new vector.
 *    @luatparam number y If set, the Y value for the new vector.
 *    @luatreturn Vec2 The new vector.
 * @luafunc new( x, y )
 */
static int vectorL_new( lua_State *L )
{
   Vector2d v;
   double x, y;

   if ((lua_gettop(L) > 1) && lua_isnumber(L,1) && lua_isnumber(L,2)) {
      x = lua_tonumber(L,1);
      y = lua_tonumber(L,2);
   }
   else {
      x = 0.;
      y = 0.;
   }

   vect_cset( &v, x, y );
   lua_pushvector(L, v);
   return 1;
}

/**
 * @brief Creates a new vector using polar coordinates.
 *
 * @usage vec2.newP( 1000, 90 ) -- creates a vector at (0,1000)
 * @usage vec2.newP() -- creates a vector at (0,0)
 *
 *    @luatparam[opt=0] number m If set, the modulus for the new vector.
 *    @luatparam[opt=0] number a If set, the angle for the new vector, in degrees.
 *    @luatreturn Vec2 The new vector.
 * @luafunc newP( m, a )
 */
static int vectorL_newP( lua_State *L )
{
   Vector2d v;
   double m, a;

   if ((lua_gettop(L) > 1) && lua_isnumber(L,1) && lua_isnumber(L,2)) {
      m = lua_tonumber(L, 1);
      a = lua_tonumber(L, 2) / 180. * M_PI;
   }
   else {
      m = 0.;
      a = 0.;
   }

   vect_pset( &v, m, a );
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
 *    @luatparam Vector v Vector getting stuff subtracted from.
 *    @luatparam number|Vec2 x X coordinate or vector to add to.
 *    @luatparam number|nil y Y coordinate or nil to add to.
 *    @luatreturn Vec2 The result of the vector operation.
 * @luafunc add( v, x, y )
 */
static int vectorL_add( lua_State *L )
{
   Vector2d vout, *v1, *v2;
   double x, y;

   /* Get self. */
   v1    = luaL_checkvector(L,1);

   /* Get rest of parameters. */
   v2 = NULL;
   if (lua_isvector(L,2)) {
      v2 = lua_tovector(L,2);
      x = v2->x;
      y = v2->y;
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
   vect_cset( &vout, v1->x + x, v1->y + y );
   lua_pushvector( L, vout );

   return 1;
}
static int vectorL_add__( lua_State *L )
{
   Vector2d *v1, *v2;
   double x, y;

   /* Get self. */
   v1    = luaL_checkvector(L,1);

   /* Get rest of parameters. */
   v2 = NULL;
   if (lua_isvector(L,2)) {
      v2 = lua_tovector(L,2);
      x = v2->x;
      y = v2->y;
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
   vect_cset( v1, v1->x + x, v1->y + y );
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
 *    @luatparam Vec2 v Vector getting stuff subtracted from.
 *    @luatparam number|Vec2 x X coordinate or vector to subtract.
 *    @luatparam number|nil y Y coordinate or nil to subtract.
 *    @luatreturn Vec2 The result of the vector operation.
 * @luafunc sub( v, x, y )
 */
static int vectorL_sub( lua_State *L )
{
   Vector2d vout, *v1, *v2;
   double x, y;

   /* Get self. */
   v1    = luaL_checkvector(L,1);

   /* Get rest of parameters. */
   v2 = NULL;
   if (lua_isvector(L,2)) {
      v2 = lua_tovector(L,2);
      x = v2->x;
      y = v2->y;
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
   vect_cset( &vout, v1->x - x, v1->y - y );
   lua_pushvector( L, vout );
   return 1;
}
static int vectorL_sub__( lua_State *L )
{
   Vector2d *v1, *v2;
   double x, y;

   /* Get self. */
   v1    = luaL_checkvector(L,1);

   /* Get rest of parameters. */
   v2 = NULL;
   if (lua_isvector(L,2)) {
      v2 = lua_tovector(L,2);
      x = v2->x;
      y = v2->y;
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
   vect_cset( v1, v1->x - x, v1->y - y );
   lua_pushvector( L, *v1 );
   return 1;
}

/**
 * @brief Multiplies a vector by a number.
 *
 * @usage my_vec = my_vec * 3
 * @usage my_vec:mul( 3 )
 *
 *    @luatparam Vec2 v Vector to multiply.
 *    @luatparam number mod Amount to multiply by.
 *    @luatreturn Vec2 The result of the vector operation.
 * @luafunc mul( v, mod )
 */
static int vectorL_mul( lua_State *L )
{
   Vector2d vout, *v1;
   double mod;

   /* Get parameters. */
   v1    = luaL_checkvector(L,1);
   mod   = luaL_checknumber(L,2);

   /* Actually add it */
   vect_cset( &vout, v1->x * mod, v1->y * mod );
   lua_pushvector( L, vout );
   return 1;
}
static int vectorL_mul__( lua_State *L )
{
   Vector2d *v1;
   double mod;

   /* Get parameters. */
   v1    = luaL_checkvector(L,1);
   mod   = luaL_checknumber(L,2);

   /* Actually add it */
   vect_cset( v1, v1->x * mod, v1->y * mod );
   lua_pushvector( L, *v1 );
   return 1;
}

/**
 * @brief Divides a vector by a number.
 *
 * @usage my_vec = my_vec / 3
 * @usage my_vec:div(3)
 *
 *    @luatparam Vec2 v Vector to divide.
 *    @luatparam number mod Amount to divide by.
 *    @luatreturn Vec2 The result of the vector operation.
 * @luafunc div( v, mod )
 */
static int vectorL_div( lua_State *L )
{
   Vector2d vout, *v1;
   double mod;

   /* Get parameters. */
   v1    = luaL_checkvector(L,1);
   mod   = luaL_checknumber(L,2);

   /* Actually add it */
   vect_cset( &vout, v1->x / mod, v1->y / mod );
   lua_pushvector( L, vout );
   return 1;
}
static int vectorL_div__( lua_State *L )
{
   Vector2d *v1;
   double mod;

   /* Get parameters. */
   v1    = luaL_checkvector(L,1);
   mod   = luaL_checknumber(L,2);

   /* Actually add it */
   vect_cset( v1, v1->x / mod, v1->y / mod );
   lua_pushvector( L, *v1 );
   return 1;
}


/**
 * @brief Gets the cartesian positions of the vector.
 *
 * @usage x,y = my_vec:get()
 *
 *    @luatparam Vec2 v Vector to get position of.
 *    @luatreturn number X position of the vector.
 *    @luatreturn number Y position of the vector.
 * @luafunc get(v)
 */
static int vectorL_get( lua_State *L )
{
   Vector2d *v1;

   /* Get self. */
   v1 = luaL_checkvector(L,1);

   /* Push the vector. */
   lua_pushnumber(L, v1->x);
   lua_pushnumber(L, v1->y);
   return 2;
}

/**
 * @brief Gets polar coordinates of a vector.
 *
 * The angle is in degrees, not radians.
 *
 * @usage modulus, angle = my_vec:polar()
 *
 *    @luatparam Vec2 v Vector to get polar coordinates of.
 *    @luatreturn number The modulus of the vector.
 *    @luatreturn number The angle of the vector.
 * @luafunc polar(v)
 */
static int vectorL_polar( lua_State *L )
{
   Vector2d *v1;

   /* Get self. */
   v1 = luaL_checkvector(L,1);

   lua_pushnumber(L, VMOD(*v1));
   lua_pushnumber(L, VANGLE(*v1)*180./M_PI);
   return 2;
}

/**
 * @brief Sets the vector by cartesian coordinates.
 *
 * @usage my_vec:set(5, 3) -- my_vec is now (5,3)
 *
 *    @luatparam Vec2 v Vector to set coordinates of.
 *    @luatparam number x X coordinate to set.
 *    @luatparam number y Y coordinate to set.
 * @luafunc set( v, x, y )
 */
static int vectorL_set( lua_State *L )
{
   Vector2d *v1;
   double x, y;

   /* Get parameters. */
   v1 = luaL_checkvector(L,1);
   x  = luaL_checknumber(L,2);
   y  = luaL_checknumber(L,3);

   vect_cset( v1, x, y );
   return 0;
}

/**
 * @brief Sets the vector by polar coordinates.
 *
 * @usage my_vec:setP( 1, 90 ) -- my_vec is now (0,1)
 *
 *    @luatparam Vec2 v Vector to set coordinates of.
 *    @luatparam number m Modulus to set.
 *    @luatparam number a Angle to set, in degrees.
 * @luafunc setP( v, m, a )
 */
static int vectorL_setP( lua_State *L )
{
   Vector2d *v1;
   double m, a;

   /* Get parameters. */
   v1 = luaL_checkvector(L,1);
   m  = luaL_checknumber(L,2);
   a  = luaL_checknumber(L,3)/180.*M_PI;

   vect_pset( v1, m, a );
   return 0;
}

/**
 * @brief Gets the distance from the Vec2.
 *
 * @usage my_vec:dist() -- Gets length of the vector (distance from origin).
 * @usage my_vec:dist( your_vec ) -- Gets distance from both vectors (your_vec - my_vec).
 *
 *    @luatparam Vec2 v Vector to act as origin.
 *    @luatparam Vec2 v2 Vector to get distance from, uses origin (0,0) if not set.
 *    @luatreturn number The distance calculated.
 * @luafunc dist( v, v2 )
 */
static int vectorL_distance( lua_State *L )
{
   Vector2d *v1, *v2;
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
      dist = vect_odist(v1);
   else
      dist = vect_dist(v1, v2);

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
 *    @luatparam Vec2 v Vector to act as origin.
 *    @luatparam Vec2 v2 Vector to get squared distance from, uses origin (0,0) if not set.
 *    @luatreturn number The distance calculated.
 * @luafunc dist2( v, v2 )
 */
static int vectorL_distance2( lua_State *L )
{
   Vector2d *v1, *v2;
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
      dist2 = vect_odist2(v1);
   else
      dist2 = vect_dist2(v1, v2);

   /* Return the distance. */
   lua_pushnumber(L, dist2);
   return 1;
}

/**
 * @brief Gets the modulus of the vector.
 *    @luatparam Vec2 v Vector to get modulus of.
 *    @luatreturn number The modulus of the vector.
 * @luafunc mod(v)
 */
static int vectorL_mod( lua_State *L )
{
   Vector2d *v;

   v = luaL_checkvector(L,1);
   lua_pushnumber(L, VMOD(*v));
   return 1;
}

