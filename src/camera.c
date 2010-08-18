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


static unsigned int camera_followpilot = 0; /**< Pilot to follow. */
static double camera_Z     = 1.; /**< Current in-game zoom. */
static double camera_X     = 0.; /**< X position of camera. */
static double camera_Y     = 0.; /**< Y position of camera. */
static double old_X        = 0.; /**< Old X positiion. */
static double old_Y        = 0.; /**< Old Y position. */


/*
 * Prototypes.
 */
static void cam_updatePilot( Pilot *follow, double dt );
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
 * @brief Sets the target to follow.
 */
void cam_setTargetPilot( unsigned int follow, int soft_over )
{
   Pilot *p;
   double x, y;

   /* Set the target. */
   camera_followpilot   = follow;

   /* Set camera if necessary. */
   if (!soft_over && (follow != 0)) {
      p = pilot_get( follow );
      if (p != NULL) {
         x = p->solid->pos.x;
         y = p->solid->pos.y;
         camera_X = x;
         camera_Y = y;
         old_X    = x;
         old_Y    = y;
      }
   }
}


/**
 * @brief Sets the camera target to a position.
 */
void cam_setTargetPos( double x, double y, int soft_over )
{
   /* Disable pilot override. */
   camera_followpilot = 0;

   /* Handle non soft. */
   if (!soft_over) {
         camera_X = x;
         camera_Y = y;
         old_X    = x;
         old_Y    = y;
   }
}


/**
 * @brief Updates the camera.
 *
 *    @param dt Current delta tick.
 */
void cam_update( double dt )
{
   Pilot *p;

   if (camera_followpilot != 0) {
      p = pilot_get( camera_followpilot );
      if (p == NULL)
         camera_followpilot = 0;
      else
         cam_updatePilot( p, dt );
   }
}


/**
 * @brief Updates a camera following a pilot.
 */
static void cam_updatePilot( Pilot *follow, double dt )
{
   Pilot *target;
   double diag2, a, r, dir, k;
   double x,y, dx,dy, targ_x,targ_y, bias_x,bias_y, vx,vy;

   /* Get target. */
   if (follow->target != follow->id)
      target = pilot_get( follow->target );
   else
      target = NULL;

   /* Real diagonal might be a bit too harsh since it can cut out the ship,
    * we'll just use the largest of the two. */
   /*diag2 = pow2(SCREEN_W) + pow2(SCREEN_H);*/
   /*diag2 = pow2( MIN(SCREEN_W, SCREEN_H) );*/
   diag2 = 100*100;
   x = follow->solid->pos.x;
   y = follow->solid->pos.y;

   /* Compensate player movement. */
   camera_X += x - old_X;
   camera_Y += y - old_Y;

   /* Set old position. */
   old_X     = x;
   old_Y     = y;

   /* No bias by default. */
   bias_x = 0.;
   bias_y = 0.;

   /* Bias towards target. */
   if (target != NULL) {
      bias_x += target->solid->pos.x - x;
      bias_y += target->solid->pos.y - y;
   }

   /* Bias towards velocity and facing. */
   vx       = follow->solid->vel.x*1.5;
   vy       = follow->solid->vel.y*1.5;
   dir      = angle_diff( atan2(vy,vx), follow->solid->dir);
   dir      = (M_PI - fabs(dir)) /  M_PI; /* Normalize. */
   vx      *= dir;
   vy      *= dir;
   bias_x  += vx;
   bias_y  += vy;

   /* Limit bias. */
   if (pow2(bias_x)+pow2(bias_y) > diag2/2.) {
      a        = atan2( bias_y, bias_x );
      r        = sqrt(diag2)/2.;
      bias_x   = r*cos(a);
      bias_y   = r*sin(a);
   }

   /* Compose the target. */
   targ_x   = x + bias_x;
   targ_y   = y + bias_y;

   /* Head towards target. */
   k = 0.5*dt/dt_mod;
   dx = (targ_x-camera_X)*k;
   dy = (targ_y-camera_Y)*k;
   background_moveStars( dx, dy );

   /* Update camera. */
   camera_X += dx;
   camera_Y += dy;

   /* DEBUG. */
#if 0
   glColor4d( 1., 1., 1., 1. );
   glBegin(GL_LINES);
   gl_gameToScreenCoords( &x, &y, x, y );
   glVertex2d( x, y );
   gl_gameToScreenCoords( &x, &y, camera_X, camera_Y );
   glVertex2d( x, y );
   glEnd(); /* GL_LINES */
#endif

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


