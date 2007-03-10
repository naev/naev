

#include "physics.h"

#include <math.h>
#include <stdlib.h>
#include <assert.h>


#ifndef M_PI
#define M_PI	3.14159265358979323846f
#endif /* M_PI */

#define SIN(dir)	(sinf(dir))
#define COS(dir)	(cosf(dir))


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
static void simple_update (Solid *obj, const FP dt)
{
	if (obj->force) { /* force applied on object */
		Vector2d acc;
		acc.x = obj->force*COS(obj->dir);
		acc.y = obj->force*SIN(obj->dir);

		obj->vel.x += acc.x*dt;
		obj->vel.y += acc.y*dt;

		obj->pos.x += obj->vel.x*dt + 0.5*acc.x / obj->mass*dt*dt;
		obj->pos.y += obj->vel.y*dt + 0.5*acc.y / obj->mass*dt*dt;
	}
	else {
		obj->pos.x += obj->vel.x*dt;
		obj->pos.y += obj->vel.y*dt;
	}
}

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
#define RK4_N	4
static void rk4_update (Solid *obj, const FP dt)
{
	FP h = dt / RK4_N; /* step */

	if (obj->force) { /* force applied on object */
		int i;
		Vector2d initial, temp;

		Vector2d acc;
		acc.x = obj->force*COS(obj->dir);
		acc.y = obj->force*SIN(obj->dir);

		for (i=0; i < RK4_N; i++) { /* iterations */

			/* x component */
			temp.x = initial.x = obj->vel.x;
			temp.x += 2*initial.x + h*temp.x;
			temp.x += 2*initial.x + h*temp.x;
			temp.x += initial.x + h*temp.x;
			temp.x *= h/6;

			obj->pos.x += temp.x;
			obj->vel.x += acc.x*h;

			/* y component */
			temp.y = initial.y = obj->vel.y; 
			temp.y += 2*(initial.y + h/2*temp.y);
			temp.y += 2*(initial.y + h/2*temp.y);
			temp.y += initial.y + h*temp.y;
			temp.y *= h/6;

			obj->pos.y += temp.y;
			obj->vel.y += acc.y*h;
		}
	}
	else {
		obj->pos.x += dt*obj->vel.x;
		obj->pos.y += dt*obj->vel.y;
	}
}


/*
 * Initializes a new Solid
 */
void solid_init( Solid* dest, const FP mass, const Vector2d* vel, const Vector2d* pos )
{
	dest->mass = mass;

	dest->force = 0;
	dest->dir = 0;

	dest->vel.x = vel->x;
	dest->vel.y = vel->y;

	dest->pos.x = pos->x;
	dest->pos.y = pos->y;

	dest->update = rk4_update;
}

/*
 * Creates a new Solid
 */
Solid* solid_create( const FP mass, const Vector2d* vel, const Vector2d* pos )
{
	Solid* dyn = MALLOC_ONE(Solid);
	assert(dyn != 0);
	solid_init( dyn, mass, vel, pos );
	return dyn;
}
