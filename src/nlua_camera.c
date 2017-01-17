/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nlua_camera.c
 *
 * @brief Bindings for Camera functionality from Lua.
 */

#include "nlua_camera.h"

#include "naev.h"

#include <lauxlib.h>

#include "nluadef.h"
#include "log.h"
#include "nlua_vec2.h"
#include "nlua_pilot.h"
#include "camera.h"
#include "player.h"


/* Camera methods. */
static int camL_set( lua_State *L );
static const luaL_reg cameraL_methods[] = {
   { "set", camL_set },
   {0,0}
}; /**< Camera Lua methods. */




/**
 * @brief Loads the camera library.
 *
 *    @param L State to load camera library into.
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
 *    @luatparam[opt=false] boolean soft_over Indicates that the camera should fly over rather than instantly teleport.
 *    @luaparam[opt=2500] speed Speed at which to fly over if soft_over is true.
 * @luafunc set( target, soft_over, speed )
 */
static int camL_set( lua_State *L )
{
   LuaPilot lp;
   Vector2d *vec;
   Pilot *p;
   int soft_over, speed;

   NLUA_CHECKRW(L);

   /* Handle arguments. */
   lp = 0;
   vec = NULL;
   if (lua_ispilot(L,1))
      lp = lua_topilot(L,1);
   else if (lua_isvector(L,1))
      vec = lua_tovector(L,1);
   soft_over = lua_toboolean(L,2);
   if (lua_isnumber(L,3))
      speed = luaL_checkinteger(L,3);
   else
      speed = 2500;

   /* Set the camera. */
   if (lp != 0) {
      p = pilot_get( lp );
      if (p != NULL)
         cam_setTargetPilot( p->id, soft_over*speed );
   }
   else if (vec != NULL)
      cam_setTargetPos( vec->x, vec->y, soft_over*speed );
   else {
      if (player.p != NULL)
         cam_setTargetPilot( player.p->id, soft_over*speed );
   }
   return 0;
}

