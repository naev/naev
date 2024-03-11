/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

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
   GLint tex;
} LuaUniform_t;

typedef struct LuaTexture_s {
   GLenum active;
   GLuint texid;
   GLint uniform;
   GLint value;
} LuaTexture_t;

typedef struct LuaShader_s {
   GLuint program;
   /* Shared Uniforms. */
   GLint ViewSpaceFromLocal;
   GLint ClipSpaceFromView;
   GLint ClipSpaceFromLocal;
   GLint ViewNormalFromLocal;
   GLint love_ScreenSize;
   /* Fragment Shader. */
   GLint MainTex;
   /* Vertex Shader. */
   GLint VertexPosition;
   GLint VertexTexCoord;
   GLint VertexColour;
   GLint ConstantColour;
   /* Other uniforms. */
   LuaUniform_t *uniforms;
   GLint nuniforms;
   /* Other stuff. */
   LuaTexture_t *tex;
   unsigned int pp_id; /**< Post-processing ID if set. */
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
