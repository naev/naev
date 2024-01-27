/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file camera.c
 *
 * @brief Handles the camera.
 */

/** @cond */
#include "naev.h"
/** @endcond */

#include "camera.h"

#include "background.h"
#include "conf.h"
#include "gui.h"
#include "log.h"
#include "nebula.h"
#include "pause.h"
#include "player.h"
#include "space.h"
#include "ntracing.h"

#define CAMERA_DIR      (M_PI/2.)

static unsigned int camera_followpilot = 0; /**< Pilot to follow. */
static int zoom_override   = 0; /**< Whether or not to override the zoom. */
/* Current camera position. */
static double camera_Z     = 1.; /**< Current in-game zoom. */
static double camera_X     = 0.; /**< X position of camera. */
static double camera_Y     = 0.; /**< Y position of camera. */
static double camera_DX    = 0.; /**< Derivative of X position (velocity) of the camera. */
static double camera_DY    = 0.; /**< Derivative of Y position (velocity) of the camera. */
static double camera_VX    = 0.;
static double camera_VY    = 0.;
/* Old is used to compensate pilot movement. */
static double old_X        = 0.; /**< Old X positiion. */
static double old_Y        = 0.; /**< Old Y position. */
/* Target is used why flying over with a target set. */
static double target_Z     = 0.; /**< Target zoom. */
static double target_X     = 0.; /**< Target X position. */
static double target_Y     = 0.; /**< Target Y position. */
static int camera_fly      = 0; /**< Camera is flying to target. */
static double camera_flyspeed = 0.; /**< Speed when flying. */
static double camera_zoomspeed = 0.; /**< Speed when zooming. */

/*
 * Prototypes.
 */
static void cam_updateFly( double x, double y, double dt );
static void cam_updatePilot( Pilot *follow, double dt );
static void cam_updatePilotZoom( const Pilot *follow, const Pilot *target, double dt );
static void cam_updateManualZoom( double dt );

/**
 * @brief Overrides the zoom system.
 *
 *    @param enable Whether or not to override the zoom system.
 */
void cam_zoomOverride( int enable )
{
   zoom_override = enable;
}

/**
 * @brief Sets the camera zoom.
 *
 * This is the zoom used in game coordinates.
 *
 *    @param zoom Zoom to set to.
 */
void cam_setZoom( double zoom )
{
   camera_Z = CLAMP( conf.zoom_far, conf.zoom_near, zoom );
}

/**
 * @brief Sets the camera zoom target.
 *
 * This should be used in conjunction with manual zoom.
 *
 *    @param zoom Zoom to try to set camera to.
 *    @param speed Speed of change to use.
 */
