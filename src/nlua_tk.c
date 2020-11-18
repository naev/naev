/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_tk.c
 *
 * @brief Naev toolkit Lua module.
 */

#include "nlua_tk.h"

#include "naev.h"

#include <stdlib.h>

#include <lauxlib.h>

#include "nluadef.h"
#include "conf.h"
#include "log.h"
#include "dialogue.h"
#include "nlua_outfit.h"
#include "toolkit.h"
#include "land_outfits.h"

/* Merchant Outfit window */
#define MERCH_OUTFIT_WIDTH   RESOLUTION_W_MIN /**< Merchant Outfit window width. */
#define MERCH_OUTFIT_HEIGHT  RESOLUTION_H_MIN /**< Merchant Outfit window height. */


/* Toolkit methods. */
static int tk_msg( lua_State *L );
static int tk_yesno( lua_State *L );
static int tk_input( lua_State *L );
static int tk_choice( lua_State *L );
static int tk_list( lua_State *L );
static int tk_merchantOutfit( lua_State *L );
static const luaL_Reg tk_methods[] = {
   { "msg", tk_msg },
   { "yesno", tk_yesno },
   { "input", tk_input },
   { "choice", tk_choice },
   { "list", tk_list },
   { "merchantOutfit", tk_merchantOutfit },
   {0,0}
}; /**< Toolkit Lua methods. */



/**
 * @brief Loads the Toolkit Lua library.
 *
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadTk( nlua_env env )
{
   nlua_register(env, "tk", tk_methods, 0);
   return 0;
}


/**
 * @brief Bindings for interacting with the Toolkit.
 *
 * These toolkit bindings are all blocking, meaning that your Lua code won't
 *  continue executing until the user closes the dialogue that popped up.
 *
 *  A typical example  would be:
 *  @code
 *  tk.msg( "Title", "This is a message." )
 *  if tk.yesno( "YesNo popup box", "Click yes to do something." ) then
 *    -- Player clicked yes, do something
 *  else
 *    -- Player clicked no
 *  end
 *  @endcode
 *
 *  @luamod tk
 */
/**
 * @brief Creates a window with an ok button, and optionally an image.
 *
 * @usage tk.msg( "Title", "This is a message." )
 * @usage tk.msg( "Title", "This is a message.", "character.png" )
 *
 *    @luatparam string title Title of the window.
 *    @luatparam string message Message to display in the window.
 *    @luatparam[opt=-1] string image Image file (*.png) to display in the window.
 *    @luatparam[opt=-1] number width width of the image to display. Negative values use image width.
 *    @luatparam[opt=-1] number height height of the image to display. Negative values use image height.
 * @luafunc msg( title, message )
 */
static int tk_msg( lua_State *L )
{
   const char *title, *str, *img;
   int width, height;
   NLUA_MIN_ARGS(2);

   // Get fixed arguments : title, string to display and image filename
   title = luaL_checkstring(L,1);
   str   = luaL_checkstring(L,2);

   if (lua_gettop(L) > 2) {
      img   = luaL_checkstring(L,3);

      // Get optional arguments : width and height
      width  = (lua_gettop(L) < 4) ? -1 : luaL_checkinteger(L,4);
      height = (lua_gettop(L) < 5) ? -1 : luaL_checkinteger(L,5);

      dialogue_msgImgRaw( title, str, img, width, height );
      return 0;
   }
   dialogue_msgRaw( title, str );
   return 0;
}
/**
 * @brief Displays a window with Yes and No buttons.
 *
 * @usage if tk.yesno( "YesNo popup box", "Click yes to do something." ) then -- Clicked yes
 *
 *    @luatparam string title Title of the window.
 *    @luatparam string message Message to display in the window.
 *    @luatreturn boolean true if yes was clicked, false if no was clicked.
 * @luafunc yesno( title, message )
 */
static int tk_yesno( lua_State *L )
{
   int ret;
   const char *title, *str;
   NLUA_MIN_ARGS(2);

   title = luaL_checkstring(L,1);
   str   = luaL_checkstring(L,2);

   ret = dialogue_YesNoRaw( title, str );
   lua_pushboolean(L,ret);
   return 1;
}
/**
 * @brief Creates a window that allows player to write text input.
 *
 * @usage name = tk.input( "Name", 3, 20, "Enter your name:" )
 *
 *    @luatparam string title Title of the window.
 *    @luatparam number min Minimum characters to accept (must be greater than 0).
 *    @luatparam number max Maximum characters to accept.
 *    @luatparam string str Text to display in the window.
 *    @luatreturn string|nil nil if input was canceled or a string with the text written.
 * @luafunc input( title, min, max, str )
 */
static int tk_input( lua_State *L )
{
   const char *title, *str;
   char *ret;
   int min, max;
   NLUA_MIN_ARGS(4);

   title = luaL_checkstring(L,1);
   min   = luaL_checkint(L,2);
   max   = luaL_checkint(L,3);
   str   = luaL_checkstring(L,4);

   ret = dialogue_inputRaw( title, min, max, str );
   if (ret != NULL) {
      lua_pushstring(L, ret);
      free(ret);
   }
   else
      lua_pushnil(L);
   return 1;
}


