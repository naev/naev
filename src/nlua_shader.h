/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef NLUA_SHADER_H
#  define NLUA_SHADER_H


#include "nlua.h"
#include "opengl.h"


#define SHADER_METATABLE      "shader" /**< Shader metatable identifier. */

#define SHADER_NAME_MAXLEN    32 /**< Maximum length of the name of a uniform variable. */


/* Helper. */
#define luaL_optshader(L,ind,def)   nluaL_optarg(L,ind,def,luaL_checkshader)


typedef struct LuaUniform_s {
   GLuint id;
   GLint size;
   GLenum type;
   char name[SHADER_NAME_MAXLEN];
} LuaUniform_t;

typedef struct LuaShader_s {
   GLuint program;
   /* Shared Uniforms. */
   GLuint ViewSpaceFromLocal;
   GLuint ClipSpaceFromView;
   GLuint ClipSpaceFromLocal;
   GLuint ViewNormalFromLocal;
   GLuint love_ScreenSize;
   /* Fragment Shader. */
   GLuint MainTex;
   /* Vertex Shader. */
   GLuint VertexPosition;
   GLuint VertexTexCoord;
   GLuint VertexColor;
   GLuint ConstantColor;
   /* Other uniforms. */
   LuaUniform_t *uniforms;
   GLint nuniforms;
} LuaShader_t;


/*
 * Library loading
 */
int nlua_loadShader( nlua_env env );

/*
 * Shader operations
 */
LuaShader_t* lua_toshader( lua_State *L, int ind );
LuaShader_t* luaL_checkshader( lua_State *L, int ind );
LuaShader_t* lua_pushshader( lua_State *L, LuaShader_t shader );
int lua_isshader( lua_State *L, int ind );


#endif /* NLUA_SHADER_H */


