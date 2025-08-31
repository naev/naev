/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_tk.c
 *
 * @brief Naev toolkit Lua module.
 */
/** @cond */
#include <lauxlib.h>
#include <lua.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_tk.h"

#include "array.h"
#include "dialogue.h"
#include "input.h"
#include "land.h"
#include "land_outfits.h"
#include "nlua_colour.h"
#include "nlua_gfx.h"
#include "nlua_outfit.h"
#include "nluadef.h"
#include "toolkit.h"

/* Stuff for the custom toolkit. */
#define TK_CUSTNAME "__cstdlg"
#define TK_CUSTOMDONE "__customDone"
typedef struct custom_functions_s {
   lua_State *L;    /**< Assosciated Lua state. */
   nlua_env  *env;  /**< Associated environment registry key. */
   int        done; /**< Whether or not it is done. */
   /* Function references. */
   int update;
   int draw;
   int keyboard;
   int mouse;
   int resize;
   int textinput;
} custom_functions_t;
static void cust_cleanup( void *data );
static int  cust_update( double dt, void *data );
static void cust_render( double x, double y, double w, double h, void *data );
static int  cust_event( unsigned int wid, SDL_Event *event, void *data );
static int cust_key( SDL_Keycode key, SDL_Keymod mod, int pressed, int isrepeat,
                     custom_functions_t *cf );
static int cust_text( const char *str, custom_functions_t *cf );
static int cust_mouse( int type, int button, double x, double y,
                       custom_functions_t *cf );
static int cust_event_window( Uint32 event, Sint32 w, Sint32 h,
                              custom_functions_t *cf );

/* Toolkit methods. */
static int tkL_isOpen( lua_State *L );
static int tkL_query( lua_State *L );
static int tkL_msg( lua_State *L );
static int tkL_yesno( lua_State *L );
static int tkL_input( lua_State *L );
static int tkL_choice( lua_State *L );
static int tkL_list( lua_State *L );
static int tkL_merchantOutfit( lua_State *L );
static int tkL_custom( lua_State *L );
static int tkL_customRename( lua_State *L );
static int tkL_customFullscreen( lua_State *L );
static int tkL_customResize( lua_State *L );
static int tkL_customSize( lua_State *L );
static int tkL_customDone( lua_State *L );
static int tkL_refresh( lua_State *L );

static const luaL_Reg tkL_methods[] = {
   { "isOpen", tkL_isOpen },
   { "query", tkL_query },
   { "msg", tkL_msg },
   { "yesno", tkL_yesno },
   { "input", tkL_input },
   { "choice", tkL_choice },
   { "list", tkL_list },
   { "merchantOutfit", tkL_merchantOutfit },
   { "custom", tkL_custom },
   { "customRename", tkL_customRename },
   { "customFullscreen", tkL_customFullscreen },
   { "customResize", tkL_customResize },
   { "customSize", tkL_customSize },
   { "customDone", tkL_customDone },
   { "refresh", tkL_refresh },
   { 0, 0 } }; /**< Toolkit Lua methods. */

/**
 * @brief Loads the Toolkit Lua library.
 *
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadTk( nlua_env *env )
{
   nlua_register( env, "tk", tkL_methods, 0 );
   nlua_loadCol( env );
   nlua_loadGFX( env );
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
 * @brief Gets if the toolkit is open. This include any love2d windows.
 *
 *    @luatreturn boolean Whether or not the toolkit is open.
 * @luafunc isOpen
 */
static int tkL_isOpen( lua_State *L )
{
   lua_pushboolean( L, toolkit_isOpen() );
   return 1;
}

/**
 * @brief Gets the position and dimensions of either a window or a widget.
 *
 *    @luatparam string wdwname Name of the window.
 *    @luatparam[opt] string wgtname Name of the widget to get dimensions of. If
 * not specified, the dimensions of the window are returned.
 *    @luatreturn number X position of the window/widget.
 *    @luatreturn number Y position of the window/widget.
 *    @luatreturn number Width of the window/widget.
 *    @luatreturn number Height of the window/widget.
 * @luafunc query
 */
