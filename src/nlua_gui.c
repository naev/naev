/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_gui.c
 *
 * @brief Bindings for GUI functionality from Lua.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_gui.h"

#include "gui.h"
#include "gui_omsg.h"
#include "gui_osd.h"
#include "log.h"
#include "map_overlay.h"
#include "nlua_tex.h"
#include "nluadef.h"

/* GUI methods. */
static int guiL_viewport( lua_State *L );
static int guiL_fpsPos( lua_State *L );
static int guiL_osdInit( lua_State *L );
static int guiL_mesgInit( lua_State *L );
static int guiL_omsgInit( lua_State *L );
static int guiL_radarInit( lua_State *L );
static int guiL_radarRender( lua_State *L );
static int guiL_targetSpobGFX( lua_State *L );
static int guiL_targetPilotGFX( lua_State *L );
static int guiL_mouseClickEnable( lua_State *L );
static int guiL_mouseMoveEnable( lua_State *L );
static int guiL_setMapOverlayBounds( lua_State *L );
static const luaL_Reg guiL_methods[] = {
   { "viewport", guiL_viewport },
   { "fpsPos", guiL_fpsPos },
   { "osdInit", guiL_osdInit },
   { "mesgInit", guiL_mesgInit },
   { "omsgInit", guiL_omsgInit },
   { "radarInit", guiL_radarInit },
   { "radarRender", guiL_radarRender },
   { "targetSpobGFX", guiL_targetSpobGFX },
   { "targetPilotGFX", guiL_targetPilotGFX },
   { "mouseClickEnable", guiL_mouseClickEnable },
   { "mouseMoveEnable", guiL_mouseMoveEnable },
   { "setMapOverlayBounds", guiL_setMapOverlayBounds },
   {0,0}
}; /**< GUI methods. */

/**
 * @brief Loads the GUI library.
 *
 *    @param env Environment to load GUI library into.
 *    @return 0 on success.
 */
