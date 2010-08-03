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
#include "nlua_tex.h"


/* Ship metatable methods. */
static int guiL_radarInit( lua_State *L );
static int guiL_radarRender( lua_State *L );
static const luaL_reg guiL_methods[] = {
   { "radarInit", guiL_radarInit },
   { "radarRender", guiL_radarRender },
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
 * @luamod gfx
 */


/**
 * @brief Initializes the radar.
 *
 * @usage gui.radarInit( true, 82 ) -- Circular radar with 82 radius.
 *
 *    @luaparam circle Whether or not it should be a circle.
 *    @luaparam radius Radius of the circle.
 * @luafunc radarInit( circle, radius )
 */
static int guiL_radarInit( lua_State *L )
{
   int id, circle, radius;

   /* Parse parameters. */
   circle = lua_toboolean( L, 1 );
   radius = luaL_checkinteger( L, 2 );

   /* Create the radar. */
   id = gui_radarInit( circle, radius, radius );
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



