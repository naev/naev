/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nlua_camera.c
 *
 * @brief Bindings for Camera functionality from Lua.
 */
/** @cond */
#include <lauxlib.h>

#include "naev.h"
/** @endcond */

#include "nlua_camera.h"

#include "camera.h"
#include "conf.h"
#include "log.h"
#include "nlua_pilot.h"
#include "nlua_vec2.h"
#include "nluadef.h"
#include "player.h"
#include "spfx.h"

/* Camera methods. */
static int camL_set( lua_State *L );
static int camL_pos( lua_State *L );
static int camL_get( lua_State *L );
static int camL_setZoom( lua_State *L );
static int camL_getZoom( lua_State *L );
static int camL_shake( lua_State *L );
static const luaL_Reg cameraL_methods[] = {
   { "get", camL_get },
   { "set", camL_set },
   { "pos", camL_pos },
   { "setZoom", camL_setZoom },
   { "getZoom", camL_getZoom },
   { "shake", camL_shake },
   {0,0}
}; /**< Camera Lua methods. */

/**
 * @brief Loads the camera library.
 *
 *    @param env Lua environment.
 *    @return 0 on success.
 */
int nlua_loadCamera( nlua_env env )
{
   nlua_register(env, "camera", cameraL_methods, 0);
   return 0;
}

/**
 * @brief Lua bindings to interact with the Camera.
 *
 * An example would be:
 * @code
 * @endcode
 *
 * @luamod camera
 */

/**
 * @brief Sets the camera.
 *
 * Make sure to reset camera after using it or we'll run into trouble.
 *
 * @usage camera.set() -- Resets the camera to the pilot hard.
 * @usage camera.set( a_pilot, true ) -- Flies camera over to a_pilot.
 * @usage camera.set( vec2.new() ) -- Jumps camera to 0,0
 *
 *    @luatparam Pilot|Vec2|nil target It will follow pilots around. If nil, it follows the player.
 *    @luatparam[opt=false] boolean hard_over Indicates that the camera should instantly teleport instead of fly over.
 *    @luaparam[opt=math.min(2000,distance)] speed Speed at which to fly over if hard_over is false.
 * @luafunc set
 */
static int camL_set( lua_State *L )
{
   Pilot *p;
   vec2 *vec;
   double x, y, d;
   int hard_over, speed;

   /* Handle arguments. */
   p = NULL;
   vec = NULL;
   if (lua_ispilot(L,1)) {
      LuaPilot lp = lua_topilot(L,1);
      p = pilot_get( lp );
      if (p==NULL)
         return 0;
      vec = &p->solid.pos;
   }
   else if (lua_isvector(L,1))
      vec = lua_tovector(L,1);
   else if (lua_isnoneornil(L,1)) {
      if (player.p != NULL) {
         p = player.p;
         vec = &player.p->solid.pos;
      }
   }
   else
      NLUA_INVALID_PARAMETER(L,1);
   hard_over = !lua_toboolean(L,2);
   cam_getPos( &x, &y );
   if (vec != NULL)
      d = MOD( vec->x-x, vec->y-y );
   else
      d = 5000.;
   speed = luaL_optinteger(L,3,MIN(2000.,d));

   /* Set the camera. */
   if (p != NULL)
      cam_setTargetPilot( p->id, hard_over*speed );
   else if (vec != NULL)
      cam_setTargetPos( vec->x, vec->y, hard_over*speed );
   return 0;
}

/**
 * @brief Gets the x/y position and zoom of the camera.
 *
 *    @luatreturn number X position of the camera.
 *    @luatreturn number Y position of the camera.
 *    @luatreturn number Zoom level of the camera.
 * @luafunc get
 */
static int camL_get( lua_State *L )
{
   double x, y;
   cam_getPos( &x, &y );
   lua_pushnumber( L, x );
   lua_pushnumber( L, y );
   lua_pushnumber( L, 1.0/cam_getZoom() );
   return 3;
}

/**
 * @brief Gets the camera position.
 *
 *    @luatreturn Vec2 Position of the camera.
 * @luafunc get
 */
static int camL_pos( lua_State *L )
{
   vec2 v;
   double x, y;
   cam_getPos( &x, &y );
   vec2_cset( &v, x, y );
   lua_pushvector( L, v );
   return 1;
}

/**
 * @brief Sets the camera zoom.
 *
 * Make sure to reset camera the zoom after using it or we'll run into trouble.
 *
 * @usage camera.setZoom() -- Resets the camera zoom
 *
 *    @luatparam number zoom Level of zoom to use (1 would indicate 1 unit = 1 pixel while 2 would be 1 unit = 2 pixels)
 *    @luatparam[opt=false] boolean hard_over Indicates that the camera should change the zoom gradually instead of instantly.
 *    @luatparam[opt=naev.conf().zoom_speed]  number speed Rate of change to use.
 * @luafunc setZoom
 */
static int camL_setZoom( lua_State *L )
{
   double zoom = luaL_optnumber(L,1,-1.);
   int hard_over = lua_toboolean(L,2);
   double speed = luaL_optnumber(L,3,conf.zoom_speed);

   /* Handle arguments. */
   if (zoom > 0.) {
      zoom = 1.0 / zoom;
      cam_zoomOverride( 1 );
      if (hard_over) {
         cam_setZoom( zoom );
         cam_setZoomTarget( zoom, speed );
      }
      else
         cam_setZoomTarget( zoom, speed );
   }
   else {
      cam_zoomOverride( 0 );
      cam_setZoomTarget( 1., speed );
   }
   return 0;
}

/**
 * @brief Gets the camera zoom.
 *
 *    @luatreturn number Zoom level of the camera.
 *    @luatreturn number Maximum zoom level of the camera (furthest).
 *    @luatreturn number Minimum zoom level of the camera (closest).
 * @luafunc get
 */
static int camL_getZoom( lua_State *L )
{
   lua_pushnumber( L, 1.0/cam_getZoom() );
   lua_pushnumber( L, 1.0/conf.zoom_far );
   lua_pushnumber( L, 1.0/conf.zoom_near );
   return 3;
}

/**
 * @brief Makes the camera shake.
 *
 * @usage camera.shake() -- Shakes the camera with amplitude 1.
 * @usage camera.shake( 0.5 ) -- Shakes the camera with amplitude .5
 *
 *    @luatparam number amplitude Amplitude of the shaking.
 * @luafunc shake
 */
static int camL_shake( lua_State *L )
{
   double amplitude = luaL_optnumber(L,1,1.);
   spfx_shake( amplitude );
   return 0;
}
