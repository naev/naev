/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file camera.c
 *
 * @brief Handles the camera.
 */


#include "camera.h"

#include "naev.h"

#include "log.h"
#include "conf.h"
#include "space.h"
#include "gui.h"
#include "nebula.h"
#include "pause.h"
#include "background.h"


static Vector2d* camera_pos = NULL; /**< Camera we are using. */
static double camera_Z    = 1.; /**< Current in-game zoom. */
static double camera_X    = 0.; /**< X position of camera. */
static double camera_Y    = 0.; /**< Y position of camera. */


/*
 * Prototypes.
 */
static void cam_updatePilotZoom( Pilot *follow, Pilot *target, double dt );


/**
 * @brief Sets the camera zoom.
 *
 * This is the zoom used in game coordinates.
 *
 *    @param zoom Zoom to set to.
 */
void cam_setZoom( double zoom )
{
   camera_Z = zoom;
}


/**
 * @brief Gets the camera zoom.
 *
 *    @return The camera's zoom.
 */
double cam_getZoom (void)
{
   return camera_Z;
}


/**
 * @brief Makes the camera static and set on a position.
 *
 *    @param x X position to set camera to.
 *    @param y Y position to set camera to.
 */
void cam_setStatic( double x, double y )
{
   camera_X = x;
   camera_Y = y;
   camera_pos  = NULL;
}


/**
 * @brief Gets the camera position.
 *
 *    @param[out] x X position to get.
 *    @param[out] y Y position to get.
 */
void cam_getPos( double *x, double *y )
{
   *x = camera_X;
   *y = camera_Y;
}


/**
 * @brief Updates a camera following a pilot.
 */
void cam_updatePilot( Pilot *follow, Pilot *target, double dt )
{
   double diag2, a, r;
   double dx,dy, targ_x,targ_y, bias_x,bias_y;

   diag2 = pow2(SCREEN_W) + pow2(SCREEN_H);

   /* No bias by default. */
   bias_x = 0.;
   bias_y = 0.;

   /* Bias towards target. */
   if (target != NULL) {
      bias_x = target->solid->pos.x - follow->solid->pos.x;
      bias_y = target->solid->pos.y - follow->solid->pos.y;
   }

   /* Limit bias. */
   if (pow2(bias_x)+pow2(bias_y) > diag2/2.) {
      a        = atan2( bias_y, bias_x );
      r        = sqrt(diag2)/2.;
      bias_x   = r*cos(a);
      bias_y   = r*sin(a);
   }

   /* Compose the target. */
   targ_x   = follow->solid->pos.x + bias_x;
   targ_y   = follow->solid->pos.y + bias_y;

   /* Head towards target. */
   dx = (targ_x-camera_X)*dt/dt_mod;
   dy = (targ_y-camera_Y)*dt/dt_mod;
   background_moveStars( dx, dy );

   /* Update camera. */
   camera_X += dx;
   camera_Y += dy;

   /* Update zoom. */
   cam_updatePilotZoom( follow, target, dt );
}


/**
 * @brief Updates the camera zoom.
 */
static void cam_updatePilotZoom( Pilot *follow, Pilot *target, double dt )
{
   double d, x,y, z,tz, dx, dy;
   double zfar, znear;
   double c;

   /* Minimum depends on velocity normally.
    *
    * w*h = A, cte    area constant
    * w/h = K, cte    proportion constant
    * d^2 = A, cte    geometric longitud
    *
    * A_v = A*(1+v/d)   area of view is based on speed
    * A_v / A = (1 + v/d)
    *
    * z = A / A_v = 1. / (1 + v/d)
    */
   d     = sqrt(SCREEN_W*SCREEN_H);
   znear = MIN( conf.zoom_near, 1. / (0.8 + VMOD(follow->solid->vel)/d) );

   /* Maximum is limited by nebulae. */
   if (cur_system->nebu_density > 0.) {
      c    = MIN( SCREEN_W, SCREEN_H ) / 2;
      zfar = CLAMP( conf.zoom_far, conf.zoom_near, c / nebu_getSightRadius() );
   }
   else {
      zfar = conf.zoom_far;
   }

   /*
    * Set Zoom to pilot target.
    */
   z = cam_getZoom();
   if (target != NULL) {
      /* Get current relative target position. */
      gui_getOffset( &x, &y );
      x += target->solid->pos.x - follow->solid->pos.x;
      y += target->solid->pos.y - follow->solid->pos.y;

      /* Get distance ratio. */
      dx = (SCREEN_W/2.) / (FABS(x) + 2*target->ship->gfx_space->sw);
      dy = (SCREEN_H/2.) / (FABS(y) + 2*target->ship->gfx_space->sh);

      /* Get zoom. */
      tz = MIN( dx, dy );
   }
   else {
      tz = znear; /* Aim at in. */
   }

   /* Gradually zoom in/out. */
   d  = CLAMP(-conf.zoom_speed, conf.zoom_speed, tz - z);
   d *= dt / dt_mod; /* Remove dt dependence. */
   if (d < 0) /** Speed up if needed. */
      d *= 2.;
   camera_Z =  CLAMP( zfar, znear, z + d);
}


