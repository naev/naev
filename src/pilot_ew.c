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

#include "array.h"
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
static int pilot_ewStealthGetNearby( const Pilot *p );


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
   return 7500 / ew_interference;
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

   /* Stealth detection. */
   if (pilot_isFlag( target, PILOT_STEALTH )) {
      if (d < pow2(p->stats.ew_detect * target->ew_stealth))
         return 1;
   }
   /* No stealth so normal detection. */
   else {
      if (d < pow2(p->stats.ew_detect * p->stats.ew_track * target->ew_evasion))
         return 1;
      else if  (d < pow2(p->stats.ew_detect * target->ew_detection))
         return -1;
   }

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
 *    @param trackmin Minimum track limit of the weapon.
 *    @param trackmax Maximum track limit of the weapon.
 *    @return The lead angle of the weapon.
 */
double pilot_ewWeaponTrack( const Pilot *p, const Pilot *t, double trackmin, double trackmax )
{
   double mod = p->stats.ew_track * p->stats.ew_detect;
   return CLAMP( 0., 1., (t->ew_evasion * mod - trackmin) / (trackmax - trackmin) );
}


static int pilot_ewStealthGetNearby( const Pilot *p )
{
   Pilot *t;
   Pilot *const* ps;
   int i, n;

   /* Check nearby non-allies. */
   n = 0;
   ps = pilot_getAll();
   for (i=0; i<array_size(ps); i++) {
      t = ps[i];
      if (areAllies( p->faction, t->faction ) ||
            ((p->id == PLAYER_ID) && pilot_isFriendly(t)))
         continue;
      if (pilot_isDisabled(t))
         continue;
      if (!pilot_validTarget(t,p)) /* does inrange check */
         continue;

      /* We found a pilot that is in range. */
      n++;
   }

   return n;
}


/**
 * @brief Updates the stealth mode and checks to see if it is getting broken.
 */
void pilot_ewUpdateStealth( Pilot *p, double dt )
{
   int n;

   if (!pilot_isFlag( p, PILOT_STEALTH ))
      return;

   /* Get nearby pilots. */
   n = pilot_ewStealthGetNearby( p );

   /* Increases if nobody nearby. */
   if (n == 0) {
      p->ew_stealth_timer += dt * 5000. / p->ew_stealth;
      if (p->ew_stealth_timer > 1.)
         p->ew_stealth_timer = 1.;
   }
   /* Otherwise decreases. */
   else {
      p->ew_stealth_timer -= dt * p->ew_stealth / 5000. * (double)n;
      if (p->ew_stealth_timer < 0.) {
         pilot_destealth( p );
         if (pilot_isPlayer(p))
            player_message(_("You have been discovered!"));
      }
   }
}


/**
 * @brief Stealths a pilot.
 */
int pilot_stealth( Pilot *p )
{
   int n;

   if (pilot_isFlag( p, PILOT_STEALTH ))
      return 0;

   /* Can't stealth if pilots nearby. */
   pilot_setFlag( p, PILOT_STEALTH );
   n = pilot_ewStealthGetNearby( p );
   if (n>0) {
      pilot_rmFlag( p, PILOT_STEALTH );
      return 0;
   }

   /* Got into stealth. */
   p->ew_stealth_timer = 0.;
   return 1;
}


/**
 * @brief Destealths a pilot.
 */
void pilot_destealth( Pilot *p )
{
   if (!pilot_isFlag( p, PILOT_STEALTH ))
      return;
   pilot_rmFlag( p, PILOT_STEALTH );
   p->ew_stealth_timer = 0.;
}
