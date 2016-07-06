/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_gfx.c
 *
 * @brief Handles the rendering of graphics on the screen.
 */

#include "nlua_gfx.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "opengl.h"
#include "font.h"
#include "nlua_col.h"
#include "nlua_tex.h"
#include "ndata.h"


/* GFX methods. */
static int gfxL_dim( lua_State *L );
static int gfxL_renderTex( lua_State *L );
static int gfxL_renderTexRaw( lua_State *L );
static int gfxL_renderRect( lua_State *L );
static int gfxL_fontSize( lua_State *L );
static int gfxL_printDim( lua_State *L );
static int gfxL_print( lua_State *L );
static int gfxL_printText( lua_State *L );
static const luaL_reg gfxL_methods[] = {
   /* Information. */
   { "dim", gfxL_dim },
   /* Render stuff. */
   { "renderTex", gfxL_renderTex },
   { "renderTexRaw", gfxL_renderTexRaw },
   { "renderRect", gfxL_renderRect },
   /* Printing. */
   { "fontSize", gfxL_fontSize },
   { "printDim", gfxL_printDim },
   { "print", gfxL_print },
   { "printText", gfxL_printText },
   {0,0}
}; /**< GFX methods. */




/**
 * @brief Loads the graphics library.
 *
 *    @param L State to load graphics library into.
 *    @return 0 on success.
 */
int nlua_loadGFX( lua_State *L, int readonly )
{
   if (readonly) /* Nothing is read only */
      return 0;

   /* Register the values */
   luaL_register(L, "gfx", gfxL_methods);

   /* We also load the texture and colour modules as dependencies. */
   nlua_loadTex( L, readonly );
   nlua_loadCol( L, readonly );

   return 0;
}


/**
 * @brief Lua bindings to interact with rendering and the Naev graphical environment.
 *
 * An example would be:
 * @code
 * t  = tex.open( GFX_PATH"foo/bar.png" ) -- Loads the texture
 * gfx.renderTex( t, 0., 0. ) -- Draws texture at origin
 * @endcode
 *
 * @luamod gfx
 */


/**
 * @brief Gets the dimensions of the Naev window.
 *
 * @usage screen_w, screen_h = gfx.dim()
 *
 *    @luatreturn number The width of the Naev window.
 *    @luatreturn number The height of the Naev window.
 * @luafunc dim()
 */
static int gfxL_dim( lua_State *L )
{
   lua_pushnumber( L, SCREEN_W );
   lua_pushnumber( L, SCREEN_H );
   return 2;
}


/**
 * @brief Renders a texture.
 *
 * This function has variable parameters depending on how you want to render.
 *
 * @usage gfx.renderTex( tex, 0., 0. ) -- Render tex at origin
 * @usage gfx.renderTex( tex, 0., 0., col ) -- Render tex at origin with colour col
 * @usage gfx.renderTex( tex, 0., 0., 4, 3 ) -- Render sprite at position 4,3 (top-left is 1,1)
 * @usage gfx.renderTex( tex, 0., 0., 4, 3, col ) -- Render sprite at position 4,3 (top-left is 1,1) with colour col
 *
 *    @luatparam Tex tex Texture to render.
 *    @luatparam number pos_x X position to render texture at.
 *    @luatparam number pos_y Y position to render texture at.
 *    @luatparam[opt=0] int sprite_x X sprite to render.
 *    @luatparam[opt=0] int sprite_y Y sprite to render.
 *    @luatparam[opt] Colour colour Colour to use when rendering.
 * @luafunc renderTex( tex, pos_x, pos_y, sprite_x, sprite_y, colour )
 */
static int gfxL_renderTex( lua_State *L )
{
   glTexture *tex;
   glColour *col;
   double x, y;
   int sx, sy;

   /* Parameters. */
   col = NULL;
   tex = luaL_checktex( L, 1 );
   x   = luaL_checknumber( L, 2 );
   y   = luaL_checknumber( L, 3 );
   if (lua_isnumber( L, 4 )) {
      sx    = luaL_checkinteger( L, 4 ) - 1;
      sy    = luaL_checkinteger( L, 5 ) - 1;
      if (lua_iscolour(L, 6))
         col = luaL_checkcolour(L,6);
   }
   else {
      sx    = 0;
      sy    = 0;
      if (lua_iscolour(L, 4))
         col = luaL_checkcolour(L,4);
   }

   /* Some sanity checking. */
#if DEBUGGING
   if (sx >= tex->sx)
      NLUA_ERROR( L, "Texture '%s' trying to render out of bounds (X position) sprite: %d > %d.",
            tex->name, sx+1, tex->sx );
   if (sx >= tex->sx)
      NLUA_ERROR( L, "Texture '%s' trying to render out of bounds (Y position) sprite: %d > %d.",
            tex->name, sy+1, tex->sy );
#endif /* DEBUGGING */

   /* Render. */
   gl_blitStaticSprite( tex, x, y, sx, sy, col );

   return 0;
}


