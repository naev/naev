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
#include "hook.h"
#include "log.h"
#include "pilot.h"
#include "player.h"
#include "player_autonav.h"
#include "space.h"

static double ew_interference = 1.; /**< Interference factor. */

/*
 * Prototypes.
 */
static void pilot_ewUpdate( Pilot *p );
static double pilot_ewMass( double mass );
static double pilot_ewAsteroid( const Pilot *p );
static double pilot_ewJumpPoint( const Pilot *p );
static int pilot_ewStealthGetNearby( const Pilot *p, double *mod, int *close, int *isplayer );

/**
 * @brief Gets the time it takes to scan a pilot.
 *
 *    @param p Pilot to scan.
 *    @return Time it takes to scan the pilot.
 */
double pilot_ewScanTime( const Pilot *p )
{
   /* Here larger is "better", so we divide by ew_hide and ew_signature instead of multiplying. */
   return pow( p->solid.mass, 1./3. ) * 1.25 / (p->stats.ew_hide * p->stats.ew_signature) * p->stats.ew_scanned_time;
}

/**
 * @brief Initializes the scan timer for a pilot.
 *
 *    @param p Pilot to set scan timer for.
 */
void pilot_ewScanStart( Pilot *p )
{
   const Pilot *target = pilot_getTarget( p );
   if (target==NULL)
      return;

   /* Player did bad stuff and is getting scanned. */
   if (pilot_isPlayer(target) && pilot_hasIllegal(target,p->faction))
      player_autonavResetSpeed();

   /* Scan time. */
   p->scantimer = pilot_ewScanTime(p);
}

/**
 * @brief Checks to see if a scan is done.
 *
 *    @param p Pilot to check.
 *    @return 1 if scan is done, 0 otherwise.
 */
int pilot_ewScanCheck( const Pilot *p )
{
   if (p->target == p->id)
      return 0;
   return (p->scantimer < 0.);
}

/**
 * @brief Updates all the internal values.
 */
static void pilot_ewUpdate( Pilot *p )
{
   p->ew_detection = p->ew_mass * p->ew_asteroid * p->stats.ew_hide;
   p->ew_signature   = p->ew_detection * 0.75 * ew_interference * p->stats.ew_signature;
   /* For stealth we apply the ew_asteroid and ew_interference bonus outside of the max, so that it can go below 1000 with in-system features. */
   p->ew_stealth   = MAX( 1000., p->ew_mass * p->stats.ew_hide * 0.25 * p->stats.ew_stealth ) * p->ew_asteroid * ew_interference * p->ew_jumppoint;
}

/**
 * @brief Updates the pilot's static electronic warfare properties.
 *
 *    @param p Pilot to update.
 */
void pilot_ewUpdateStatic( Pilot *p )
{
   p->ew_mass = pilot_ewMass( p->solid.mass );
   pilot_ewUpdate( p );
}

/**
 * @brief Updates the pilot's dynamic electronic warfare properties.
 *
 *    @param p Pilot to update.
 *    @param dt Delta time increment (seconds).
 */
void pilot_ewUpdateDynamic( Pilot *p, double dt )
{
   Pilot *t;

   /* Electronic warfare values. */
   p->ew_asteroid = pilot_ewAsteroid( p );
   p->ew_jumppoint = pilot_ewJumpPoint( p );
   pilot_ewUpdate( p );

   /* Already scanned so skipping. */
   if (p->scantimer < 0.)
      return;

   /* Get the target pilot. */
   t = pilot_getTarget( p );
   if (t == NULL)
      return;

   /* Must be in evasion range. */
   if (vec2_dist2( &p->solid.pos, &t->solid.pos ) < pow2( MAX( 0., p->stats.ew_detect * p->stats.ew_track * t->ew_signature ) )) {
      p->scantimer -= dt;

      if (p->scantimer < 0.) {
         HookParam hparam = { .type = HOOK_PARAM_PILOT };
         /* Run scan hook. */
         hparam.u.lp = t->id;
         pilot_runHookParam( p, PILOT_HOOK_SCAN, &hparam, 1 );
         /* Run scanned hook. */
         hparam.u.lp = p->id;
         pilot_runHookParam( t, PILOT_HOOK_SCANNED, &hparam, 1 );

         /* Run outfit stuff. */
         pilot_outfitLOnscan( p );
         pilot_outfitLOnscanned( t, p );

         /* TODO handle case the player finished scanning by setting a flag or something. */
      }
   }
}

