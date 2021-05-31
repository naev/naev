/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot_ew.c
 *
 * @brief Pilot electronic warfare information.
 */



/** @cond */
#include <math.h>

#include "naev.h"
/** @endcond */

#include "log.h"
#include "pilot.h"
#include "player.h"
#include "space.h"


#define EW_JUMPDETECT_DIST    7500.
#define EW_PLANETDETECT_DIST  pilot_ewMass(10e3) /* TODO something better than this. */


static double ew_interference = 1.; /**< Interference factor. */


/*
 * Prototypes.
 */
static double pilot_ewMovement( double vmod );
static double pilot_ewMass( double mass );
static double pilot_ewAsteroid( Pilot *p );


static void pilot_ewUpdate( Pilot *p )
{
   p->ew_detection = p->ew_mass * p->ew_asteroid / p->stats.ew_hide;
   p->ew_evasion   = p->ew_detection * 0.75 * ew_interference / p->stats.ew_evade;
   p->ew_stealth   = p->ew_detection * 0.25 * p->ew_movement / p->stats.ew_stealth;
}


/**
 * @brief Updates the pilot's static electronic warfare properties.
 *
 *    @param p Pilot to update.
 */
void pilot_ewUpdateStatic( Pilot *p )
{
   p->ew_mass     = pilot_ewMass( p->solid->mass );

   pilot_ewUpdate( p );
}


/**
 * @brief Updates the pilot's dynamic electronic warfare properties.
 *
 *    @param p Pilot to update.
 */
void pilot_ewUpdateDynamic( Pilot *p )
{
   p->ew_movement = pilot_ewMovement( VMOD(p->solid->vel) );
   p->ew_asteroid = pilot_ewAsteroid( p );

   pilot_ewUpdate( p );
}


/**
 * @brief Gets the electronic warfare movement modifier for a given velocity.
 *
 *    @param vmod Velocity to get electronic warfare movement modifier of.
 *    @return The electronic warfare movement modifier.
 */
static double pilot_ewMovement( double vmod )
{
   return 1. + vmod / 100.;
}


/**
 * @brief Gets the electronic warfare mass modifier for a given mass.
 *
 *    @param mass Mass to get the electronic warfare mass modifier of.
 *    @return The electronic warfare mass modifier.
 */
double pilot_ewMass( double mass )
{
   return pow(mass, 1./1.5) * 150.;
}


/**
 * @brief Gets the electronic warfare asteroid modifier.
 *
 *    @param p Pilot.
 *    @return The electronic warfare asteroid modifier.
 */
double pilot_ewAsteroid( Pilot *p )
{
   int i;

   i = space_isInField(&p->solid->pos);
   if (i>=0)
      return 1. / (1. + 0.4*cur_system->asteroids[i].density);
   else
      return 1.;
}


/**
 * @brief Updates the system's base sensor range.
 */
void pilot_updateSensorRange (void)
{
   ew_interference = 800. / (cur_system->interference + 800.);
}


/**
 * @brief Returns the default sensor range for the current system.
 *
 *    @return Sensor range.
 */
double pilot_sensorRange( void )
{
   return pilot_ewMovement( 100. );
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

   sense = pilot_sensorRange() * p->stats.ew_detect;
   if (d < pow2(sense))
      return 1;

   return 0;
}


/**
 * @brief Check to see if a pilot is in sensor range of another.
 *
 *    @param p Pilot who is trying to check to see if other is in sensor range.
 *    @param target Target of p to check to see if is in sensor range.
 *    @param[out] dist2 Distance squared of the two pilots. Set to NULL if you're not interested.
 *    @return 1 if they are in range, 0 if they aren't and -1 if they are detected fuzzily.
 */
int pilot_inRangePilot( const Pilot *p, const Pilot *target, double *dist2 )
{
   double d;

   /* Get distance if needed. */
   if (dist2 != NULL) {
      d = vect_dist2( &p->solid->pos, &target->solid->pos );
      *dist2 = d;
   }

   /* Special case player or omni-visible. */
   if ((pilot_isPlayer(p) && pilot_isFlag(target, PILOT_VISPLAYER)) ||
         pilot_isFlag(target, PILOT_VISIBLE) ||
         target->parent == p->id)
      return 1;

   /* Get distance if still needed */
   if (dist2 == NULL)
      d = vect_dist2( &p->solid->pos, &target->solid->pos );

   if (d < pow2(p->stats.ew_detect * p->stats.ew_track * target->ew_evasion))
      return 1;
   else if  (d < pow2(p->stats.ew_detect * target->ew_detection))
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

   sense = EW_PLANETDETECT_DIST;

   /* Get distance. */
   d = vect_dist2( &p->solid->pos, &pnt->pos );

   if (d / p->stats.ew_detect < sense )
      return 1;

   return 0;
}


/**
 * @brief Check to see if an asteroid is in sensor range of the pilot.
 *
 *    @param p Pilot who is trying to check to see if the asteroid is in sensor range.
 *    @param ast Asteroid to see if is in sensor range.
 *    @param fie Field the Asteroid belongs to to see if is in sensor range.
 *    @return 1 if they are in range, 0 if they aren't.
 */
int pilot_inRangeAsteroid( const Pilot *p, int ast, int fie )
{
   double d;
   Asteroid *as;
   AsteroidAnchor *f;
   double sense;

   /* pilot must exist */
   if ( p == NULL )
      return 0;

   /* Get the asteroid. */
   f = &cur_system->asteroids[fie];
   as = &f->asteroids[ast];

   /* TODO something better than this. */
   sense = pilot_ewMass( 500. );

   /* Get distance. */
   d = vect_dist2( &p->solid->pos, &as->pos );

   if (d / p->stats.ew_detect < sense ) /* By default, asteroid's hide score is 1. It could be made changeable via xml.*/
      return 1;

   return 0;
}


/**
 * @brief Check to see if a jump point is in sensor range of the pilot.
 *
 *    @param p Pilot who is trying to check to see if the jump point is in sensor range.
 *    @param i target Jump point to see if is in sensor range.
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
      sense = EW_JUMPDETECT_DIST * p->ew_jump_detect;

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
   /*
   double limit, lead;

   limit = track;
   if (p != NULL)
      limit *= p->ew_detect;

   if (t->ew_evasion * t->ew_movement < limit)
      lead = 1.;
   else
      lead = MAX( 0., 1. - 0.5*((t->ew_evasion * t->ew_movement)/limit - 1.));
   return lead;
   */
   return 1.;
}
