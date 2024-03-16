/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_transform.c
 *
 * @brief Handles transforms.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_transform.h"

#include "log.h"
#include "nluadef.h"

/* Transform metatable methods. */
static int transformL_eq( lua_State *L );
static int transformL_new( lua_State *L );
static int transformL_mul( lua_State *L );
static int transformL_get( lua_State *L );
static int transformL_set( lua_State *L );
static int transformL_scale( lua_State *L );
static int transformL_translate( lua_State *L );
static int transformL_rotate2d( lua_State *L );
static int transformL_ortho( lua_State *L );
static int transformL_applyPoint( lua_State *L );
static int transformL_applyDim( lua_State *L );

static const luaL_Reg transformL_methods[] = {
   { "__eq", transformL_eq },
   { "__mul", transformL_mul },
   { "get", transformL_get },
   { "set", transformL_set },
   { "new", transformL_new },
   { "scale", transformL_scale },
   { "translate", transformL_translate },
   { "rotate2d", transformL_rotate2d },
   { "ortho", transformL_ortho },
   { "applyPoint", transformL_applyPoint },
   { "applyDim", transformL_applyDim },
   { 0, 0 } }; /**< Transform metatable methods. */

/**
 * @brief Loads the transform library.
 *
 *    @param env Environment to load transform library into.
 *    @return 0 on success.
 */