static int tkL_query( lua_State *L )
{
   const char  *wdwname, *wgtname;
   unsigned int wid;
   int          bx, by, x, y, w, h;
   wdwname = luaL_checkstring( L, 1 );
   wgtname = luaL_optstring( L, 2, NULL );
   wid     = window_get( wdwname );
   if ( wid == 0 )
      return 0;
   window_posWindow( wid, &bx, &by );
   if ( wgtname == NULL ) {
      x = bx;
      y = by;
      window_dimWindow( wid, &w, &h );
   } else {
      window_posWidget( wid, wgtname, &x, &y );
      window_dimWidget( wid, wgtname, &w, &h );
      x += bx;
      y += by;
   }
   lua_pushinteger( L, x );
   lua_pushinteger( L, y );
   lua_pushinteger( L, w );
   lua_pushinteger( L, h );
   return 4;
}

/**
 * @brief Creates a window with an ok button, and optionally an image.
 *
 * @usage tk.msg( "Title", "This is a message." )
 * @usage tk.msg( "Title", "This is a message.", "character.png" )
 *
 *    @luatparam string title Title of the window.
 *    @luatparam string message Message to display in the window.
 *    @luatparam[opt=-1] string image Image file to display in the window.
 *    @luatparam[opt=-1] number width width of the image to display. Negative
 * values use image width.
 *    @luatparam[opt=-1] number height height of the image to display. Negative
 * values use image height.
 * @luafunc msg
 */
static int tkL_msg( lua_State *L )
{
   /* Get fixed arguments : title, string to display and image filename. */
   const char *title = luaL_checkstring( L, 1 );
   const char *str   = luaL_checkstring( L, 2 );

   if ( lua_gettop( L ) > 2 ) {
      int         width, height;
      const char *img = luaL_checkstring( L, 3 );

      // Get optional arguments : width and height
      width  = ( lua_gettop( L ) < 4 ) ? -1 : luaL_checkinteger( L, 4 );
      height = ( lua_gettop( L ) < 5 ) ? -1 : luaL_checkinteger( L, 5 );

      dialogue_msgImgRaw( title, str, img, width, height );
      return 0;
   }
   dialogue_msgRaw( title, str );
   return 0;
}
/**
 * @brief Displays a window with Yes and No buttons.
 *
 * @usage if tk.yesno( "YesNo popup box", "Click yes to do something." ) then --
 * Clicked yes
 *
 *    @luatparam string title Title of the window.
 *    @luatparam string message Message to display in the window.
 *    @luatreturn boolean true if yes was clicked, false if no was clicked.
 * @luafunc yesno
 */
static int tkL_yesno( lua_State *L )
{
   const char *title = luaL_checkstring( L, 1 );
   const char *str   = luaL_checkstring( L, 2 );

   int ret = dialogue_YesNoRaw( title, str );
   lua_pushboolean( L, ret );
   return 1;
}
/**
 * @brief Creates a window that allows player to write text input.
 *
 * @usage name = tk.input( "Name", 3, 20, "Enter your name:" )
 *
 *    @luatparam string title Title of the window.
 *    @luatparam number min Minimum characters to accept (must be greater than
 * 0).
 *    @luatparam number max Maximum characters to accept.
 *    @luatparam string str Text to display in the window.
 *    @luatreturn string|nil nil if input was canceled or a string with the text
 * written.
 * @luafunc input
 */
static int tkL_input( lua_State *L )
{
   const char *title, *str;
   char       *ret;
   int         min, max;

   title = luaL_checkstring( L, 1 );
   min   = luaL_checkint( L, 2 );
   max   = luaL_checkint( L, 3 );
   str   = luaL_checkstring( L, 4 );

   ret = dialogue_inputRaw( title, min, max, str );
   if ( ret != NULL ) {
      lua_pushstring( L, ret );
      free( ret );
   } else
      lua_pushnil( L );
   return 1;
}

/**
 * @brief Creates a window with a number of selectable options
 *
 * @usage num, chosen = tk.choice( "Title", "Ready to go?", "Yes", "No" ) -- If
 * "No" was clicked it would return 2, "No"
 *
 *    @luatparam string title Title of the window.
 *    @luatparam string msg Message to display.
 *    @luatparam string choices Option choices.
 *    @luatreturn number The number of the choice chosen.
 *    @luatreturn string The name of the choice chosen.
 * @luafunc choice
 */
