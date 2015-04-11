/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file nlua_camera.c
 *
 * @brief Bindings for Camera functionality from Lua.
 */


#include "nlua_camera.h"

#include "naev.h"

#include <lauxlib.h>

#include "nlua.h"
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
int nlua_loadCamera( lua_State *L, int readonly )
{
   if (readonly) /* Nothing is read only */
      return 0;

   /* Register the values */
   luaL_register(L, "camera", cameraL_methods);

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
 *    @luaparam target Should be either a vec2 or a pilot to focus on. It will follow pilots around. If nil, it follows the player.
 *    @luaparam soft_over Defaults to false, indicates if the camera should fly over or instantly teleport.
 *    @luaparam speed Speed at which to fly over. Defaults to 2500 if omitted and soft_over is true.
 * @luafunc set( target, soft_over )
 */
static int camL_set( lua_State *L )
{
   LuaPilot *lp;
   LuaVector *lv;
   Pilot *p;
   int soft_over, speed;

   /* Handle arguments. */
   lp = NULL;
   lv = NULL;
   if (lua_ispilot(L,1))
      lp = lua_topilot(L,1);
   else if (lua_isvector(L,1))
      lv = lua_tovector(L,1);
   soft_over = lua_toboolean(L,2);
   if (lua_isnumber(L,3))
      speed = luaL_checkinteger(L,3);
   else
      speed = 2500;

   /* Set the camera. */
   if (lp != NULL) {
      p = pilot_get( lp->pilot );
      if (p != NULL)
         cam_setTargetPilot( p->id, soft_over*speed );
   }
   else if (lv != NULL)
      cam_setTargetPos( lv->vec.x, lv->vec.y, soft_over*speed );
   else {
      if (player.p != NULL)
         cam_setTargetPilot( player.p->id, soft_over*speed );
   }
   return 0;
}

