/*
 * See Licensing and Copyright notice in naev.h
 */
/** @cond */
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "log.h"
#include "nstring.h"
#include "physics.h"

/**
 * Lists of names for some internal units we use. These just translate them
 * game values to human readable form.
 */
const char UNIT_TIME[]        = N_("sec");
const char UNIT_PER_TIME[]    = N_("/sec");
const char UNIT_DISTANCE[]    = N_("m");
const char UNIT_SPEED[]       = N_("m/sec");
const char UNIT_ENERGY[]      = N_("MJ");
const char UNIT_POWER[]       = N_("MW");
const char UNIT_ANGLE[]       = N_("°");
const char UNIT_ROTATION[]    = N_("°/sec");
const char UNIT_MASS[]        = N_("t");
const char UNIT_CPU[]         = N_("PFLOP");
const char UNIT_UNIT[]        = N_("u");
const char UNIT_PERCENT[]     = N_("%");

/**
 * @brief Converts an angle to the [0, 2*M_PI] range.
 */
static double angle_cleanup( double a )
{
   if (FABS(a) >= 2.*M_PI) {
      a = fmod(a, 2.*M_PI);
   }
   if (a < 0.) {
       a += 2.*M_PI;
   }
   return a;
}
/**
 * @brief Gets the difference between two angles.
 *
 *    @param ref Reference angle.
 *    @param a Angle to get distance from ref.
 */
double angle_diff( double ref, double a )
{
   /* Get angles. */
   double a1 = angle_cleanup(ref);
   double a2 = angle_cleanup(a);
   double d  = a2 - a1;

   /* Filter offsets. */
   d  = (d < M_PI)  ? d : d - 2.*M_PI;
   d  = (d > -M_PI) ? d : d + 2.*M_PI;
   return d;
}

/**
 * @brief Updates the solid's position using an Euler integration.
 *
 * Simple method
 *
 *   d^2 x(t) / d t^2 = a, a = constant (acceleration)
 *   x'(0) = v, x(0) = p
 *
 *   d x(t) / d t = a*t + v, v = constant (initial velocity)
 *   x(t) = a/2*t + v*t + p, p = constant (initial position)
 *
 *   since dt isn't actually differential this gives us ERROR!
 *   so watch out with big values for dt
 */
static void solid_update_euler( Solid *obj, double dt )
{
   double px,py, vx,vy, ax,ay, th;
   double cdir, sdir;

   /* make sure angle doesn't flip */
   obj->dir += obj->dir_vel*dt;
   if (obj->dir >= 2*M_PI)
      obj->dir -= 2*M_PI;
   if (obj->dir < 0.)
      obj->dir += 2*M_PI;

   /* Initial positions. */
   px = obj->pos.x;
   py = obj->pos.y;
   vx = obj->vel.x;
   vy = obj->vel.y;
   th = obj->thrust;

   /* Save direction. */
   sdir = sin(obj->dir);
   cdir = cos(obj->dir);

   /* Get acceleration. */
   ax = th*cdir / obj->mass;
   ay = th*sdir / obj->mass;

   /* Symplectic Euler should reduce a bit the approximation error. */
   vx += ax*dt;
   vy += ay*dt;
   px += vx*dt;
   py += vy*dt;

   /* Update position and velocity. */
   vec2_cset( &obj->vel, vx, vy );
   vec2_cset( &obj->pos, px, py );
}

/**
 * @brief Runge-Kutta method of updating a solid based on its acceleration.
 *
 * Runge-Kutta 4 method
 *
 *   d^2 x(t) / d t^2 = a, a = constant (acceleration)
 *   x'(0) = v, x(0) = p
 *   x'' = f( t, x, x' ) = ( x' , a )
 *
 *   x_{n+1} = x_n + h/6 (k1 + 2*k2 + 3*k3 + k4)
 *    h = (b-a)/2
 *    k1 = f(t_n, X_n ), X_n = (x_n, x'_n)
 *    k2 = f(t_n + h/2, X_n + h/2*k1)
 *    k3 = f(t_n + h/2, X_n + h/2*k2)
 *    k4 = f(t_n + h, X_n + h*k3)
 *
 *   x_{n+1} = x_n + h/6*(6x'_n + 3*h*a, 4*a)
 *
 *
 * Main advantage comes thanks to the fact that Naev is on a 2d plane.
 *  Therefore RK chops it up in chunks and actually creates a tiny curve
 *  instead of approximating the curve for a tiny straight line.
 */