static int tkL_choice( lua_State *L )
{
   int         ret, opts;
   const char *title, *str;
   char       *result;

   /* Handle parameters. */
   opts  = lua_gettop( L ) - 2;
   title = luaL_checkstring( L, 1 );
   str   = luaL_checkstring( L, 2 );

   /* Do an initial scan for invalid arguments. */
   for ( int i = 0; i < opts; i++ )
      luaL_checkstring( L, i + 3 );

   /* Create dialogue. */
   dialogue_makeChoice( title, str, opts );
   for ( int i = 0; i < opts; i++ )
      dialogue_addChoice( title, str, luaL_checkstring( L, i + 3 ) );
   result = dialogue_runChoice();
   if ( result == NULL ) /* Something went wrong, return nil. */
      return 0;

   /* Handle results. */
   ret = -1;
   for ( int i = 0; i < opts && ret == -1; i++ ) {
      if ( strcmp( result, luaL_checkstring( L, i + 3 ) ) == 0 )
         ret = i + 1; /* Lua uses 1 as first index. */
   }

   /* Push parameters. */
   lua_pushnumber( L, ret );
   lua_pushstring( L, result );

   /* Clean up. */
   free( result );

   return 2;
}

/**
 * @brief Creates a window with an embedded list of choices.
 *
 * @usage num, chosen = tk.list( "Title", "Foo or bar?", "Foo", "Bar" ) -- If
 * "Bar" is clicked, it would return 2, "Bar"
 *
 *    @luatparam string title Title of the window.
 *    @luatparam string msg Message to display.
 *    @luatparam string choices Option choices.
 *    @luatreturn number The number of the choice chosen.
 *    @luatreturn string The name of the choice chosen.
 * @luafunc list
 */
static int tkL_list( lua_State *L )
{
   int         ret, opts;
   const char *title, *str;
   char      **choices;
   NLUA_MIN_ARGS( 3 );

   /* Handle parameters. */
   opts  = lua_gettop( L ) - 2;
   title = luaL_checkstring( L, 1 );
   str   = luaL_checkstring( L, 2 );

   /* Do an initial scan for invalid arguments. */
   for ( int i = 0; i < opts; i++ )
      luaL_checkstring( L, i + 3 );

   /* Will be freed by the toolkit. */
   choices = malloc( sizeof( char * ) * opts );
   for ( int i = 0; i < opts; i++ )
      choices[i] = strdup( luaL_checkstring( L, i + 3 ) );

   ret = dialogue_listRaw( title, choices, opts, str );

   /* Cancel returns -1, do nothing. */
   if ( ret == -1 )
      return 0;

   /* Push index and choice string. */
   lua_pushnumber( L, ret + 1 );
   lua_pushstring( L, choices[ret] );

   return 2;
}

/**
 * @brief Opens an outfit merchant window.
 *
 * @usage tk.merchantOutfit( 'Laser Merchant', {'Laser Cannon MK0', 'Laser
 * Cannon MK1'} )
 * @usage tk.merchantOutfit( 'Laser Merchant', {outfit.get('Laser Cannon MK0'),
 * outfit.get('Laser Cannon MK1')} )
 *
 *    @luatparam String name Name of the window.
 *    @luatparam Table outfits Table of outfits to sell/buy. It is possible to
 * use either outfits or outfit names (strings).
 *    @luatparam boolean blackmarket Whether or not the merchant is a black
 * market merchant.
 * @luafunc merchantOutfit
 */
static int tkL_merchantOutfit( lua_State *L )
{
   const Outfit **outfits;
   unsigned int   wid;
   const char    *name;
   int            w, h;

   name = luaL_checkstring( L, 1 );

   if ( !lua_istable( L, 2 ) )
      NLUA_INVALID_PARAMETER( L, 2 );

   outfits = array_create_size( const Outfit *, lua_objlen( L, 2 ) );
   /* Iterate over table. */
   lua_pushnil( L );
   while ( lua_next( L, -2 ) != 0 ) {
      array_push_back( &outfits, luaL_validoutfit( L, -1 ) );
      lua_pop( L, 1 );
   }

   /* Create window. */
   if ( SCREEN_W < LAND_WIDTH || SCREEN_H < LAND_HEIGHT ) {
      w = -1; /* Fullscreen. */
      h = -1;
   } else {
      w = LAND_WIDTH + 0.5 * ( SCREEN_W - LAND_WIDTH );
      h = LAND_HEIGHT + 0.5 * ( SCREEN_H - LAND_HEIGHT );
   }
   wid = window_create( "wdwMerchantOutfit", name, -1, -1, w, h );
   outfits_open( wid, outfits, lua_toboolean( L, 3 ) );

   return 0;
}

/**
 * @brief Cleans up after the window.
 */