void cam_setZoomTarget( double zoom, double speed )
{
   target_Z = CLAMP( conf.zoom_far, conf.zoom_near, zoom );
   camera_zoomspeed = speed;
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
 * @brief Gets the camera zoom.
 *
 *    @return The camera's zoom.
 */
double cam_getZoomTarget (void)
{
   return target_Z;
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
 * @brief Gets the camera position differential (change in last frame).
 */
void cam_getDPos( double *dx, double *dy )
{
   *dx = camera_DX;
   *dy = camera_DY;
}

/**
 * @brief Gets the camera velocity.
 */
void cam_getVel( double *vx, double *vy )
{
   *vx = camera_VX;
   *vy = camera_VY;
}

/**
 * @brief Sets the target to follow.
 */
void cam_setTargetPilot( unsigned int follow, int soft_over )
{
   double dir;

   /* Set the target. */
   camera_followpilot   = follow;
   dir                  = CAMERA_DIR;

   /* Set camera if necessary. */
   if (!soft_over) {
      if (follow != 0) {
         Pilot *p = pilot_get( follow );
         if (p != NULL) {
            double x, y;
            dir      = p->solid.dir;
            x        = p->solid.pos.x;
            y        = p->solid.pos.y;
            camera_X = x;
            camera_Y = y;
            old_X    = x;
            old_Y    = y;
         }
      }
      camera_fly = 0;
   }
   else {
      old_X    = camera_X;
      old_Y    = camera_Y;
      camera_fly = 1;
      camera_flyspeed = (double) soft_over;
   }
   sound_updateListener( dir, camera_X, camera_Y, 0., 0. );
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
      camera_fly = 0;
   }
   else {
      target_X = x;
      target_Y = y;
      old_X    = camera_X;
      old_Y    = camera_Y;
      camera_fly = 1;
      camera_flyspeed = (double) soft_over;
   }
   sound_updateListener( CAMERA_DIR, camera_X, camera_Y, 0., 0. );
}

/**
 * @brief Returns the camera's current target.
 *
 *    @return 0 if focused on position, else returns pilot ID.
 */
int cam_getTarget( void )
{
   return camera_followpilot;
}

/**
 * @brief Updates the camera.
 *
 *    @param dt Current delta tick.
 */
void cam_update( double dt )
{
   NTracingZone( _ctx, 1 );
   Pilot *p;
   double ox, oy;

   /* Calculate differential. */
   camera_DX = camera_X;
   camera_DY = camera_Y;
   ox    = old_X;
   oy    = old_Y;

   /* Going to position. */
   p   = NULL;
   if (camera_fly) {
      if (camera_followpilot != 0) {
         p = pilot_get( camera_followpilot );
         if (p == NULL) {
            camera_followpilot = 0;
            camera_fly = 0;
         }
         else {
            cam_updateFly( p->solid.pos.x, p->solid.pos.y, dt );
            cam_updatePilotZoom( p, NULL, dt );
         }
      }
      else
         cam_updateFly( target_X, target_Y, dt );
   }
   else {
      /* Track pilot. */
      if (camera_followpilot != 0) {
         p = pilot_get( camera_followpilot );
         if (p == NULL)
            camera_followpilot = 0;
         else
            cam_updatePilot( p, dt );
      }
   }

   /* Update manual zoom. */
   if (conf.zoom_manual || zoom_override)
      cam_updateManualZoom( dt );

   /* Set the sound. */
   if ((p==NULL) || !SOUND_PILOT_RELATIVE) {
      double dx = dt*(ox-camera_X);
      double dy = dt*(oy-camera_Y);
      sound_updateListener( CAMERA_DIR, camera_X, camera_Y, dx, dy );
   }
   else {
      sound_updateListener( p->solid.dir,
            p->solid.pos.x, p->solid.pos.y,
            p->solid.vel.x, p->solid.vel.y );
   }

   /* Compute the position differential. */
   camera_DX = (camera_X - camera_DX);
   camera_DY = (camera_Y - camera_DY);

   /* Compute velocity. */
   camera_VX = camera_DX / dt;
   camera_VY = camera_DY / dt;

   NTracingZoneEnd( _ctx );
}

/**
 * @brief Updates the camera flying to a position.
 */
static void cam_updateFly( double x, double y, double dt )
{
   double k, dx,dy, max;

   max = camera_flyspeed*dt;
   k   = 25.*dt;
   dx  = (x - camera_X)*k;
   dy  = (y - camera_Y)*k;
   if (pow2(dx)+pow2(dy) > pow2(max)) {
      double a = atan2( dy, dx );
      double r = max;
      dx = r*cos(a);
      dy = r*sin(a);
   }
   camera_X += dx;
   camera_Y += dy;
   background_moveDust( -dx, -dy );

   /* Stop within 100 pixels. */
   if (FABS((pow2(camera_X)+pow2(camera_Y)) - (pow2(x)+pow2(y))) < 100*100) {
      old_X = camera_X;
      old_Y = camera_Y;
      camera_fly = 0;
   }
}

/**
 * @brief Updates a camera following a pilot.
 */
static void cam_updatePilot( Pilot *follow, double dt )
{
   Pilot *target;
   double dir, k;
   double x,y, dx,dy, mx,my, targ_x,targ_y, bias_x,bias_y, vx,vy;

   /* Get target. */
   if (!pilot_isFlag(follow, PILOT_HYPERSPACE) && (follow->target != follow->id))
      target = pilot_getTarget( follow );
   else
      target = NULL;

   /* Real diagonal might be a bit too harsh since it can cut out the ship,
    * we'll just use the largest of the two. */
   /*diag2 = pow2(SCREEN_W) + pow2(SCREEN_H);*/
   /*diag2 = pow2( MIN(SCREEN_W, SCREEN_H) );*/
   const double diag2 = 100*100;
   x = follow->solid.pos.x;
   y = follow->solid.pos.y;

   /* Compensate player movement. */
   mx        = x - old_X;
   my        = y - old_Y;
   camera_X += mx;
   camera_Y += my;

   /* Set old position. */
   old_X     = x;
   old_Y     = y;

   /* No bias by default. */
   bias_x = 0.;
   bias_y = 0.;

   /* Bias towards target. */
   if (target != NULL) {
      bias_x += target->solid.pos.x - x;
      bias_y += target->solid.pos.y - y;
   }

   /* Bias towards velocity and facing. */
   vx       = follow->solid.vel.x*1.5;
   vy       = follow->solid.vel.y*1.5;
   dir      = angle_diff( atan2(vy,vx), follow->solid.dir);
   dir      = (M_PI - FABS(dir)) /  M_PI; /* Normalize. */
   vx      *= dir;
   vy      *= dir;
   bias_x  += vx;
   bias_y  += vy;

   /* Limit bias. */
   if (pow2(bias_x)+pow2(bias_y) > diag2/2.) {
      double a = atan2( bias_y, bias_x );
      double r = sqrt(diag2)/2.;
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
   background_moveDust( -(mx+dx), -(my+dy) );

   /* Update camera. */
   camera_X += dx;
   camera_Y += dy;

   /* Update zoom. */
   cam_updatePilotZoom( follow, target, dt );
}

/**
 * @brief Updates the manual zoom target.
 */
static void cam_updateManualZoom( double dt )
{
   double d;

   if (player.p == NULL)
      return;

   /* Gradually zoom in/out. */
   d  = CLAMP( -camera_zoomspeed, camera_zoomspeed, target_Z - camera_Z);
   d *= dt / dt_mod; /* Remove dt dependence. */
   if (d < 0.) /** Speed up if needed. */
      d *= 2.;
   camera_Z =  CLAMP( conf.zoom_far, conf.zoom_near, camera_Z + d );
}

/**
 * @brief Updates the camera zoom.
 */
static void cam_updatePilotZoom( const Pilot *follow, const Pilot *target, double dt )
{
   double d, x,y, z,tz, dx, dy;
   double zfar, znear;
   double c;

   /* Must have auto zoom enabled. */
   if (conf.zoom_manual || zoom_override)
      return;

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
   znear = MIN( conf.zoom_near, 1. / (0.8 + VMOD(follow->solid.vel)/d) );

   /* Maximum is limited by nebulae. */
   if (cur_system->nebu_density > 0.) {
      c     = MIN( SCREEN_W, SCREEN_H ) / 2;
      zfar  = CLAMP( conf.zoom_far, conf.zoom_near, c / nebu_getSightRadius() );
   }
   else
      zfar = conf.zoom_far;
   znear = MAX( znear, zfar );

   /*
    * Set Zoom to pilot target.
    */
   z = cam_getZoom();
   if (pilot_isFlag( follow, PILOT_STEALTH )) {
      tz = zfar;
   }
   else {
      if (target != NULL) {
         /* Get current relative target position. */
         gui_getOffset( &x, &y );
         x += target->solid.pos.x - follow->solid.pos.x;
         y += target->solid.pos.y - follow->solid.pos.y;

         /* Get distance ratio. */
         dx = (SCREEN_W/2.) / (FABS(x) + 2.*target->ship->size);
         dy = (SCREEN_H/2.) / (FABS(y) + 2.*target->ship->size);

         /* Get zoom. */
         tz = MIN( dx, dy );
      }
      else
         tz = znear; /* Aim at in. */
   }

   /* Gradually zoom in/out. */
   d  = CLAMP(-conf.zoom_speed, conf.zoom_speed, tz - z);
   d *= dt / dt_mod; /* Remove dt dependence. */
   if (d < 0.) /** Speed up if needed. */
      d *= 2.;
   camera_Z =  CLAMP( zfar, znear, z + d);
}
