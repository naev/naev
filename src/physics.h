/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "vec2.h"

/*
 * Update options.
 */
#define SOLID_UPDATE_RK4 0   /**< Default Runge-Kutta 3-4 update. */
#define SOLID_UPDATE_EULER 1 /**< Simple Euler update. */

extern const char _UNIT_TIME[];
extern const char _UNIT_PER_TIME[];
extern const char _UNIT_DISTANCE[];
extern const char _UNIT_SPEED[];
extern const char _UNIT_ACCEL[];
extern const char _UNIT_ENERGY[];
extern const char _UNIT_POWER[];
extern const char _UNIT_ANGLE[];
extern const char _UNIT_ROTATION[];
extern const char _UNIT_MASS[];
extern const char _UNIT_CPU[];
extern const char _UNIT_UNIT[];
extern const char _UNIT_PERCENT[];
#define UNIT_TIME _( _UNIT_TIME )
#define UNIT_PER_TIME _( _UNIT_PER_TIME )
#define UNIT_DISTANCE _( _UNIT_DISTANCE )
#define UNIT_SPEED _( _UNIT_SPEED )
#define UNIT_ACCEL _( _UNIT_ACCEL )
#define UNIT_ENERGY _( _UNIT_ENERGY )
#define UNIT_POWER _( _UNIT_POWER )
#define UNIT_ANGLE _( _UNIT_ANGLE )
#define UNIT_ROTATION _( _UNIT_ROTATION )
#define UNIT_MASS _( _UNIT_MASS )
#define UNIT_CPU _( _UNIT_CPU )
#define UNIT_UNIT _( _UNIT_UNIT )
#define UNIT_PERCENT _( _UNIT_PERCENT )

/**
 * @brief Represents a solid in the game.
 */
typedef struct Solid_ {
   double mass;      /**< Solid's mass. */
   double dir;       /**< Direction solid is facing in rad. */
   double dir_vel;   /**< Velocity at which solid is rotating in rad/s. */
   vec2   vel;       /**< Velocity of the solid. */
   vec2   pos;       /**< Position of the solid. */
   vec2   pre;       /**< Previous position of the solid. For collisions. */
   double accel;     /**< Relative X acceleration, basically simplified for our
                        model. */
   double speed_max; /**< Maximum speed. */
   void ( *update )( struct Solid_ *, double ); /**< Update method. */
} Solid;

/*
 * solid manipulation
 */
double solid_maxspeed( const Solid *s, double speed, double accel );
void   solid_init( Solid *dest, double mass, double dir, const vec2 *pos,
                   const vec2 *vel, int update );

/*
 * misc
 */
double angle_diff( double ref, double a );