static void cust_cleanup( void *data )
{
   custom_functions_t *cf = data;
   lua_State          *L  = cf->L;
   luaL_unref( L, LUA_REGISTRYINDEX, cf->update );
   luaL_unref( L, LUA_REGISTRYINDEX, cf->draw );
   luaL_unref( L, LUA_REGISTRYINDEX, cf->keyboard );
   luaL_unref( L, LUA_REGISTRYINDEX, cf->mouse );
   luaL_unref( L, LUA_REGISTRYINDEX, cf->resize );
   luaL_unref( L, LUA_REGISTRYINDEX, cf->textinput );
   nlua_freeEnv( cf->env );
   free( cf );
}

/**
 * @brief Creates a custom widget window.
 *
 *    @luatparam String title Title of the window.
 *    @luatparam Number width Width of the drawable area of the widget.
 *    @luatparam Number height Height of the drawable area of the widget.
 *    @luatparam Function update Function to call when updating. Should take a
 * single parameter which is a number indicating how many seconds passed from
 * previous update.
 *    @luatparam Function draw Function to call when drawing.
 *    @luatparam Function keyboard Function to call when keyboard events are
 * received.
 *    @luatparam Function mouse Function to call when mouse events are received.
 *    @luatparam Function resize Function to call when mouse events are
 * received.
 *    @luatparam Function textinput Function to call when textinput events are
 * received.
 *    @luatparam boolean nodynamic Whether or not the dialogue should be not
 * dynamic, or use `tk.refresh` to force re-rendering.
 * @luafunc custom
 */
static int tkL_custom( lua_State *L )
{
   int                 w       = luaL_checkinteger( L, 2 );
   int                 h       = luaL_checkinteger( L, 3 );
   const char         *caption = luaL_checkstring( L, 1 );
   custom_functions_t *cf      = calloc( 1, sizeof( custom_functions_t ) );
   int                 nodynamic;
   char               *buf, dlgname[STRMAX_SHORT];
   ;

   /* Set up custom function pointers. */
   cf->L   = L;
   cf->env = nlua_dupEnv( __NLUA_CURENV );
#define GETFUNC( address, pos )                                                \
   do {                                                                        \
      lua_pushvalue( L, ( pos ) );                                             \
      luaL_checktype( L, -1, LUA_TFUNCTION );                                  \
      ( address ) = luaL_ref( L, LUA_REGISTRYINDEX );                          \
   } while ( 0 )
   GETFUNC( cf->update, 4 );
   GETFUNC( cf->draw, 5 );
   GETFUNC( cf->keyboard, 6 );
   GETFUNC( cf->mouse, 7 );
   GETFUNC( cf->resize, 8 );
   GETFUNC( cf->textinput, 9 );
#undef GETFUNC
   nodynamic = lua_toboolean( L, 10 );

   /* Set done condition. */
   lua_pushboolean( L, 0 );
   nlua_setenv( L, cf->env, TK_CUSTOMDONE );

   /* Get current dialogue. */
   buf = NULL;
   nlua_getenv( L, cf->env, TK_CUSTNAME );
   if ( lua_isstring( L, -1 ) )
      buf = strdup( lua_tostring( L, -1 ) );
   lua_pop( L, 1 );
   /* Set new dialogue. */
   static unsigned int dlgcust_counter = 0;
   snprintf( dlgname, sizeof( dlgname ), "dlgCust%u", ++dlgcust_counter );
   lua_pushstring( L, dlgname );
   nlua_setenv( L, cf->env, TK_CUSTNAME );

   /* Create the dialogue. */
   dialogue_custom( dlgname, caption, w, h, cust_update, cust_render,
                    cust_event, cf, !nodynamic, cust_cleanup );
   cf->done = 1;

   /* Restore previous value. */
   if ( buf == NULL )
      lua_pushnil( L );
   else
      lua_pushstring( L, buf );
   nlua_setenv( L, cf->env, TK_CUSTNAME );
   free( buf );

   return 0;
}

/* Get the dialogue WID. */
static unsigned int dlgWid( lua_State *L )
{
   nlua_getenv( L, __NLUA_CURENV, TK_CUSTNAME );
   unsigned int wid = window_get( luaL_checkstring( L, -1 ) );
   lua_pop( L, 1 );
   return wid;
}

/**
 * @brief Renames the custom widget window.
 *
 *    @luatparam string displayname Name to give the custom widget window.
 * @luafunc customRename
 */
static int tkL_customRename( lua_State *L )
{
   const char  *s   = luaL_checkstring( L, 1 );
   unsigned int wid = dlgWid( L );
   if ( wid == 0 )
      return NLUA_ERROR( L, _( "custom dialogue not open" ) );
   window_setDisplayname( wid, s );
   return 0;
}

