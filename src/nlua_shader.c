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
#include "array.h"
#include "nlua_tex.h"
#include "render.h"

/* Shader metatable methods. */
static int shaderL_gc( lua_State *L );
static int shaderL_eq( lua_State *L );
static int shaderL_new( lua_State *L );
static int shaderL_send( lua_State *L );
static int shaderL_sendRaw( lua_State *L );
static int shaderL_hasUniform( lua_State *L );
static int shaderL_addPostProcess( lua_State *L );
static int shaderL_rmPostProcess( lua_State *L );
static const luaL_Reg shaderL_methods[] = {
   { "__gc", shaderL_gc },
   { "__eq", shaderL_eq },
   { "new", shaderL_new },
   { "send", shaderL_send },
   { "sendRaw", shaderL_sendRaw },
   { "hasUniform", shaderL_hasUniform },
   { "addPPShader", shaderL_addPostProcess },
   { "rmPPShader", shaderL_rmPostProcess },
   {0,0}
}; /**< Shader metatable methods. */

/* Useful stuff. */
static int shader_compareUniform( const void *a, const void *b);
static int shader_searchUniform( const void *id, const void *u );
static LuaUniform_t *shader_getUniform( const LuaShader_t *ls, const char *name );
static int shaderL_sendHelper( lua_State *L, int ignore_missing );

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
   LuaShader_t *c = (LuaShader_t*) lua_newuserdata(L, sizeof(LuaShader_t));
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
   if (shader->pp_id > 0)
      render_postprocessRm( shader->pp_id );
   glDeleteProgram( shader->program );
   array_free( shader->tex );
   free(shader->uniforms);
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

/*
 * For qsort.
 */
static int shader_compareUniform( const void *a, const void *b )
{
   const LuaUniform_t *u1, *u2;
   u1 = (const LuaUniform_t*) a;
   u2 = (const LuaUniform_t*) b;
   return strcmp(u1->name, u2->name);
}

static int shader_searchUniform( const void *id, const void *u )
{
   return strcmp( (const char*)id, ((LuaUniform_t*)u)->name );
}

static LuaUniform_t *shader_getUniform( const LuaShader_t *ls, const char *name )
{
   return bsearch( name, ls->uniforms, ls->nuniforms, sizeof(LuaUniform_t), shader_searchUniform );
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
   GLint ntex;
   GLsizei length;

   /* Get arguments. */
   pixelcode  = luaL_checkstring(L,1);
   vertexcode = luaL_checkstring(L,2);

   /* Initialize. */
   memset( &shader, 0, sizeof(shader) );

   /* Do from string. */
   shader.program = gl_program_vert_frag_string( vertexcode, strlen(vertexcode),  pixelcode, strlen(pixelcode) );
   if (shader.program == 0)
      return NLUA_ERROR(L,_("shader failed to compile!"));

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
   UNIFORM( ConstantColour );
   UNIFORM( love_ScreenSize );
   ATTRIB( VertexPosition );
   ATTRIB( VertexTexCoord );
   ATTRIB( VertexColour );
#undef ATTRIB
#undef UNIFORM

   /* Do other uniforms. */
   glGetProgramiv( shader.program, GL_ACTIVE_UNIFORMS, &shader.nuniforms );
   shader.uniforms = calloc( shader.nuniforms, sizeof(LuaUniform_t) );
   ntex = 0;
   for (GLint i=0; i<shader.nuniforms; i++) {
      LuaUniform_t *u = &shader.uniforms[i];
      glGetActiveUniform( shader.program, (GLuint)i, SHADER_NAME_MAXLEN, &length, &u->size, &u->type, u->name );
      u->id = glGetUniformLocation( shader.program, u->name );
      u->tex = -1;

      /* Textures need special care. */
      if ((u->type==GL_SAMPLER_2D) && (strcmp(u->name,"MainTex")!=0)) {
         if (shader.tex == NULL)
            shader.tex = array_create(LuaTexture_t);
         LuaTexture_t *t = &array_grow( &shader.tex );
         ntex++;
         t->active = GL_TEXTURE0+ntex;
         t->texid = 0;
         t->uniform = u->id;
         t->value = ntex;
         u->tex = ntex-1;
      }
   }
   qsort( shader.uniforms, shader.nuniforms, sizeof(LuaUniform_t), shader_compareUniform );

   /* Check if there are textures. */

   gl_checkErr();

   lua_pushshader( L, shader );
   return 1;
}

/**
 * @brief Helper to parse up float vector (or arguments).
 */
void shader_parseUniformArgsFloat( GLfloat values[4], lua_State *L, int idx, int n )
{
   if (lua_istable(L,idx)) {
      for (int j=0; j<n; j++) {
         lua_pushnumber(L,j+1);
         lua_gettable(L,idx);
         values[j] = luaL_checknumber(L,-1);
      }
      lua_pop(L,n);
   }
   else {
      for (int j=0; j<n; j++)
         values[j] = luaL_checknumber(L,idx+j);
   }
}

/**
 * @brief Helper to parse up integer vector (or arguments).
 */
void shader_parseUniformArgsInt( GLint values[4], lua_State *L, int idx, int n )
{
   if (lua_istable(L,idx)) {
      for (int j=0; j<n; j++) {
         lua_pushnumber(L,j+1);
         lua_gettable(L,idx);
         values[j] = luaL_checkint(L,-1);
      }
      lua_pop(L,n);
   }
   else {
      for (int j=0; j<n; j++)
         values[j] = luaL_checkint(L,idx+j);
   }
}