/**
 * @brief Renders a texture using the core render function.
 *
 * This function is far more complex than renderTex, however it allows much
 *  more fine grained control over the entire render process and puts you
 *  closer to the actual OpenGL calls.
 *
 * @usage gfx.renderTexRaw( tex, 0., 0., 100., 100., 1, 1, 0., 0., 0.5, 0.5 ) -- Renders the bottom quarter of the sprite 1,1 of the image.
 *
 *    @luatparam Tex tex Texture to render.
 *    @luatparam number pos_x X position to render texture at.
 *    @luatparam number pos_y Y position to render texture at.
 *    @luatparam number pos_w Width of the image on screen.
 *    @luatparam number pos_h Height of the image on screen.
 *    @luatparam number sprite_x X sprite to render.
 *    @luatparam number sprite_y Y sprite to render.
 *    @luatparam number tex_x X sprite texture offset as [0.:1.].
 *    @luatparam number tex_y Y sprite texture offset as [0.:1.].
 *    @luatparam number tex_w Sprite width to display as [-1.:1.]. Note if negative, it will flip the image horizontally.
 *    @luatparam number tex_h Sprite height to display as [-1.:1.] Note if negative, it will flip the image vertically.
 *    @luatparam[opt] Colour colour Colour to use when rendering.
 * @luafunc renderTexRaw( tex, pos_x, pos_y, pos_w, pos_h, sprite_x, sprite_y, tex_x, tex_y, tex_w, tex_h, colour )
 */
static int gfxL_renderTexRaw( lua_State *L )
{
   glTexture *t;
   glColour *col;
   double px,py, pw,ph, tx,ty, tw,th;
   int sx, sy;

   /* Parameters. */
   col = NULL;
   t  = luaL_checktex( L, 1 );
   px = luaL_checknumber( L, 2 );
   py = luaL_checknumber( L, 3 );
   pw = luaL_checknumber( L, 4 );
   ph = luaL_checknumber( L, 5 );
   sx = luaL_checkinteger( L, 6 ) - 1;
   sy = luaL_checkinteger( L, 7 ) - 1;
   tx = luaL_checknumber( L, 8 );
   ty = luaL_checknumber( L, 9 );
   tw = luaL_checknumber( L, 10 );
   th = luaL_checknumber( L, 11 );
   if (lua_iscolour( L, 12 ))
      col = lua_tocolour( L, 12 );

   /* Some sanity checking. */
#if DEBUGGING
   if (sx >= t->sx)
      NLUA_ERROR( L, "Texture '%s' trying to render out of bounds (X position) sprite: %d > %d.",
            t->name, sx+1, t->sx );
   if (sx >= t->sx)
      NLUA_ERROR( L, "Texture '%s' trying to render out of bounds (Y position) sprite: %d > %d.",
            t->name, sy+1, t->sy );
#endif /* DEBUGGING */

   /* Translate as needed. */
   tx = (tx * t->sw + t->sw * (double)(sx)) / t->rw;
   tw = tw * t->srw;
   if (tw < 0)
      tx -= tw;
   ty = (ty * t->sh + t->sh * (t->sy - (double)sy-1)) / t->rh;
   th = th * t->srh;
   if (th < 0)
      ty -= th;

   /* Render. */
   gl_blitTexture( t, px, py, pw, ph, tx, ty, tw, th, col );
   return 0;
}


/**
 * @brief Renders a rectangle.
 *
 * @usage gfx.renderRect( 10., 30,. 40., 40., col ) -- Renders a 40 side square at position 10,30 of colour col
 * @usage gfx.renderRect( 10., 30,. 40., 40., col, True ) -- Renders a 40 side empty square at position 10,30 of colour col
 *
 *    @luatparam number x X position to render at.
 *    @luatparam number y Y position to render at.
 *    @luatparam number w Width of the rectangle.
 *    @luatparam number h Height of the rectangle.
 *    @luatparam Colour col Colour to use.
 *    @luatparam[opt=false] boolean empty Whether or not it should be empty.
 * @luafunc renderRect( x, y, w, h, col, empty )
 */
static int gfxL_renderRect( lua_State *L )
{
   glColour *col;
   double x,y, w,h;
   int empty;

   /* Parse parameters. */
   x     = luaL_checknumber( L, 1 );
   y     = luaL_checknumber( L, 2 );
   w     = luaL_checknumber( L, 3 );
   h     = luaL_checknumber( L, 4 );
   col   = luaL_checkcolour( L, 5 );
   empty = lua_toboolean( L, 6 );

   /* Render. */
   if (empty)
      gl_renderRectEmpty( x, y, w, h, col );
   else
      gl_renderRect( x, y, w, h, col );

   return 0;
}


