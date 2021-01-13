/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_shader.c
 *
 * @brief Handles shaders.
 */

/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_shader.h"

#include "log.h"
#include "ndata.h"
#include "nluadef.h"


/* Shader metatable methods. */
static int shaderL_gc( lua_State *L );
static int shaderL_eq( lua_State *L );
static int shaderL_new( lua_State *L );
static const luaL_Reg shaderL_methods[] = {
   { "__gc", shaderL_gc },
   { "__eq", shaderL_eq },
   { "new", shaderL_new },
   {0,0}
}; /**< Shader metatable methods. */


/**
 * @brief Loads the shader library.
 *
 *    @param env Environment to load shader library into.
 *    @return 0 on success.
 */
int nlua_loadShader( nlua_env env )
{
   nlua_register(env, SHADER_METATABLE, shaderL_methods, 1);
   return 0;
}


/**
 * @brief Lua bindings to interact with shaders.
 *
 * @luamod shader
 */
/**
 * @brief Gets shader at index.
 *
 *    @param L Lua state to get shader from.
 *    @param ind Index position to find the shader.
 *    @return Shader found at the index in the state.
 */
LuaShader_t* lua_toshader( lua_State *L, int ind )
{
   return (LuaShader_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets shader at index or raises error if there is no shader at index.
 *
 *    @param L Lua state to get shader from.
 *    @param ind Index position to find shader.
 *    @return Shader found at the index in the state.
 */
LuaShader_t* luaL_checkshader( lua_State *L, int ind )
{
   if (lua_isshader(L,ind))
      return lua_toshader(L,ind);
   luaL_typerror(L, ind, SHADER_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a shader on the stack.
 *
 *    @param L Lua state to push shader into.
 *    @param shader Shader to push.
 *    @return Newly pushed shader.
 */
LuaShader_t* lua_pushshader( lua_State *L, LuaShader_t shader )
{
   LuaShader_t *c;
   c = (LuaShader_t*) lua_newuserdata(L, sizeof(LuaShader_t));
   *c = shader;
   luaL_getmetatable(L, SHADER_METATABLE);
   lua_setmetatable(L, -2);
   return c;
}
/**
 * @brief Checks to see if ind is a shader.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a shader.
 */
int lua_isshader( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, SHADER_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Frees a shader.
 *
 *    @luatparam Shader shader Shader to free.
 * @luafunc __gc
 */
static int shaderL_gc( lua_State *L )
{
   LuaShader_t *shader = luaL_checkshader(L,1);
   glDeleteProgram( shader->program );
   return 0;
}


/**
 * @brief Compares two shaders to see if they are the same.
 *
 *    @luatparam Shader f1 Shader 1 to compare.
 *    @luatparam Shader f2 Shader 2 to compare.
 *    @luatreturn boolean true if both shaders are the same.
 * @luafunc __eq
 */
static int shaderL_eq( lua_State *L )
{
   LuaShader_t *f1, *f2;
   f1 = luaL_checkshader(L,1);
   f2 = luaL_checkshader(L,2);
   lua_pushboolean( L, (memcmp( f1, f2, sizeof(LuaShader_t) )==0) );
   return 1;
}


/**
 * @brief Creates a new shader.
 *
 *    @luatparam string vertex Script containing the vertex shader.
 *    @luatparam string fragment String containing the fragment shader.
 *    @luatreturn Shader A newly created shader.
 * @luafunc new
 */
static int shaderL_new( lua_State *L )
{
   LuaShader_t shader;
   const char *pixelcode, *vertexcode;

   /* Get arguments. */
   pixelcode  = luaL_checkstring(L,1);
   vertexcode = luaL_checkstring(L,2);

   /* Do from string. */
   shader.program = gl_program_vert_frag_string( vertexcode, strlen(vertexcode),  pixelcode, strlen(pixelcode) );
   if (shader.program == 0)
      NLUA_ERROR(L,_("shader failed to compile!"));

   /* Set up defaults. */
#define ATTRIB(name) \
   shader.name = glGetAttribLocation( shader.program, #name )
#define UNIFORM(name) \
   shader.name = glGetUniformLocation( shader.program, #name )
   UNIFORM( ViewSpaceFromLocal );
   UNIFORM( ClipSpaceFromView );
   UNIFORM( ClipSpaceFromLocal );
   UNIFORM( ViewNormalFromLocal );
   UNIFORM( MainTex );
   ATTRIB( VertexPosition );
   ATTRIB( VertexTexCoord );
   ATTRIB( VertexColor );
   ATTRIB( ConstantColor );

   gl_checkErr();

   lua_pushshader( L, shader );
   return 1;
}