int nlua_loadTransform( nlua_env env )
{
   nlua_register( env, TRANSFORM_METATABLE, transformL_methods, 1 );
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
mat4 *lua_totransform( lua_State *L, int ind )
{
   return (mat4 *)lua_touserdata( L, ind );
}
/**
 * @brief Gets transform at index or raises error if there is no transform at
 * index.
 *
 *    @param L Lua state to get transform from.
 *    @param ind Index position to find transform.
 *    @return Transform found at the index in the state.
 */
mat4 *luaL_checktransform( lua_State *L, int ind )
{
   if ( lua_istransform( L, ind ) )
      return lua_totransform( L, ind );
   luaL_typerror( L, ind, TRANSFORM_METATABLE );
   return NULL;
}
/**
 * @brief Pushes a transform on the stack.
 *
 *    @param L Lua state to push transform into.
 *    @param transform Transform to push.
 *    @return Newly pushed transform.
 */
mat4 *lua_pushtransform( lua_State *L, mat4 transform )
{
   mat4 *t = (mat4 *)lua_newuserdata( L, sizeof( mat4 ) );
   *t      = transform;
   luaL_getmetatable( L, TRANSFORM_METATABLE );
   lua_setmetatable( L, -2 );
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

   if ( lua_getmetatable( L, ind ) == 0 )
      return 0;
   lua_getfield( L, LUA_REGISTRYINDEX, TRANSFORM_METATABLE );

   ret = 0;
   if ( lua_rawequal( L, -1, -2 ) ) /* does it have the correct mt? */
      ret = 1;

   lua_pop( L, 2 ); /* remove both metatables */
   return ret;
}

/**
 * @brief Compares two transforms to see if they are the same.
 *
 *    @luatparam Transform t1 Transform 1 to compare.
 *    @luatparam Transform t2 Transform 2 to compare.
 *    @luatreturn boolean true if both transforms are the same.
 * @luafunc __eq
 */
static int transformL_eq( lua_State *L )
{
   const mat4 *t1 = luaL_checktransform( L, 1 );
   const mat4 *t2 = luaL_checktransform( L, 2 );
   lua_pushboolean( L, ( memcmp( t1, t2, sizeof( mat4 ) ) == 0 ) );
   return 1;
}

/**
 * @brief Creates a new identity transform.Gets a transform.
 *
 *    @luatreturn Transform A new transform corresponding to an identity matrix.
 * @luafunc new
 */
static int transformL_new( lua_State *L )
{
   if ( lua_istransform( L, 1 ) ) {
      const mat4 *M = lua_totransform( L, 1 );
      lua_pushtransform( L, *M );
   } else
      lua_pushtransform( L, mat4_identity() );
   return 1;
}

/**
 * @brief Multiplies two transforms (A*B).
 *
 *    @luatparam Transform A First element to multiply.
 *    @luatparam Transform B Second element to multiply.
 *    @luatreturn Transform Result of multiplication.
 * @luafunc __mul
 */
static int transformL_mul( lua_State *L )
{
   const mat4 *A = luaL_checktransform( L, 1 );
   const mat4 *B = luaL_checktransform( L, 2 );
   mat4        C;
   mat4_mul( &C, A, B );
   lua_pushtransform( L, C );
   return 1;
}

/**
 * @brief Gets all the values of the transform.
 *
 * Note, this returns in column-major.
 *
 *    @luatparam Transform T Transform te get parameters of.
 *    @luatreturn table 2D table containing all the values of the transform.
 * @luafunc get
 */
static int transformL_get( lua_State *L )
{
   mat4 *M = luaL_checktransform( L, 1 );
   lua_newtable( L ); /* t */
   for ( int i = 0; i < 4; i++ ) {
      lua_newtable( L ); /* t, t */
      for ( int j = 0; j < 4; j++ ) {
         lua_pushnumber( L, M->m[j][i] ); /* t, t, n */
         lua_rawseti( L, -2, j + 1 );     /* t, t */
      }
      lua_rawseti( L, -2, i + 1 ); /* t */
   }
   return 1;
}

/**
 * @brief Sets an element of a transform.
 *
 *    @luatparam Transform T Transform to set element of.
 *    @luatparam number i Column to set.
 *    @luatparam number j Row to set.
 *    @luatparam number v Value to set to.
 * @luafunc set
 */
static int transformL_set( lua_State *L )
{
   mat4  *M = luaL_checktransform( L, 1 );
   int    i = luaL_checkinteger( L, 2 ) - 1;
   int    j = luaL_checkinteger( L, 3 ) - 1;
   double v = luaL_checknumber( L, 4 );
#if DEBUGGING
   if ( i < 0 || i > 3 ) {
      WARN( _( "Matrix column value not in range: %d" ), i );
      i = CLAMP( 0, 3, i );
   }
   if ( j < 0 || j > 3 ) {
      WARN( _( "Matrix row value not in range: %d" ), j );
      j = CLAMP( 0, 3, j );
   }
#endif /* DEBUGGING */
   M->m[i][j] = v;
   return 0;
}

/**
 * @brief Applies scaling to a transform.
 *
 *    @luatparam Transform T Transform to apply scaling to.
 *    @luatparam number x X-axis scaling.
 *    @luatparam number y Y-axis scaling.
 *    @luatparam number z Z-axis scaling.
 *    @luatreturn Transform A new transformation.
 * @luafunc scale
 */
static int transformL_scale( lua_State *L )
{
   const mat4 *M   = luaL_checktransform( L, 1 );
   double      x   = luaL_checknumber( L, 2 );
   double      y   = luaL_checknumber( L, 3 );
   double      z   = luaL_optnumber( L, 4, 1. );
   mat4        out = *M;
   mat4_scale( &out, x, y, z );
   lua_pushtransform( L, out );
   return 1;
}

/**
 * @brief Applies translation to a transform.
 *
 *    @luatparam Transform T Transform to apply translation to.
 *    @luatparam number x X-axis translation.
 *    @luatparam number y Y-axis translation.
 *    @luatparam number z Z-axis translation.
 *    @luatreturn Transform A new transformation.
 * @luafunc translate
 */
static int transformL_translate( lua_State *L )
{
   const mat4 *M   = luaL_checktransform( L, 1 );
   double      x   = luaL_checknumber( L, 2 );
   double      y   = luaL_checknumber( L, 3 );
   double      z   = luaL_optnumber( L, 4, 0. );
   mat4        out = *M;
   mat4_translate( &out, x, y, z );
   lua_pushtransform( L, out );
   return 1;
}

/**
 * @brief Applies a 2D rotation (along Z-axis) to a transform.
 *
 *    @luatparam Transform T Transform to apply rotation to.
 *    @luatparam number angle Angle to rotate (radians).
 * @luafunc rotate2d
 */
static int transformL_rotate2d( lua_State *L )
{
   const mat4 *M   = luaL_checktransform( L, 1 );
   double      a   = luaL_checknumber( L, 2 );
   mat4        out = *M;
   mat4_rotate2d( &out, a );
   lua_pushtransform( L, out );
   return 1;
}

/**
 * @brief Creates an orthogonal matrix.
 *
 *    @luatparam number left Left value.
 *    @luatparam number right Right value.
 *    @luatparam number bottom Bottom value.
 *    @luatparam number top Top value.
 *    @luatparam number nearVal value.
 *    @luatparam number farVal value.
 *    @luatreturn Transform A new transformation.
 * @luafunc translate
 */
static int transformL_ortho( lua_State *L )
{
   double left    = luaL_checknumber( L, 1 );
   double right   = luaL_checknumber( L, 2 );
   double bottom  = luaL_checknumber( L, 3 );
   double top     = luaL_checknumber( L, 4 );
   double nearVal = luaL_checknumber( L, 5 );
   double farVal  = luaL_checknumber( L, 6 );
   lua_pushtransform( L,
                      mat4_ortho( left, right, bottom, top, nearVal, farVal ) );
   return 1;
}

/**
 * @brief Applies a transformation to a point.
 *
 *    @luatparam Transform T Transform to apply.
 *    @luatparam number x Point X-coordinate.
 *    @luatparam number y Point Y-coordinate.
 *    @luatparam number z Point Z-coordinate.
 *    @luatreturn number New X coordinate.
 *    @luatreturn number New Y coordinate.
 *    @luatreturn number New Z coordinate.
 * @luafunc applyPoint
 */
static int transformL_applyPoint( lua_State *L )
{
   double gp[3], p[3];
   mat4  *M = luaL_checktransform( L, 1 );
   gp[0]    = luaL_checknumber( L, 2 );
   gp[1]    = luaL_checknumber( L, 3 );
   gp[2]    = luaL_checknumber( L, 4 );

   for ( int i = 0; i < 3; i++ )
      p[i] = M->m[0][i] * gp[0] + M->m[1][i] * gp[1] + M->m[2][i] * gp[2] +
             M->m[3][i];

   lua_pushnumber( L, p[0] );
   lua_pushnumber( L, p[1] );
   lua_pushnumber( L, p[2] );
   return 3;
}

/**
 * @brief Applies a transformation to a dimension.
 *
 * @note This is similar to Transform.applyPoint, except the translation is not
 * applied.
 *
 *    @luatparam Transform T Transform to apply.
 *    @luatparam number x Dimension X-coordinate.
 *    @luatparam number y Dimension Y-coordinate.
 *    @luatparam number z Dimension Z-coordinate.
 *    @luatreturn number New X coordinate.
 *    @luatreturn number New Y coordinate.
 *    @luatreturn number New Z coordinate.
 * @luafunc applyDim
 */
static int transformL_applyDim( lua_State *L )
{
   double gp[3], p[3];
   mat4  *M = luaL_checktransform( L, 1 );
   gp[0]    = luaL_checknumber( L, 2 );
   gp[1]    = luaL_checknumber( L, 3 );
   gp[2]    = luaL_checknumber( L, 4 );

   for ( int i = 0; i < 3; i++ )
      p[i] = M->m[0][i] * gp[0] + M->m[1][i] * gp[1] + M->m[2][i] * gp[2];

   lua_pushnumber( L, p[0] );
   lua_pushnumber( L, p[1] );
   lua_pushnumber( L, p[2] );
   return 3;
}