/**
 * @brief Gets the electronic warfare mass modifier for a given mass.
 *
 *    @param mass Mass to get the electronic warfare mass modifier of.
 *    @return The electronic warfare mass modifier.
 */
static double pilot_ewMass( double mass )
{
   return pow(mass, 1./1.8) * 350.;
}

/**
 * @brief Gets the electronic warfare asteroid modifier.
 *
 *    @param p Pilot.
 *    @return The electronic warfare asteroid modifier.
 */
static double pilot_ewAsteroid( const Pilot *p )
{
   int infield = asteroids_inField(&p->solid.pos);
   if (infield < 0)
      return 1.;
   return 1. / (1. + 0.4*cur_system->asteroids[infield].density);
}

/**
 * @brief Gets the electronic warfare jump point modifier.
 *
 *    @param p Pilot.
 *    @return The electronic warfare jump point modifier.
 */
static double pilot_ewJumpPoint( const Pilot *p )
{
   /* Don't have to really check when not in stealth. */
   if (!pilot_isFlag(p,PILOT_STEALTH))
      return 1.;

   /* Gets lower when near jump. */
   for (int i=0; i<array_size(cur_system->jumps); i++) {
      JumpPoint *jp = &cur_system->jumps[i];
      if (jp_isFlag(jp,JP_EXITONLY) || jp_isFlag(jp,JP_HIDDEN))
         continue;
      double d2 = vec2_dist2( &jp->pos, &p->solid.pos );
      if (d2 <= pow2(EW_JUMP_BONUS_RANGE))
         return MAX( 0.5, sqrt(d2) / EW_JUMP_BONUS_RANGE);
   }
   return 1.;
}

/**
 * @brief Updates the system's base sensor range.
 */
void pilot_updateSensorRange (void)
{
   ew_interference = 1. / (1. + cur_system->interference/100.);
}

/**
 * @brief Returns the default sensor range for the current system.
 *
 *    @return Sensor range.
 */
double pilot_sensorRange( void )
{
   return 7500.;
}

/**
 * @brief Check to see if a position is in range of the pilot.
 *
 *    @param p Pilot to check to see if position is in their sensor range.
 *    @param x X position to check.
 *    @param y Y position to check.
 *    @return 1 if the position is in range, -1 if it is fuzzy, 0 if it isn't.
 */
int pilot_inRange( const Pilot *p, double x, double y )
{
   double d = pow2(x-p->solid.pos.x) + pow2(y-p->solid.pos.y);
   double sr = pilot_sensorRange();
   double sense = MAX( 0., sr * p->stats.ew_detect );
   if (d < pow2(sense))
      return 1;
   /* points can't evade.
   sense = MAX( 0., sr * p->stats.ew_signature );
   if (d > pow2(sense))
      return -1;
   */
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
   if (dist2 != NULL)
      *dist2 = vec2_dist2( &p->solid.pos, &target->solid.pos );

   /* Special case player or omni-visible. */
   if ((pilot_isPlayer(p) && pilot_isFlag(target, PILOT_VISPLAYER)) ||
         pilot_isFlag(target, PILOT_VISIBLE) ||
         target->parent == p->id)
      return 1;

   /* Stealth detection. */
   if (pilot_isFlag( target, PILOT_STEALTH ))
      return 0;

   /* No stealth so normal detection. */
   d = (dist2!=NULL ? *dist2 : vec2_dist2( &p->solid.pos, &target->solid.pos ) );
   if (d < pow2( MAX( 0., p->stats.ew_detect * p->stats.ew_track * target->ew_signature )))
      return 1;
   else if  (d < pow2( MAX( 0., p->stats.ew_detect * target->ew_detection )))
      return -1;

   return 0;
}

