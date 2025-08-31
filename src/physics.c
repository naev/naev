/*
 * See Licensing and Copyright notice in naev.h
 */
/** @cond */
#include <stdio.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "lualib.h"

#include "constants.h"
#include "log.h"
#include "ndata.h"
#include "nlua.h"
#include "physics.h"

/**
 * Lists of names for some internal units we use. These just translate them
 * game values to human readable form.
 */
const char _UNIT_TIME[]     = N_( "sec" );
const char _UNIT_PER_TIME[] = N_( "/sec" );
const char _UNIT_DISTANCE[] = N_( "km" );
const char _UNIT_SPEED[]    = N_( "km/s" );
const char _UNIT_ACCEL[]    = N_( "km/s²" );
const char _UNIT_ENERGY[]   = N_( "GJ" );
const char _UNIT_POWER[]    = N_( "GW" );
const char _UNIT_ANGLE[]    = N_( "°" );
const char _UNIT_ROTATION[] = N_( "°/s" );
const char _UNIT_MASS[]     = N_( "t" );
const char _UNIT_CPU[]      = N_( "PFLOP" );
const char _UNIT_UNIT[]     = N_( "u" );
const char _UNIT_PERCENT[]  = N_( "%" );

/**
 * @brief Converts an angle to the [0, 2*M_PI] range.
 */
double angle_clean( double a )
{
   if ( FABS( a ) >= 2. * M_PI ) {
      a = fmod( a, 2. * M_PI );
   }
   if ( a < 0. ) {
      a += 2. * M_PI;
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
   double a1 = angle_clean( ref );
   double a2 = angle_clean( a );
   double d  = a2 - a1;

   /* Filter offsets. */
   d = ( d < M_PI ) ? d : d - 2. * M_PI;
   d = ( d > -M_PI ) ? d : d + 2. * M_PI;
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
   double px, py, vx, vy, ax, ay, th;
   double cdir, sdir;

   /* Save previous position. */
   obj->pre = obj->pos;

   /* Make sure angle doesn't flip */
   obj->dir += obj->dir_vel * dt;
   obj->dir = angle_clean( obj->dir );

   /* Initial positions. */
   px = obj->pos.x;
   py = obj->pos.y;
   vx = obj->vel.x;
   vy = obj->vel.y;
   th = obj->accel;

   /* Save direction. */
   sdir = sin( obj->dir );
   cdir = cos( obj->dir );

   /* Get acceleration. */
   ax = th * cdir;
   ay = th * sdir;

   /* Symplectic Euler should reduce a bit the approximation error. */
   vx += ax * dt;
   vy += ay * dt;
   px += vx * dt;
   py += vy * dt;

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
 * Therefore Runge-Kutta chops it up in chunks and actually creates a tiny curve
 * instead of approximating the curve for a tiny straight line.
 */
#define RK4_MIN_H 0.01 /**< Minimal pass we want. */
static void solid_update_rk4( Solid *obj, double dt )
{
   int    N;                 /* for iteration, and pass calculation */
   double h, px, py, vx, vy; /* pass, and position/velocity values */
   double vmod, vang, th;
   int    vint;
   int    limit; /* limit speed? */

   /* Save previous position. */
   obj->pre = obj->pos;

   /* Initial positions and velocity. */
   px    = obj->pos.x;
   py    = obj->pos.y;
   vx    = obj->vel.x;
   vy    = obj->vel.y;
   limit = ( obj->speed_max >= 0. );

   /* Initial RK parameters. */
   if ( dt > RK4_MIN_H )
      N = (int)( dt / RK4_MIN_H );
   else
      N = 1;
   vmod = MOD( vx, vy );
   vint = (int)vmod / 100.;
   if ( N < vint )
      N = vint;
   h = dt / (double)N; /* step */

   /* Movement Quantity Theorem:  m*a = \sum f */
   th = obj->accel;

   for ( int i = 0; i < N; i++ ) { /* iterations */
      double ix, iy, tx, ty;
      /* Calculate acceleration for the frame. */
      double ax = th * cos( obj->dir );
      double ay = th * sin( obj->dir );

      /* Limit the speed. */
      if ( limit ) {
         vmod = MOD( vx, vy );
         if ( vmod > obj->speed_max ) {
            /* We limit by applying a force against it. */
            vang = ANGLE( vx, vy ) + M_PI;
            vmod = CTS.PHYSICS_SPEED_DAMP / obj->aerodynamics *
                   ( vmod - obj->speed_max );

            /* Update accel. */
            ax += vmod * cos( vang );
            ay += vmod * sin( vang );
         }
      }

      /* x component */
      tx = ix = ax;
      tx += 2. * ix + h * tx;
      tx += 2. * ix + h * tx;
      tx += ix + h * tx;
      tx *= h / 6.;

      vx += tx;
      px += vx * h;

      /* y component */
      ty = iy = ay;
      ty += 2. * iy + h * ty;
      ty += 2. * iy + h * ty;
      ty += iy + h * ty;
      ty *= h / 6.;

      vy += ty;
      py += vy * h;

      /* rotation. */
      obj->dir += obj->dir_vel * h;
   }
   vec2_cset( &obj->vel, vx, vy );
   vec2_cset( &obj->pos, px, py );

   /* Validity check. */
   obj->dir = angle_clean( obj->dir );
}

/**
 * @brief Gets the maximum speed of any object with speed and accel.
 */
double solid_maxspeed( const Solid *s, double speed, double accel )
{
   // s->speed_max can get overwritten to limit speed. Here we want the true
   // max_speed.
   return speed + accel * s->aerodynamics / CTS.PHYSICS_SPEED_DAMP;
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
void solid_init( Solid *dest, double mass, double dir, const vec2 *pos,
                 const vec2 *vel, int update )
{
   memset( dest, 0, sizeof( Solid ) );

   dest->mass = mass;

   /* Set direction velocity. */
   dest->dir_vel = 0.;

   /* Set acceleration. */
   dest->accel = 0.;

   /* Set direction. */
   dest->dir = angle_clean( dir );

   /* Set velocity. */
   if ( vel == NULL )
      vectnull( &dest->vel );
   else
      dest->vel = *vel;

   /* Set position. */
   if ( pos == NULL )
      vectnull( &dest->pos );
   else
      dest->pos = *pos;
   dest->pre = dest->pos; /* Store previous position. */

   /* Misc. */
   dest->speed_max    = -1.; /* Negative is invalid. */
   dest->aerodynamics = 1.;

   /* Handle update. */
   switch ( update ) {
   case SOLID_UPDATE_RK4:
      dest->update = solid_update_rk4;
      break;

   case SOLID_UPDATE_EULER:
      dest->update = solid_update_euler;
      break;

   default:
      WARN(
         _( "Solid initialization did not specify correct update function!" ) );
      dest->update = solid_update_rk4;
      break;
   }
}