/**
 * @brief Gets the size of the font.
 *
 *    @luatparam boolean small Whether or not to get the size of the small font.
 *    @luatreturn[opt=false] The size in pixels of the font.
 * @luafunc fontSize( small )
 */
static int gfxL_fontSize( lua_State *L )
{
   int small;
   small = lua_toboolean(L,1);
   lua_pushnumber( L, small ? gl_smallFont.h : gl_defFont.h );
   return 1;
}


/**
 * @brief Gets the size of the text to print.
 *
 * @usage len = gfx.printDim( nil, "Hello World!" ) -- Length of string with normal font
 * @usage height = gfx.printDim( true, "Longer text", 20 ) -- Dimensions of text block
 *
 *    @luatparam boolean small Whether or not to use the small font.
 *    @luatparam string str Text to calculate length of.
 *    @luatparam[opt] int width Optional parameter to indicate it is a block of text and to use this width.
 * @luafunc printDim( small, str, width )
 */
static int gfxL_printDim( lua_State *L )
{
   const char *str;
   int width;
   glFont *font;

   /* Parse parameters. */
   font  = lua_toboolean(L,1) ? &gl_smallFont : &gl_defFont;
   str   = luaL_checkstring(L,2);
   if (lua_gettop(L) > 2)
      width = luaL_checkinteger(L,3);
   else
      width = 0;

   /* Print length. */
   if (width == 0)
      lua_pushnumber( L, gl_printWidthRaw( font, str ) );
   else
      lua_pushnumber( L, gl_printHeightRaw( font, width, str ) );
   return 1;
}


/**
 * @brief Prints text on the screen.
 *
 * @usage gfx.print( nil, "Hello World!", 50, 50, colour.new("Red") ) -- Displays text in red at 50,50.
 * @usage gfx.print( true, "Hello World!", 50, 50, col, 100 ) -- Displays text to a maximum of 100 pixels wide.
 * @usage gfx.print( true, str, 50, 50, col, 100, true ) -- Displays centered text to a maximum of 100 pixels.
 *
 *    @luatparam bookean small Whether or not to use a small font.
 *    @luatparam string str String to print.
 *    @luatparam number x X position to print at.
 *    @luatparam number y Y position to print at.
 *    @luatparam Colour col Colour to print text.
 *    @luatparam[opt] int max Maximum width to render up to.
 *    @luatparam[opt] boolean center Whether or not to center it.
 * @luafunc print( small, str, x, y, col, max, center )
 */
static int gfxL_print( lua_State *L )
{
   glFont *font;
   const char *str;
   double x, y;
   glColour *col;
   int max, mid;

   /* Parse parameters. */
   font  = lua_toboolean(L,1) ? &gl_smallFont : &gl_defFont;
   str   = luaL_checkstring(L,2);
   x     = luaL_checknumber(L,3);
   y     = luaL_checknumber(L,4);
   col   = luaL_checkcolour(L,5);
   if (lua_gettop(L) >= 6)
      max = luaL_checkinteger(L,6);
   else
      max = 0;
   mid   = lua_toboolean(L,7);

   /* Render. */
   if (mid)
      gl_printMidRaw( font, max, x, y, col, str );
   else if (max > 0)
      gl_printMaxRaw( font, max, x, y, col, str );
   else
      gl_printRaw( font, x, y, col, str );
   return 0;
}


/**
 * @brief Prints a block of text on the screen.
 *
 * @usage gfx.printText( true, 100, 50, 50, 100, 100, col ) -- Displays a 100x100 block of text
 *
 *    @luatparam boolean small Whether or not to use a small font.
 *    @luatparam string str String to print.
 *    @luatparam number x X position to print at.
 *    @luatparam number y Y position to print at.
 *    @luatparam number w Width of the block of text.
 *    @luatparam number h Height of the block of text.
 *    @luatparam Colour col Colour to print text.
 * @luafunc printText( small, str, x, y, w, h, col )
 */
static int gfxL_printText( lua_State *L )
{
   glFont *font;
   const char *str;
   int w, h;
   double x, y;
   glColour *col;

   /* Parse parameters. */
   font  = lua_toboolean(L,1) ? &gl_smallFont : &gl_defFont;
   str   = luaL_checkstring(L,2);
   x     = luaL_checknumber(L,3);
   y     = luaL_checknumber(L,4);
   w     = luaL_checkinteger(L,5);
   h     = luaL_checkinteger(L,6);
   col   = luaL_checkcolour(L,7);

   /* Render. */
   gl_printTextRaw( font, w, h, x, y, col, str );

   return 0;
}




