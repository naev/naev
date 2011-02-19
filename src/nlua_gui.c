/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_gui.c
 *
 * @brief Bindings for GUI functionality from Lua.
 */

#include "nlua_gui.h"

#include "naev.h"

#include "lauxlib.h"

#include "nlua.h"
#include "nluadef.h"
#include "log.h"
#include "gui.h"
#include "gui_osd.h"
#include "gui_omsg.h"
#include "nlua_tex.h"
#include "menu.h"
#include "info.h"


/* GUI methods. */
static int guiL_viewport( lua_State *L );
static int guiL_fpsPos( lua_State *L );
static int guiL_osdInit( lua_State *L );
static int guiL_mesgInit( lua_State *L );
static int guiL_omsgInit( lua_State *L );
static int guiL_radarInit( lua_State *L );
static int guiL_radarRender( lua_State *L );
static int guiL_targetPlanetGFX( lua_State *L );
static int guiL_targetPilotGFX( lua_State *L );
static int guiL_mouseClickEnable( lua_State *L );
static int guiL_mouseMoveEnable( lua_State *L );
static int guiL_menuInfo( lua_State *L );
static int guiL_menuSmall( lua_State *L );
static const luaL_reg guiL_methods[] = {
   { "viewport", guiL_viewport },
   { "fpsPos", guiL_fpsPos },
   { "osdInit", guiL_osdInit },
   { "mesgInit", guiL_mesgInit },
   { "omsgInit", guiL_omsgInit },
   { "radarInit", guiL_radarInit },
   { "radarRender", guiL_radarRender },
   { "targetPlanetGFX", guiL_targetPlanetGFX },
   { "targetPilotGFX", guiL_targetPilotGFX },
   { "mouseClickEnable", guiL_mouseClickEnable },
   { "mouseMoveEnable", guiL_mouseMoveEnable },
   { "menuInfo", guiL_menuInfo },
   { "menuSmall", guiL_menuSmall },
   {0,0}
}; /**< GUI methods. */




/**
 * @brief Loads the GUI library.
 *
 *    @param L State to load GUI library into.
 *    @return 0 on success.
 */
int nlua_loadGUI( lua_State *L, int readonly )
{
   if (readonly) /* Nothing is read only */
      return 0;

   /* Register the values */
   luaL_register(L, "gui", guiL_methods);

   return 0;
}


/**
 * @brief Lua bindings to interact with the GUI elements.
 *
 * An example would be:
 * @code
 * gui.radarRender( 0, 0 ) -- Renders the radar in the bottom left.
 * @endcode
 *
 * @luamod gui
 */


/**
 * @brief Sets the gui viewport.
 *
 * Basically this limits what the rest of the game considers as the screen.
 *  Careful when using this or you can make the game look ugly and uplayable.
 *  So use common sense and try to avoid windows smaller than 800x600 if
 *  possible.
 *
 * @usage gui.viewport( 0, 0, screen_w, screen_h ) -- Resets viewport.
 * @usage gui.viewport( 0, 20, screen_w, screen_h-20 ) -- Gives 20 pixels for a bottombar.
 *
 *    @luaparam x X position to start clipping (bottom left is 0.)
 *    @luaparam y Y position to start clipping (bottom left is 0.)
 *    @luaparam w Width of the clipping (width of the screen is default).
 *    @luaparam h Height of the clipping (height of the screen is default).
 * @luafunc viewport( x, y, w, h )
 */
static int guiL_viewport( lua_State *L )
{
   double x,y, w,h;

   /* Parameters. */
   x = luaL_checknumber(L,1);
   y = luaL_checknumber(L,2);
   w = luaL_checknumber(L,3);
   h = luaL_checknumber(L,4);

   /* Set the viewport. */
   gui_setViewport( x, y, w, h );
   return 0;
}


/**
 * @brief Sets the position for the fps stuff.
 *
 * It can display the FPS and the current speed mod.
 *
 *    @luaparam x X position for the fps stuff.
 *    @luaparam y Y position for the fps stuff.
 * @luafunc fpsPos( x, y )
 */
static int guiL_fpsPos( lua_State *L )
{
   double x,y;
   x = luaL_checknumber(L,1);
   y = luaL_checknumber(L,2);
   fps_setPos( x, y );
   return 0;
}


/**
 * @brief Initializes the mission OSD (on-screen display).
 *
 *    @luaparam x X position of the OSD display.
 *    @luaparam y Y position of the OSD display.
 *    @luaparam w Width of the OSD display.
 *    @luaparam h Height of the OSD display.
 * @luafunc osdInit( x, y, w, h )
 */
static int guiL_osdInit( lua_State *L )
{
   int x,y, w,h;

   /* Parameters. */
   x = luaL_checkinteger(L,1);
   y = luaL_checkinteger(L,2);
   w = luaL_checkinteger(L,3);
   h = luaL_checkinteger(L,4);

   /* Set up. */
   osd_setup( x, y, w, h );
   return 0;
}


/**
 * @brief Sets up the message box from which the player recieves input.
 *
 *    @luaparam width Width of the message box.
 *    @luaparam x X position of message box.
 *    @luaparam y Y position of message box.
 * @luafunc mesgInit( width, x, y )
 */
static int guiL_mesgInit( lua_State *L )
{
   int w, x, y;

   /* Parse parameters. */
   w = luaL_checkinteger( L, 1 );
   x = luaL_checkinteger( L, 2 );
   y = luaL_checkinteger( L, 3 );

   /* Initialize. */
   gui_messageInit( w, x, y );
   return 0;
}


/**
 * @brief Sets the center of the omsg messages and width.
 *
 *    @luaparam width Width of the omsg messages.
 *    @luaparam x X center of the omsg messages.
 *    @luaparam y Y center of the omsg messages.
 * @luafunc omsgInit( width, x, y )
 */
