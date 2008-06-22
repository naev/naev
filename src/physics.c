/*
 * See Licensing and Copyright notice in naev.h
 */



#include "physics.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "naev.h"
#include "log.h"


/*
 * M I S C
 */
double angle_diff( const double ref, double a )
{
   double d;

   if (a < M_PI) a += 2*M_PI;
   d = fmod((a-ref),2*M_PI);
   return (d <= M_PI) ? d : d - 2*M_PI ;
}
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
/*
 * set the vector value using cartesian coordinates
 */
void vect_cset( Vector2d* v, const double x, const double y )
{
   v->x = x;
   v->y = y;
   v->mod = MOD(x,y);
   v->angle = ANGLE(x,y);
}
/*
 * creates a minimal vector only valid for blitting and not other operations
 */
void vect_csetmin( Vector2d* v, const double x, const double y )
{
   v->x = x;
   v->y = y;
}
/*
 * set the vector value using polar coordinates
 */
void vect_pset( Vector2d* v, const double mod, const double angle )
{
   v->mod = mod;
   v->angle = angle;
   v->x = v->mod*cos(v->angle);
   v->y = v->mod*sin(v->angle);
}
/*
 * copies vector src to dest
 */
void vectcpy( Vector2d* dest, const Vector2d* src )
{
   dest->x = src->x;
   dest->y = src->y;
   dest->mod = src->mod;
   dest->angle = src->angle;
}
/*
 * makes a vector NULL
 */
void vectnull( Vector2d* v )
{
   v->x = v->y = v->mod = v->angle = 0.;
}

/*
 * get the direction pointed to by two vectors (from ref to v)
 */
double vect_angle( const Vector2d* ref, const Vector2d* v )
{
   return ANGLE( v->x - ref->x, v->y - ref->y);
}


/*
 * adds x and y to the current vector
 */
void vect_cadd( Vector2d* v, const double x, const double y )
{
   v->x += x;
   v->y += y;
   v->mod = MOD(v->x,v->y);
   v->angle = ANGLE(v->x,v->y);
}


/*
 * Mirrors a vector off another, stores results in vector.
 */
void vect_reflect( Vector2d* r, Vector2d* v, Vector2d* n )
{
   double dot;

   dot = (v->x*n->x) + (v->y*n->y);
   r->x = v->x - ((2. * dot) * n->x);
   r->y = v->y - ((2. * dot) * n->y);
   r->mod = MOD(r->x,r->y);
   r->angle = MOD(r->x,r->y);
}


/*
 * S O L I D
 */
/*
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
#if 0
static void simple_update (Solid *obj, const double dt)
{
   /* make sure angle doesn't flip */
   obj->dir += M_PI/360.*obj->dir_vel*dt;
   if (obj->dir > 2*M_PI) obj->dir -= 2*M_PI;
   if (obj->dir < 0.) obj->dir += 2*M_PI;

   double px, py, vx, vy;
   px = obj->pos->x;
   py = obj->pos->y;
   vx = obj->vel->x;
   vy = obj->vel->y;

   if (obj->force.mod) { /* force applied on object */
      double ax, ay;
      ax = obj->force->x/obj->mass;
      ay = obj->force->y/obj->mass;

      vx += ax*dt;
      vy += ay*dt;

      px += vx*dt + 0.5*ax * dt*dt;
      py += vy*dt + 0.5*ay * dt*dt;

      obj->vel.mod = MOD(vx,vy);
      obj->vel.angle = ANGLE(vx,vy);
   }
   else {
      px += vx*dt;
      py += vy*dt;
   }
   obj->pos.mod = MOD(px,py);
   obj->pos.angle = ANGLE(px,py);
}
#endif

/*
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
 */
#define RK4_MIN_H 0.01 /* minimal pass we want */
static void rk4_update (Solid *obj, const double dt)
{
   int i, N; /* for iteration, and pass calcualtion */
   double h, px,py, vx,vy; /* pass, and position/velocity values */
   double ix,iy, tx,ty, ax,ay; /* initial and temporary cartesian vector values */

   /* make sure angle doesn't flip */
   obj->dir += M_PI/180.*obj->dir_vel*dt;
   if (obj->dir >= 2.*M_PI) obj->dir -= 2*M_PI;
   else if (obj->dir < 0.) obj->dir += 2*M_PI;

   N = (dt>RK4_MIN_H) ? (int)(dt/RK4_MIN_H) : 1 ;
   h = dt / (double)N; /* step */

   px = obj->pos.x;
   py = obj->pos.y;
   vx = obj->vel.x;
   vy = obj->vel.y;


   if (obj->force.mod) { /* force applied on object */

      /* Movement Quantity Theorem:  m*a = \sum f */
      ax = obj->force.x / obj->mass;
      ay = obj->force.y / obj->mass;

      for (i=0; i < N; i++) { /* iterations */

         /* x component */
         tx = ix = vx;
         tx += 2.*ix + h*tx;
         tx += 2.*ix + h*tx;
         tx += ix + h*tx;
         tx *= h/6.;

         px += tx;
         vx += ax*h;

         /* y component */
         ty = iy = vy; 
         ty += 2.*(iy + h/2.*ty);
         ty += 2.*(iy + h/2.*ty);
         ty += iy + h*ty;
         ty *= h/6.;

         py += ty;
         vy += ay*h;
      }
      vect_cset( &obj->vel, vx, vy );
   }
   else { /* euler method -> p = v*t + 0.5*a*t^2 (no accel, so no error) */
      px += dt*vx;
      py += dt*vy;
   }
   vect_cset( &obj->pos, px, py );
}


/*
 * Initializes a new Solid
 */
void solid_init( Solid* dest, const double mass, const double dir,
      const Vector2d* pos, const Vector2d* vel )
{
   dest->mass = mass;

   dest->dir_vel = 0.;

   vect_cset( &dest->force, 0., 0.);
   dest->dir = dir;
   if ((dest->dir > 2.*M_PI) || (dest->dir < 0.))
      dest->dir = fmod(dest->dir,2*M_PI);

   if (vel == NULL) vectnull( &dest->vel );
   else vectcpy( &dest->vel, vel );

   if (pos == NULL) vectnull( &dest->pos );
   else vectcpy( &dest->pos, pos);

   dest->update = rk4_update;
}

/*
 * Creates a new Solid
 */
Solid* solid_create( const double mass, const double dir,
      const Vector2d* pos, const Vector2d* vel )
{
   Solid* dyn = MALLOC_ONE(Solid);
   if (dyn==NULL) ERR("Out of Memory");
   solid_init( dyn, mass, dir, pos, vel );
   return dyn;
}

/*
 * Frees an existing solid
 */
void solid_free( Solid* src )
{
   free(src);
   src = NULL;
}

