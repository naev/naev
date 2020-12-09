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
#include "log.h"
#include "dialogue.h"
#include "nlua_outfit.h"
#include "toolkit.h"
#include "land.h"
#include "land_outfits.h"
#include "input.h"


/* Stuff for the custom toolkit. */
#define TK_CUSTOMDONE   "__customDone"
typedef struct custom_functions_s {
   lua_State *L; /**< Assosciated Lua state. */
   /* Function references. */
   int update;
   int draw;
   int keyboard;
   int mouse;
} custom_functions_t;
static int cust_update( double dt, void* data );
static void cust_render( double x, double y, double w, double h, void* data );
static int cust_key( unsigned int wid, SDL_Keycode key, SDL_Keymod mod, void* data );
static int  cust_mouse( unsigned int wid, SDL_Event* event, double x, double y, double w, double h, double rx, double ry, void* data );


/* Toolkit methods. */
static int tk_msg( lua_State *L );
static int tk_yesno( lua_State *L );
static int tk_input( lua_State *L );
static int tk_choice( lua_State *L );
static int tk_list( lua_State *L );
static int tk_merchantOutfit( lua_State *L );
static int tk_custom( lua_State *L );
static int tk_customDone( lua_State *L );
static const luaL_Reg tk_methods[] = {
   { "msg", tk_msg },
   { "yesno", tk_yesno },
   { "input", tk_input },
   { "choice", tk_choice },
   { "list", tk_list },
   { "merchantOutfit", tk_merchantOutfit },
   { "custom", tk_custom },
   { "customDone", tk_customDone },
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
   if (SCREEN_W < LAND_WIDTH || SCREEN_H < LAND_HEIGHT) {
      w = -1; /* Fullscreen. */
      h = -1;
   }
   else {
      w = LAND_WIDTH + 0.5 * (SCREEN_W - LAND_WIDTH);
      h = LAND_HEIGHT + 0.5 * (SCREEN_H - LAND_HEIGHT);
   }
   wid = window_create( "wdwMerchantOutfit", name, -1, -1, w, h );
   outfits_open( wid, outfits, noutfits );

   return 0;
}



/**
 * @brief Creates a custom widget window.
 *
 *    @luatparam String title Title of the window.
 *    @luatparam Number width Width of the drawable area of the widget.
 *    @luatparam Number height Height of the drawable area of the widget.
 *    @luatparam Function update Function to call when updating. Should take a single parameter which is a number indicating how many seconds passed from previous update.
 *    @luatparam Function draw Function to call when drawing.
 *    @luatparam Function keyboard Function to call when keyboard events are received. 
 *    @luatparam Function mouse Function to call when mouse events are received.
 * @luafunc custom( title, width, height, update, draw, keyboard, mouse )
 */
static int tk_custom( lua_State *L )
{
   int w, h;
   const char *caption;
   custom_functions_t cf;

   caption = luaL_checkstring(L, 1);
   w = luaL_checkinteger(L, 2);
   h = luaL_checkinteger(L, 3);

   luaL_checktype(L, 4, LUA_TFUNCTION);
   luaL_checktype(L, 5, LUA_TFUNCTION);
   luaL_checktype(L, 6, LUA_TFUNCTION);
   luaL_checktype(L, 7, LUA_TFUNCTION);
   /* Set up custom function pointers. */
   cf.L = L;
   lua_pushvalue(L, 4);
   cf.update   = luaL_ref(L, LUA_REGISTRYINDEX);
   lua_pushvalue(L, 5);
   cf.draw     = luaL_ref(L, LUA_REGISTRYINDEX);
   lua_pushvalue(L, 6);
   cf.keyboard = luaL_ref(L, LUA_REGISTRYINDEX);
   lua_pushvalue(L, 7);
   cf.mouse    = luaL_ref(L, LUA_REGISTRYINDEX);

   /* Set done condition. */
   lua_pushboolean(L, 0);
   lua_setglobal(L, TK_CUSTOMDONE );

   /* Create the dialogue. */
   dialogue_custom( caption, w, h, cust_update, cust_render, cust_key, cust_mouse, &cf );

   /* Clean up. */
   luaL_unref(L, LUA_REGISTRYINDEX, cf.update);
   luaL_unref(L, LUA_REGISTRYINDEX, cf.draw);
   luaL_unref(L, LUA_REGISTRYINDEX, cf.keyboard);
   luaL_unref(L, LUA_REGISTRYINDEX, cf.mouse);

   return 0;
}