static int guiL_omsgInit( lua_State *L )
{
   double w, x, y;

   /* Parse parameters. */
   w = luaL_checkinteger( L, 1 );
   x = luaL_checkinteger( L, 2 );
   y = luaL_checkinteger( L, 3 );

   /* Initialize. */
   omsg_position( x, y, w );
   return 0;
}


/**
 * @brief Initializes the radar.
 *
 * @usage gui.radarInit( true, 82 ) -- Circular radar with 82 radius.
 *
 *    @luaparam circle Whether or not it should be a circle.
 *    @luaparam width Width if it's not a circle or radius if it is a circle.
 *    @luaparam height Only needed if not a circle.
 * @luafunc radarInit( circle, width, height )
 */
static int guiL_radarInit( lua_State *L )
{
   int id, circle, width, height;

   /* Parse parameters. */
   circle = lua_toboolean( L, 1 );
   width = luaL_checkinteger( L, 2 );
   if (!circle)
      height = luaL_checkinteger( L, 3 );
   else
      height = width;

   /* Create the radar. */
   id = gui_radarInit( circle, width, height );
   lua_pushnumber( L, id );
   return 1;
}


/**
 * @brief Renders the radar.
 *
 * @usage gui.radarRender( 50, 50 )
 *
 *    @luaparam x X position to render at.
 *    @luaparam y Y position to render at.
 * @luafunc radarRender( x, y )
 */
static int guiL_radarRender( lua_State *L )
{
   double x, y;

   /* Parse parameters. */
   x     = luaL_checknumber( L, 1 );
   y     = luaL_checknumber( L, 2 );

   /* Render the radar. */
   gui_radarRender( x, y );
   return 0;
}


/**
 * @brief Sets the Lua planet target GFX.
 *
 *    @luaparam tex Texture to set for the planet targetting.
 * @luafunc targetPlanetGFX( tex )
 */
static int guiL_targetPlanetGFX( lua_State *L )
{
   LuaTex *lt;
   lt = luaL_checktex( L, 1 );
   gui_targetPlanetGFX( lt->tex );
   return 0;
}


/**
 * @brief Sets the Lua planet target GFX.
 *
 *    @luaparam tex Texture to set for the planet targetting.
 * @luafunc targetPlanetGFX( tex )
 */
static int guiL_targetPilotGFX( lua_State *L )
{
   LuaTex *lt;
   lt = luaL_checktex( L, 1 );
   gui_targetPilotGFX( lt->tex );
   return 0;
}


/**
 * @brief Enables mouse clicking callback.
 *
 * It enables recieving mouse clicks with a callback function like:<br />
 * function mouse_click( button, x, y, state ) <br />
 * With button being the ID of the button, x/y being the position clicked and state being true if pressed, false if lifted. It should return true if it used the mouse event or false if it let it through.
 *
 *    @luaparam enable Whether or not to enable the mouse click callback.
 * @luafunc mouseClickEnable()
 */
static int guiL_mouseClickEnable( lua_State *L )
{
   int b;
   if (lua_gettop(L) > 0)
      b = lua_toboolean(L,1);
   else
      b = 1;
   gui_mouseClickEnable( b );
   return 0;
}


/**
 * @brief Enables mouse movement callback.
 *
 * It enables recieving mouse movements with a callback function like:<br />
 * function mouse_move( x, y ) <br />
 * With x/y being the position of the mouse.
 *
 *    @luaparam enable Whether or not to enable the mouse movement callback.
 * @luafunc mouseMoveEnable()
 */
static int guiL_mouseMoveEnable( lua_State *L )
{
   int b;
   if (lua_gettop(L) > 0)
      b = lua_toboolean(L,1);
   else
      b = 1;
   gui_mouseMoveEnable( b );
   return 0;
}


/**
 * @brief Opens the info menu window.
 *
 * Possible window targets are: <br />
 *  - "main" : Main window.<br />
 *  - "ship" : Ship info window.<br />
 *  - "weapons" : Weapon configuration window.<br />
 *  - "cargo" : Cargo view window.<br />
 *  - "missions" : Mission view window.<br />
 *  - "standings" : Standings view window.<br />
 *
 * @usage gui.menuInfo( "ship" ) -- Opens ship tab
 *
 *    @luaparam window Optional window parameter indicating the tab to open at.
 * @luafunc menuInfo( window )
 */
static int guiL_menuInfo( lua_State *L )
{
   const char *str;
   int window;

   if (menu_open)
      return 0;

   if (lua_gettop(L) > 0)
      str = luaL_checkstring(L,1);
   else {
      /* No parameter. */
      menu_info( INFO_MAIN );
      return 0;
   }

   /* Parse string. */
   if (strcasecmp( str, "main" )==0)
      window = INFO_MAIN;
   else if (strcasecmp( str, "ship" )==0)
      window = INFO_SHIP;
   else if (strcasecmp( str, "weapons" )==0)
      window = INFO_WEAPONS;
   else if (strcasecmp( str, "cargo" )==0)
      window = INFO_CARGO;
   else if (strcasecmp( str, "missions" )==0)
      window = INFO_MISSIONS;
   else if (strcasecmp( str, "standings" )==0)
      window = INFO_STANDINGS;

   /* Open window. */
   menu_info( window );

   return 0;
}


/**
 * @brief Opens the small menu window.
 *
 * @usage gui.menuSmall()
 *
 * @luafunc menuSmall()
 */
static int guiL_menuSmall( lua_State *L )
{
   (void) L;
   if (menu_open)
      return 0;
   menu_small();
   return 0;
}