int nlua_loadGUI( nlua_env env )
{
   /* Register the values */
   nlua_register(env, "gui", guiL_methods, 0);

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
 *  Careful when using this or you can make the game look ugly and unplayable.
 *  So use common sense and try to avoid windows smaller than 800x600 if
 *  possible.
 *
 * @usage gui.viewport( 0, 0, screen_w, screen_h ) -- Resets viewport.
 * @usage gui.viewport( 0, 20, screen_w, screen_h-20 ) -- Gives 20 pixels for a bottom bar.
 *
 *    @luatparam number x X position to start clipping (bottom left is 0.)
 *    @luatparam number y Y position to start clipping (bottom left is 0.)
 *    @luatparam number w Width of the clipping (width of the screen is default).
 *    @luatparam number h Height of the clipping (height of the screen is default).
 * @luafunc viewport
 */
static int guiL_viewport( lua_State *L )
{
   /* Parameters. */
   double x = luaL_checknumber(L,1);
   double y = luaL_checknumber(L,2);
   double w = luaL_checknumber(L,3);
   double h = luaL_checknumber(L,4);

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
 * @luafunc fpsPos
 */
static int guiL_fpsPos( lua_State *L )
{
   double x = luaL_checknumber(L,1);
   double y = luaL_checknumber(L,2);
   fps_setPos( x, y );
   return 0;
}

/**
 * @brief Initializes the mission OSD (on-screen display).
 *
 *    @luatparam number x X position of the OSD display.
 *    @luatparam number y Y position of the OSD display.
 *    @luatparam number w Width of the OSD display.
 *    @luatparam number h Height of the OSD display.
 * @luafunc osdInit
 */
static int guiL_osdInit( lua_State *L )
{
   /* Parameters. */
   int x = luaL_checkinteger(L,1);
   int y = luaL_checkinteger(L,2);
   int w = luaL_checkinteger(L,3);
   int h = luaL_checkinteger(L,4);

   /* Set up. */
   osd_setup( x, y, w, h );
   return 0;
}

/**
 * @brief Sets up the message box from which the player receives input.
 *
 *    @luatparam number width Width of the message box.
 *    @luatparam number x X position of message box.
 *    @luatparam number y Y position of message box.
 * @luafunc mesgInit
 */
static int guiL_mesgInit( lua_State *L )
{
   /* Parse parameters. */
   int w = luaL_checkinteger( L, 1 );
   int x = luaL_checkinteger( L, 2 );
   int y = luaL_checkinteger( L, 3 );

   /* Initialize. */
   gui_messageInit( w, x, y );
   return 0;
}

/**
 * @brief Sets the center of the omsg messages and width.
 *
 *    @luatparam number width Width of the omsg messages.
 *    @luatparam number x X center of the omsg messages.
 *    @luatparam number y Y center of the omsg messages.
 * @luafunc omsgInit
 */
static int guiL_omsgInit( lua_State *L )
{
   /* Parse parameters. */
   double w = luaL_checkinteger( L, 1 );
   double x = luaL_checkinteger( L, 2 );
   double y = luaL_checkinteger( L, 3 );

   /* Initialize. */
   omsg_position( x, y, w );
   return 0;
}

/**
 * @brief Initializes the radar.
 *
 * @usage gui.radarInit( true, 82 ) -- Circular radar with 82 radius.
 *
 *    @luatparam number circle Whether or not it should be a circle.
 *    @luatparam number width Width if it's not a circle or radius if it is a circle.
 *    @luatparam number height Only needed if not a circle.
 * @luafunc radarInit
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
 *    @luatparam number x X position to render at.
 *    @luatparam number y Y position to render at.
 * @luafunc radarRender
 */
static int guiL_radarRender( lua_State *L )
{
   /* Parse parameters. */
   double x = luaL_checknumber( L, 1 );
   double y = luaL_checknumber( L, 2 );

   /* Render the radar. */
   gui_radarRender( x, y );
   return 0;
}

/**
 * @brief Sets the Lua spob target GFX.
 *
 *    @luatparam Tex tex Texture to set for the spob targeting.
 * @luafunc targetSpobGFX
 */
static int guiL_targetSpobGFX( lua_State *L )
{
   gui_targetSpobGFX( luaL_checktex( L, 1 ) );
   return 0;
}

/**
 * @brief Sets the Lua pilot target GFX.
 *
 *    @luatparam Tex tex Texture to set for the pilot targeting.
 * @luafunc targetPilotGFX
 */
static int guiL_targetPilotGFX( lua_State *L )
{
   gui_targetPilotGFX( luaL_checktex( L, 1 ) );
   return 0;
}

/**
 * @brief Enables mouse clicking callback.
 *
 * It enables receiving mouse clicks with a callback function like:<br />
 * function mouse_click( button, x, y, state ) <br />
 * With button being the ID of the button, x/y being the position clicked and state being true if pressed, false if lifted. It should return true if it used the mouse event or false if it let it through.
 *
 *    @luatparam[opt=true] boolean enable Whether or not to enable the mouse click callback.
 * @luafunc mouseClickEnable
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
 * It enables receiving mouse movements with a callback function like:<br />
 * function mouse_move( x, y ) <br />
 * With x/y being the position of the mouse.
 *
 *    @luatparam[opt] boolean enable Whether or not to enable the mouse movement callback.
 * @luafunc mouseMoveEnable
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
 * @brief Sets map boundaries
 *
 *    @luatparam number top Top boundary in pixels
 *    @luatparam number right Right boundary in pixels
 *    @luatparam number bottom Bottom boundary in pixels
 *    @luatparam number left Left boundary in pixels
 * @luafunc setMapOverlayBounds
 */
static int guiL_setMapOverlayBounds( lua_State *L )
{
   int top    = luaL_checkinteger(L,1);
   int right  = luaL_checkinteger(L,2);
   int bottom = luaL_checkinteger(L,3);
   int left   = luaL_checkinteger(L,4);

   ovr_boundsSet( top, right, bottom, left );
   return 0;
}
