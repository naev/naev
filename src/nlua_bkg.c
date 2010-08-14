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

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "nlua_tex.h"
#include "nlua_col.h"
#include "log.h"
#include "background.h"


/* Background metatable methods. */
static int bkgL_clear( lua_State *L );
static int bkgL_image( lua_State *L );
static const luaL_reg bkgL_methods[] = {
   { "clear", bkgL_clear },
   { "image", bkgL_image },
   {0,0}
}; /**< Background metatable methods. */




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
 * @usage bkg.image( img, 0, 0, 0.1, 1. ) -- Adds the image without scaling that moves at 0.1 the player speed
 *
 *    @luaparam image Image to use.
 *    @luaparam x X position.
 *    @luaparam y Y position.
 *    @luaparam move Fraction of a pixel to move when the player moves one pixel.
 *    @luaparam scale How much to scale the image.
 *    @luaparam col Colour to tint image (optional parameter).
 * @luafunc image( image, x, y, movee, scale, col )
 */
static int bkgL_image( lua_State *L )
{
   LuaTex *lt;
   double x,y, move, scale;
   LuaColour *lc;

   /* Parse parameters. */
   lt    = luaL_checktex(L,1);
   x     = luaL_checknumber(L,2);
   y     = luaL_checknumber(L,3);
   move  = luaL_checknumber(L,4);
   scale = luaL_checknumber(L,5);
   if (lua_iscolour(L,6))
      lc    = lua_tocolour(L,6);
   else
      lc    = NULL;
   
   /* Create image. */
   background_addImage( lt->tex, x, y, move, scale, NULL );
   return 0;
}



