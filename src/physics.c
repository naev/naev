/*
 * See Licensing and Copyright notice in naev.h
 */



#include "physics.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
/**
 * @brief Limits the speed of an object.
 *
 *    @param vel Velocity vector to limit.
 *    @param speed Maximum speed.
 *    @param dt Current delta tick.
 */
void limit_speed( Vector2d* vel, const double speed, const double dt )
{
   double vmod;

   vmod = VMOD(*vel);
   if (vmod > speed) /* shouldn't go faster */
      vect_pset( vel, (vmod-speed)*(1. - dt*3.) +  speed, VANGLE(*vel) );
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

/*
 * S O L I D
 */
#if HAS_FREEBSD
/**
 * @brief Updates the solid's position using an euler integration.
 *
 * Simple method
 *
 *   d^2 x(t) / d t^2 = a, a = constant (acceleration)
 *   x'(0) = v, x(0) = p
 *
 *   d x(t) / d t = a*t + v, v = constant (initial velocity)
 *   x(t) = a/2*t + v*t + p, p = constant (initial position)
 *
 *   since d t isn't actually diferential this gives us ERROR!
 *   so watch out with big values for dt
 *
 */
static void simple_update (Solid *obj, const double dt)
{
   double px,py, vx,vy, ax;

   /* make sure angle doesn't flip */
   obj->dir += M_PI/180.*obj->dir_vel*dt;
   if (obj->dir >= 2*M_PI)
      obj->dir -= 2*M_PI;
   if (obj->dir < 0.)
      obj->dir += 2*M_PI;

   /* Initial positions. */
   px = obj->pos.x;
   py = obj->pos.y;
   vx = obj->vel.x;
   vy = obj->vel.y;

   if (obj->force_x != 0.) { /* force applied on object */
      ax = obj->force_x / obj->mass;
      /*ay = obj->force_x / obj->mass;*/

      vx += ax*cos(obj->dir) * dt;
      vy += ax*sin(obj->dir) * dt;

      px += vx*dt + 0.5*ax * dt*dt;
      py += vy*dt; /* + 0.5*ay * dt*dt; */

      obj->vel.mod = MOD(vx,vy);
      obj->vel.angle = ANGLE(vx,vy);
   }
   else {
      px += vx*dt;
      py += vy*dt;
   }

   /* Update position and velocity. */
   vect_cset( &obj->vel, vx, vy );
   vect_cset( &obj->pos, px, py );
}
#endif /* HAS_FREEBSD */


/**
 * @fn static void rk4_update (Solid *obj, const double dt)
 *
 * @brief Runge-Kutta method of updating a solid based on it's acceleration.
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
 * Main advantage comes thanks to the fact that NAEV is on a 2d plane.
 *  Therefore RK chops it up in chunks and actually creates a tiny curve
 *  instead of aproximating the curve for a tiny straight line.
 */
#if !HAS_FREEBSD
#define RK4_MIN_H 0.01 /**< Minimal pass we want. */
static void rk4_update (Solid *obj, const double dt)
{
   int i, N; /* for iteration, and pass calcualtion */
   double h, px,py, vx,vy; /* pass, and position/velocity values */
   double ix,iy, tx,ty, ax; /* initial and temporary cartesian vector values */

   /* Initial RK parameters. */
   N = (dt>RK4_MIN_H) ? (int)(dt/RK4_MIN_H) : 1 ;
   h = dt / (double)N; /* step */

   /* Initial positions and velocity. */
   px = obj->pos.x;
   py = obj->pos.y;
   vx = obj->vel.x;
   vy = obj->vel.y;

   if (obj->force_x > 0.) { /* force applied on object */

      /* Movement Quantity Theorem:  m*a = \sum f */
      ax = obj->force_x / obj->mass;
      /*ay = obj->force.x / obj->mass;*/

      for (i=0; i < N; i++) { /* iterations */

         /* x component */
         tx = ix = vx;
         tx += 2.*ix + h*tx;
         tx += 2.*ix + h*tx;
         tx += ix + h*tx;
         tx *= h/6.;

         px += tx;
         vx += ax*cos(obj->dir) * h;

         /* y component */
         ty = iy = vy;
         ty += 2.*(iy + h/2.*ty);
         ty += 2.*(iy + h/2.*ty);
         ty += iy + h*ty;
         ty *= h/6.;

         py += ty;
         vy += ax*sin(obj->dir) * h;

         /* rotation. */
         obj->dir += M_PI/180.*obj->dir_vel*h;
      }
      vect_cset( &obj->vel, vx, vy );
   }
   else { /* euler method -> p = v*t + 0.5*a*t^2 (no accel, so no error) */
      px += dt*vx;
      py += dt*vy;
      obj->dir += M_PI/180.*obj->dir_vel*dt;
   }
   vect_cset( &obj->pos, px, py );

   /* Sanity check. */
   if (obj->dir >= 2.*M_PI)
      obj->dir -= 2.*M_PI;
   else if (obj->dir < 0.)
      obj->dir += 2.*M_PI;
}
#endif /* !HAS_FREEBSD */


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
      const Vector2d* pos, const Vector2d* vel )
{
   memset(dest, 0, sizeof(Solid));

   dest->mass = mass;

   /* Set direction velocity. */
   dest->dir_vel = 0.;

   /* Set force. */
   dest->force_x = 0.;
   /*dest->force_y = 0.;*/

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

/*
 * FreeBSD seems to have a bug with optimizations in rk4_update causing it to
 * eventually become NaN.
 */
#if HAS_FREEBSD
   dest->update = simple_update;
#else /* HAS_FREEBSD */
   dest->update = rk4_update;
#endif /* HAS_FREEBSD */
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
      const Vector2d* pos, const Vector2d* vel )
{
   Solid* dyn = malloc(sizeof(Solid));
   if (dyn==NULL) ERR("Out of Memory");
   solid_init( dyn, mass, dir, pos, vel );
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