#define RK4_MIN_H 0.01 /**< Minimal pass we want. */
static void solid_update_rk4( Solid *obj, double dt )
{
   int N; /* for iteration, and pass calculation */
   double h, px,py, vx,vy; /* pass, and position/velocity values */
   double ix,iy, tx,ty, th; /* initial and temporary cartesian vector values */
   double vmod, vang;
   int vint;
   int limit; /* limit speed? */

   /* Initial positions and velocity. */
   px = obj->pos.x;
   py = obj->pos.y;
   vx = obj->vel.x;
   vy = obj->vel.y;
   limit = (obj->speed_max >= 0.);

   /* Initial RK parameters. */
   if (dt > RK4_MIN_H)
      N = (int)(dt / RK4_MIN_H);
   else
      N = 1;
   vmod = MOD( vx, vy );
   vint = (int) vmod/100.;
   if (N < vint)
      N = vint;
   h = dt / (double)N; /* step */

   /* Movement Quantity Theorem:  m*a = \sum f */
   th = obj->thrust  / obj->mass;

   for (int i=0; i < N; i++) { /* iterations */
      /* Calculate acceleration for the frame. */
      double ax = th*cos(obj->dir);
      double ay = th*sin(obj->dir);

      /* Limit the speed. */
      if (limit) {
         vmod = MOD( vx, vy );
         if (vmod > obj->speed_max) {
            /* We limit by applying a force against it. */
            vang  = ANGLE( vx, vy ) + M_PI;
            vmod  = 3. * (vmod - obj->speed_max);

            /* Update accel. */
            ax += vmod * cos(vang);
            ay += vmod * sin(vang);
         }
      }

      /* x component */
      tx = ix = ax;
      tx += 2.*ix + h*tx;
      tx += 2.*ix + h*tx;
      tx += ix + h*tx;
      tx *= h/6.;

      vx += tx;
      px += vx * h;

      /* y component */
      ty = iy = ay;
      ty += 2.*iy + h*ty;
      ty += 2.*iy + h*ty;
      ty += iy + h*ty;
      ty *= h/6.;

      vy += ty;
      py += vy * h;

      /* rotation. */
      obj->dir += obj->dir_vel*h;
   }
   vec2_cset( &obj->vel, vx, vy );
   vec2_cset( &obj->pos, px, py );

   /* Validity check. */
   if (obj->dir >= 2.*M_PI)
      obj->dir -= 2.*M_PI;
   else if (obj->dir < 0.)
      obj->dir += 2.*M_PI;
}

/**
 * @brief Gets the maximum speed of any object with speed and thrust.
 */
double solid_maxspeed( const Solid *s, double speed, double thrust )
{
   return speed + thrust / (s->mass * 3.);
}

/**
 * @brief Initializes a new Solid.
 *
 *    @param dest Solid to initialize.
 *    @param mass Mass to set solid to.
 *    @param dir Solid initial direction.
 *    @param pos Initial solid position.
 *    @param vel Initial solid velocity.
 */
void solid_init( Solid* dest, double mass, double dir,
      const vec2* pos, const vec2* vel, int update )
{
   memset(dest, 0, sizeof(Solid));

   dest->mass = mass;

   /* Set direction velocity. */
   dest->dir_vel = 0.;

   /* Set force. */
   dest->thrust  = 0.;

   /* Set direction. */
   dest->dir = dir;
   if ((dest->dir > 2.*M_PI) || (dest->dir < 0.))
      dest->dir = fmod(dest->dir, 2.*M_PI);

   /* Set velocity. */
   if (vel == NULL)
      vectnull( &dest->vel );
   else
      dest->vel = *vel;

   /* Set position. */
   if (pos == NULL)
      vectnull( &dest->pos );
   else
      dest->pos = *pos;

   /* Misc. */
   dest->speed_max = -1.; /* Negative is invalid. */

   /* Handle update. */
   switch (update) {
      case SOLID_UPDATE_RK4:
         dest->update = solid_update_rk4;
         break;

      case SOLID_UPDATE_EULER:
         dest->update = solid_update_euler;
         break;

      default:
         WARN(_("Solid initialization did not specify correct update function!"));
         dest->update = solid_update_rk4;
         break;
   }
}
