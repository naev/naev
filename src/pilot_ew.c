/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot_ew.c
 *
 * @brief Pilot electronic warfare information.
 */


#include "pilot.h"

#include "naev.h"

#include <math.h>

#include "log.h"
#include "space.h"
#include "player.h"

static double sensor_curRange    = 0.; /**< Current base sensor range, used to calculate
                                         what is in range and what isn't. */

#define EVASION_SCALE        1.3225 /**< 1.15 squared. Ensures that ships have higher evasion than hide. */
#define SENSOR_DEFAULT_RANGE 7500   /**< The default sensor range for all ships. */

/**
 * @brief Updates the pilot's static electronic warfare properties.
 *
 *    @param p Pilot to update.
 */
void pilot_ewUpdateStatic( Pilot *p )
{
   /*
    * Unlike the other values, heat isn't squared. The ew_hide formula is thus
    * equivalent to: ew_base_hide * ew_mass * sqrt(ew_heat)
    */
   p->ew_mass     = pow2( pilot_ewMass( p->solid->mass ) );
   p->ew_heat     = pilot_ewHeat( p->heat_T );
   p->ew_asteroid = pilot_ewAsteroid( p );
   p->ew_hide     = p->ew_base_hide * p->ew_mass * p->ew_heat * p->ew_asteroid;
   p->ew_evasion  = p->ew_hide * EVASION_SCALE;
}


/**
 * @brief Updates the pilot's dynamic electronic warfare properties.
 *
 *    @param p Pilot to update.
 */
void pilot_ewUpdateDynamic( Pilot *p )
{
   /* Update hide. */
   p->ew_heat     = pilot_ewHeat( p->heat_T );
   p->ew_asteroid = pilot_ewAsteroid( p );
   p->ew_hide     = p->ew_base_hide * p->ew_mass * p->ew_heat * p->ew_asteroid;

   /* Update evasion. */
   p->ew_movement = pilot_ewMovement( VMOD(p->solid->vel) );
   p->ew_evasion  = p->ew_hide * EVASION_SCALE;
}


/**
 * @brief Gets the electronic warfare movement modifier for a given velocity.
 *
 *    @param vmod Velocity to get electronic warfare movement modifier of.
 *    @return The electronic warfare movement modifier.
 */
double pilot_ewMovement( double vmod )
{
   return 1. + vmod / 100.;
}


/**
 * @brief Gets the electronic warfare heat modifier for a given temperature.
 *
 *    @param T Temperature of the ship.
 *    @return The electronic warfare heat modifier.
 */
double pilot_ewHeat( double T )
{
   return 1. - 0.001 * (T - CONST_SPACE_STAR_TEMP);
}


/**
 * @brief Gets the electronic warfare mass modifier for a given mass.
 *
 *    @param mass Mass to get the electronic warfare mass modifier of.
 *    @return The electronic warfare mass modifier.
 */
double pilot_ewMass( double mass )
{
   return 1. / (.3 + sqrt(mass) / 30. );
}


/**
 * @brief Gets the electronic warfare asteroid modifier.
 *
 *    @param pilot.
 *    @return The electronic warfare asteroid modifier.
 */
double pilot_ewAsteroid( Pilot *p )
{
   int i;

   i = space_isInField(&p->solid->pos);
   if ( i>=0 )
      return 1. + cur_system->asteroids[i].density;
   else
      return 1.;
}


/**
 * @brief Updates the system's base sensor range.
 */
void pilot_updateSensorRange (void)
{
   /* Adjust sensor range based on system interference. */
   /* See: http://www.wolframalpha.com/input/?i=y+%3D+7500+%2F+%28%28x+%2B+200%29+%2F+200%29+from+x%3D0+to+1000 */
   sensor_curRange = SENSOR_DEFAULT_RANGE / ((cur_system->interference + 200) / 200.);

   /* Speeds up calculations as we compare it against vectors later on
    * and we want to avoid actually calculating the sqrt(). */
   sensor_curRange = pow2(sensor_curRange);
}


/**
 * @brief Returns the default sensor range for the current system.
 *
 *    @return Sensor range.
 */
double pilot_sensorRange( void )
{
   return sensor_curRange;
}


