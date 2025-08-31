/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_font.c
 *
 * @brief Handles fonts.
 */
/** @cond */
#include <lauxlib.h>
/** @endcond */

#include "nlua_font.h"

#include "ndata.h"
#include "nluadef.h"

/* Font metatable methods. */
static int fontL_gc( lua_State *L );
static int fontL_eq( lua_State *L );
static int fontL_new( lua_State *L );
static int fontL_height( lua_State *L );
static int fontL_width( lua_State *L );
static int fontL_setFilter( lua_State *L );
static int fontL_addFallback( lua_State *L );

static const luaL_Reg fontL_methods[] = {
   { "__gc", fontL_gc },
   { "__eq", fontL_eq },
   { "new", fontL_new },
   { "height", fontL_height },
   { "width", fontL_width },
   { "setFilter", fontL_setFilter },
   { "addFallback", fontL_addFallback },
   { 0, 0 } }; /**< Font metatable methods. */

/**
 * @brief Loads the font library.
 *
 *    @param env Environment to load font library into.
 *    @return 0 on success.
 */
int nlua_loadFont( nlua_env *env )
{
   nlua_register( env, FONT_METATABLE, fontL_methods, 1 );
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
glFont *lua_tofont( lua_State *L, int ind )
{
   return (glFont *)lua_touserdata( L, ind );
}
/**
 * @brief Gets font at index or raises error if there is no font at index.
 *
 *    @param L Lua state to get font from.
 *    @param ind Index position to find font.
 *    @return Font found at the index in the state.
 */
glFont *luaL_checkfont( lua_State *L, int ind )
{
   if ( lua_isfont( L, ind ) )
      return lua_tofont( L, ind );
   luaL_typerror( L, ind, FONT_METATABLE );
   return NULL;
}
/**
 * @brief Pushes a font on the stack.
 *
 *    @param L Lua state to push font into.
 *    @param font Font to push.
 *    @return Newly pushed font.
 */
glFont *lua_pushfont( lua_State *L, glFont font )
{
   glFont *c = (glFont *)lua_newuserdata( L, sizeof( glFont ) );
   *c        = font;
   luaL_getmetatable( L, FONT_METATABLE );
   lua_setmetatable( L, -2 );
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

   if ( lua_getmetatable( L, ind ) == 0 )
      return 0;
   lua_getfield( L, LUA_REGISTRYINDEX, FONT_METATABLE );

   ret = 0;
   if ( lua_rawequal( L, -1, -2 ) ) /* does it have the correct mt? */
      ret = 1;

   lua_pop( L, 2 ); /* remove both metatables */
   return ret;
}

/**
 * @brief Frees a font.
 *
 *    @luatparam Font font Font to free.
 * @luafunc __gc
 */
static int fontL_gc( lua_State *L )
{
   gl_freeFont( luaL_checkfont( L, 1 ) );
   return 0;
}

/**
 * @brief Compares two fonts to see if they are the same.
 *
 *    @luatparam Font f1 Font 1 to compare.
 *    @luatparam Font f2 Font 2 to compare.
 *    @luatreturn boolean true if both fonts are the same.
 * @luafunc __eq
 */
static int fontL_eq( lua_State *L )
{
   glFont *f1, *f2;
   f1 = luaL_checkfont( L, 1 );
   f2 = luaL_checkfont( L, 2 );
   lua_pushboolean( L, ( memcmp( f1, f2, sizeof( glFont ) ) == 0 ) );
   return 1;
}

/**
 * @brief Gets a font.
 *
 *    @luatparam String|Number fontname Name of the font.
 *    @luatparam[opt=1.] size Number Font height.
 *    @luatreturn Font A newly created font.
 *    @luatreturn String Name of the newly created font.
 *    @luatreturn String Prefix of the newly created font.
 * @luafunc new
 */
static int fontL_new( lua_State *L )
{
   glFont      font;
   int         h;
   const char *fname, *prefix;

   if ( lua_gettop( L ) == 1 ) {
      fname = NULL;
      h     = luaL_checkint( L, 1 );
   } else {
      fname = luaL_optstring( L, 1, NULL );
      h     = luaL_checkint( L, 2 );
   }

   if ( fname == NULL ) {
      fname  = _( FONT_DEFAULT_PATH );
      prefix = FONT_PATH_PREFIX;
   } else
      prefix = "";

   if ( gl_fontInit( &font, fname, h, prefix, FONT_FLAG_DONTREUSE ) )
      return NLUA_ERROR( L, _( "failed to load font '%s'" ), fname );

   lua_pushfont( L, font );
   lua_pushstring( L, fname );
   lua_pushstring( L, prefix );
   return 3;
}

/**
 * @brief Gets the height of a font.
 *
 *    @luatparam Font f Font to get the height of.
 *    @luatreturn number Height of the font.
 * @luafunc height
 */
static int fontL_height( lua_State *L )
{
   glFont *font = luaL_checkfont( L, 1 );
   lua_pushnumber( L, font->h );
   return 1;
}

/**
 * @brief Gets the width of some text for a font.
 *
 *    @luatparam Font f Font to use.
 *    @luatparam string text Text to get width of.
 *    @luatreturn number Height of the font.
 * @luafunc width
 */
static int fontL_width( lua_State *L )
{
   const glFont *font  = luaL_checkfont( L, 1 );
   const char   *text  = luaL_checkstring( L, 2 );
   int           width = gl_printWidthRaw( font, text );
   lua_pushinteger( L, width );
   return 1;
}

/**
 * @brief Sets the font minification and magnification filters.
 *
 *    @luatparam Font font Font to set filter.
 *    @luatparam string min Minification filter (`"nearest"` or `"linear"`)
 *    @luatparam[opt] string mag Magnification filter (`"nearest"` or
 * `"linear"`). Defaults to min.
 * @luafunc setFilter
 */
static int fontL_setFilter( lua_State *L )
{
   const glFont *font = luaL_checkfont( L, 1 );
   const char   *smin = luaL_checkstring( L, 2 );
   const char   *smag = luaL_optstring( L, 3, smin );
   GLint         min, mag;

   min = gl_stringToFilter( smin );
   mag = gl_stringToFilter( smag );

   if ( min == 0 || mag == 0 )
      NLUA_INVALID_PARAMETER( L, 2 );

   gl_fontSetFilter( font, min, mag );

   return 0;
}

/**
 * @brief Adds a fallback to a font.
 *
 *    @luatparam Font font Font to set fallback to.
 *    @luatparam string filename Name of the font to add.
 *    @luatparam[opt=""] string prefix Prefix to use for the fonts.
 * @luafunc addFallback
 */
static int fontL_addFallback( lua_State *L )
{
   glFont     *font   = luaL_checkfont( L, 1 );
   const char *s      = luaL_checkstring( L, 2 );
   const char *prefix = luaL_optstring( L, 3, "" );
   int         ret    = gl_fontAddFallback( font, s, prefix );
   lua_pushboolean( L, !ret );
   return 1;
}