/**
 * @brief Creates a window with a number of selectable options
 *
 * @usage num, chosen = tk.choice( "Title", "Ready to go?", "Yes", "No" ) -- If "No" was clicked it would return 2, "No"
 *
 *    @luatparam string title Title of the window.
 *    @luatparam string msg Message to display.
 *    @luatparam string choices Option choices.
 *    @luatreturn number The number of the choice chosen.
 *    @luatreturn string The name of the choice chosen.
 * @luafunc choice( title, msg, ... )
 */
static int tk_choice( lua_State *L )
{
   int ret, opts, i;
   const char *title, *str;
   char *result;
   NLUA_MIN_ARGS(3);

   /* Handle parameters. */
   opts  = lua_gettop(L) - 2;
   title = luaL_checkstring(L,1);
   str   = luaL_checkstring(L,2);

   /* Do an initial scan for invalid arguments. */
   for (i=0; i<opts; i++)
      luaL_checkstring(L, i+3);

   /* Create dialogue. */
   dialogue_makeChoice( title, str, opts );
   for (i=0; i<opts; i++)
      dialogue_addChoice( title, str, luaL_checkstring(L,i+3) );
   result = dialogue_runChoice();
   if (result == NULL) /* Something went wrong, return nil. */
      return 0;

   /* Handle results. */
   ret = -1;
   for (i=0; i<opts && ret==-1; i++) {
      if (strcmp(result, luaL_checkstring(L,i+3)) == 0)
         ret = i+1; /* Lua uses 1 as first index. */
   }

   /* Push parameters. */
   lua_pushnumber(L,ret);
   lua_pushstring(L,result);

   /* Clean up. */
   free(result);

   return 2;
}


/**
 * @brief Creates a window with an embedded list of choices.
 *
 * @usage num, chosen = tk.list( "Title", "Foo or bar?", "Foo", "Bar" ) -- If "Bar" is clicked, it would return 2, "Bar"
 *
 *    @luatparam string title Title of the window.
 *    @luatparam string msg Message to display.
 *    @luatparam string choices Option choices.
 *    @luatreturn number The number of the choice chosen.
 *    @luatreturn string The name of the choice chosen.
 * @luafunc list( title, msg, ... )
 */
static int tk_list( lua_State *L )
{
   int ret, opts, i;
   const char *title, *str;
   char **choices;
   NLUA_MIN_ARGS(3);

   /* Handle parameters. */
   opts  = lua_gettop(L) - 2;
   title = luaL_checkstring(L,1);
   str   = luaL_checkstring(L,2);

   /* Do an initial scan for invalid arguments. */
   for (i=0; i<opts; i++)
      luaL_checkstring(L, i+3);

   /* Will be freed by the toolkit. */
   choices = malloc( sizeof(char*) * opts );
   for (i=0; i<opts; i++)
      choices[i] = strdup( luaL_checkstring(L, i+3) );

   ret = dialogue_listRaw( title, choices, opts, str );

   /* Cancel returns -1, do nothing. */
   if (ret == -1)
      return 0;

   /* Push index and choice string. */
   lua_pushnumber(L, ret+1);
   lua_pushstring(L, choices[ret]);

   return 2;
}

/**
 * @brief Opens an outfit merchant window.
 *
 * @usage tk.merchantOutfit( 'Laser Merchant', {'Laser Cannon MK0', 'Laser Cannon MK1'} )
 * @usage tk.merchantOutfit( 'Laser Merchant', {outfit.get('Laser Cannon MK0'), outfit.get('Laser Cannon MK1')} )
 *
 *    @luatparam String name Name of the window.
 *    @luatparam Table outfits Table of outfits to sell/buy. It is possible to use either outfits or outfit names (strings).
 * @luafunc merchantOutfit( name, outfits )
 */
static int tk_merchantOutfit( lua_State *L )
{
   Outfit **outfits;
   int i, noutfits;
   unsigned int wid;
   const char *name;
   int w, h;

   name = luaL_checkstring(L,1);

   if (!lua_istable(L,2))
      NLUA_INVALID_PARAMETER(L);

   noutfits = (int) lua_objlen(L,2);
   if (noutfits == 0)
      NLUA_ERROR(L, _("Unable to create empty outfit merchant."));
   outfits = malloc( sizeof(Outfit*) * noutfits );
   /* Iterate over table. */
   lua_pushnil(L);
   i = 0;
   while (lua_next(L, -2) != 0) {
      outfits[i++] = luaL_validoutfit(L, -1);
      lua_pop(L,1);
   }

   /* Create window. */
   if (SCREEN_W < MERCH_OUTFIT_WIDTH || SCREEN_H < MERCH_OUTFIT_HEIGHT) {
      w = -1; /* Fullscreen. */
      h = -1;
   }
   else {
      w = MERCH_OUTFIT_WIDTH + 0.5 * (SCREEN_W - MERCH_OUTFIT_WIDTH);
      h = MERCH_OUTFIT_HEIGHT + 0.5 * (SCREEN_H - MERCH_OUTFIT_HEIGHT);
   }
   wid = window_create( "wdwMerchantOutfit", name, -1, -1, w, h );
   outfits_open( wid, outfits, noutfits );

   return 0;
}