/**
 * @brief Check to see if a position is in range of the pilot.
 *
 *    @param p Pilot to check to see if position is in their sensor range.
 *    @param x X position to check.
 *    @param y Y position to check.
 *    @return 1 if the position is in range, 0 if it isn't.
 */
int pilot_inRange( const Pilot *p, double x, double y )
{
   double d, sense;

   /* Get distance. */
   d = pow2(x-p->solid->pos.x) + pow2(y-p->solid->pos.y);

   sense = sensor_curRange * p->ew_detect;
   if (d < sense)
      return 1;

   return 0;
}


/**
 * @brief Check to see if a pilot is in sensor range of another.
 *
 *    @param p Pilot who is trying to check to see if other is in sensor range.
 *    @param target Target of p to check to see if is in sensor range.
 *    @return 1 if they are in range, 0 if they aren't and -1 if they are detected fuzzily.
 */
int pilot_inRangePilot( const Pilot *p, const Pilot *target )
{
   double d, sense;

   /* Special case player or omni-visible. */
   if ((pilot_isPlayer(p) && pilot_isFlag(target, PILOT_VISPLAYER)) ||
         pilot_isFlag(target, PILOT_VISIBLE) ||
         target->parent == p->id)
      return 1;

   /* Get distance. */
   d = vect_dist2( &p->solid->pos, &target->solid->pos );

   sense = sensor_curRange * p->ew_detect;
   if (d * target->ew_evasion < sense)
      return 1;
   else if  (d * target->ew_hide < sense)
      return -1;

   return 0;
}


/**
 * @brief Check to see if a planet is in sensor range of the pilot.
 *
 *    @param p Pilot who is trying to check to see if the planet is in sensor range.
 *    @param target Planet to see if is in sensor range.
 *    @return 1 if they are in range, 0 if they aren't.
 */
int pilot_inRangePlanet( const Pilot *p, int target )
{
   double d;
   Planet *pnt;
   double sense;

   /* pilot must exist */
   if ( p == NULL )
      return 0;

   /* Get the planet. */
   pnt = cur_system->planets[target];

   /* target must not be virtual */
   if ( !pnt->real )
      return 0;

   sense = sensor_curRange * p->ew_detect;

   /* Get distance. */
   d = vect_dist2( &p->solid->pos, &pnt->pos );

   if (d * pnt->hide < sense )
      return 1;

   return 0;
}

/**
 * @brief Check to see if a jump point is in sensor range of the pilot.
 *
 *    @param p Pilot who is trying to check to see if the jump point is in sensor range.
 *    @param target Jump point to see if is in sensor range.
 *    @return 1 if they are in range, 0 if they aren't.
 */
int pilot_inRangeJump( const Pilot *p, int i )
{
   double d;
   JumpPoint *jp;
   double sense;
   double hide;

   /* pilot must exist */
   if ( p == NULL )
      return 0;

   /* Get the jump point. */
   jp = &cur_system->jumps[i];

   /* We don't want exit-only jumps. */
   if (jp_isFlag(jp, JP_EXITONLY))
      return 0;

   /* Handle hidden jumps separately, as they use a special range parameter. */
   if (jp_isFlag(jp, JP_HIDDEN))
      sense = pow(p->stats.misc_hidden_jump_detect, 2);
   else
      sense = sensor_curRange * p->ew_jump_detect;

   hide = jp->hide;

   /* Get distance. */
   d = vect_dist2( &p->solid->pos, &jp->pos );

   if (d * hide < sense)
      return 1;

   return 0;
}

/**
 * @brief Calculates the weapon lead (1. is 100%, 0. is 0%)..
 *
 *    @param p Pilot tracking.
 *    @param t Pilot being tracked.
 *    @param track Track limit of the weapon.
 *    @return The lead angle of the weapon.
 */
double pilot_ewWeaponTrack( const Pilot *p, const Pilot *t, double track )
{
   double limit, lead;

   limit = track * p->ew_detect;
   if (t->ew_evasion * t->ew_movement < limit)
      lead = 1.;
   else
      lead = MAX( 0., 1. - 0.5*((t->ew_evasion  * t->ew_movement)/limit - 1.));
   return lead;
}



