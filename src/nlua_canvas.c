/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @canvas nlua_canvas.c
 *
 * @brief Handles canvass.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_canvas.h"

#include "log.h"
#include "nluadef.h"
#include "nlua_tex.h"
#include "nlua_colour.h"
#include "render.h"

static int nlua_canvas_counter = 0;
static GLuint previous_fbo = 0;
static int previous_fbo_set = 0;
static int was_scissored    = 0;

/* Canvas metatable methods. */
static int canvasL_gc( lua_State *L );
static int canvasL_eq( lua_State *L );
static int canvasL_new( lua_State *L );
static int canvasL_set( lua_State *L );
static int canvasL_dims( lua_State *L );
static int canvasL_getTex( lua_State *L );
static int canvasL_clear( lua_State *L );
static const luaL_Reg canvasL_methods[] = {
   { "__gc", canvasL_gc },
   { "__eq", canvasL_eq },
   { "new", canvasL_new },
   { "set", canvasL_set },
   { "dims", canvasL_dims },
   { "getTex", canvasL_getTex },
   { "clear", canvasL_clear },
   {0,0}
}; /**< Canvas metatable methods. */

/**
 * @brief Loads the canvas library.
 *
 *    @param env Environment to load canvas library into.
 *    @return 0 on success.
 */
int nlua_loadCanvas( nlua_env env )
{
   nlua_register(env, CANVAS_METATABLE, canvasL_methods, 1);
   return 0;
}

/**
 * @brief Lua bindings to interact with canvass.
 *
 * @note The API here is designed to be compatible with that of LÖVE.
 *
 * @luamod canvas
 */
/**
 * @brief Gets canvas at index.
 *
 *    @param L Lua state to get canvas from.
 *    @param ind Index position to find the canvas.
 *    @return Canvas found at the index in the state.
 */
LuaCanvas_t* lua_tocanvas( lua_State *L, int ind )
{
   return (LuaCanvas_t*) lua_touserdata(L,ind);
}
/**
 * @brief Gets canvas at index or raises error if there is no canvas at index.
 *
 *    @param L Lua state to get canvas from.
 *    @param ind Index position to find canvas.
 *    @return Canvas found at the index in the state.
 */
LuaCanvas_t* luaL_checkcanvas( lua_State *L, int ind )
{
   if (lua_iscanvas(L,ind))
      return lua_tocanvas(L,ind);
   luaL_typerror(L, ind, CANVAS_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a canvas on the stack.
 *
 *    @param L Lua state to push canvas into.
 *    @param canvas Canvas to push.
 *    @return Newly pushed canvas.
 */
LuaCanvas_t* lua_pushcanvas( lua_State *L, LuaCanvas_t canvas )
{
   LuaCanvas_t *c = (LuaCanvas_t*) lua_newuserdata(L, sizeof(LuaCanvas_t));
   *c = canvas;
   luaL_getmetatable(L, CANVAS_METATABLE);
   lua_setmetatable(L, -2);
   return c;
}
/**
 * @brief Checks to see if ind is a canvas.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a canvas.
 */
int lua_iscanvas( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, CANVAS_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}

/**
 * @brief Frees a canvas.
 *
 *    @luatparam Canvas canvas Canvas to free.
 * @luafunc __gc
 */
static int canvasL_gc( lua_State *L )
{
   const LuaCanvas_t *lc = luaL_checkcanvas(L,1);
   glDeleteFramebuffers( 1, &lc->fbo );
   gl_freeTexture( lc->tex );
   gl_checkErr();
   return 0;
}

/**
 * @brief Compares two canvass to see if they are the same.
 *
 *    @luatparam Canvas f1 Canvas 1 to compare.
 *    @luatparam Canvas f2 Canvas 2 to compare.
 *    @luatreturn boolean true if both canvass are the same.
 * @luafunc __eq
 */
static int canvasL_eq( lua_State *L )
{
   LuaCanvas_t *c1, *c2;
   c1 = luaL_checkcanvas(L,1);
   c2 = luaL_checkcanvas(L,2);
   lua_pushboolean( L, (memcmp( c1, c2, sizeof(LuaCanvas_t) )==0) );
   return 1;
}

/**
 * @brief Initializes a new canvas.
 *
 *    @param lc Canvas to initialize.
 *    @param w Width to use.
 *    @param h Height to use.
 */
int canvas_new( LuaCanvas_t *lc, int w, int h )
{
   GLenum status;
   char *name;

   memset( lc, 0, sizeof(LuaCanvas_t) );

   /* Create the texture. */
   SDL_asprintf( &name, "nlua_canvas_%03d", ++nlua_canvas_counter );
   lc->tex = gl_loadImageData( NULL, w, h, 1, 1, name );
   lc->tex->flags |= OPENGL_TEX_VFLIP; /* Long story, but love stuff inverts Y axis for canvases so we have to redo that here for spob targetting stuff to work properly. */
   free( name );

   /* Create the frame buffer. */
   glGenFramebuffers( 1, &lc->fbo );
   glBindFramebuffer(GL_FRAMEBUFFER, lc->fbo);

   /* Attach the colour buffer. */
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lc->tex->texture, 0);

   /* Check status. */
   status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (status != GL_FRAMEBUFFER_COMPLETE)
      return -1;

   /* Clear the canvas. */
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* Restore state. */
   glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);

   gl_checkErr();
   return 0;
}

