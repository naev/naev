/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_font.c
 *
 * @brief Handles fonts.
 */

#include "nlua_font.h"

#include "naev.h"

#include <lauxlib.h>

#include "ndata.h"
#include "nluadef.h"
#include "log.h"


/* Font metatable methods. */
static int fontL_gc( lua_State *L );
static int fontL_eq( lua_State *L );
static int fontL_new( lua_State *L );
static int fontL_height( lua_State *L );
static const luaL_Reg fontL_methods[] = {
   { "__gc", fontL_gc },
   { "__eq", fontL_eq },
   { "new", fontL_new },
   { "height", fontL_height },
   {0,0}
}; /**< Font metatable methods. */




/**
 * @brief Loads the font library.
 *
 *    @param env Environment to load font library into.
 *    @return 0 on success.
 */
int nlua_loadFont( nlua_env env )
{
   nlua_register(env, FONT_METATABLE, fontL_methods, 1);
   return 0;
}


/**
 * @brief Lua bindings to interact with fonts.
 *
 * @luamod font
 */
/**
 * @brief Gets font at index.
 *
 *    @param L Lua state to get font from.
 *    @param ind Index position to find the font.
 *    @return Font found at the index in the state.
 */
glFont* lua_tofont( lua_State *L, int ind )
{
   return (glFont*) lua_touserdata(L,ind);
}
/**
 * @brief Gets font at index or raises error if there is no font at index.
 *
 *    @param L Lua state to get font from.
 *    @param ind Index position to find font.
 *    @return Font found at the index in the state.
 */
glFont* luaL_checkfont( lua_State *L, int ind )
{
   if (lua_isfont(L,ind))
      return lua_tofont(L,ind);
   luaL_typerror(L, ind, FONT_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a font on the stack.
 *
 *    @param L Lua state to push font into.
 *    @param font Font to push.
 *    @return Newly pushed font.
 */
glFont* lua_pushfont( lua_State *L, glFont font )
{
   glFont *c;
   c = (glFont*) lua_newuserdata(L, sizeof(glFont));
   *c = font;
   luaL_getmetatable(L, FONT_METATABLE);
   lua_setmetatable(L, -2);
   return c;
}
/**
 * @brief Checks to see if ind is a font.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a font.
 */
int lua_isfont( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, FONT_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Frees a font.
 *
 *    @luatparam Font font Font to free.
 * @luafunc __gc( font )
 */
static int fontL_gc( lua_State *L )
{
   gl_freeFont( luaL_checkfont(L, 1) );
   return 0;
}


/**
 * @brief Compares two fonts to see if they are the same.
 *
 *    @luatparam Font f1 Font 1 to compare.
 *    @luatparam Font f2 Font 2 to compare.
 *    @luatreturn boolean true if both fonts are the same.
 * @luafunc __eq( f1, f2 )
 */
static int fontL_eq( lua_State *L )
{
   glFont *f1, *f2;
   f1 = luaL_checkfont(L,1);
   f2 = luaL_checkfont(L,2);
   lua_pushboolean( L, (memcmp( f1, f2, sizeof(glFont) )==0) );
   return 1;
}


/**
 * @brief Gets a font.
 *
 *    @luatparam String|Number fontname Name of the font.
 *    @luatparam[opt=1.] Number Font height.
 *    @luatreturn Font A newly created font.
 * @luafunc new( r, g, b, a )
 */
static int fontL_new( lua_State *L )
{
   glFont font;
   int h;
   const char *fname;

   if (lua_gettop(L) > 1) {
      fname = FONT_DEFAULT_PATH;
      h = luaL_checkint(L,1);
   }
   else {
      fname = luaL_checkstring(L,1);
      h = luaL_checkint(L,2);
   }
   if (gl_fontInit( &font, fname, h ))
      NLUA_ERROR(L, _("failed to load font '%s'"), fname);

   lua_pushfont( L, font );
   return 1;
}


/**
 * @brief Gets the height of a font.
 *
 *    @luatparam Font f Font to get the height of.
 *    @luatreturn number Height of the font.
 * @luafunc height( f )
 */
static int fontL_height( lua_State *L )
{
   glFont *font = luaL_checkfont(L,1);
   lua_pushnumber(L, font->h);
   return 1;
}

