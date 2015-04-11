/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file nlua_col.c
 *
 * @brief Handles colours.
 */


#include "nlua_col.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
#include "nluadef.h"
#include "log.h"


/* Colour metatable methods. */
static int colL_eq( lua_State *L );
static int colL_new( lua_State *L );
static int colL_alpha( lua_State *L );
static int colL_rgb( lua_State *L );
static int colL_hsv( lua_State *L );
static int colL_setrgb( lua_State *L );
static int colL_sethsv( lua_State *L );
static int colL_setalpha( lua_State *L );
static const luaL_reg colL_methods[] = {
   { "__eq", colL_eq },
   { "new", colL_new },
   { "alpha", colL_alpha },
   { "rgb", colL_rgb },
   { "hsv", colL_hsv },
   { "setRGB", colL_setrgb },
   { "setHSV", colL_sethsv },
   { "setAlpha", colL_setalpha },
   {0,0}
}; /**< Colour metatable methods. */




/**
 * @brief Loads the colour library.
 *
 *    @param L State to load colour library into.
 *    @return 0 on success.
 */
int nlua_loadCol( lua_State *L, int readonly )
{
   if (readonly) /* Nothing is read only */
      return 0;

   /* Create the metatable */
   luaL_newmetatable(L, COL_METATABLE);

   /* Create the access table */
   lua_pushvalue(L,-1);
   lua_setfield(L,-2,"__index");

   /* Register the values */
   luaL_register(L, NULL, colL_methods);

   /* Clean up. */
   lua_setfield(L, LUA_GLOBALSINDEX, COL_METATABLE);

   return 0;
}


/**
 * @brief Lua bindings to interact with colours.
 *
 * An example would be:
 * @code
 * col1 = colour.new( "Red" ) -- Get by name
 * col2 = colour.new( 0.5, 0.5, 0.5, 0.3 ) -- Create with RGB values
 * col3 = colour.new() -- White by default
 * col2:setHSV( col1:hsv() ) -- Set colour 2 with colour 1's HSV values
 * @endcode
 *
 * @luamod colour
 */
/**
 * @brief Gets colour at index.
 *
 *    @param L Lua state to get colour from.
 *    @param ind Index position to find the colour.
 *    @return Colour found at the index in the state.
 */
LuaColour* lua_tocolour( lua_State *L, int ind )
{
   return (LuaColour*) lua_touserdata(L,ind);
}
/**
 * @brief Gets colour at index or raises error if there is no colour at index.
 *
 *    @param L Lua state to get colour from.
 *    @param ind Index position to find colour.
 *    @return Colour found at the index in the state.
 */
LuaColour* luaL_checkcolour( lua_State *L, int ind )
{
   if (lua_iscolour(L,ind))
      return lua_tocolour(L,ind);
   luaL_typerror(L, ind, COL_METATABLE);
   return NULL;
}
/**
 * @brief Pushes a colour on the stack.
 *
 *    @param L Lua state to push colour into.
 *    @param colour Colour to push.
 *    @return Newly pushed colour.
 */
LuaColour* lua_pushcolour( lua_State *L, LuaColour colour )
{
   LuaColour *c;
   c = (LuaColour*) lua_newuserdata(L, sizeof(LuaColour));
   *c = colour;
   luaL_getmetatable(L, COL_METATABLE);
   lua_setmetatable(L, -2);
   return c;
}
/**
 * @brief Checks to see if ind is a colour.
 *
 *    @param L Lua state to check.
 *    @param ind Index position to check.
 *    @return 1 if ind is a colour.
 */
int lua_iscolour( lua_State *L, int ind )
{
   int ret;

   if (lua_getmetatable(L,ind)==0)
      return 0;
   lua_getfield(L, LUA_REGISTRYINDEX, COL_METATABLE);

   ret = 0;
   if (lua_rawequal(L, -1, -2))  /* does it have the correct mt? */
      ret = 1;

   lua_pop(L, 2);  /* remove both metatables */
   return ret;
}


/**
 * @brief Compares two colours to see if they are the same.
 *
 *    @luaparam c1 Colour 1 to compare.
 *    @luaparam c2 Colour 2 to compare.
 *    @luareturn true if both colours are the same.
 * @luafunc __eq( c1, c2 )
 */
static int colL_eq( lua_State *L )
{
   LuaColour *c1, *c2;
   c1 = luaL_checkcolour(L,1);
   c2 = luaL_checkcolour(L,2);
   lua_pushboolean( L, (memcmp( &c1->col, &c2->col, sizeof(glColour) )==0) );
   return 1;
}


/**
 * @brief Gets a colour.
 *
 * @usage colour.new( "Red" ) -- Gets colour by name
 * @usage colour.new( "Red", 0.5 ) -- Gets colour by name with alpha 0.5
 * @usage colour.new() -- Creates a white (blank) colour
 * @usage colour.new( 1., 0., 0. ) -- Creates a bright red colour
 * @usage colour.new( 1., 0., 0., 0.5 ) -- Creates a bright red colour with alpha 0.5
 *
 *    @luaparam r Red value of the colour.
 *    @luaparam g Green value of the colour.
 *    @luaparam b Blue value of the colour.
 *    @luaparam a Alpha value of the colour.
 *    @luareturn A newly created colour.
 * @luafunc new( r, g, b, a )
 */
