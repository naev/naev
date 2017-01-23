/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_col.c
 *
 * @brief Handles colours.
 */

#include "nlua_col.h"

#include "naev.h"

#include <lauxlib.h>

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
 *    @param env Environment to load colour library into.
 *    @return 0 on success.
 */
int nlua_loadCol( nlua_env env )
{
   nlua_register(env, COL_METATABLE, colL_methods, 1);
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
glColour* lua_tocolour( lua_State *L, int ind )
{
   return (glColour*) lua_touserdata(L,ind);
}
/**
 * @brief Gets colour at index or raises error if there is no colour at index.
 *
 *    @param L Lua state to get colour from.
 *    @param ind Index position to find colour.
 *    @return Colour found at the index in the state.
 */
glColour* luaL_checkcolour( lua_State *L, int ind )
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
glColour* lua_pushcolour( lua_State *L, glColour colour )
{
   glColour *c;
   c = (glColour*) lua_newuserdata(L, sizeof(glColour));
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
 *    @luatparam Colour c1 Colour 1 to compare.
 *    @luatparam Colour c2 Colour 2 to compare.
 *    @luatreturn boolean true if both colours are the same.
 * @luafunc __eq( c1, c2 )
 */
static int colL_eq( lua_State *L )
{
   glColour *c1, *c2;
   c1 = luaL_checkcolour(L,1);
   c2 = luaL_checkcolour(L,2);
   lua_pushboolean( L, (memcmp( c1, c2, sizeof(glColour) )==0) );
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
 *    @luatparam number r Red value of the colour.
 *    @luatparam number g Green value of the colour.
 *    @luatparam number b Blue value of the colour.
 *    @luatparam[opt=1.] number a Alpha value of the colour.
 *    @luatreturn Colour A newly created colour.
 * @luafunc new( r, g, b, a )
 */
static int colL_new( lua_State *L )
{
   glColour col;
   const glColour *col2;

   NLUA_CHECKRW(L);

   if (lua_gettop(L)==0) {
      col.r = col.g = col.b = col.a = 1.;
   }
   else if (lua_isnumber(L,1)) {
      col.r = luaL_checknumber(L,1);
      col.g = luaL_checknumber(L,2);
      col.b = luaL_checknumber(L,3);
      if (lua_isnumber(L,4))
         col.a = luaL_checknumber(L,4);
      else
         col.a = 1.;
   }
   else if (lua_isstring(L,1)) {
      col2 = col_fromName( lua_tostring(L,1) );
      if (col2 == NULL) {
         NLUA_ERROR( L, "Colour '%s' does not exist!", lua_tostring(L,1) );
         return 0;
      }
      col = *col2;
      if (lua_isnumber(L,2))
         col.a = luaL_checknumber(L,2);
      else
         col.a = 1.;
   }
   else if (lua_iscolour(L,1))
      col = *lua_tocolour(L,1);
   else
      NLUA_INVALID_PARAMETER(L);

   lua_pushcolour( L, col );
   return 1;
}


/**
 * @brief Gets the alpha of a colour.
 *
 * Value is from from 0. (transparent) to 1. (opaque).
 *
 * @usage colour_alpha = col:alpha()
 *
 *    @luatparam Colour col Colour to get alpha of.
 *    @luatreturn number The alpha of the colour.
 * @luafunc alpha( col )
 */
static int colL_alpha( lua_State *L )
{
   glColour *col;
   col = luaL_checkcolour(L,1);
   lua_pushnumber( L, col->a );
   return 1;
}


/**
 * @brief Gets the RGB values of a colour.
 *
 * Values are from 0. to 1.
 *
 * @usage r,g,b = col:rgb()
 *
 *    @luatparam Colour col Colour to get RGB values of.
 *    @luatreturn number The red value of the colour.
 *    @luatreturn number The green value of the colour.
 *    @luatreturn number The blue value of the colour.
 * @luafunc rgb( col )
 */
static int colL_rgb( lua_State *L )
{
   glColour *col;
   col = luaL_checkcolour(L,1);
   lua_pushnumber( L, col->r );
   lua_pushnumber( L, col->g );
   lua_pushnumber( L, col->b );
   return 3;
}


/**
 * @brief Gets the HSV values of a colour.
 *
 * Values are from 0. to 1.
 *
 * @usage h,s,v = col:rgb()
 *
 *    @luatparam Colour col Colour to get HSV values of.
 *    @luatreturn number The hue of the colour.
 *    @luatreturn number The saturation of the colour.
 *    @luatreturn number The value of the colour.
 * @luafunc hsv( col )
 */
static int colL_hsv( lua_State *L )
{
   double h, s, v;
   glColour *col;
   col = luaL_checkcolour(L,1);
   col_rgb2hsv( &h, &s, &v, col->r, col->g, col->b );
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
 *    @luatparam Colour col Colour to set RGB values.
 *    @luatparam number r Red value to set.
 *    @luatparam number g Green value to set.
 *    @luatparam number b Blue value to set.
 * @luafunc setRGB( col, r, g, b )
 */
static int colL_setrgb( lua_State *L )
{
   NLUA_CHECKRW(L);
   glColour *col;
   col     = luaL_checkcolour(L,1);
   col->r  = luaL_checknumber(L,2);
   col->g  = luaL_checknumber(L,3);
   col->b  = luaL_checknumber(L,4);
   return 0;
}


/**
 * @brief Sets the colours values from the HSV colourspace.
 *
 * Values are from 0. to 1.
 *
 * @usage col:setHSV( h, s, v )
 *
 *    @luatparam Colour col Colour to set HSV values.
 *    @luatparam number h Hue value to set.
 *    @luatparam number s Saturation value to set.
 *    @luatparam number v Value to set.
 * @luafunc setHSV( col, h, s, v )
 */
static int colL_sethsv( lua_State *L )
{
   double r, g, b, h, s, v;
   glColour *col;
   NLUA_CHECKRW(L);
   col = luaL_checkcolour(L,1);
   h  = luaL_checknumber(L,2);
   s  = luaL_checknumber(L,3);
   v  = luaL_checknumber(L,4);
   col_hsv2rgb( &r, &g, &b,  h, s, v );
   col->r = r;
   col->g = g;
   col->b = b;
   return 0;
}


/**
 * @brief Sets the alpha of a colour.
 *
 * Value is from 0. (transparent) to 1. (opaque).
 *
 * @usage col:setAlpha( 0.5 ) -- Make colour half transparent
 *
 *    @luatparam Colour col Colour to set alpha of.
 *    @luatparam number alpha Alpha value to set.
 * @luafunc setAlpha( col, alpha )
 */
static int colL_setalpha( lua_State *L )
{
   glColour *col;
   NLUA_CHECKRW(L);
   col = luaL_checkcolour(L,1);
   col->a = luaL_checknumber(L,2);
   return 0;
}