/**
 * @brief Allows setting values of uniforms for a shader. Errors out if the uniform is unknown or unused (as in optimized out by the compiler).
 *
 *    @luatparam Shader shader Shader to send uniform to.
 *    @luatparam string name Name of the uniform.
 * @luafunc send
 */
static int shaderL_send( lua_State *L )
{
   return shaderL_sendHelper( L, 0 );
}

/**
 * @brief Allows setting values of uniforms for a shader, while ignoring unknown (or unused) uniforms.
 *
 *    @luatparam Shader shader Shader to send uniform to.
 *    @luatparam string name Name of the uniform.
 * @luafunc send
 */
static int shaderL_sendRaw( lua_State *L )
{
   return shaderL_sendHelper( L, 1 );
}

/**
 * @brief Helper to set the uniform while handling unknown/inactive uniforms.
 */
static int shaderL_sendHelper( lua_State *L, int ignore_missing )
{
   LuaShader_t *ls;
   LuaUniform_t *u;
   const char *name;
   int idx;
   GLfloat values[4];
   GLint ivalues[4];
   glTexture *tex;

   ls = luaL_checkshader(L,1);
   name = luaL_checkstring(L,2);

   u = shader_getUniform( ls, name );
   if (u==NULL) {
      if (ignore_missing)
         return 0;
      return NLUA_ERROR(L,_("Shader does not have uniform '%s'!"), name);
   }

   /* With OpenGL 4.1 or ARB_separate_shader_objects, there
    * is no need to set the program first. */
   glUseProgram( ls->program );
   idx = 3;
   switch (u->type) {
      case GL_FLOAT:
         shader_parseUniformArgsFloat( values, L, idx, 1 );
         glUniform1f( u->id, values[0] );
         break;
      case GL_FLOAT_VEC2:
         shader_parseUniformArgsFloat( values, L, idx, 2 );
         glUniform2f( u->id, values[0], values[1] );
         break;
      case GL_FLOAT_VEC3:
         shader_parseUniformArgsFloat( values, L, idx, 3 );
         glUniform3f( u->id, values[0], values[1], values[2] );
         break;
      case GL_FLOAT_VEC4:
         shader_parseUniformArgsFloat( values, L, idx, 4 );
         glUniform4f( u->id, values[0], values[1], values[2], values[3] );
         break;

      case GL_INT:
         shader_parseUniformArgsInt( ivalues, L, idx, 1 );
         glUniform1i( u->id, ivalues[0] );
         break;
      case GL_INT_VEC2:
         shader_parseUniformArgsInt( ivalues, L, idx, 2 );
         glUniform2i( u->id, ivalues[0], ivalues[1] );
         break;
      case GL_INT_VEC3:
         shader_parseUniformArgsInt( ivalues, L, idx, 3 );
         glUniform3i( u->id, ivalues[0], ivalues[1], ivalues[2] );
         break;
      case GL_INT_VEC4:
         shader_parseUniformArgsInt( ivalues, L, idx, 4 );
         glUniform4i( u->id, ivalues[0], ivalues[1], ivalues[2], ivalues[3] );
         break;

      case GL_SAMPLER_2D:
         tex = luaL_checktex(L,idx);
         ls->tex[ u->tex ].texid = tex->texture;
         break;

      default:
         WARN(_("Unsupported shader uniform type '%d' for uniform '%s'. Ignoring."), u->type, u->name );
   }
   glUseProgram( 0 );

   gl_checkErr();

   return 0;
}

/**
 * @brief Checks to see if a shader has a uniform.
 *
 *    @luatparam Shader shader Shader to send uniform to.
 *    @luatparam string name Name of the uniform to check.
 *    @luatreturn boolean true if the shader has the uniform.
 * @luafunc hasUniform
 */
static int shaderL_hasUniform( lua_State *L )
{
   /* Parameters. */
   const LuaShader_t *ls = luaL_checkshader(L,1);
   const char *name = luaL_checkstring(L,2);

   /* Search. */
   lua_pushboolean(L, shader_getUniform(ls,name)!=NULL);
   return 1;
}

/**
 * @brief Sets a shader as a post-processing shader.
 *
 *    @luatparam Shader shader Shader to set as a post-processing shader.
 *    @luatparam[opt="final"] string layer Layer to add the shader to.
 *    @luatparam[opt=0] number priority Priority of the shader to set. Higher values mean it is run later.
 *    @luatreturn boolean true on success.
 * @luafunc addPPShader
 */
static int shaderL_addPostProcess( lua_State *L )
{
   LuaShader_t *ls = luaL_checkshader(L,1);
   const char *str = luaL_optstring(L,2,"final");
   int priority = luaL_optinteger(L,3,0);
   int layer = PP_LAYER_FINAL;

   if (strcmp(str,"final")==0)
      layer = PP_LAYER_FINAL;
   else if (strcmp(str,"game")==0)
      layer = PP_LAYER_GAME;
   else if (strcmp(str,"gui")==0)
      layer = PP_LAYER_GUI;
   else
      return NLUA_ERROR(L,_("Layer was '%s', but must be one of 'final' or 'game'"), str);

   if (ls->pp_id == 0)
      ls->pp_id = render_postprocessAdd( ls, layer, priority, 0 );
   lua_pushboolean(L, ls->pp_id>0);
   return 1;
}

/**
 * @brief Removes a shader as a post-processing shader.
 *
 *    @luatparam Shader shader Shader to disable as post-processing shader.
 *    @luatreturn boolean True on success.
 * @luafunc rmPPShader
 */
static int shaderL_rmPostProcess( lua_State *L )
{
   LuaShader_t *ls = luaL_checkshader(L,1);
   lua_pushboolean( L, render_postprocessRm( ls->pp_id ) );
   ls->pp_id = 0;
   return 1;
}