/**
 * @brief Opens a new canvas.
 *
 *    @luatparam number width Width of the new canvas.
 *    @luatparam number height Height of the new canvas.
 *    @luatreturn Canvas New canvas object.
 * @luafunc new
 */
static int canvasL_new( lua_State *L )
{
   LuaCanvas_t lc;

   int w = luaL_checkint(L,1);
   int h = luaL_checkint(L,2);

   if (canvas_new( &lc, w, h ))
      return NLUA_ERROR( L, _("Error setting up framebuffer!"));
   lua_pushcanvas( L, lc );
   return 1;
}

/**
 * @brief Sets the active canvas.
 *
 *    @luatparam Canvas|nil arg Either a canvas object or nil to disable.
 * @luafunc set
 * @todo Add actual graphics state maintenance! For now, just disable the scissor region.
 */
static int canvasL_set( lua_State *L )
{
   if (!lua_isnoneornil(L,1)) {
      const LuaCanvas_t *lc = luaL_checkcanvas(L,1);
      if (!previous_fbo_set) {
         previous_fbo = gl_screen.current_fbo;
         previous_fbo_set = 1;
         was_scissored = glIsEnabled(GL_SCISSOR_TEST);
      }
      gl_screen.current_fbo = lc->fbo;
      glDisable(GL_SCISSOR_TEST);
      glViewport( 0, 0, lc->tex->w, lc->tex->h );
      glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);
      render_needsReset();
   }
   else
      canvas_reset();

   return 0;
}

/**
 * @brief Gets the size of the canvas.
 *
 *    @luatreturn number Width of the canvas.
 *    @luatreturn number Height of the canvas.
 * @luafunc dims
 */
static int canvasL_dims( lua_State *L )
{
   const LuaCanvas_t *lc = luaL_checkcanvas(L,1);
   lua_pushnumber( L, lc->tex->w );
   lua_pushnumber( L, lc->tex->h );
   return 2;
}

/**
 * @brief Gets the texture associated with the canvas.
 *
 *    @luatparam Canvas canvas Canvas to get the texture from.
 *    @luatreturn Tex Texture associated with the canvas.
 * @luafunc getTex
 */
static int canvasL_getTex( lua_State *L )
{
   const LuaCanvas_t *lc = luaL_checkcanvas(L,1);
   lua_pushtex( L, gl_dupTexture(lc->tex) );
   return 1;
}

/**
 * @brief Clears a canvas.
 *
 *    @luatparam Canvas canvas Canvas to clear.
 *    @luatparam Colour col Colour to clear to.
 * @luafunc clear
 */
static int canvasL_clear( lua_State *L )
{
   const LuaCanvas_t *lc = luaL_checkcanvas(L,1);
   (void) lc; /* Just to enforce good practice, canvas should be already set. */
   const glColour *c = luaL_optcolour(L,2,&cBlack);
   glClearColor( c->r, c->g, c->b, c->a );
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glClearColor( 0.0, 0.0, 0.0, 1.0 );
   return 0;
}

/**
 * @brief Resets the canvas state if applicable.
 */
void canvas_reset (void)
{
   if (!previous_fbo_set)
      return;
   gl_screen.current_fbo = previous_fbo;
   previous_fbo_set = 0;
   if (was_scissored)
      glEnable(GL_SCISSOR_TEST);
   glViewport( 0, 0, gl_screen.rw, gl_screen.rh );
   glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);
}