/**
 * @brief Check to see if a spob is in sensor range of the pilot.
 *
 *    @param p Pilot who is trying to check to see if the spob is in sensor range.
 *    @param target Spob to see if is in sensor range.
 *    @return 1 if they are in range, 0 if they aren't.
 */
int pilot_inRangeSpob( const Pilot *p, int target )
{
   double d;
   Spob *pnt;
   double sense;

   /* pilot must exist */
   if (p == NULL)
      return 0;

   /* Get the spob. */
   pnt = cur_system->spobs[target];
   sense = EW_SPOBDETECT_DIST;

   /* Get distance. */
   d = vec2_dist2( &p->solid.pos, &pnt->pos );
   if (d < pow2( MAX( 0., sense * p->stats.ew_detect * pnt->hide) ) )
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
   if (p == NULL)
      return 0;

   /* Get the asteroid. */
   f = &cur_system->asteroids[fie];
   as = &f->asteroids[ast];

   /* TODO something better than this. */
   sense = EW_ASTEROID_DIST;

   /* Get distance. */
   d = vec2_dist2( &p->solid.pos, &as->pos );

   /* By default, asteroid's hide score is 1. It could be made changeable via xml.*/
   if (d < pow2( MAX( 0., sense * p->stats.ew_detect ) ) )
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
   if (p == NULL)
      return 0;

   /* Get the jump point. */
   jp = &cur_system->jumps[i];

   /* We don't want exit-only jumps. */
   if (jp_isFlag(jp, JP_EXITONLY))
      return 0;

   /* Jumps with 0. hide are considered to be highway points and always visible. */
   hide = jp->hide;
   if (hide==0.)
      return 1;

   sense = EW_JUMPDETECT_DIST * p->stats.ew_jump_detect * p->stats.ew_detect;
   /* Handle hidden jumps separately, as they use a special range parameter. */
   if (jp_isFlag(jp, JP_HIDDEN))
      sense *= p->stats.misc_hidden_jump_detect;

   /* Get distance. */
   d = vec2_dist2( &p->solid.pos, &jp->pos );
   if (d < pow2( MAX( 0., sense * hide )))
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
   return CLAMP( 0., 1., (t->ew_signature * mod - trackmin) / (trackmax - trackmin + 1e-5) ); /* Avoid divide by zero if trackmax==trackmin. */
}

/**
 * @brief Checks to see if there are pilots nearby to a stealthed pilot that could break stealth.
 *
 *    @param p Pilot to check.
 *    @param mod Distance-dependent trength modifier.
 *    @param close Number of pilots nearby.
 *    @param isplayer Whether or not the player is the pilot breaking stealth.
 *    @return Number of stealth-breaking pilots nearby.
 */