/**
 * @brief Sets the custom widget fullscreen.
 *
 *    @luatparam boolean enable Enable fullscreen or not.
 * @luafunc customFullscreen
 */
static int tkL_customFullscreen( lua_State *L )
{
   int          enable = lua_toboolean( L, 1 );
   unsigned int wid    = dlgWid( L );
   if ( wid == 0 )
      return NLUA_ERROR( L, _( "custom dialogue not open" ) );
   dialogue_customFullscreen( wid, enable );
   return 0;
}

/**
 * @brief Sets the custom widget fullscreen.
 *
 *    @luatparam number width Width of the widget to resize to.
 *    @luatparam number height Height of the widget to resize to.
 * @luafunc customResize
 */
static int tkL_customResize( lua_State *L )
{
   int          w, h;
   unsigned int wid = dlgWid( L );
   if ( wid == 0 )
      return NLUA_ERROR( L, _( "custom dialogue not open" ) );
   w = luaL_checkinteger( L, 1 );
   h = luaL_checkinteger( L, 2 );
   dialogue_customResize( wid, w, h );
   return 0;
}

/**
 * @brief Gets the size of a custom widget.
 *
 *    @luatreturn number Width of the window.
 *    @luatreturn number Height of the window.
 * @luafunc customSize
 */
static int tkL_customSize( lua_State *L )
{
   int          w, h;
   unsigned int wid = dlgWid( L );
   if ( wid == 0 )
      return NLUA_ERROR( L, _( "custom dialogue not open" ) );
   window_dimWindow( wid, &w, &h );
   lua_pushinteger( L, w );
   lua_pushinteger( L, h );
   return 2;
}

/**
 * @brief Ends the execution of a custom widget.
 *
 * Can only stop the execution from the same environment that spawned it.
 *
 * @luafunc customDone
 */
static int tkL_customDone( lua_State *L )
{
   unsigned int wid = dlgWid( L );
   if ( wid == 0 )
      return NLUA_ERROR( L, _( "custom dialogue not open" ) );
   lua_pushboolean( L, 1 );
   nlua_setenv( L, __NLUA_CURENV,
                TK_CUSTOMDONE ); /* Can only stop from same env... */
   return 0;
}

