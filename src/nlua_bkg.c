/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_bkg.c
 *
 * @brief Bindings for modifying the space background.
 */

#include "nlua_bkg.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "nlua_tex.h"
#include "nlua_col.h"
#include "log.h"
#include "background.h"


/* Background methods. */
static int bkgL_clear( lua_State *L );
static int bkgL_image( lua_State *L );
static const luaL_reg bkgL_methods[] = {
   { "clear", bkgL_clear },
   { "image", bkgL_image },
   {0,0}
}; /**< Background methods. */




/**
 * @brief Loads the graphics library.
 *
 *    @param L State to load graphics library into.
 *    @return 0 on success.
 */
int nlua_loadBackground( lua_State *L, int readonly )
{
   if (readonly) /* Nothing is read only */
      return 0;

   /* Register the values */
   luaL_register(L, "bkg", bkgL_methods);

   return 0;
}


/**
 * @brief Lua bindings to interact with the background.
 *
 * An example would be:
 * @code
 * @endcode
 *
 * @luamod bkg
 */



/**
 * @brief Clears any backgrounds that may currently be displaying.
 *
 * @luafunc clear()
 */
static int bkgL_clear( lua_State *L )
{
   (void) L;
   background_clear();
   return 0;
}


/**
 * @brief Adds a background image.
 *
 * If the colour parameter is a boolean it's treated as the foreground parameter instead.
 *
 * @usage bkg.image( img, 0, 0, 0.1, 1. ) -- Adds the image without scaling that moves at 0.1 the player speed
 * @usage bkg.image( img, 0, 0, 0.1, 1., true ) -- Now on the foreground
 * @usage bkg.image( img, 0, 0, 0.1, 1., col.new(1,0,0) ) -- Now with colour
 * @usage bkg.image( img, 0, 0, 0.1, 1., col.new(1,0,0), true ) -- Now with colour and on the foreground
 *
 *    @luatparam Tex image Image to use.
 *    @luatparam number x X position.
 *    @luatparam number y Y position.
 *    @luatparam number move Fraction of a pixel to move when the player moves one pixel.
 *    @luatparam number scale How much to scale the image.
 *    @luatparam[opt] Colour col Colour to tint image.
 *    @luatparam[opt=false] boolean foreground Whether or not it should be rendered above the stars.
 *    @luatreturn number ID of the background.
 * @luafunc image( image, x, y, move, scale, col, foreground )
 */
static int bkgL_image( lua_State *L )
{
   glTexture *tex;
   double x,y, move, scale;
   const glColour *col;
   unsigned int id;
   int foreground;

   /* Parse parameters. */
   tex   = luaL_checktex(L,1);
   x     = luaL_checknumber(L,2);
   y     = luaL_checknumber(L,3);
   move  = luaL_checknumber(L,4);
   scale = luaL_checknumber(L,5);
   if (lua_iscolour(L,6)) {
      col = lua_tocolour(L,6);
      foreground = lua_toboolean(L,7);
   }
   else {
      col = &cWhite;
      foreground = lua_toboolean(L,6);
   }

   /* Create image. */
   id = background_addImage( tex, x, y, move, scale, col, foreground );
   lua_pushnumber(L,id);
   return 1;
}



