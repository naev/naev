/*
 * See Licensing and Copyright notice in naev.h
 */



#include "physics.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include "nstring.h"

#include "log.h"


/*
 * M I S C
 */
static double angle_cleanup( double a )
{
   double na;
   if (FABS(a) >= 2.*M_PI) {
      na = fmod(a, 2.*M_PI);
      if (a < 0.)
         na += 2.*M_PI;
      return  na;
   }
   return a;
}
/**
 * @brief Gets the difference between two angles.
 *
 *    @param ref Reference angle.
 *    @param a Angle to get distance from ref.
 */
double angle_diff( const double ref, double a )
{
   double d;
   double a1, a2;

   /* Get angles. */
   a1 = angle_cleanup(ref);
   a2 = angle_cleanup(a);
   d  = a2 - a1;

   /* Filter offsets. */
   d  = (d < M_PI)  ? d : d - 2.*M_PI;
   d  = (d > -M_PI) ? d : d + 2.*M_PI;
   return d;
}


/*
 *
 * V E C T O R 2 D
 *
 */
/**
 * @brief Set the vector value using cartesian coordinates
 *
 *    @param v Vector to set.
 *    @param x X value for vector.
 *    @param y Y value for vector.
 */
void vect_cset( Vector2d* v, const double x, const double y )
{
   v->x     = x;
   v->y     = y;
   v->mod   = MOD(x,y);
   v->angle = ANGLE(x,y);
}


/**
 * @brief Creates a minimal vector only valid for blitting and not other operations.
 *
 *    @param v Vector to set.
 *    @param x X value for vector.
 *    @param y Y value for vector.
 */
void vect_csetmin( Vector2d* v, const double x, const double y )
{
   v->x = x;
   v->y = y;
}


/**
 * @brief Set the vector value using polar coordinates.
 *
 *    @param v Vector to set.
 *    @param mod Modulus of the vector.
 *    @param angle Angle of the vector.
 */
void vect_pset( Vector2d* v, const double mod, const double angle )
{
   v->mod   = mod;
   v->angle = angle;
   v->x     = v->mod*cos(v->angle);
   v->y     = v->mod*sin(v->angle);
}


/**
 * @brief Copies vector src to dest.
 *
 *    @param dest Destination vector.
 *    @param src Vector to copy.
 */
void vectcpy( Vector2d* dest, const Vector2d* src )
{
   dest->x     = src->x;
   dest->y     = src->y;
   dest->mod   = src->mod;
   dest->angle = src->angle;
}


/**
 * @brief Sets a vector to NULL.
 *
 *    @param v Vector to set to NULL.
 */
void vectnull( Vector2d* v )
{
   v->x     = 0.;
   v->y     = 0.;
   v->mod   = 0.;
   v->angle = 0.;
}


/**
 * @brief Get the direction pointed to by two vectors (from ref to v).
 *
 *    @param ref Reference vector.
 *    @param v Vector to get angle from reference vector.
 *    @return Angle between ref and v.
 */
double vect_angle( const Vector2d* ref, const Vector2d* v )
{
   double x,y;

   x = v->x - ref->x;
   y = v->y - ref->y;

   return ANGLE( x, y );
}


/**
 * @brief Adds x and y to the current vector
 *
 *    @param v Vector to add x and y to.
 *    @param x X value to add to vector.
 *    @param y Y value to add to vector.
 */
void vect_cadd( Vector2d* v, const double x, const double y )
{
   v->x    += x;
   v->y    += y;
   v->mod   = MOD(v->x,v->y);
   v->angle = ANGLE(v->x,v->y);
}


/**
 * @brief Adds a polar 2d vector to the current vector.
 *
 *    @param v Vector to add x and y to.
 *    @param m Module of vector to add.
 *    @param a Angle of vector to add.
 */
void vect_padd( Vector2d* v, const double m, const double a )
{
   v->x    += m*cos(a);
   v->y    += m*sin(a);
   v->mod   = MOD(v->x,v->y);
   v->angle = ANGLE(v->x,v->y);
}


/**
 * @brief Mirrors a vector off another, stores results in vector.
 *
 *    @param r Resulting vector of the reflection.
 *    @param v Vector to reflect.
 *    @param n Normal to reflect off of.
 */
void vect_reflect( Vector2d* r, Vector2d* v, Vector2d* n )
{
   double dot;

   dot      = vect_dot( v, n );
   r->x     = v->x - ((2. * dot) * n->x);
   r->y     = v->y - ((2. * dot) * n->y);
   r->mod   = MOD(r->x,r->y);
   r->angle = MOD(r->x,r->y);
}


