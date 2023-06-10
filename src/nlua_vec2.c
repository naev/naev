/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_vec2.c
 *
 * @brief Handles the Lua vector handling bindings.
 *
 * These bindings control the spobs and systems.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_vec2.h"

#include "log.h"
#include "nluadef.h"
#include "collision.h"

/* Vector metatable methods */
static int vectorL_new( lua_State *L );
static int vectorL_newP( lua_State *L );
static int vectorL_tostring( lua_State *L );
static int vectorL_add__( lua_State *L );
static int vectorL_add( lua_State *L );
static int vectorL_sub__( lua_State *L );
static int vectorL_sub( lua_State *L );
static int vectorL_mul__( lua_State *L );
static int vectorL_mul( lua_State *L );
static int vectorL_div__( lua_State *L );
static int vectorL_div( lua_State *L );
static int vectorL_dot( lua_State *L );
static int vectorL_cross( lua_State *L );
static int vectorL_get( lua_State *L );
static int vectorL_polar( lua_State *L );
static int vectorL_set( lua_State *L );
static int vectorL_setP( lua_State *L );
static int vectorL_distance( lua_State *L );
static int vectorL_distance2( lua_State *L );
static int vectorL_mod( lua_State *L );
static int vectorL_angle( lua_State *L );
static int vectorL_normalize( lua_State *L );
static int vectorL_collideLineLine( lua_State *L );
static int vectorL_collideCircleLine( lua_State *L );
static const luaL_Reg vector_methods[] = {
   { "new", vectorL_new },
   { "newP", vectorL_newP },
   { "__tostring", vectorL_tostring },
   { "__add", vectorL_add },
   { "add", vectorL_add__ },
   { "__sub", vectorL_sub },
   { "sub", vectorL_sub__ },
   { "__mul", vectorL_mul },
   { "mul", vectorL_mul__ },
   { "__div", vectorL_div },
   { "div", vectorL_div__ },
   { "dot", vectorL_dot },
   { "cross", vectorL_cross },
   { "get", vectorL_get },
   { "polar", vectorL_polar },
   { "set", vectorL_set },
   { "setP", vectorL_setP },
   { "dist", vectorL_distance },
   { "dist2", vectorL_distance2 },
   { "mod", vectorL_mod },
   { "angle", vectorL_angle },
   { "normalize", vectorL_normalize },
   { "collideLineLine", vectorL_collideLineLine },
   { "collideCircleLine", vectorL_collideCircleLine },
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
 *    @return The vec2 at ind.
 */
vec2* lua_tovector( lua_State *L, int ind )
{
   return (vec2*) lua_touserdata(L,ind);
}
/**
 * @brief Gets vector at index making sure type is valid.
 *
 *    @param L Lua state to get vector from.
 *    @param ind Index position of vector.
 *    @return The vec2 at ind.
 */
vec2* luaL_checkvector( lua_State *L, int ind )
{
   if (lua_isvector(L,ind))
      return (vec2*) lua_touserdata(L,ind);
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
vec2* lua_pushvector( lua_State *L, vec2 vec )
{
   vec2 *v = (vec2*) lua_newuserdata(L, sizeof(vec2));
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
 *    @luatparam[opt=0] number x If set, the X value for the new vector.
 *    @luatparam[opt=x] number y If set, the Y value for the new vector.
 *    @luatreturn Vec2 The new vector.
 * @luafunc new
 */
static int vectorL_new( lua_State *L )
{
   vec2 v;
   double x, y;

   if (!lua_isnoneornil(L,1))
      x = luaL_checknumber(L,1);
   else
      x = 0.;

   if (!lua_isnoneornil(L,2))
      y = luaL_checknumber(L,2);
   else
      y = x;

   vec2_cset( &v, x, y );
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
 *    @luatparam[opt=0] number a If set, the angle for the new vector, in radians.
 *    @luatreturn Vec2 The new vector.
 * @luafunc newP
 */
static int vectorL_newP( lua_State *L )
{
   vec2 v;
   double m, a;

   if (!lua_isnoneornil(L,1))
      m = luaL_checknumber(L, 1);
   else
      m = 0.;

   if (!lua_isnoneornil(L,2))
      a = luaL_checknumber(L, 2);
   else
      a = 0.;

   vec2_pset( &v, m, a );
   lua_pushvector(L, v);
   return 1;
}

/**
 * @brief Converts a vector to a string.
 *
 *    @luatparam Vector v Vector to convert to as string.
 *    @luatreturn string String version of v.
 * @luafunc __tostring
 */
static int vectorL_tostring( lua_State *L )
{
   char buf[STRMAX_SHORT];
   vec2 *v = luaL_checkvector(L,1);
   snprintf( buf, sizeof(buf), "vec2( %g, %g )", v->x, v->y );
   lua_pushstring(L, buf);
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
 *    @luatparam Vector v Vector getting stuff added to.
 *    @luatparam number|Vec2 x X coordinate or vector to add to.
 *    @luatparam number|nil y Y coordinate or nil to add to.
 *    @luatreturn Vec2 The result of the vector operation.
 * @luafunc add
 */
static int vectorL_add( lua_State *L )
{
   vec2 vout, *v1;
   double x, y;

   if (lua_isnumber(L,1)) {
      x = y = lua_tonumber(L,1);
      v1 = luaL_checkvector(L,2);
   }
   else {
      /* Get self. */
      v1    = luaL_checkvector(L,1);

      /* Get rest of parameters. */
      if (lua_isvector(L,2)) {
         vec2 *v2 = lua_tovector(L,2);
         x = v2->x;
         y = v2->y;
      }
      else {
         x = luaL_checknumber(L,2);
         if (!lua_isnoneornil(L,3))
            y = luaL_checknumber(L,3);
         else
            y = x;
      }
   }

   /* Actually add it */
   vec2_cset( &vout, v1->x + x, v1->y + y );
   lua_pushvector( L, vout );

   return 1;
}
static int vectorL_add__( lua_State *L )
{
   vec2 *v1;
   double x, y;

   /* Get self. */
   v1 = luaL_checkvector(L,1);

   /* Get rest of parameters. */
   if (lua_isvector(L,2)) {
      vec2 *v2 = lua_tovector(L,2);
      x = v2->x;
      y = v2->y;
   }
   else {
      x = luaL_checknumber(L,2);
      if (!lua_isnoneornil(L,3))
         y = luaL_checknumber(L,3);
      else
         y = x;
   }

   /* Actually add it */
   vec2_cset( v1, v1->x + x, v1->y + y );
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
 * @luafunc sub
 */
static int vectorL_sub( lua_State *L )
{
   vec2 vout, *v1;
   double x, y;

   /* Get self. */
   v1    = luaL_checkvector(L,1);

   /* Get rest of parameters. */
   if (lua_isvector(L,2)) {
      vec2 *v2 = lua_tovector(L,2);
      x = v2->x;
      y = v2->y;
   }
   else {
      x = luaL_checknumber(L,2);
      if (!lua_isnoneornil(L,3))
         y = luaL_checknumber(L,3);
      else
         y = x;
   }

   /* Actually add it */
   vec2_cset( &vout, v1->x - x, v1->y - y );
   lua_pushvector( L, vout );
   return 1;
}
static int vectorL_sub__( lua_State *L )
{
   vec2 *v1;
   double x, y;

   /* Get self. */
   v1    = luaL_checkvector(L,1);

   /* Get rest of parameters. */
   if (lua_isvector(L,2)) {
      vec2 *v2 = lua_tovector(L,2);
      x = v2->x;
      y = v2->y;
   }
   else {
      x = luaL_checknumber(L,2);
      if (!lua_isnoneornil(L,3))
         y = luaL_checknumber(L,3);
      else
         y = x;
   }

   /* Actually add it */
   vec2_cset( v1, v1->x - x, v1->y - y );
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
 * @luafunc mul
 */
static int vectorL_mul( lua_State *L )
{
   vec2 vout;

   if (lua_isnumber(L,1)) {
      double d = lua_tonumber(L,1);
      vec2 *v  = luaL_checkvector(L,2);
      vec2_cset( &vout, v->x * d, v->y * d );
   }
   else {
      if (lua_isnumber(L,2)) {
         vec2 *v  = luaL_checkvector(L,1);
         double d = lua_tonumber(L,2);
         vec2_cset( &vout, v->x * d, v->y * d );
      }
      else {
         vec2 *v1 = luaL_checkvector(L,1);
         vec2 *v2 = luaL_checkvector(L,2);
         vec2_cset( &vout, v1->x * v2->x, v1->y * v2->y );
      }
   }

   /* Actually add it */
   lua_pushvector( L, vout );
   return 1;
}
static int vectorL_mul__( lua_State *L )
{
   vec2 *v1;

   /* Get parameters. */
   v1 = luaL_checkvector(L,1);
   if (lua_isnumber(L,2)) {
      double mod = luaL_checknumber(L,2);
      vec2_cset( v1, v1->x * mod, v1->y * mod );
   }
   else {
      vec2 *v2 = luaL_checkvector(L,2);
      vec2_cset( v1, v1->x * v2->x, v1->y * v2->y );
   }

   /* Actually add it */
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
 * @luafunc div
 */
static int vectorL_div( lua_State *L )
{
   vec2 vout, *v1;

   /* Get parameters. */
   v1    = luaL_checkvector(L,1);
   if (lua_isnumber(L,2)) {
      double mod = lua_tonumber(L,2);
      vec2_cset( &vout, v1->x / mod, v1->y / mod );
   }
   else {
      vec2 *v2 = luaL_checkvector(L,2);
      vec2_cset( &vout, v1->x / v2->x, v1->y / v2->y );
   }

   lua_pushvector( L, vout );
   return 1;
}
static int vectorL_div__( lua_State *L )
{
   vec2 *v1;

   /* Get parameters. */
   v1    = luaL_checkvector(L,1);
   if (lua_isnumber(L,2)) {
      double mod = lua_tonumber(L,2);
      vec2_cset( v1, v1->x / mod, v1->y / mod );
   }
   else {
      vec2 *v2 = luaL_checkvector(L,2);
      vec2_cset( v1, v1->x / v2->x, v1->y / v2->y );
   }

   lua_pushvector( L, *v1 );
   return 1;
}

/**
 * @brief Dot product of two vectors.
 *
 *    @luatparam Vec2 a First vector.
 *    @luatparam Vec2 b Second vector.
 *    @luatreturn number The dot product.
 * @luafunc dot
 */
static int vectorL_dot( lua_State *L )
{
   vec2 *a = luaL_checkvector(L,1);
   vec2 *b = luaL_checkvector(L,2);
   lua_pushnumber( L, a->x*b->x + a->y*b->y );
   return 1;
}

/**
 * @brief Cross product of two vectors.
 *
 *    @luatparam Vec2 a First vector.
 *    @luatparam Vec2 b Second vector.
 *    @luatreturn number The cross product.
 * @luafunc cross
 */
static int vectorL_cross( lua_State *L )
{
   vec2 *a = luaL_checkvector(L,1);
   vec2 *b = luaL_checkvector(L,2);
   lua_pushnumber( L, a->x*b->y - a->y*b->x );
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
 * @luafunc get
 */
static int vectorL_get( lua_State *L )
{
   vec2 *v1 = luaL_checkvector(L,1);

   /* Push the vector. */
   lua_pushnumber(L, v1->x);
   lua_pushnumber(L, v1->y);
   return 2;
}

/**
 * @brief Gets polar coordinates of a vector.
 *
 * The angle is in radians.
 *
 * @usage modulus, angle = my_vec:polar()
 *
 *    @luatparam Vec2 v Vector to get polar coordinates of.
 *    @luatreturn number The modulus of the vector.
 *    @luatreturn number The angle of the vector.
 * @luafunc polar
 */
static int vectorL_polar( lua_State *L )
{
   vec2 *v1 = luaL_checkvector(L,1);
   lua_pushnumber(L, VMOD(*v1));
   lua_pushnumber(L, VANGLE(*v1));
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
 * @luafunc set
 */
static int vectorL_set( lua_State *L )
{
   vec2 *v1;
   double x, y;

   /* Get parameters. */
   v1 = luaL_checkvector(L,1);
   x  = luaL_checknumber(L,2);
   y  = luaL_checknumber(L,3);

   vec2_cset( v1, x, y );
   return 0;
}

/**
 * @brief Sets the vector by polar coordinates.
 *
 * @usage my_vec:setP( 1, 90 ) -- my_vec is now (0,1)
 *
 *    @luatparam Vec2 v Vector to set coordinates of.
 *    @luatparam number m Modulus to set.
 *    @luatparam number a Angle to set, in radians.
 * @luafunc setP
 */
static int vectorL_setP( lua_State *L )
{
   vec2 *v1;
   double m, a;

   /* Get parameters. */
   v1 = luaL_checkvector(L,1);
   m  = luaL_checknumber(L,2);
   a  = luaL_checknumber(L,3);

   vec2_pset( v1, m, a );
   return 0;
}

/**
 * @brief Gets the distance from the Vec2.
 *
 * @usage my_vec:dist() -- Gets length of the vector (distance from origin).
 * @usage my_vec:dist( your_vec ) -- Gets distance from both vectors (your_vec - my_vec).
 *
 *    @luatparam Vec2 v Vector to act as origin.
 *    @luatparam[opt=vec2.new()] Vec2 v2 Vector to get distance from, uses origin (0,0) if not set.
 *    @luatreturn number The distance calculated.
 * @luafunc dist
 */
static int vectorL_distance( lua_State *L )
{
   vec2 *v1, *v2;
   double dist;

   /* Get self. */
   v1 = luaL_checkvector(L,1);

   /* Get rest of parameters. */
   if (!lua_isnoneornil(L,2))
      v2 = luaL_checkvector(L,2);
   else
      v2 = NULL;

   /* Get distance. */
   if (v2 == NULL)
      dist = vec2_odist(v1);
   else
      dist = vec2_dist(v1, v2);

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
 *    @luatparam[opt=vec2.new()] Vec2 v2 Vector to get squared distance from, uses origin (0,0) if not set.
 *    @luatreturn number The distance calculated.
 * @luafunc dist2
 */
static int vectorL_distance2( lua_State *L )
{
   vec2 *v1, *v2;
   double dist2;

   /* Get self. */
   v1 = luaL_checkvector(L,1);

   /* Get rest of parameters. */
   if (!lua_isnoneornil(L,2))
      v2 = luaL_checkvector(L,2);
   else
      v2 = NULL;

   /* Get distance. */
   if (v2 == NULL)
      dist2 = vec2_odist2(v1);
   else
      dist2 = vec2_dist2(v1, v2);

   /* Return the distance. */
   lua_pushnumber(L, dist2);
   return 1;
}

/**
 * @brief Gets the modulus of the vector.
 *    @luatparam Vec2 v Vector to get modulus of.
 *    @luatreturn number The modulus of the vector.
 * @luafunc mod
 */
static int vectorL_mod( lua_State *L )
{
   vec2 *v = luaL_checkvector(L,1);
   lua_pushnumber(L, VMOD(*v));
   return 1;
}

/**
 * @brief Gets the angle of the vector.
 *    @luatparam Vec2 v Vector to get angle of.
 *    @luatreturn number The angle of the vector.
 * @luafunc angle
 */
static int vectorL_angle( lua_State *L )
{
   vec2 *v = luaL_checkvector(L,1);
   lua_pushnumber(L, VANGLE(*v));
   return 1;
}

/**
 * @brief Normalizes a vector.
 *    @luatparam Vec2 v Vector to normalize.
 *    @luatreturn Vec2 Normalized vector.
 * @luafunc normalize
 */
static int vectorL_normalize( lua_State *L )
{
   vec2 *v = luaL_checkvector(L,1);
   double m = VMOD(*v);
   v->x /= m;
   v->y /= m;
   lua_pushvector(L, *v);
   return 1;
}

/**
 * @brief Sees if two line segments collide.
 *
 *    @luatparam Vec2 s1 Start point of the first segment.
 *    @luatparam Vec2 e1 End point of the first segment.
 *    @luatparam Vec2 s2 Start point of the second segment.
 *    @luatparam Vec2 e2 End point of the second segment.
 *    @luatreturn integer 0 if they don't collide, 1 if they collide on a point, 2 if they are parallel, and 3 if they are coincident.
 * @luafunc collideLineLine
 */
static int vectorL_collideLineLine( lua_State *L )
{
   vec2 *s1 = luaL_checkvector(L,1);
   vec2 *e1 = luaL_checkvector(L,2);
   vec2 *s2 = luaL_checkvector(L,3);
   vec2 *e2 = luaL_checkvector(L,4);
   vec2 crash;
   int ret = CollideLineLine( s1->x, s1->y, e1->x, e1->y, s2->x, s2->y, e2->x, e2->y, &crash );
   lua_pushinteger( L, ret );
   lua_pushvector( L, crash );
   return 2;
}

/**
 * @brief Computes the intersection of a line segment and a circle.
 *
 *    @luatparam Vector center Center of the circle.
 *    @luatparam number radius Radius of the circle.
 *    @luatparam Vector p1 First point of the line segment.
 *    @luatparam Vector p2 Second point of the line segment.
 *    @luatreturn Vector|nil First point of collision or nil if no collision.
 *    @luatreturn Vector|nil Second point of collision or nil if single-point collision.
 * @luafunc collideCircleLine
 */
static int vectorL_collideCircleLine( lua_State *L )
{
   vec2 *center, *p1, *p2, crash[2];
   double radius;

   center = luaL_checkvector( L, 1 );
   radius = luaL_checknumber( L, 2 );
   p1     = luaL_checkvector( L, 3 );
   p2     = luaL_checkvector( L, 4 );

   int cnt = CollideLineCircle( p1, p2, center, radius, crash );
   if (cnt>0)
      lua_pushvector( L, crash[0] );
   if (cnt>1)
      lua_pushvector( L, crash[1] );

   return cnt;
}
