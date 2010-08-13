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
#include "nlua_tex.h"


/* Ship metatable methods. */
static int guiL_osdInit( lua_State *L );
static int guiL_mesgInit( lua_State *L );
static int guiL_radarInit( lua_State *L );
static int guiL_radarRender( lua_State *L );
static int guiL_targetPlanetGFX( lua_State *L );
static int guiL_targetPilotGFX( lua_State *L );
static const luaL_reg guiL_methods[] = {
   { "osdInit", guiL_osdInit },
   { "mesgInit", guiL_mesgInit },
   { "radarInit", guiL_radarInit },
   { "radarRender", guiL_radarRender },
   { "targetPlanetGFX", guiL_targetPlanetGFX },
   { "targetPilotGFX", guiL_targetPilotGFX },
   {0,0}
}; /**< Ship metatable methods. */




/**
 * @brief Loads the graphics library.
 *
 *    @param L State to load graphics library into.
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
 * @endcode
 *
 * @luamod gui
 */


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