static int colL_new( lua_State *L )
{
   const glColour *col;
   LuaColour lc, *lc2;

   if (lua_gettop(L)==0) {
      lc.col.r = lc.col.g = lc.col.b = lc.col.a = 1.;
   }
   else if (lua_isnumber(L,1)) {
      lc.col.r = luaL_checknumber(L,1);
      lc.col.g = luaL_checknumber(L,2);
      lc.col.b = luaL_checknumber(L,3);
      if (lua_isnumber(L,4))
         lc.col.a = luaL_checknumber(L,4);
      else
         lc.col.a = 1.;
   }
   else if (lua_isstring(L,1)) {
      col = col_fromName( lua_tostring(L,1) );
      if (col == NULL) {
         NLUA_ERROR( L, "Colour '%s' does not exist!", lua_tostring(L,1) );
         return 0;
      }
      memcpy( &lc.col, col, sizeof(glColour) );
      if (lua_isnumber(L,2))
         lc.col.a = luaL_checknumber(L,2);
      else
         lc.col.a = 1.;
   }
   else if (lua_iscolour(L,1)) {
      lc2 = lua_tocolour(L,1);
      memcpy( &lc, lc2, sizeof(LuaColour) );
   }
   else
      NLUA_INVALID_PARAMETER(L);

   lua_pushcolour( L, lc );
   return 1;
}


/**
 * @brief Gets the alpha of a colour.
 *
 * Value is from from 0. (transparent) to 1. (opaque).
 *
 * @usage colour_alpha = col:alpha()
 *
 *    @luaparam col Colour to get alpha of.
 *    @luareturn The alpha of the colour.
 * @luafunc alpha( col )
 */
static int colL_alpha( lua_State *L )
{
   LuaColour *lc;
   lc = luaL_checkcolour(L,1);
   lua_pushnumber( L, lc->col.a );
   return 1;
}


/**
 * @brief Gets the RGB values of a colour.
 *
 * Values are from 0. to 1.
 *
 * @usage r,g,b = col:rgb()
 *
 *    @luaparam col Colour to get RGB values of.
 *    @luareturn The red, green and blue values of the colour.
 * @luafunc rgb( col )
 */
static int colL_rgb( lua_State *L )
{
   LuaColour *lc;
   lc = luaL_checkcolour(L,1);
   lua_pushnumber( L, lc->col.r );
   lua_pushnumber( L, lc->col.g );
   lua_pushnumber( L, lc->col.b );
   return 3;
}


/**
 * @brief Gets the HSV values of a colour.
 *
 * Values are from 0. to 1.
 *
 * @usage h,s,v = col:rgb()
 *
 *    @luaparam col Colour to get HSV values of.
 *    @luareturn The hue, saturation and value values of the colour.
 * @luafunc hsv( col )
 */
static int colL_hsv( lua_State *L )
{
   double h, s, v;
   LuaColour *lc;
   lc = luaL_checkcolour(L,1);
   col_rgb2hsv( &h, &s, &v, lc->col.r, lc->col.g, lc->col.b );
   lua_pushnumber( L, h );
   lua_pushnumber( L, s );
   lua_pushnumber( L, v );
   return 3;
}


/**
 * @brief Sets the colours values from the RGB colourspace.
 *
 * Values are from 0. to 1.
 *
 * @usage col:setRGB( r, g, b )
 *
 *    @luaparam col Colour to set RGB values.
 *    @luaparam r Red value to set.
 *    @luaparam g Green value to set.
 *    @luaparam b Blue value to set.
 * @luafunc setRGB( col, r, g, b )
 */
static int colL_setrgb( lua_State *L )
{
   LuaColour *lc;
   lc          = luaL_checkcolour(L,1);
   lc->col.r   = luaL_checknumber(L,2);
   lc->col.g   = luaL_checknumber(L,3);
   lc->col.b   = luaL_checknumber(L,4);
   return 0;
}


/**
 * @brief Sets the colours values from the HSV colourspace.
 *
 * Values are from 0. to 1.
 *
 * @usage col:setHSV( h, s, v )
 *
 *    @luaparam col Colour to set HSV values.
 *    @luaparam h Hue value to set.
 *    @luaparam s Saturation value to set.
 *    @luaparam v Value to set.
 * @luafunc setHSV( col, h, s, v )
 */
static int colL_sethsv( lua_State *L )
{
   double r, g, b, h, s, v;
   LuaColour *lc;
   lc = luaL_checkcolour(L,1);
   h  = luaL_checknumber(L,2);
   s  = luaL_checknumber(L,3);
   v  = luaL_checknumber(L,4);
   col_hsv2rgb( &r, &g, &b,  h, s, v );
   lc->col.r = r;
   lc->col.g = g;
   lc->col.b = b;
   return 0;
}


/**
 * @brief Sets the alpha of a colour.
 *
 * Value is from 0. (transparent) to 1. (opaque).
 *
 * @usage col:setAlpha( 0.5 ) -- Make colour half transparent
 *
 *    @luaparam col Colour to set alpha of.
 *    @luaparam alpha Alpha value to set.
 * @luafunc setAlpha( col, alpha )
 */
static int colL_setalpha( lua_State *L )
{
   LuaColour *lc;
   lc          = luaL_checkcolour(L,1);
   lc->col.a   = luaL_checknumber(L,2);
   return 0;
}