static int pilot_ewStealthGetNearby( const Pilot *p, double *mod, int *close, int *isplayer )
{
   Pilot *const* ps;
   int n;

   /* Check nearby non-allies. */
   if (mod != NULL)
      *mod = 0.;
   if (close != NULL)
      *close = 0;
   if (isplayer != NULL)
      *isplayer = 0;
   n = 0;
   ps = pilot_getAll();
   for (int i=0; i<array_size(ps); i++) {
      double dist;
      Pilot *t = ps[i];

      /* Quick checks first. */
      if (pilot_isDisabled(t))
         continue;
      if (!pilot_canTarget(t))
         continue;

      /* Must not be landing nor taking off. */
      if (pilot_isFlag(t, PILOT_LANDING) ||
            pilot_isFlag(t, PILOT_TAKEOFF))
         continue;

      /* Allies are ignored. */
      if (areAllies( p->faction, t->faction ) ||
            ((p->faction == FACTION_PLAYER) && pilot_isFriendly(t)) ||
            ((t->faction == FACTION_PLAYER) && pilot_isFriendly(p)))
         continue;

      /* Stealthed pilots don't reduce stealth. */
      //if (pilot_isFlag(t, PILOT_STEALTH))
      //   continue;

      /* Compute distance. */
      dist = vec2_dist2( &p->solid.pos, &t->solid.pos );
      /* TODO maybe not hardcode the close value. */
      if ((close != NULL) && !pilot_isFlag(t,PILOT_STEALTH) &&
            (dist < pow2( MAX( 0., p->ew_stealth * t->stats.ew_detect * 1.5 ))))
         (*close)++;
      if (dist > pow2( MAX( 0., p->ew_stealth * t->stats.ew_detect )))
         continue;

      if (mod != NULL)
         *mod += 1.0 - sqrt(dist) / (p->ew_stealth * t->stats.ew_detect);

      /* We found a pilot that is in range. */
      n++;
      if ((isplayer != NULL) && pilot_isPlayer(t))
         *isplayer = 1;
   }

   return n;
}

/**
 * @brief Updates the stealth mode and checks to see if it is getting broken.
 *
 *    @param p Pilot to update.
 *    @param dt Current delta-tick.
 */
void pilot_ewUpdateStealth( Pilot *p, double dt )
{
   int n, close, isplayer;
   double mod;

   if (!pilot_isFlag( p, PILOT_STEALTH ))
      return;

   /* Get nearby pilots. */
   if (pilot_isPlayer(p)) {
      if (pilot_isFlag(p, PILOT_NONTARGETABLE))
         return;

      n = pilot_ewStealthGetNearby( p, &mod, &close, &isplayer );

      /* Stop autonav if pilots are nearby. */
      if (close>0)
         player_autonavResetSpeed();
   }
   else
      n = pilot_ewStealthGetNearby( p, &mod, NULL, &isplayer );

   /* Increases if nobody nearby. */
   if (n == 0) {
      p->ew_stealth_timer += dt * 5e3 / p->ew_stealth;
      if (p->ew_stealth_timer > 1.)
         p->ew_stealth_timer = 1.;
   }
   /* Otherwise decreases. */
   else {
      p->ew_stealth_timer -= dt * (p->ew_stealth / 10e3 + mod) * p->stats.ew_stealth_timer;
      if (p->ew_stealth_timer < 0.) {
         pilot_destealth( p );
         if (pilot_isPlayer(p))
            player_message( "#r%s", _("You have been uncovered!"));
         else if (isplayer)
            player_message(_("You have uncovered '#%c%s#0'!"), pilot_getFactionColourChar(p), p->name);
         ai_discovered( p );
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
      return 1;

   /* Can't stealth if locked on. */
   if (p->lockons > 0)
      return 0;

   /* Can't stealth if pilots nearby. */
   pilot_setFlag( p, PILOT_STEALTH );
   n = pilot_ewStealthGetNearby( p, NULL, NULL, NULL );
   if (n > 0) {
      pilot_rmFlag( p, PILOT_STEALTH );
      return 0;
   }

   /* Turn off outfits. */
   pilot_outfitOffAll( p );

   /* Got into stealth. */
   if (!pilot_outfitLOnstealth( p ))
      pilot_calcStats(p);
   p->ew_stealth_timer = 0.;

   /* Run hook. */
   const HookParam hparam = { .type = HOOK_PARAM_BOOL, .u.b = 1 };
   pilot_runHookParam( p, PILOT_HOOK_STEALTH, &hparam, 1 );
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
   if (!pilot_outfitLOnstealth( p ))
      pilot_calcStats(p);

   /* Run hook. */
   const HookParam hparam = { .type = HOOK_PARAM_BOOL, .u.b = 0 };
   pilot_runHookParam( p, PILOT_HOOK_STEALTH, &hparam, 1 );
}