/**
 * @brief Vector dot product.
 *
 *    @param a Vector 1 for dot product.
 *    @param b Vector 2 for dot product.
 *    @return Dot product of vectors.
 */
double vect_dot( Vector2d* a, Vector2d* b )
{
   return a->x * b->x + a->y * b->y;
}


/**
 * @brief Determines the magnitude of the source vector components.
 *
 *    @param[out] u Parallel component to reference vector.
 *    @param[out] v Perpendicular component to reference vector.
 *    @param source Source vector.
 *    @param reference_vector Reference vector.
 */
void vect_uv( double* u, double* v, Vector2d* source, Vector2d* reference_vector )
{
   Vector2d unit_parallel, unit_perpendicular;

   vect_uv_decomp(&unit_parallel, &unit_perpendicular, reference_vector);

   *u = vect_dot(source, &unit_parallel);
   *v = vect_dot(source, &unit_perpendicular);
}

/**
 * @brief Does UV decomposition of the reference vector.
 *
 *    @param[out] u Parallel component of the reference vector.
 *    @param[out] v Perpendicular component of the reference vector.
 *    @param reference_vector The reference vector to decompose.
 */
void vect_uv_decomp( Vector2d* u, Vector2d* v, Vector2d* reference_vector )
{
   vect_pset(u, 1, VANGLE(*reference_vector));
   vect_pset(v, 1, VANGLE(*reference_vector)+M_PI_2);
}


/*
 * S O L I D
 */
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
 *
 */
static void solid_update_euler (Solid *obj, const double dt)
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

   /* p = v*dt + 0.5*a*dt^2 */
   px += vx*dt + 0.5*ax * dt*dt;
   py += vy*dt + 0.5*ay * dt*dt;

   /* Update position and velocity. */
   vect_cset( &obj->vel, vx, vy );
   vect_cset( &obj->pos, px, py );
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
static void solid_update_rk4 (Solid *obj, const double dt)
{
   int i, N; /* for iteration, and pass calculation */
   double h, px,py, vx,vy; /* pass, and position/velocity values */
   double ix,iy, tx,ty, ax,ay, th; /* initial and temporary cartesian vector values */
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

   for (i=0; i < N; i++) { /* iterations */

      /* Calculate acceleration for the frame. */
      ax = th*cos(obj->dir);
      ay = th*sin(obj->dir);

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
      tx = ix = vx;
      tx += 2.*ix + h*tx;
      tx += 2.*ix + h*tx;
      tx += ix + h*tx;
      tx *= h/6.;

      px += tx;
      vx += ax * h;

      /* y component */
      ty = iy = vy;
      ty += 2.*(iy + h/2.*ty);
      ty += 2.*(iy + h/2.*ty);
      ty += iy + h*ty;
      ty *= h/6.;

      py += ty;
      vy += ay * h;

      /* rotation. */
      obj->dir += obj->dir_vel*h;
   }
   vect_cset( &obj->vel, vx, vy );
   vect_cset( &obj->pos, px, py );

   /* Sanity check. */
   if (obj->dir >= 2.*M_PI)
      obj->dir -= 2.*M_PI;
   else if (obj->dir < 0.)
      obj->dir += 2.*M_PI;
}


/**
 * @brief Gets the maximum speed of any object with speed and thrust.
 */
double solid_maxspeed( Solid *s, double speed, double thrust )
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
void solid_init( Solid* dest, const double mass, const double dir,
      const Vector2d* pos, const Vector2d* vel, int update )
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
      vectcpy( &dest->vel, vel );

   /* Set position. */
   if (pos == NULL)
      vectnull( &dest->pos );
   else
      vectcpy( &dest->pos, pos);

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
         WARN("Solid initialization did not specify correct update function!");
         dest->update = solid_update_rk4;
         break;
   }
}


/**
 * @brief Creates a new Solid.
 *
 *    @param mass Mass to set solid to.
 *    @param dir Solid initial direction.
 *    @param pos Initial solid position.
 *    @param vel Initial solid velocity.
 *    @return A newly created solid.
 */
Solid* solid_create( const double mass, const double dir,
      const Vector2d* pos, const Vector2d* vel, int update )
{
   Solid* dyn = malloc(sizeof(Solid));
   if (dyn==NULL)
      ERR("Out of Memory");
   solid_init( dyn, mass, dir, pos, vel, update );
   return dyn;
}


/**
 * @brief Frees an existing solid.
 *
 *    @param src Solid to free.
 */
void solid_free( Solid* src )
{
   free(src);
}

