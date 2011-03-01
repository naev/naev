/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_tk.c
 *
 * @brief NAEV toolkit Lua module.
 */

#include "nlua_tk.h"

#include "naev.h"

#include <stdlib.h>

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "dialogue.h"


/* Toolkit methods. */
static int tk_msg( lua_State *L );
static int tk_yesno( lua_State *L );
static int tk_input( lua_State *L );
static int tk_choice( lua_State *L );
static const luaL_reg tk_methods[] = {
   { "msg", tk_msg },
   { "yesno", tk_yesno },
   { "input", tk_input },
   { "choice", tk_choice },
   {0,0}
}; /**< Toolkit Lua methods. */



/**
 * @brief Loads the Toolkit Lua library.
 *
 *    @param L Lua state.
 *    @return 0 on success.
 */
int nlua_loadTk( lua_State *L )
{
   luaL_register(L, "tk", tk_methods);
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
 * @brief Creates a window with an ok button.
 *
 * @usage tk.msg( "Title", "This is a message." )
 *
 *    @luaparam title Title of the window.
 *    @luaparam message Message to display in the window.
 * @luafunc msg( title, message )
 */
static int tk_msg( lua_State *L )
{
   const char *title, *str;
   NLUA_MIN_ARGS(2);

   title = luaL_checkstring(L,1);
   str   = luaL_checkstring(L,2);

   dialogue_msgRaw( title, str );
   return 0;
}
/**
 * @brief Displays a window with Yes and No buttons.
 *
 * @usage if tk.yesno( "YesNo popup box", "Click yes to do something." ) then -- Clicked yes
 *
 *    @luaparam title Title of the window.
 *    @luaparam message Message to display in the window.
 *    @luareturn true if yes was clicked, false if no was clicked.
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
 *    @luaparam title Title of the window.
 *    @luaparam min Minimum characters to accept (must be greater than 0).
 *    @luaparam max Maximum characters to accept.
 *    @luaparam str Text to display in the window.
 *    @luareturn nil if input was canceled or a string with the text written.
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
 *    @luaparam title Title of the window.
 *    @luaparam msg Message to display.
 *    @luaparam choices Option choices.
 *    @luareturn Returns the number of the choice and the name of the choice chosen.
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

   /* Create dialogue. */
   dialogue_makeChoice( title, str, opts );
   for (i=0; i<opts; i++)
      dialogue_addChoice( title, str, luaL_checkstring(L,i+3) );
   result = dialogue_runChoice();

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