/**
 * @brief Ends the execution of a custom widget.
 * @luafunc customDone()
 */
static int tk_customDone( lua_State *L )
{
   lua_pushboolean(L, 1);
   lua_setglobal(L, TK_CUSTOMDONE );
   return 0;
}


static int cust_update( double dt, void* data )
{
   int ret;
   custom_functions_t *cf = (custom_functions_t*) data;
   lua_State *L = cf->L;
   lua_rawgeti(L, LUA_REGISTRYINDEX, cf->update);
   lua_pushnumber(L, dt);
   /* TODO It seems like when it errors out it crashes because the dialogue gets unset. */
   if (lua_pcall(L, 1, 0, 0))
      lua_error(L); /* propagate error */
   lua_getglobal(L, TK_CUSTOMDONE );
   ret = lua_toboolean(L, -1);
   lua_pop(L, 1);
   return ret;
}
static void cust_render( double x, double y, double w, double h, void* data )
{
   (void) x;
   (void) y;
   (void) w;
   (void) h;
   custom_functions_t *cf = (custom_functions_t*) data;
   lua_State *L = cf->L;
   lua_rawgeti(L, LUA_REGISTRYINDEX, cf->draw);
   if (lua_pcall(L, 0, 0, 0))
      lua_error(L); /* propagate error */
}
static int cust_key( unsigned int wid, SDL_Keycode key, SDL_Keymod mod, void* data )
{
   (void) wid;
   (void) key;
   (void) mod;
   int b;
   custom_functions_t *cf = (custom_functions_t*) data;
   lua_State *L = cf->L;
   lua_rawgeti(L, LUA_REGISTRYINDEX, cf->keyboard);
   lua_pushstring(L, SDL_GetKeyName(key));
   lua_pushstring(L, input_modToText(mod));
   if (lua_pcall(L, 2, 1, 0))
      lua_error(L); /* propagate error */
   b = lua_toboolean(L, -1);
   lua_pop(L,1);
   return b;
}
static int cust_mouse( unsigned int wid, SDL_Event* event, double x, double y, double w, double h, double rx, double ry, void* data )
{
   (void) wid;
   (void) w;
   (void) h;
   (void) rx;
   (void) ry;
   int b, button, type;
   custom_functions_t *cf = (custom_functions_t*) data;
   lua_State *L = cf->L;
   switch (event->type) {
      case SDL_MOUSEBUTTONDOWN: type=1; break;
      case SDL_MOUSEBUTTONUP:   type=2; break;
      case SDL_MOUSEMOTION:     type=3; break;
      default: return 0;
   }
   lua_rawgeti(L, LUA_REGISTRYINDEX, cf->mouse);
   lua_pushnumber(L, x);
   lua_pushnumber(L, y);
   lua_pushnumber(L, type);
   if (type < 3) {
      switch (event->button.button) {
         case SDL_BUTTON_LEFT:  button=1; break;
         case SDL_BUTTON_RIGHT: button=2; break;
         default:               button=3; break;
      }
      lua_pushnumber(L, button);
      if (lua_pcall(L, 4, 1, 0))
         lua_error(L); /* propagate error */
   }
   else
      if (lua_pcall(L, 3, 1, 0))
         lua_error(L); /* propagate error */
   b = lua_toboolean(L, -1);
   lua_pop(L,1);
   return b;
}