static int cust_update( double dt, void *data )
{
   int                 ret;
   custom_functions_t *cf = (custom_functions_t *)data;
   if ( cf->done )
      return 1; /* Mark done. */
   lua_State *L = cf->L;
   lua_rawgeti( L, LUA_REGISTRYINDEX, cf->update );
   lua_pushnumber( L, dt );
   if ( nlua_pcall( cf->env, 1, 0 ) ) {
      cf->done = 1;
      NLUA_WARN( L, _( "Custom dialogue internal error: %s" ),
                 lua_tostring( L, -1 ) );
      lua_pop( L, 1 );
      return 1;
   }
   /* Check if done. */
   nlua_getenv( L, cf->env, TK_CUSTOMDONE );
   ret = lua_toboolean( L, -1 );
   lua_pop( L, 1 );
   return ret;
}
static void cust_render( double x, double y, double w, double h, void *data )
{
   custom_functions_t *cf = (custom_functions_t *)data;
   if ( cf->done )
      return;
   lua_State *L = cf->L;
   lua_rawgeti( L, LUA_REGISTRYINDEX, cf->draw );
   lua_pushnumber( L, x );
   lua_pushnumber( L, y );
   lua_pushnumber( L, w );
   lua_pushnumber( L, h );
   if ( nlua_pcall( cf->env, 4, 0 ) ) {
      cf->done = 1;
      NLUA_WARN( L, _( "Custom dialogue internal error: %s" ),
                 lua_tostring( L, -1 ) );
      lua_pop( L, 1 );
   }
}
static int cust_event( unsigned int wid, SDL_Event *event, void *data )
{
   (void)wid;
   custom_functions_t *cf = (custom_functions_t *)data;
   if ( cf->done )
      return 0;

   /* Handle all the events. */
   switch ( event->type ) {
   case SDL_EVENT_MOUSE_BUTTON_DOWN:
      return cust_mouse( 1, event->button.button, event->button.x,
                         event->button.y, cf );
   case SDL_EVENT_MOUSE_BUTTON_UP:
      return cust_mouse( 2, event->button.button, event->button.x,
                         event->button.y, cf );
   case SDL_EVENT_MOUSE_MOTION:
      return cust_mouse( 3, -1, event->button.x, event->button.y, cf );
   case SDL_EVENT_MOUSE_WHEEL:
      return cust_mouse( 4, -1, event->wheel.x, event->wheel.y, cf );

   case SDL_EVENT_KEY_DOWN:
      return cust_key( event->key.key, event->key.mod, 1, event->key.repeat,
                       cf );
   case SDL_EVENT_KEY_UP:
      return cust_key( event->key.key, event->key.mod, 0, event->key.repeat,
                       cf );

   case SDL_EVENT_TEXT_INPUT:
      return cust_text( event->text.text, cf );

   case SDL_EVENT_WINDOW_RESIZED:
   case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
   case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
   case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
   case SDL_EVENT_WINDOW_MAXIMIZED:
      return cust_event_window( event->window.type, event->window.data1,
                                event->window.data2, cf );

   default:
      return 0;
   }

   return 0;
}
static int cust_key( SDL_Keycode key, SDL_Keymod mod, int pressed, int isrepeat,
                     custom_functions_t *cf )
{
   int        b;
   lua_State *L = cf->L;
   lua_rawgeti( L, LUA_REGISTRYINDEX, cf->keyboard );
   lua_pushboolean( L, pressed );
   lua_pushstring( L, SDL_GetKeyName( key ) );
   lua_pushstring( L, input_modToText( mod ) );
   lua_pushboolean( L, isrepeat );
   if ( nlua_pcall( cf->env, 4, 1 ) ) {
      cf->done = 1;
      NLUA_WARN( L, _( "Custom dialogue internal error: %s" ),
                 lua_tostring( L, -1 ) );
      lua_pop( L, 1 );
      return 0;
   }
   b = lua_toboolean( L, -1 );
   lua_pop( L, 1 );
   return b;
}
static int cust_text( const char *str, custom_functions_t *cf )
{
   int        b;
   lua_State *L = cf->L;
   lua_rawgeti( L, LUA_REGISTRYINDEX, cf->textinput );
   lua_pushstring( L, str );
   if ( nlua_pcall( cf->env, 1, 1 ) ) {
      cf->done = 1;
      NLUA_WARN( L, _( "Custom dialogue internal error: %s" ),
                 lua_tostring( L, -1 ) );
      lua_pop( L, 1 );
      return 0;
   }
   b = lua_toboolean( L, -1 );
   lua_pop( L, 1 );
   return b;
}
static int cust_mouse( int type, int button, double x, double y,
                       custom_functions_t *cf )
{
   int        b, nargs = 3;
   lua_State *L = cf->L;
   lua_rawgeti( L, LUA_REGISTRYINDEX, cf->mouse );
   lua_pushnumber( L, x );
   lua_pushnumber( L, y );
   lua_pushnumber( L, type );
   if ( type < 3 ) {
      switch ( button ) {
      case SDL_BUTTON_LEFT:
         button = 1;
         break;
      case SDL_BUTTON_RIGHT:
         button = 2;
         break;
      default:
         button = 3;
         break;
      }
      lua_pushnumber( L, button );
      nargs++;
   }
   if ( nlua_pcall( cf->env, nargs, 1 ) ) {
      cf->done = 1;
      NLUA_WARN( L, _( "Custom dialogue internal error: %s" ),
                 lua_tostring( L, -1 ) );
      lua_pop( L, 1 );
      return 0;
   }
   b = lua_toboolean( L, -1 );
   lua_pop( L, 1 );
   return b;
}
static int cust_event_window( Uint32 event, Sint32 w, Sint32 h,
                              custom_functions_t *cf )
{
   int        b;
   lua_State *L = cf->L;

   if ( naev_event_resize( event ) )
      return 1;

   lua_rawgeti( L, LUA_REGISTRYINDEX, cf->resize );
   lua_pushinteger( L, w );
   lua_pushinteger( L, h );
   if ( nlua_pcall( cf->env, 2, 1 ) ) {
      cf->done = 1;
      NLUA_WARN( L, _( "Custom dialogue internal error: %s" ),
                 lua_tostring( L, -1 ) );
      lua_pop( L, 1 );
      return 0;
   }
   b = lua_toboolean( L, -1 );
   lua_pop( L, 1 );
   return b;
}

/**
 * @brief Forces the toolkit to rerender the screen.
 */
static int tkL_refresh( lua_State *L )
{
   (void)L;
   toolkit_rerender();
   return 0;
}
