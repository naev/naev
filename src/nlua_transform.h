/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_TRANSFORM_H
#  define NLUA_TRANSFORM_H


#include "nlua.h"
#include "opengl.h"
#include "opengl_matrix.h"


#define TRANSFORM_METATABLE      "transform" /**< Transform metatable identifier. */


/*
 * Library loading
 */
int nlua_loadTransform( nlua_env env );

/*
 * Transform operations
 */
gl_Matrix4* lua_totransform( lua_State *L, int ind );
gl_Matrix4* luaL_checktransform( lua_State *L, int ind );
gl_Matrix4* lua_pushtransform( lua_State *L, gl_Matrix4 Transform );
int lua_istransform( lua_State *L, int ind );


#endif /* NLUA_TRANSFORM_H */


