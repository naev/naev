/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot.c
 *
 * @brief Handles the pilot stuff.
 */


/** @cond */
#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "naev.h"
/** @endcond */

#include "pilot.h"

#include "ai.h"
#include "array.h"
#include "board.h"
#include "camera.h"
#include "damagetype.h"
#include "debris.h"
#include "debug.h"
#include "escort.h"
#include "explosion.h"
#include "faction.h"
#include "font.h"
#include "gui.h"
#include "hook.h"
#include "land.h"
#include "land_outfits.h"
#include "land_shipyard.h"
#include "log.h"
#include "map.h"
#include "music.h"
#include "ndata.h"
#include "nstring.h"
#include "ntime.h"
#include "nxml.h"
#include "pause.h"
#include "player.h"
#include "player_autonav.h"
#include "rng.h"
#include "weapon.h"


#define PILOT_SIZE_MIN 128 /**< Minimum chunks to increment pilot_stack by */

/* ID Generators. */
static unsigned int pilot_id = PLAYER_ID; /**< Stack of pilot ids to assure uniqueness */


/* stack of pilots */
static Pilot** pilot_stack = NULL; /**< All the pilots in space. (Player may have other Pilot objects, e.g. backup ships.) */


/* misc */
static const double pilot_commTimeout  = 15.; /**< Time for text above pilot to time out. */
static const double pilot_commFade     = 5.; /**< Time for text above pilot to fade out. */



/*
 * Prototypes
 */
/* Create. */
static void pilot_init( Pilot* dest, Ship* ship, const char* name, int faction, const char *ai,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const PilotFlags flags, unsigned int dockpilot, int dockslot );
/* Update. */
static void pilot_hyperspace( Pilot* pilot, double dt );
static void pilot_refuel( Pilot *p, double dt );
/* Clean up. */
static void pilot_dead( Pilot* p, unsigned int killer );
/* Targeting. */
static int pilot_validEnemy( const Pilot* p, const Pilot* target );
/* Misc. */
static void pilot_setCommMsg( Pilot *p, const char *s );
static int pilot_getStackPos( const unsigned int id );
static void pilot_init_trails( Pilot* p );
static int pilot_trail_generated( Pilot* p, int generator );


/**
 * @brief Gets the pilot stack.
 */
Pilot*const* pilot_getAll (void)
{
   return pilot_stack;
}


/**
 * @brief Compare id (for use with bsearch)
 */
static int compid(const void *id, const void *p) {
    return *((const unsigned int*)id) - (*((Pilot**)p))->id;
}


/**
 * @brief Gets the pilot's position in the stack.
 *
 *    @param id ID of the pilot to get.
 *    @return Position of pilot in stack or -1 if not found.
 */
static int pilot_getStackPos( const unsigned int id )
{
   /* binary search */
   Pilot **pp = bsearch(&id, pilot_stack, array_size(pilot_stack), sizeof(Pilot*), compid);
   if (pp == NULL)
      return -1;
   else
      return pp - pilot_stack;
}


/**
 * @brief Gets the next pilot based on id.
 *
 *    @param id ID of current pilot.
 *    @param mode Method to use when cycling.  0 is normal, 1 is hostiles.
 *    @return ID of next pilot or PLAYER_ID if no next pilot.
 */
unsigned int pilot_getNextID( const unsigned int id, int mode )
{
   int m, p;

   /* Player must exist. */
   if (player.p == NULL)
      return PLAYER_ID;

   /* Get the pilot. */
   m = pilot_getStackPos(id);

   /* Unselect. */
   if ((m == (array_size(pilot_stack)-1)) || (m == -1))
      return PLAYER_ID;

   /* Get first one in range. */
   p = m+1;
   if (mode == 0) {
      while (p < array_size(pilot_stack)) {
         if ((!pilot_isWithPlayer(pilot_stack[p]) ||
                  pilot_isDisabled(pilot_stack[p])) &&
               pilot_validTarget( player.p, pilot_stack[p] ))
            return pilot_stack[p]->id;
         p++;
      }
   }
   /* Get first hostile in range. */
   if (mode == 1) {
      while (p < array_size(pilot_stack)) {
         if (!pilot_isWithPlayer(pilot_stack[p]) &&
               pilot_validTarget( player.p, pilot_stack[p] ) &&
               pilot_isHostile( pilot_stack[p] ))
            return pilot_stack[p]->id;
         p++;
      }
   }

   /* None found. */
   return PLAYER_ID;
}


/**
 * @brief Gets the previous pilot based on ID.
 *
 *    @param id ID of the current pilot.
 *    @param mode Method to use when cycling.  0 is normal, 1 is hostiles.
 *    @return ID of previous pilot or PLAYER_ID if no previous pilot.
 */
unsigned int pilot_getPrevID( const unsigned int id, int mode )
{
   int m, p;

   /* Player must exist. */
   if (player.p == NULL)
      return PLAYER_ID;

   /* Get the pilot. */
   m = pilot_getStackPos(id);

   /* Check to see what position to try. */
   if (m == -1)
      return PLAYER_ID;
   else if (m == 0)
      p = array_size(pilot_stack)-1;
   else
      p = m-1;

   /* Get first one in range. */
   if (mode == 0) {
      while (p >= 0) {
         if ((!pilot_isWithPlayer(pilot_stack[p]) ||
                  (pilot_isDisabled(pilot_stack[p]))) &&
               pilot_validTarget( player.p, pilot_stack[p] ))
            return pilot_stack[p]->id;
         p--;
      }
   }
   /* Get first hostile in range. */
   else if (mode == 1) {
      while (p >= 0) {
         if (!pilot_isWithPlayer(pilot_stack[p]) &&
               !pilot_isFlag( pilot_stack[p], PILOT_HIDE ) &&
               pilot_validTarget( player.p, pilot_stack[p] ) &&
               pilot_isHostile( pilot_stack[p] ) )
            return pilot_stack[p]->id;
         p--;
      }
   }

   /* None found. */
   return PLAYER_ID;
}


/**
 * @brief Checks to see if a pilot is a valid target for another pilot.
 *
 * @TODO this calls inRangePilot which can be called again right after. This should probably be optimized.
 *
 *    @param p Reference pilot.
 *    @param target Pilot to see if is a valid target of the reference.
 *    @return 1 if it is valid, 0 otherwise.
 */
int pilot_validTarget( const Pilot* p, const Pilot* target )
{
   /* Must not be dead. */
   if (pilot_isFlag( target, PILOT_DELETE ) ||
         pilot_isFlag( target, PILOT_DEAD ))
      return 0;

   /* Must not be hidden nor invisible. */
   if (pilot_isFlag( target, PILOT_HIDE ) ||
         pilot_isFlag( target, PILOT_INVISIBLE))
      return 0;

   /* Must be in range. */
   if (!pilot_inRangePilot( p, target, NULL ))
      return 0;

   /* Pilot is a valid target. */
   return 1;
}


/**
 * @brief Same as pilot_validTarget but without the range check.
 */
int pilot_canTarget( const Pilot* p )
{
   /* Must not be dead. */
   if (pilot_isFlag( p, PILOT_DELETE ) ||
         pilot_isFlag( p, PILOT_DEAD ))
      return 0;

   /* Must not be hidden nor invisible. */
   if (pilot_isFlag( p, PILOT_HIDE ) ||
         pilot_isFlag( p, PILOT_INVISIBLE))
      return 0;

   /* Pilot is a valid target. */
   return 1;
}


/**
 * @brief Checks to see if a pilot is a valid enemy for another pilot.
 *
 *    @param p Reference pilot.
 *    @param target Pilot to see if is a valid enemy of the reference.
 *    @return 1 if it is valid, 0 otherwise.
 */
static int pilot_validEnemy( const Pilot* p, const Pilot* target )
{
   /* Should either be hostile by faction or by player. */
   if ( !( areEnemies( p->faction, target->faction )
            || (pilot_isWithPlayer(target)
               && pilot_isHostile(p))))
      return 0;

   /* Shouldn't be bribed by player. */
   if (pilot_isWithPlayer(target) && pilot_isFlag(p, PILOT_BRIBED))
      return 0;

   /* Shouldn't be disabled. */
   if (pilot_isDisabled(target))
      return 0;

   /* Shouldn't be invincible. */
   if (pilot_isFlag( target, PILOT_INVINCIBLE ))
      return 0;

   /* Must be a valid target. */
   if (!pilot_validTarget( p, target ))
      return 0;

   /* Must not be fuzzy. */
   if (pilot_inRangePilot( p, target, NULL ) != 1)
      return 0;

   /* He's ok. */
   return 1;
}


/**
 * @brief Gets the nearest enemy to the pilot.
 *
 *    @param p Pilot to get the nearest enemy of.
 *    @return ID of their nearest enemy.
 */
unsigned int pilot_getNearestEnemy( const Pilot* p )
{
   unsigned int tp;
   int i;
   double d, td;

   tp = 0;
   d  = 0.;
   for (i=0; i<array_size(pilot_stack); i++) {

      if (!pilot_validEnemy( p, pilot_stack[i] ))
         continue;

      /* Check distance. */
      td = vect_dist2(&pilot_stack[i]->solid->pos, &p->solid->pos);
      if (!tp || (td < d)) {
         d  = td;
         tp = pilot_stack[i]->id;
      }
   }
   return tp;
}

/**
 * @brief Gets the nearest enemy to the pilot closest to the pilot whose mass is between LB and UB.
 *
 *    @param p Pilot to get the nearest enemy of.
 *    @param target_mass_LB the lower bound for target mass
 *    @param target_mass_UB the upper bound for target mass
 *    @return ID of their nearest enemy.
 */
unsigned int pilot_getNearestEnemy_size( const Pilot* p, double target_mass_LB, double target_mass_UB )
{
   unsigned int tp;
   int i;
   double d, td;

   tp = 0;
   d  = 0.;
   for (i=0; i<array_size(pilot_stack); i++) {

      if (!pilot_validEnemy( p, pilot_stack[i] ))
         continue;

      if (pilot_stack[i]->solid->mass < target_mass_LB || pilot_stack[i]->solid->mass > target_mass_UB)
         continue;

      /* Check distance. */
      td = vect_dist2(&pilot_stack[i]->solid->pos, &p->solid->pos);
      if (!tp || (td < d)) {
         d = td;
         tp = pilot_stack[i]->id;
      }
   }

   return tp;
}

/**
 * @brief Gets the nearest enemy to the pilot closest to the pilot whose mass is between LB and UB.
 *
 *    @param p Pilot to get the nearest enemy of.
 *    @param mass_factor parameter for target mass (0-1, 0.5 = current mass)
 *    @param health_factor parameter for target shields/armour (0-1, 0.5 = current health)
 *    @param damage_factor parameter for target dps (0-1, 0.5 = current dps)
 *    @param range_factor weighting for range (typically >> 1)
 *    @return ID of their nearest enemy.
 */
unsigned int pilot_getNearestEnemy_heuristic( const Pilot* p,
      double mass_factor, double health_factor,
      double damage_factor, double range_factor )
{
   unsigned int tp;
   int i;
   double temp, current_heuristic_value;
   Pilot *target;

   current_heuristic_value = 10000.;

   tp = 0;
   for (i=0; i<array_size(pilot_stack); i++) {
      target = pilot_stack[i];

      if (!pilot_validEnemy( p, target ))
         continue;

      /* Check distance. */
      temp = range_factor *
               vect_dist2( &target->solid->pos, &p->solid->pos )
            + FABS( pilot_relsize( p, target ) - mass_factor )
            + FABS( pilot_relhp(   p, target ) - health_factor )
            + FABS( pilot_reldps(  p, target ) - damage_factor );

      if ((tp == 0) || (temp < current_heuristic_value)) {
         current_heuristic_value = temp;
         tp = target->id;
      }
   }

   return tp;
}

/**
 * @brief Get the nearest pilot to a pilot.
 *
 *    @param p Pilot to get the nearest pilot of.
 *    @return The nearest pilot.
 */
unsigned int pilot_getNearestPilot( const Pilot* p )
{
   unsigned int t;
   pilot_getNearestPos( p, &t, p->solid->pos.x, p->solid->pos.y, 0 );
   return t;
}


/**
 * @brief Get the strongest ally in a given range.
 *
 *    @param p Pilot to get the boss of.
 *    @return The boss.
 */
unsigned int pilot_getBoss( const Pilot* p )
{
   unsigned int t;
   int i;
   double td, dx, dy;
   double relpower, ppower, curpower;
   /* TODO : all the parameters should be adjustable with arguments */

   relpower = 0;
   t = 0;

   /* Initialized to 0.25 which would mean equivalent power. */
   ppower = 0.5*0.5;

   for (i=0; i<array_size(pilot_stack); i++) {

      /* Must be in range. */
      if (!pilot_inRangePilot( p, pilot_stack[i], NULL ))
         continue;

      /* Must not be self. */
      if (pilot_stack[i] == p)
         continue;

      /* Shouldn't be disabled. */
      if (pilot_isDisabled(pilot_stack[i]))
         continue;

      /* Must be a valid target. */
      if (!pilot_validTarget( p, pilot_stack[i] ))
         continue;

      /* Maximum distance in 2 seconds. */
      dx = pilot_stack[i]->solid->pos.x + 2*pilot_stack[i]->solid->vel.x -
           p->solid->pos.x - 2*p->solid->vel.x;
      dy = pilot_stack[i]->solid->pos.y + 2*pilot_stack[i]->solid->vel.y -
           p->solid->pos.y - 2*p->solid->vel.y;
      td = sqrt( pow2(dx) + pow2(dy) );
      if (td > 5000)
         continue;

      /* Must have the same faction. */
      if (pilot_stack[i]->faction != p->faction)
         continue;

      /* Must be slower. */
      if (pilot_stack[i]->speed > p->speed)
         continue;

      /* Should not be weaker than the current pilot*/
      curpower = pilot_reldps(  pilot_stack[i], p ) * pilot_relhp(  pilot_stack[i], p );
      if (ppower >= curpower )
         continue;

      if (relpower < curpower ) {
         relpower = curpower;
         t = pilot_stack[i]->id;
      }
   }
   return t;
}

/**
 * @brief Get the nearest pilot to a pilot from a certain position.
 *
 *    @param p Pilot to get the nearest pilot of.
 *    @param[out] tp The nearest pilot.
 *    @param x X position to calculate from.
 *    @param y Y position to calculate from.
 *    @param disabled Whether to return disabled pilots.
 *    @return The distance to the nearest pilot.
 */
double pilot_getNearestPos( const Pilot *p, unsigned int *tp, double x, double y, int disabled )
{
   int i;
   double d, td;

   *tp = PLAYER_ID;
   d  = 0;
   for (i=0; i<array_size(pilot_stack); i++) {
      /* Must not be self. */
      if (pilot_stack[i] == p)
         continue;

      /* Player doesn't select escorts (unless disabled is active). */
      if (!disabled && pilot_isPlayer(p) &&
            pilot_isWithPlayer(pilot_stack[i]))
         continue;

      /* Shouldn't be disabled. */
      if (!disabled && pilot_isDisabled(pilot_stack[i]))
         continue;

      /* Must be a valid target. */
      if (!pilot_validTarget( p, pilot_stack[i] ))
         continue;

      /* Minimum distance. */
      td = pow2(x-pilot_stack[i]->solid->pos.x) + pow2(y-pilot_stack[i]->solid->pos.y);
      if (((*tp==PLAYER_ID) || (td < d))) {
         d = td;
         *tp = pilot_stack[i]->id;
      }
   }
   return d;
}


/**
 * @brief Get the pilot closest to an angle extending from another pilot.
 *
 *    @param p Pilot to get the nearest pilot of.
 *    @param[out] tp The nearest pilot.
 *    @param ang Angle to compare against.
 *    @param disabled Whether to return disabled pilots.
 *    @return Angle between the pilot and the nearest pilot.
 */
double pilot_getNearestAng( const Pilot *p, unsigned int *tp, double ang, int disabled )
{
   int i;
   double a, ta;
   double rx, ry;

   *tp = PLAYER_ID;
   a   = ang + M_PI;
   for (i=0; i<array_size(pilot_stack); i++) {

      /* Must not be self. */
      if (pilot_stack[i] == p)
         continue;

      /* Player doesn't select escorts (unless disabled is active). */
      if (!disabled && pilot_isPlayer(p) &&
            pilot_isWithPlayer(pilot_stack[i]))
         continue;

      /* Shouldn't be disabled. */
      if (!disabled && pilot_isDisabled(pilot_stack[i]))
         continue;

      /* Must be a valid target. */
      if (!pilot_validTarget( p, pilot_stack[i] ))
         continue;

      /* Must be in range. */
      if (!pilot_inRangePilot( p, pilot_stack[i], NULL ))
         continue;

      /* Only allow selection if off-screen. */
      if (gui_onScreenPilot( &rx, &ry, pilot_stack[i] ))
         continue;

      ta = atan2( p->solid->pos.y - pilot_stack[i]->solid->pos.y,
            p->solid->pos.x - pilot_stack[i]->solid->pos.x );
      if (ABS(angle_diff(ang, ta)) < ABS(angle_diff(ang, a))) {
         a = ta;
         *tp = pilot_stack[i]->id;
      }
   }
   return a;
}


/**
 * @brief Pulls a pilot out of the pilot_stack based on ID.
 *
 * It's a binary search ( O(logn) ) therefore it's pretty fast and can be
 *  abused all the time.  Maximum iterations is 32 on a platform with 32 bit
 *  unsigned ints.
 *
 *    @param id ID of the pilot to get.
 *    @return The actual pilot who has matching ID or NULL if not found.
 */
Pilot* pilot_get( const unsigned int id )
{
   int m;

   if (id==PLAYER_ID)
      return player.p; /* special case player.p */

   m = pilot_getStackPos(id);

   if ((m==-1) || (pilot_isFlag(pilot_stack[m], PILOT_DELETE)))
      return NULL;
   else
      return pilot_stack[m];
}


/**
 * @brief Sets the pilot's thrust.
 */
void pilot_setThrust( Pilot *p, double thrust )
{
   p->solid->thrust = p->thrust * thrust;
}


/**
 * @brief Sets the pilot's turn.
 */
void pilot_setTurn( Pilot *p, double turn )
{
   p->solid->dir_vel = p->turn * turn;
}


/**
 * @brief Checks to see if pilot is hostile to the player.
 *
 *    @param p Player to see if is hostile.
 *    @return 1 if pilot is hostile to the player.
 */
int pilot_isHostile( const Pilot *p )
{
   if ( !pilot_isFriendly( p )
         && !pilot_isFlag( p, PILOT_BRIBED )
         && (pilot_isFlag( p, PILOT_HOSTILE ) ||
            areEnemies( FACTION_PLAYER, p->faction ) ) )
      return 1;

   return 0;
}


/**
 * @brief Checks to see if pilot is neutral to the player.
 *
 *    @param p Player to see if is neutral.
 *    @return 1 if pilot is neutral to the player.
 */
int pilot_isNeutral( const Pilot *p )
{
   if (!pilot_isHostile(p) && !pilot_isFriendly(p))
      return 1;
   return 0;
}


/**
 * @brief Checks to see if pilot is friendly to the player.
 *
 *    @param p Player to see if is friendly.
 *    @return 1 if pilot is friendly to the player.
 */
int pilot_isFriendly( const Pilot *p )
{
   if (pilot_isFlag( p, PILOT_FRIENDLY) ||
         (areAllies( FACTION_PLAYER,p->faction) &&
         !pilot_isFlag( p, PILOT_HOSTILE ) ) )
      return 1;

   return 0;
}


/**
 * @brief Gets the dock slot of the pilot.
 *
 *    @param p Pilot to get dock slot of.
 *    @return The dock slot as an outfit slot, or NULL if N/A.
 */
PilotOutfitSlot* pilot_getDockSlot( Pilot* p )
{
   Pilot* dockpilot;

   if ((p->dockpilot != 0) && (p->dockslot != -1)) {
      dockpilot = pilot_get(p->dockpilot);
      if (dockpilot != NULL)
         return dockpilot->outfits[p->dockslot];
   }
   p->dockpilot = 0;
   p->dockslot = -1;
   return NULL;
}


/**
 * @brief Tries to turn the pilot to face dir.
 *
 * Sets the direction velocity property of the pilot's solid, does not
 *  directly manipulate the direction.
 *
 *    @param p Pilot to turn.
 *    @param dir Direction to attempt to face.
 *    @return The distance left to turn to match dir.
 */
double pilot_face( Pilot* p, const double dir )
{
   double diff, turn;

   diff = angle_diff( p->solid->dir, dir );
   turn = CLAMP( -1., 1., -10.*diff );
   pilot_setTurn( p, -turn );

   return diff;
}


/**
 * @brief Causes the pilot to turn around and brake.
 *
 *    @param p Pilot to brake.
 *    @return 1 when braking has finished.
 */
int pilot_brake( Pilot *p )
{
   double dir, thrust, diff, ftime, btime;

   /* Face backwards by default. */
   dir    = VANGLE(p->solid->vel) + M_PI;
   thrust = 1.;

   if (p->stats.misc_reverse_thrust) {
      /* Calculate the time to face backward and apply forward thrust. */
      diff = angle_diff(p->solid->dir, VANGLE(p->solid->vel) + M_PI);
      btime = ABS(diff) / p->turn + MIN( VMOD(p->solid->vel), p->speed ) /
            (p->thrust / p->solid->mass);

      /* Calculate the time to face forward and apply reverse thrust. */
      diff = angle_diff(p->solid->dir, VANGLE(p->solid->vel));
      ftime = ABS(diff) / p->turn + MIN( VMOD(p->solid->vel), p->speed ) /
            (p->thrust / p->solid->mass * PILOT_REVERSE_THRUST);

      if (btime > ftime) {
         dir    = VANGLE(p->solid->vel);
         thrust = -PILOT_REVERSE_THRUST;
      }
   }

   diff = pilot_face(p, dir);
   if (ABS(diff) < MAX_DIR_ERR && !pilot_isStopped(p))
      pilot_setThrust(p, thrust);
   else {
      pilot_setThrust(p, 0.);
      if (pilot_isStopped(p))
         return 1;
   }

   return 0;
}


/**
 * @brief Gets the braking distance for a pilot.
 *
 *    @param p Pilot to get the braking distance of.
 *    @param[out] pos Estimated final position once braked.
 *    @return Estimated Braking distance based on current speed.
 */
double pilot_brakeDist( Pilot *p, Vector2d *pos )
{
   double fdiff, bdiff, ftime, btime;
   double vang, speed, dist;

   if (pilot_isStopped(p)) {
      if (pos != NULL)
         *pos = p->solid->pos;

      return 0;
   }

   vang  = VANGLE(p->solid->vel);
   speed = MIN( VMOD(p->solid->vel), p->speed );

   /* Calculate the time to face backward and apply forward thrust. */
   bdiff = angle_diff(p->solid->dir, vang + M_PI);
   btime = ABS(bdiff) / p->turn + speed / (p->thrust / p->solid->mass);
   dist  = (ABS(bdiff) / p->turn) * speed +
         (speed / (p->thrust / p->solid->mass)) * (speed / 2.);

   if (p->stats.misc_reverse_thrust) {
      /* Calculate the time to face forward and apply reverse thrust. */
      fdiff = angle_diff(p->solid->dir, vang);
      ftime = ABS(fdiff) / p->turn + speed /
            (p->thrust / p->solid->mass * PILOT_REVERSE_THRUST);

      /* Faster to use reverse thrust. */
      if (ftime < btime)
         dist = (ABS(fdiff) / p->turn) * speed + (speed /
               (p->thrust / p->solid->mass * PILOT_REVERSE_THRUST)) * (speed / 2.);
   }

   if (pos != NULL)
      vect_cset( pos,
            p->solid->pos.x + cos(vang) * dist,
            p->solid->pos.y + sin(vang) * dist);

   return dist;
}


/**
 * @brief Attempts to make the pilot pass through a given point.
 *
 * @todo Rewrite this using a superior method.
 *
 *    @param p Pilot to control.
 *    @param x Destination X position.
 *    @param y Destination Y position.
 *    @return 1 if pilot will pass through the point, 0 otherwise.
 */
int pilot_interceptPos( Pilot *p, double x, double y )
{
   double px, py, target, face, diff, fdiff;

   px = p->solid->pos.x;
   py = p->solid->pos.y;

   /* Target angle for the pilot's vel */
   target = atan2( y - py, x - px );

   /* Current angle error. */
   diff = angle_diff( VANGLE(p->solid->vel), target );

   if (ABS(diff) < MIN_DIR_ERR) {
      pilot_setThrust(p, 0.);
      return 1;
   }
   else if (ABS(diff) > M_PI / 1.5) {
      face = target;
      fdiff = pilot_face(p, face);

      /* Apply thrust if within 180 degrees. */
      if (FABS(fdiff) < M_PI)
         pilot_setThrust(p, 1.);
      else
         pilot_setThrust(p, 0.);

      return 0;
   }
   else if (diff > M_PI_4)
      face = target + M_PI_4;
   else if (diff < -M_PI_4)
      face = target - M_PI_4;
   else
      face = target + diff;

   fdiff = pilot_face(p, face);

   /* Must be in proper quadrant, +/- 45 degrees. */
   if (fdiff < M_PI_4)
      pilot_setThrust(p, 1.);
   else
      pilot_setThrust(p, 0.);

   return 0;
}


/**
 * @brief Begins active cooldown, reducing hull and outfit temperatures.
 *
 *    @param p Pilot that should cool down.
 */
void pilot_cooldown( Pilot *p )
{
   int i;
   double heat_capacity, heat_mean;
   PilotOutfitSlot *o;

   /* Brake if necessary. */
   if (!pilot_isStopped(p)) {
      pilot_setFlag(p, PILOT_BRAKING);
      pilot_setFlag(p, PILOT_COOLDOWN_BRAKE);
      return;
   }
   else {
      pilot_rmFlag(p, PILOT_BRAKING);
      pilot_rmFlag(p, PILOT_COOLDOWN_BRAKE);
   }

   if (p->id == PLAYER_ID)
      player_message(_("#oActive cooldown engaged."));

   /* Disable active outfits. */
   if (pilot_outfitOffAll( p ) > 0)
      pilot_calcStats( p );

   /* Calculate the ship's overall heat. */
   heat_capacity = p->heat_C;
   heat_mean = p->heat_T * p->heat_C;
   for (i=0; i<array_size(p->outfits); i++) {
      o = p->outfits[i];
      o->heat_start = o->heat_T;
      heat_capacity += p->outfits[i]->heat_C;
      heat_mean += o->heat_T * o->heat_C;
   }

   /* Paranoia - a negative mean heat will result in NaN cdelay. */
   heat_mean = MAX( heat_mean, CONST_SPACE_STAR_TEMP );

   heat_mean /= heat_capacity;

   /*
    * Base delay of about 9.5s for a Lancelot, 32.8s for a Peacemaker.
    *
    * Super heat penalty table:
    *    300K:  13.4%
    *    350K:  31.8%
    *    400K:  52.8%
    *    450K:  75.6%
    *    500K: 100.0%
    */
   p->cdelay = (5. + sqrt(p->base_mass) / 2.) *
         (1. + pow(heat_mean / CONST_SPACE_STAR_TEMP - 1., 1.25));
   p->ctimer = p->cdelay * p->stats.cooldown_time;
   p->heat_start = p->heat_T;
   pilot_setFlag(p, PILOT_COOLDOWN);

   /* Run outfit cooldown start hook. */
   pilot_outfitLCooldown(p, 0, 0, p->ctimer);
}


/**
 * @brief Terminates active cooldown.
 *
 *    @param p Pilot to stop cooling.
 *    @param reason Reason for the termination.
 */
void pilot_cooldownEnd( Pilot *p, const char *reason )
{
   if (pilot_isFlag(p, PILOT_COOLDOWN_BRAKE)) {
      pilot_rmFlag(p, PILOT_COOLDOWN_BRAKE);
      return;
   }

   /* Send message to player. */
   if (p->id == PLAYER_ID) {
      if (p->ctimer < 0.)
         player_message(_("#oActive cooldown completed."));
      else {
         if (reason != NULL)
            player_message(_("#rActive cooldown aborted: %s!"), reason);
         else
            player_message(_("#rActive cooldown aborted!"));
      }
   }

   pilot_rmFlag(p, PILOT_COOLDOWN);

   /* Cooldown finished naturally, reset heat just in case. */
   if (p->ctimer < 0.) {
      pilot_heatReset( p );
      pilot_fillAmmo( p );
      pilot_outfitLCooldown(p,1,1, 0.);
   }
   else {
      pilot_outfitLCooldown(p,1,0, 0.);
   }
}


/**
 * @brief Returns the angle for a pilot to aim at an other pilot
 *
 *    @param p Pilot that aims.
 *    @param target Pilot that is being aimed at.
 */
double pilot_aimAngle( Pilot *p, Pilot *target )
{
   double x,y;
   double t;
   Vector2d tv, approach_vector, relative_location, orthoradial_vector;
   double dist;
   double speed;
   double radial_speed;
   double orthoradial_speed;

   /* Get the distance */
   dist = vect_dist( &p->solid->pos, &target->solid->pos );

   /* Check if should recalculate weapon speed with secondary weapon. */
   speed = pilot_weapSetSpeed( p, p->active_set, -1 );

   /* determine the radial, or approach speed */
   /*
    *approach_vector (denote Va) is the relative velocites of the pilot and target
    *relative_location (denote Vr) is the vector that points from the target to the pilot
    *
    *Va dot Vr is the rate of approach between the target and the pilot.
    *If this is greater than 0, the target is approaching the pilot, if less than 0, the target is fleeing.
    *
    *Va dot Vr + ShotSpeed is the net closing velocity for the shot, and is used to compute the time of flight for the shot.
    */
   vect_cset(&approach_vector, VX(p->solid->vel) - VX(target->solid->vel), VY(p->solid->vel) - VY(target->solid->vel) );
   vect_cset(&relative_location, VX(target->solid->pos) -  VX(p->solid->pos),  VY(target->solid->pos) - VY(p->solid->pos) );
   vect_cset(&orthoradial_vector, VY(p->solid->pos) - VY(target->solid->pos), VX(target->solid->pos) -  VX(p->solid->pos) );

   radial_speed = vect_dot(&approach_vector, &relative_location);
   radial_speed = radial_speed / VMOD(relative_location);

   orthoradial_speed = vect_dot(&approach_vector, &orthoradial_vector);
   orthoradial_speed = orthoradial_speed / VMOD(relative_location);

   /* Time for shots to reach that distance */
   /* t is the real positive solution of a 2nd order equation*/
   /* if the target is not hittable (i.e., fleeing faster than our shots can fly, determinant <= 0), just face the target */
   if ( ((speed*speed - VMOD(approach_vector)*VMOD(approach_vector)) != 0) && (speed*speed - orthoradial_speed*orthoradial_speed) > 0)
      t = dist * (sqrt( speed*speed - orthoradial_speed*orthoradial_speed ) - radial_speed) /
            (speed*speed - VMOD(approach_vector)*VMOD(approach_vector));
   else
      t = 0;

   /* if t < 0, try the other solution*/
   if (t < 0)
      t = - dist * (sqrt( speed*speed - orthoradial_speed*orthoradial_speed ) + radial_speed) /
            (speed*speed - VMOD(approach_vector)*VMOD(approach_vector));

   /* if t still < 0, no solution*/
   if (t < 0)
      t = 0;

   /* Position is calculated on where it should be */
   x = target->solid->pos.x + target->solid->vel.x*t
      - (p->solid->pos.x + p->solid->vel.x*t);
   y = target->solid->pos.y + target->solid->vel.y*t
      - (p->solid->pos.y + p->solid->vel.y*t);
   vect_cset( &tv, x, y );

   return VANGLE(tv);
}

/**
 * @brief Marks pilot as hostile to player.
 *
 *    @param p Pilot to mark as hostile to player.
 */
void pilot_setHostile( Pilot* p )
{
   if ( pilot_isFriendly( p ) || pilot_isFlag( p, PILOT_BRIBED )
         || !pilot_isFlag( p, PILOT_HOSTILE ) ) {
      /* Time to play combat music. */
      music_choose("combat");

      player.enemies++;
      pilot_setFlag( p, PILOT_HOSTILE );
   }
   pilot_rmFriendly( p );
   pilot_rmFlag( p, PILOT_BRIBED );
}


/**
 * @brief Gets the faction colour char, works like faction_getColourChar but for a pilot.
 *
 * @sa faction_getColourChar
 */
char pilot_getFactionColourChar( const Pilot *p )
{
   if (pilot_isDisabled(p))
      return 'I';
   else if (pilot_isFriendly(p))
      return 'N';
   else if (pilot_isHostile(p))
      return 'H';
   return faction_getColourChar(p->faction);
}


/**
 * @brief Sets the overhead communication message of the pilot.
 */
static void pilot_setCommMsg( Pilot *p, const char *s )
{
   free( p->comm_msg );
   p->comm_msg       = strdup( s );
   p->comm_msgWidth  = gl_printWidthRaw( NULL, s );
   p->comm_msgTimer  = pilot_commTimeout;
}


/**
 * @brief Have pilot send a message to another.
 *
 *    @param p Pilot sending message.
 *    @param target Target of the message.
 *    @param msg The message.
 *    @param ignore_int Whether or not should ignore interference.
 */
void pilot_message( Pilot *p, unsigned int target, const char *msg, int ignore_int )
{
   Pilot *t;
   char c;

   /* Makes no sense with no player.p atm. */
   if (player.p==NULL)
      return;

   /* Get the target. */
   t = pilot_get(target);
   if (t == NULL)
      return;

   /* Must be in range. */
   if (!ignore_int && !pilot_inRangePilot( player.p, p, NULL ))
      return;

   /* Only really affects player.p atm. */
   if (target == PLAYER_ID) {
      c = pilot_getFactionColourChar( p );
      player_message( _("#%cComm %s>#0 \"%s\""), c, p->name, msg );

      /* Set comm message. */
      pilot_setCommMsg( p, msg );
   }
}


/**
 * @brief Has the pilot broadcast a message.
 *
 *    @param p Pilot to broadcast the message.
 *    @param msg Message to broadcast.
 *    @param ignore_int Whether or not should ignore interference.
 */
void pilot_broadcast( Pilot *p, const char *msg, int ignore_int )
{
   char c;

   /* Only display if player.p exists and is in range. */
   if (player.p==NULL)
      return;

   /* Check if should ignore interference. */
   if (!ignore_int && !pilot_inRangePilot( player.p, p, NULL ))
      return;

   c = pilot_getFactionColourChar( p );
   player_message( _("#%cBroadcast %s>#0 \"%s\""), c, p->name, msg );

   /* Set comm message. */
   pilot_setCommMsg( p, msg );
}


/**
 * @brief Has the pilot broadcast a distress signal.
 *
 * Can do a faction hit on the player.
 *
 *    @param p Pilot sending the distress signal.
 *    @param attacker Attacking pilot.
 *    @param msg Message in distress signal.
 *    @param ignore_int Whether or not should ignore interference.
 */
void pilot_distress( Pilot *p, Pilot *attacker, const char *msg, int ignore_int )
{
   int i, r;
   double d;

   /* Broadcast the message. */
   if (msg[0] != '\0')
      pilot_broadcast( p, msg, ignore_int );

   /* Use the victim's target if the attacker is unknown. */
   if (attacker == NULL)
      attacker = pilot_get( p->target );

   /* Now proceed to see if player.p should incur faction loss because
    * of the broadcast signal. */

   /* Consider not in range at first. */
   r = 0;

   if ((attacker == player.p) && !pilot_isFlag(p, PILOT_DISTRESSED)) {
      /* Check if planet is in range. */
      for (i=0; i<array_size(cur_system->planets); i++) {
         if (planet_hasService(cur_system->planets[i], PLANET_SERVICE_INHABITED) &&
               (!ignore_int && pilot_inRangePlanet(p, i)) &&
               !areEnemies(p->faction, cur_system->planets[i]->faction)) {
            r = 1;
            break;
         }
      }
   }

   /* Now we must check to see if a pilot is in range. */
   for (i=0; i<array_size(pilot_stack); i++) {
      /* Skip if unsuitable. */
      if ((pilot_stack[i]->ai == NULL) || (pilot_stack[i]->id == p->id) ||
            (pilot_isFlag(pilot_stack[i], PILOT_DEAD)) ||
            (pilot_isFlag(pilot_stack[i], PILOT_DELETE)))
         continue;

      if (!ignore_int) {
         if (!pilot_inRangePilot(p, pilot_stack[i], NULL)) {
            /*
             * If the pilots are within sensor range of each other, send the
             * distress signal, regardless of electronic warfare hide values.
             */
            d = vect_dist2( &p->solid->pos, &pilot_stack[i]->solid->pos );
            if (d > pilot_sensorRange())
               continue;
         }

         /* Send AI the distress signal. */
         ai_getDistress( pilot_stack[i], p, attacker );

         /* Check if should take faction hit. */
         if ((attacker == player.p) && !pilot_isFlag(p, PILOT_DISTRESSED) &&
               !areEnemies(p->faction, pilot_stack[i]->faction))
            r = 1;
      }
   }

   /* Player only gets one faction hit per pilot. */
   if (!pilot_isFlag(p, PILOT_DISTRESSED)) {

      /* Modify faction, about 1 for a llama, 4.2 for a hawking */
      if ((attacker != NULL) && pilot_isWithPlayer(attacker) && r)
         faction_modPlayer( p->faction, -(pow(p->base_mass, 0.2) - 1.), "distress" );

      /* Set flag to avoid a second faction hit. */
      pilot_setFlag(p, PILOT_DISTRESSED);
   }
}


/**
 * @brief Unmarks a pilot as hostile to player.
 *
 *    @param p Pilot to mark as hostile to player.
 */
void pilot_rmHostile( Pilot* p )
{
   if (pilot_isHostile(p)) {
      if (pilot_isFlag(p, PILOT_HOSTILE)) {
         player.enemies--;
         if (pilot_isDisabled(p))
            player.disabled_enemies--;

         /* Change music back to ambient if no more enemies. */
         if (player.enemies <= player.disabled_enemies) {
            music_choose("ambient");
         }

         pilot_rmFlag(p, PILOT_HOSTILE);
      }

      /* Set "bribed" flag if faction has poor reputation */
      if (areEnemies( FACTION_PLAYER, p->faction ))
         pilot_setFlag(p, PILOT_BRIBED);
   }
}


/**
 * @brief Marks pilot as friendly to player.
 *
 *    @param p Pilot to mark as friendly to player.
 */
void pilot_setFriendly( Pilot* p )
{
   pilot_rmHostile(p);
   pilot_setFlag(p, PILOT_FRIENDLY);
}


/**
 * @brief Unmarks a pilot as friendly to player.
 *
 *    @param p Pilot to mark as friendly to player.
 */
void pilot_rmFriendly( Pilot* p )
{
   pilot_rmFlag(p, PILOT_FRIENDLY);
}


/**
 * @brief Gets the amount of jumps the pilot has left.
 *
 *    @param p Pilot to get the jumps left.
 *    @return Number of jumps the pilot has left.
 */
int pilot_getJumps( const Pilot* p )
{
   return p->fuel / p->fuel_consumption;
}


/**
 * @brief Gets a pilot's colour.
 *
 *    @param p Pilot to get colour of.
 *    @return The colour of the pilot.
 */
const glColour* pilot_getColour( const Pilot* p )
{
   const glColour *col;

   if (pilot_inRangePilot(player.p, p, NULL) == -1) col = &cNeutral;
   else if (pilot_isDisabled(p) || pilot_isFlag(p,PILOT_DEAD)) col = &cInert;
   else if (pilot_isFriendly(p)) col = &cFriend;
   else if (pilot_isHostile(p)) col = &cHostile;
   else col = &cNeutral;

   return col;
}


/**
 * @brief Sets the target of the pilot.
 *
 *    @param p Pilot to set target of.
 *    @param id ID of the target (set to p->id for none).
 */
void pilot_setTarget( Pilot* p, unsigned int id )
{
   /* Case no change. */
   if (p->target == id)
      return;

   p->target = id;
   pilot_lockClear( p );

   /* Set the scan timer. */
   pilot_ewScanStart( p );
}


/**
 * @brief Damages the pilot.
 *
 *    @param p Pilot that is taking damage.
 *    @param w Solid that is hitting pilot.
 *    @param shooter Attacker that shot the pilot.
 *    @param dmg Damage being done.
 *    @param reset Whether the shield timer should be reset.
 *    @return The real damage done.
 */
double pilot_hit( Pilot* p, const Solid* w, const unsigned int shooter,
      const Damage *dmg, int reset )
{
   int mod;
   double damage_shield, damage_armour, disable, knockback, dam_mod, ddmg, absorb, dmod, start;
   double tdshield, tdarmour;
   Pilot *pshooter;

   /* Invincible means no damage. */
   if (pilot_isFlag( p, PILOT_INVINCIBLE ) ||
         pilot_isFlag( p, PILOT_HIDE ))
      return 0.;

   /* Defaults. */
   pshooter       = NULL;
   dam_mod        = 0.;
   ddmg           = 0.;

   /* Calculate the damage. */
   absorb         = 1. - CLAMP( 0., 1., p->dmg_absorb - dmg->penetration );
   disable        = dmg->disable;
   dtype_calcDamage( &damage_shield, &damage_armour, absorb, &knockback, dmg, &p->stats );

   /*
    * Delay undisable if necessary. Amount varies with damage, as e.g. a
    * single Laser Cannon shot should not reset a Peacemaker's timer.
    */
   if (!pilot_isFlag(p, PILOT_DEAD) && (p->dtimer_accum > 0.))
      p->dtimer_accum -= MIN( pow(disable, .8), p->dtimer_accum );

   /* Ships that can not be disabled take raw armour damage instead of getting disabled. */
   if (pilot_isFlag( p, PILOT_NODISABLE )) {
      damage_armour += disable * absorb;
      disable        = 0.;
   }
   else
      disable       *= absorb;
   tdshield = 0.;
   tdarmour = 0.;


   /*
    * Shields take entire blow.
    */
   if (p->shield - damage_shield > 0.) {
      start      = p->shield;
      ddmg       = damage_shield;
      p->shield -= damage_shield;
      dam_mod    = damage_shield/p->shield_max;

      /*
       * Disabling damage leaks accordingly:
       *    50% + (50% - mean shield % / 2)
       *
       * 50% leaks at 100% shield, scales to 100% by 0% shield.
       *
       * The damage is adjusted based on the mean of the starting and ending
       * shield percentages. Using the starting percentage biases towards
       * low-damage, high-ROF weapons, while using the ending percentage
       * biases towards high-damage, low-ROF weapons.
       */
      p->stress += disable * (.5 + (.5 - ((start+p->shield) / p->shield_max) / 4.));

      /* True damage. */
      tdshield = damage_shield;
   }
   /*
    * Shields take part of the blow.
    */
   else if (p->shield > 0.) {
      start      = p->shield;
      dmod        = (1. - p->shield/damage_shield);
      ddmg        = p->shield + dmod * damage_armour;
      tdshield    = p->shield;
      p->shield   = 0.;

      /* Leak some disabling damage through the remaining bit of shields. */
      p->stress += disable * (1. - dmod) * (.5 + (.5 - (start / p->shield_max / 4.)));

      /* Reduce stress as armour is eaten away. */
      p->stress  *= (p->armour - dmod * damage_armour) / p->armour;
      tdarmour    = dmod * damage_armour;
      p->armour  -= tdarmour;
      p->stress  += dmod * disable;
      dam_mod     = (damage_shield + damage_armour) /
                   ((p->shield_max + p->armour_max) / 2.);

      /* Increment shield timer or time before shield regeneration kicks in. */
      if (reset) {
         p->stimer   = 3.;
         p->sbonus   = 3.;
      }
   }
   /*
    * Armour takes the entire blow.
    */
   else if (p->armour > 0.) {
      ddmg        = damage_armour;
      /* Reduce stress as armour is eaten away. */
      p->stress  *= (p->armour - damage_armour) / p->armour;
      p->armour  -= damage_armour;
      p->stress  += disable;

      /* Increment shield timer or time before shield regeneration kicks in. */
      if (reset) {
         p->stimer  = 3.;
         p->sbonus  = 3.;
      }

      /* True damage. */
      tdarmour = ddmg;
   }

   /* Ensure stress never exceeds remaining armour. */
   if (p->stress > p->armour)
      p->stress = p->armour;

   /* Do not let pilot die. */
   if ((p->armour <= 0.) && pilot_isFlag( p, PILOT_NODEATH )) {
      p->armour = 1.;
      p->stress = 1.;
   }

   /* Disabled always run before dead to ensure combat rating boost. */
   pilot_updateDisable(p, shooter);

   /* Officially dead. */
   if (p->armour <= 0.) {
      p->armour = 0.;
      dam_mod   = 0.;

      if (!pilot_isFlag(p, PILOT_DEAD)) {
         pilot_dead( p, shooter );

         /* adjust the combat rating based on pilot mass and ditto faction */
         pshooter = pilot_get(shooter);
         if ((pshooter != NULL) && pilot_isWithPlayer(pshooter)) {

            /* About 6 for a llama, 52 for hawking. */
            mod = 2 * (pow(p->base_mass, 0.4) - 1.);

            /* Modify faction for him and friends. */
            faction_modPlayer( p->faction, -mod, "kill" );

            /* Note that player destroyed the ship. */
            player.ships_destroyed[p->ship->class]++;
         }
      }
   }

   /* Some minor effects and stuff. */
   else if (p->shield <= 0.) {
      if (p->id == PLAYER_ID) { /* a bit of shaking */
         double spfx_mod = tdarmour/p->armour_max;
         spfx_shake( 0.5 * spfx_mod );
         spfx_damage( spfx_mod );
      }
   }

   /* Update player meta-data if applicable. */
   if (p->id == PLAYER_ID) {
      player.dmg_taken_shield += tdshield;
      player.dmg_taken_armour += tdarmour;
   }
   /* TODO we might want to actually resolve shooter and check for
    * FACTION_PLAYER so that escorts also get counted... */
   else if (shooter == PLAYER_ID) {
      player.dmg_done_shield += tdshield;
      player.dmg_done_armour += tdarmour;
   }

   if (w != NULL)
      /* knock back effect is dependent on both damage and mass of the weapon
       * should probably get turned into a partial conservative collision */
      vect_cadd( &p->solid->vel,
            knockback * (w->vel.x * (dam_mod/9. + w->mass/p->solid->mass/6.)),
            knockback * (w->vel.y * (dam_mod/9. + w->mass/p->solid->mass/6.)) );

   /* On hit Lua outfits activate. */
   pilot_outfitLOnhit( p, tdarmour, tdshield, shooter );

   return ddmg;
}


/**
 * @brief Handles pilot disabling. Set or unset the disable status depending on health and stress values.
 *
 *    @param p The pilot in question.
 *    @param shooter Attacker that shot the pilot.
 */
void pilot_updateDisable( Pilot* p, const unsigned int shooter )
{
   HookParam hparam;

   if ((!pilot_isFlag(p, PILOT_DISABLED)) &&
       (!pilot_isFlag(p, PILOT_NODISABLE) || (p->armour <= 0.)) &&
       (p->armour <= p->stress)) { /* Pilot should be disabled. */

      /* Cooldown is an active process, so cancel it. */
      if (pilot_isFlag(p, PILOT_COOLDOWN))
         pilot_cooldownEnd(p, NULL);

      /* Clear other active states. */
      pilot_rmFlag(p, PILOT_COOLDOWN_BRAKE);
      pilot_rmFlag(p, PILOT_BRAKING);
      pilot_rmFlag(p, PILOT_STEALTH);

      /* Clear hyperspace flags. */
      pilot_rmFlag(p, PILOT_HYP_PREP);
      pilot_rmFlag(p, PILOT_HYP_BEGIN);
      pilot_rmFlag(p, PILOT_HYP_BRAKE);
      pilot_rmFlag(p, PILOT_HYPERSPACE);

      /* If hostile, must add counter. */
      if (pilot_isHostile(p))
         player.disabled_enemies++;

      /* Disabled ships don't use up presence. */
      if (p->presence > 0) {
         system_rmCurrentPresence( cur_system, p->faction, p->presence );
         p->presence = 0;
      }

      /* Set disable timer. This is the time the pilot will remain disabled. */
      /* 200 mass llama       => 46.78 s
       * 8000 mass peacemaker => 156 s
       */
      p->dtimer = 8. * pow( p->solid->mass, 1./3. );
      p->dtimer_accum = 0.;

      /* Disable active outfits. */
      if (pilot_outfitOffAll( p ) > 0)
         pilot_calcStats( p );

      pilot_setFlag( p,PILOT_DISABLED ); /* set as disabled */
      /* Run hook */
      if (shooter > 0) {
         hparam.type       = HOOK_PARAM_PILOT;
         hparam.u.lp       = shooter;
      }
      else {
         hparam.type       = HOOK_PARAM_NIL;
      }
      pilot_runHookParam( p, PILOT_HOOK_DISABLE, &hparam, 1 ); /* Already disabled. */
   }
   else if (pilot_isFlag(p, PILOT_DISABLED) && (p->armour > p->stress)) { /* Pilot is disabled, but shouldn't be. */
      pilot_rmFlag( p, PILOT_DISABLED ); /* Undisable. */
      pilot_rmFlag( p, PILOT_DISABLED_PERM ); /* Clear perma-disable flag if necessary. */
      pilot_rmFlag( p, PILOT_BOARDING ); /* Can get boarded again. */

      /* If hostile, must remove counter. */
      if (pilot_isHostile(p)) {
         player.disabled_enemies--;
         /* Time to play combat music. */
         music_choose("combat");
      }

      /* Reset the accumulated disable time. */
      p->dtimer_accum = 0.;

      /* TODO: Make undisabled pilot use up presence again. */
      pilot_runHook( p, PILOT_HOOK_UNDISABLE );

      /* This is sort of a hack to make sure it gets reset... */
      if (p->id==PLAYER_ID)
         player_autonavResetSpeed();
   }
}

/**
 * @brief Pilot is dead, now will slowly explode.
 *
 *    @param p Pilot that just died.
 *    @param killer Pilot killer or 0 if invalid.
 */
static void pilot_dead( Pilot* p, unsigned int killer )
{
   HookParam hparam;

   if (pilot_isFlag(p,PILOT_DEAD))
      return; /* he's already dead */

   /* basically just set timers */
   if (p->id==PLAYER_ID) {
      pilot_setFlag(p, PILOT_DISABLED );
      player_dead();
   }
   p->timer[0] = 0.; /* no need for AI anymore */
   p->ptimer = MIN( 1. + sqrt(p->armour_max * p->shield_max) / 650.,
         3 + pow(p->armour_max * p->shield_max, 0.4) / 500);
   p->timer[1] = 0.; /* explosion timer */

   /* flag cleanup - fixes some issues */
   pilot_rmFlag(p,PILOT_HYP_PREP);
   pilot_rmFlag(p,PILOT_HYP_BEGIN);
   pilot_rmFlag(p,PILOT_HYP_BRAKE);
   pilot_rmFlag(p,PILOT_HYPERSPACE);

   /* Turn off all outfits, should disable Lua stuff as necessary. */
   pilot_outfitOffAll( p );

   /* Pilot must die before setting death flag and probably messing with other flags. */
   if (killer > 0) {
      hparam.type       = HOOK_PARAM_PILOT;
      hparam.u.lp       = killer;
   }
   else
      hparam.type       = HOOK_PARAM_NIL;
   pilot_runHookParam( p, PILOT_HOOK_DEATH, &hparam, 1 );

   /* Need a check here in case the hook "regenerates" the pilot. */
   if (p->armour <= 0.)
      /* PILOT R OFFICIALLY DEADZ0R */
      pilot_setFlag( p, PILOT_DEAD );
}


/**
 * @brief Makes the pilot explosion.
 *    @param x X position of the pilot.
 *    @param y Y position of the pilot.
 *    @param radius Radius of the explosion.
 *    @param dmg Damage of the explosion.
 *    @param parent The exploding pilot.
 */
void pilot_explode( double x, double y, double radius, const Damage *dmg, const Pilot *parent )
{
   int i;
   double rx, ry;
   double dist, rad2;
   Pilot *p;
   Solid s; /* Only need to manipulate mass and vel. */
   Damage ddmg;

   rad2 = radius*radius;
   ddmg = *dmg;

   for (i=0; i<array_size(pilot_stack); i++) {
      p = pilot_stack[i];

      /* Calculate a bit. */
      rx = p->solid->pos.x - x;
      ry = p->solid->pos.y - y;
      dist = pow2(rx) + pow2(ry);
      /* Take into account ship size. */
      dist -= pow2(p->ship->gfx_space->sw);
      dist = MAX(0,dist);

      /* Pilot is hit. */
      if (dist < rad2) {

         /* Adjust damage based on distance. */
         ddmg.damage = dmg->damage * (1. - sqrt(dist / rad2));

         /* Impact settings. */
         s.mass =  pow2(dmg->damage) / 30.;
         s.vel.x = rx;
         s.vel.y = ry;

         /* Actual damage calculations. */
         pilot_hit( p, &s, (parent!=NULL) ? parent->id : 0, &ddmg, 1 );

         /* Shock wave from the explosion. */
         if (p->id == PILOT_PLAYER)
            spfx_shake( pow2(ddmg.damage) / pow2(100.) );
      }
   }
}


/**
 * @brief Renders the pilot.
 *
 *    @param p Pilot to render.
 *    @param dt Current deltatick.
 */
void pilot_render( Pilot* p, const double dt )
{
   int i, g;
   (void) dt;
   double scalew, scaleh;
   glColour c = {.r=1., .g=1., .b=1., .a=1.};

   /* Don't render the pilot. */
   if (pilot_isFlag( p, PILOT_NORENDER ))
      return;

   /* Check if needs scaling. */
   if (pilot_isFlag( p, PILOT_LANDING )) {
      scalew = CLAMP( 0., 1., p->ptimer / p->landing_delay );
      scaleh = scalew;
   }
   else if (pilot_isFlag( p, PILOT_TAKEOFF )) {
      scalew = CLAMP( 0., 1., 1. - p->ptimer / p->landing_delay );
      scaleh = scalew;
   }
   else {
      scalew = 1.;
      scaleh = 1.;
   }

   /* Add some transparency if stealthed. */
   if (pilot_isFlag(p, PILOT_STEALTH))
      c.a = 0.5;

   /* Base ship. */
   gl_blitSpriteInterpolateScale( p->ship->gfx_space, p->ship->gfx_engine,
         1.-p->engine_glow, p->solid->pos.x, p->solid->pos.y,
         scalew, scaleh,
         p->tsx, p->tsy, &c );

#ifdef DEBUGGING
   double dircos, dirsin, x, y;
   Vector2d v;
   dircos = cos(p->solid->dir);
   dirsin = sin(p->solid->dir);
#endif /* DEBUGGING */

   /* Re-draw backwards trails. */
   for (i=g=0; g<array_size(p->ship->trail_emitters); g++){

#ifdef DEBUGGING
      if (debug_isFlag(DEBUG_MARK_EMITTER)) {
         /* Visualize the trail emitters. */
         v.x = p->ship->trail_emitters[g].x_engine * dircos -
              p->ship->trail_emitters[g].y_engine * dirsin;
         v.y = p->ship->trail_emitters[g].x_engine * dirsin +
              p->ship->trail_emitters[g].y_engine * dircos +
              p->ship->trail_emitters[g].h_engine;

         gl_gameToScreenCoords( &x, &y, p->solid->pos.x + v.x,
                                p->solid->pos.y + v.y*M_SQRT1_2 );
         if (p->ship->trail_emitters[i].trail_spec->nebula)
            gl_renderCross(x, y, 2, &cFontBlue);
         else
            gl_renderCross(x, y, 4, &cInert);
      }
#endif /* DEBUGGING */

      if (pilot_trail_generated( p, g )) {
         if (p->trail[i]->ontop)
            spfx_trail_draw( p->trail[i] );
         i++;
      }
   }

}


/**
 * @brief Renders the pilot overlay.
 *
 *    @param p Pilot to render.
 *    @param dt Current deltatick.
 */
void pilot_renderOverlay( Pilot* p, const double dt )
{
   glTexture *ico_hail;
   double x, y;
   double dx, dy;
   int sx, sy;
   glColour c;

   /* Don't render the pilot. */
   if (pilot_isFlag( p, PILOT_NORENDER ))
      return;

   /* Render the hailing graphic if needed. */
   if (pilot_isFlag( p, PILOT_HAILING )) {
      ico_hail = gui_hailIcon();
      if (ico_hail != NULL) {
         /* Handle animation. */
         p->htimer -= dt;
         sx = (int)ico_hail->sx;
         sy = (int)ico_hail->sy;
         if (p->htimer < 0.) {
            p->htimer = .1;
            p->hail_pos++;
            p->hail_pos %= sx*sy;
         }
         /* Render. */
         gl_blitSprite( ico_hail,
               p->solid->pos.x + PILOT_SIZE_APPROX*p->ship->gfx_space->sw/2. + ico_hail->sw/4.,
               p->solid->pos.y + PILOT_SIZE_APPROX*p->ship->gfx_space->sh/2. + ico_hail->sh/4.,
               p->hail_pos % sx, p->hail_pos / sx, NULL );
      }
   }

   /* Text ontop if needed. */
   if (p->comm_msg != NULL) {

      /* Coordinate translation. */
      gl_gameToScreenCoords( &x, &y, p->solid->pos.x, p->solid->pos.y );

      /* Display the text. */
      p->comm_msgTimer -= dt;
      if (p->comm_msgTimer < 0.) {
         free(p->comm_msg);
         p->comm_msg = NULL;
      }
      else {
         /* Colour. */
         c.r = 1.;
         c.g = 1.;
         c.b = 1.;
         if (p->comm_msgTimer - pilot_commFade < 0.)
            c.a = p->comm_msgTimer / pilot_commFade;
         else
            c.a = 1.;

         /* Position to render at. */
         dx = x - p->comm_msgWidth/2.;
         dy = y + PILOT_SIZE_APPROX*p->ship->gfx_space->sh/2.;

         /* Background. */
         gl_renderRect( dx-2., dy-2., p->comm_msgWidth+4., gl_defFont.h+4., &cBlackHilight );

         /* Display text. */
         gl_printRaw( NULL, dx, dy, &c, -1., p->comm_msg );
      }
   }
}


/**
 * @brief Updates the pilot.
 *
 *    @param pilot Pilot to update.
 *    @param dt Current delta tick.
 */
void pilot_update( Pilot* pilot, double dt )
{
   int i, cooling, nchg;
   int ammo_threshold;
   unsigned int l;
   Pilot *target;
   double a, px,py, vx,vy;
   char buf[16];
   PilotOutfitSlot *o;
   double Q;
   Damage dmg;
   double stress_falloff;
   double efficiency, thrust;
   double reload_time;

   /* Modify the dt with speedup. */
   dt *= pilot->stats.time_speedup;

   /* Check target validity. */
   if (pilot->target != pilot->id) {
      target = pilot_get(pilot->target);
      if (target == NULL)
         pilot_setTarget( pilot, pilot->id );
   }
   else
      target = NULL;

   cooling = pilot_isFlag(pilot, PILOT_COOLDOWN);

   /*
    * Update timers.
    */
   pilot->ptimer   -= dt;
   pilot->tcontrol -= dt;
   if (cooling) {
      pilot->ctimer   -= dt;
      if (pilot->ctimer < 0.) {
         pilot_cooldownEnd(pilot, NULL);
         cooling = 0;
      }
   }
   pilot->stimer   -= dt;
   if (pilot->stimer <= 0.)
      pilot->sbonus   -= dt;
   for (i=0; i<MAX_AI_TIMERS; i++)
      if (pilot->timer[i] > 0.)
         pilot->timer[i] -= dt;
   /* Update heat. */
   a = -1.;
   Q = 0.;
   nchg = 0; /* Number of outfits that change state, processed at the end. */
   for (i=0; i<array_size(pilot->outfits); i++) {
      o = pilot->outfits[i];

      /* Picky about our outfits. */
      if (o->outfit == NULL)
         continue;
      if (!o->active)
         continue;

      /* Handle firerate timer. */
      if (o->timer > 0.)
         o->timer -= dt * pilot_heatFireRateMod( o->heat_T );

      /* Handle reload timer. (Note: this works backwards compared to
       * other timers. This helps to simplify code resetting the timer
       * elsewhere.)
       */
      if ((outfit_isLauncher(o->outfit) || outfit_isFighterBay(o->outfit)) &&
            (outfit_ammo(o->outfit) != NULL)) {

         /* Initial (raw) ammo threshold */
         if (outfit_isLauncher(o->outfit)) {
            ammo_threshold = o->outfit->u.lau.amount;
            ammo_threshold = round( (double)ammo_threshold * pilot->stats.ammo_capacity );
            reload_time = o->outfit->u.lau.reload_time / pilot->stats.launch_reload;
         }
         else { /* if (outfit_isFighterBay( o->outfit)) { */ /* Commented to shut up warning. */
            ammo_threshold = o->outfit->u.bay.amount;
            ammo_threshold = round( (double)ammo_threshold * pilot->stats.fbay_capacity );
            /* Adjust for deployed fighters if needed */
            ammo_threshold -= o->u.ammo.deployed;
            reload_time = o->outfit->u.bay.reload_time / pilot->stats.fbay_reload;
         }

         /* Add to timer. */
         if (o->rtimer < reload_time)
            o->rtimer += dt;

         /* Don't allow accumulation of the timer before reload allowed */
         if ( o->u.ammo.quantity >= ammo_threshold )
            o->rtimer = 0;

         while ((o->rtimer >= reload_time) &&
               (o->u.ammo.quantity < ammo_threshold)) {
            o->rtimer -= reload_time;
            pilot_addAmmo( pilot, o, outfit_ammo( o->outfit ), 1 );
         }

         o->rtimer = MIN( o->rtimer, reload_time );
      }

      /* Handle state timer. */
      if (o->stimer >= 0.) {
         o->stimer -= dt;
         if (o->stimer < 0.) {
            if (o->state == PILOT_OUTFIT_ON) {
               pilot_outfitOff( pilot, o );
               nchg++;
            }
            else if (o->state == PILOT_OUTFIT_COOLDOWN) {
               o->state  = PILOT_OUTFIT_OFF;
               nchg++;
            }
         }
      }

      /* Handle heat. */
      if (!cooling)
         Q  += pilot_heatUpdateSlot( pilot, o, dt );

      /* Handle lockons. */
      pilot_lockUpdateSlot( pilot, o, target, &a, dt );
   }

   /* Global heat. */
   if (!cooling)
      pilot_heatUpdateShip( pilot, Q, dt );
   else
      pilot_heatUpdateCooldown( pilot );

   /* Update electronic warfare. */
   pilot_ewUpdateDynamic( pilot, dt );

   /* Update stress. */
   if (!pilot_isFlag(pilot, PILOT_DISABLED)) { /* Case pilot is not disabled. */
      stress_falloff = 0.3*sqrt(pilot->solid->mass); /* Should be about 38 seconds for a 300 mass ship with 200 armour, and 172 seconds for a 6000 mass ship with 4000 armour. */
      pilot->stress -= stress_falloff * pilot->stats.stress_dissipation * dt;
      pilot->stress = MAX(pilot->stress, 0);
   }
   else if (!pilot_isFlag(pilot, PILOT_DISABLED_PERM)) { /* Case pilot is disabled (but not permanently so). */
      pilot->dtimer_accum += dt;
      if (pilot->dtimer_accum >= pilot->dtimer) {
         pilot->stress       = 0.;
         pilot->dtimer_accum = 0;
         pilot_updateDisable(pilot, 0);
      }
   }

   /* Handle takeoff/landing. */
   if (pilot_isFlag(pilot,PILOT_TAKEOFF)) {
      if (pilot->ptimer < 0.) {
         pilot_rmFlag(pilot,PILOT_TAKEOFF);
         return;
      }
   }
   else if (pilot_isFlag(pilot,PILOT_LANDING)) {
      if (pilot->ptimer < 0.) {
         if (pilot_isPlayer(pilot)) {
            player_setFlag( PLAYER_HOOK_LAND );
            pilot->ptimer = 0.;
         }
         else
            pilot_delete(pilot);
         return;
      }
   }
   /* he's dead jim */
   else if (pilot_isFlag(pilot,PILOT_DEAD)) {

      /* pilot death sound */
      if (!pilot_isFlag(pilot,PILOT_DEATH_SOUND) &&
            (pilot->ptimer < 0.050)) {

         /* Play random explosion sound. */
         snprintf(buf, sizeof(buf), "explosion%d", RNG(0,2));
         sound_playPos( sound_get(buf), pilot->solid->pos.x, pilot->solid->pos.y,
               pilot->solid->vel.x, pilot->solid->vel.y );

         pilot_setFlag(pilot,PILOT_DEATH_SOUND);
      }
      /* final explosion */
      else if (!pilot_isFlag(pilot,PILOT_EXPLODED) &&
            (pilot->ptimer < 0.200)) {

         /* Damage from explosion. */
         a                 = sqrt(pilot->solid->mass);
         dmg.type          = dtype_get("explosion_splash");
         dmg.damage        = MAX(0., 2. * (a * (1. + sqrt(pilot->fuel + 1.) / 28.)));
         dmg.penetration   = 1.; /* Full penetration. */
         dmg.disable       = 0.;
         expl_explode( pilot->solid->pos.x, pilot->solid->pos.y,
               pilot->solid->vel.x, pilot->solid->vel.y,
               pilot->ship->gfx_space->sw/2./PILOT_SIZE_APPROX + a, &dmg, NULL, EXPL_MODE_SHIP );
         debris_add( pilot->solid->mass, pilot->ship->gfx_space->sw/2.,
               pilot->solid->pos.x, pilot->solid->pos.y,
               pilot->solid->vel.x, pilot->solid->vel.y );
         pilot_setFlag(pilot,PILOT_EXPLODED);
         pilot_runHook( pilot, PILOT_HOOK_EXPLODED );

         /* We do a check here in case the pilot was regenerated. */
         if (pilot_isFlag(pilot, PILOT_EXPLODED)) {
            /* Release cargo */
            for (i=0; i<array_size(pilot->commodities); i++)
               commodity_Jettison( pilot->id, pilot->commodities[i].commodity,
                     pilot->commodities[i].quantity );
         }
      }
      /* reset random explosion timer */
      else if (pilot->timer[1] <= 0.) {
         pilot->timer[1] = 0.08 * (pilot->ptimer - pilot->timer[1]) /
               pilot->ptimer;

         /* random position on ship */
         a = RNGF()*2.*M_PI;
         px = VX(pilot->solid->pos) +  cos(a)*RNGF()*pilot->ship->gfx_space->sw/2.;
         py = VY(pilot->solid->pos) +  sin(a)*RNGF()*pilot->ship->gfx_space->sh/2.;
         vx = VX(pilot->solid->vel);
         vy = VY(pilot->solid->vel);

         /* set explosions */
         l = (pilot->id==PLAYER_ID) ? SPFX_LAYER_FRONT : SPFX_LAYER_MIDDLE;
         if (RNGF() > 0.8)
            spfx_add( spfx_get("ExpM"), px, py, vx, vy, l );
         else
            spfx_add( spfx_get("ExpS"), px, py, vx, vy, l );
      }

      /* completely destroyed with final explosion */
      if (pilot_isFlag(pilot,PILOT_DEAD) && (pilot->ptimer < 0.)) {
         if (pilot->id==PLAYER_ID) /* player.p handled differently */
            player_destroyed();
         pilot_delete(pilot);
         return;
      }
   }
   else if (pilot->armour <= 0.) { /* PWNED */
      if (pilot_isFlag( pilot, PILOT_NODEATH ))
         pilot->armour = 1.;
      else
         pilot_dead( pilot, 0 ); /* start death stuff - dunno who killed. */
   }

   /* Special handling for braking. */
   if (pilot_isFlag(pilot, PILOT_BRAKING )) {
      if (pilot_brake( pilot )) {
         if (pilot_isFlag(pilot, PILOT_COOLDOWN_BRAKE))
            pilot_cooldown( pilot );
         else {
            /* Normal braking is done (we're below MIN_VEL_ERR), now sidestep
             * normal physics and bring the ship to a near-complete stop.
             */
            pilot->solid->speed_max = 0.;
            pilot->solid->update( pilot->solid, dt );

            if (VMOD(pilot->solid->vel) < 1e-1) {
               vectnull( &pilot->solid->vel ); /* Forcibly zero velocity. */
               pilot_rmFlag(pilot, PILOT_BRAKING);
            }
         }
      }
   }

   /* Healing and energy usage is only done if not disabled. */
   if (!pilot_isDisabled(pilot)) {
      pilot_ewUpdateStealth(pilot, dt);

      /* Pilot is still alive */
      pilot->armour += pilot->armour_regen * dt;
      if (pilot->armour > pilot->armour_max)
         pilot->armour = pilot->armour_max;

      /* Regen shield */
      if (pilot->stimer <= 0.) {
         pilot->shield += pilot->shield_regen * dt;
         if (pilot->sbonus > 0.)
            pilot->shield += dt * (pilot->shield_regen * (pilot->sbonus / 1.5));
         pilot->shield = CLAMP( 0., pilot->shield_max, pilot->shield );
      }

      /*
      * Using RC circuit energy loading.
      *
      * Calculations (using y = [0:1])
      *
      *                                          \
      *    y = 1 - exp( -x / tau )               |
      *    y + dy = 1 - exp( -( x + dx ) / tau ) |  ==>
      *                                          /
      *
      *    ==> dy = exp( -x / tau ) * ( 1 - exp( -dx / tau ) ==>
      *    ==> [ dy = (1 - y) * ( 1 - exp( -dx / tau ) ) ]
      */
      pilot->energy += (pilot->energy_max - pilot->energy) *
            (1. - exp( -dt / pilot->energy_tau));
      pilot->energy -= pilot->energy_loss * dt;
      if (pilot->energy > pilot->energy_max)
         pilot->energy = pilot->energy_max;
      else if (pilot->energy < 0.) {
         pilot->energy = 0.;
         /* Stop all on outfits. */
         nchg += pilot_outfitOffAll( pilot );
         /* Run Lua stuff. */
         pilot_outfitLOutfofenergy( pilot );
      }

      /* Must recalculate stats because something changed state. */
      if (nchg > 0)
         pilot_calcStats( pilot );
   }

   /* purpose fallthrough to get the movement like disabled */
   if (pilot_isDisabled(pilot) || pilot_isFlag(pilot, PILOT_COOLDOWN)) {
      /* Do the slow brake thing */
      pilot->solid->speed_max = 0.;
      pilot_setThrust( pilot, 0. );
      pilot_setTurn( pilot, 0. );

      /* update the solid */
      pilot->solid->update( pilot->solid, dt );
      gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,
            pilot->ship->gfx_space, pilot->solid->dir );

      /* Engine glow decay. */
      if (pilot->engine_glow > 0.) {
         pilot->engine_glow -= pilot->speed / pilot->thrust * dt * pilot->solid->mass;
         if (pilot->engine_glow < 0.)
            pilot->engine_glow = 0.;
      }

      /* Update the trail. */
      pilot_sample_trails( pilot, 0 );

      return;
   }

   /* Player damage decay. */
   if (pilot->player_damage > 0.)
      pilot->player_damage -= dt * PILOT_HOSTILE_DECAY;
   else
      pilot->player_damage = 0.;

   /* Pilot is board/refueling.  Hack to match speeds. */
   if (pilot_isFlag(pilot, PILOT_REFUELBOARDING))
      pilot_refuel(pilot, dt);

   /* Pilot is boarding its target.  Hack to match speeds. */
   if (pilot_isFlag(pilot, PILOT_BOARDING)) {
      if (target==NULL)
         pilot_rmFlag(pilot, PILOT_BOARDING);
      else {
         /* Match speeds. */
         pilot->solid->vel = target->solid->vel;

         /* See if boarding is finished. */
         if (pilot->ptimer < 0.)
            pilot_boardComplete(pilot);
      }
   }

   /* Update weapons. */
   pilot_weapSetUpdate( pilot );

   if (!pilot_isFlag(pilot, PILOT_HYPERSPACE)) { /* limit the speed */

      /* pilot is afterburning */
      if (pilot_isFlag(pilot, PILOT_AFTERBURNER)) {
         /* Heat up the afterburner. */
         pilot_heatAddSlotTime(pilot, pilot->afterburner, dt);

         /* If the afterburner's efficiency is reduced to 0, shut it off. */
         if (pilot_heatEfficiencyMod(pilot->afterburner->heat_T,
               pilot->afterburner->outfit->u.afb.heat_base,
               pilot->afterburner->outfit->u.afb.heat_cap)==0)
            pilot_afterburnOver(pilot);
         else {
            if (pilot->id == PLAYER_ID)
               spfx_shake( 0.75*SPFX_SHAKE_DECAY * dt); /* shake goes down at quarter speed */
            efficiency = pilot_heatEfficiencyMod( pilot->afterburner->heat_T,
                  pilot->afterburner->outfit->u.afb.heat_base,
                  pilot->afterburner->outfit->u.afb.heat_cap );
            thrust = MIN( 1., pilot->afterburner->outfit->u.afb.mass_limit / pilot->solid->mass ) * efficiency;

            /* Adjust speed. Speed bonus falls as heat rises. */
            pilot->solid->speed_max = pilot->speed * (1. +
                  pilot->afterburner->outfit->u.afb.speed * thrust);

            /* Adjust thrust. Thrust bonus falls as heat rises. */
            pilot_setThrust(pilot, 1. + pilot->afterburner->outfit->u.afb.thrust * thrust);
         }
      }
      else
         pilot->solid->speed_max = pilot->speed;
   }
   else
      pilot->solid->speed_max = -1.; /* Disables max speed. */

   /* Set engine glow. */
   if (pilot->solid->thrust > 0.) {
      /*pilot->engine_glow += pilot->thrust / pilot->speed * dt;*/
      pilot->engine_glow += pilot->speed / pilot->thrust * dt * pilot->solid->mass;
      if (pilot->engine_glow > 1.)
         pilot->engine_glow = 1.;
   }
   else if (pilot->engine_glow > 0.) {
      pilot->engine_glow -= pilot->speed / pilot->thrust * dt * pilot->solid->mass;
      if (pilot->engine_glow < 0.)
         pilot->engine_glow = 0.;
   }

   /* Update the solid, must be run after limit_speed. */
   pilot->solid->update( pilot->solid, dt );
   gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,
         pilot->ship->gfx_space, pilot->solid->dir );

   /* See if there is commodities to gather. */
   if (!pilot_isDisabled(pilot))
      gatherable_gather( pilot->id );

   /* Update the trail. */
   pilot_sample_trails( pilot, 0 );

   /* Update outfits if necessary. */
   pilot->otimer += dt;
   while (pilot->otimer > PILOT_OUTFIT_LUA_UPDATE_DT) {
      pilot_outfitLUpdate( pilot, PILOT_OUTFIT_LUA_UPDATE_DT );
      pilot->otimer -= PILOT_OUTFIT_LUA_UPDATE_DT;
   }
}


/**
 * @brief Updates the given pilot's trail emissions.
 */
void pilot_sample_trails( Pilot* p, int none )
{
   int i, g;
   double dx, dy, dircos, dirsin, prod;
   TrailMode mode;

   dircos = cos(p->solid->dir);
   dirsin = sin(p->solid->dir);

   /* Identify the emission type. */
   if (none)
      mode = MODE_NONE;
   else {
      if (pilot_isFlag(p, PILOT_HYPERSPACE) || pilot_isFlag(p, PILOT_HYP_END))
         mode = MODE_JUMPING;
      else if (pilot_isFlag(p, PILOT_AFTERBURNER))
         mode = MODE_AFTERBURN;
      else if (p->engine_glow > 0.)
         mode = MODE_GLOW;
      else
         mode = MODE_IDLE;
   }

   /* Compute the engine offset and decide where to draw the trail. */
   for (i=g=0; g<array_size(p->ship->trail_emitters); g++)
      if (pilot_trail_generated( p, g )) {

         p->trail[i]->ontop = 0;
         if (!(p->ship->trail_emitters[g].always_under) && (dirsin > 0)) {
            /* See if the trail's front (tail) is in front of the ship. */
            prod = (trail_front( p->trail[i] ).x - p->solid->pos.x) * dircos +
                   (trail_front( p->trail[i] ).y - p->solid->pos.y) * dirsin;

            p->trail[i]->ontop = (prod < 0);
         }

         dx = p->ship->trail_emitters[g].x_engine * dircos -
              p->ship->trail_emitters[g].y_engine * dirsin;
         dy = p->ship->trail_emitters[g].x_engine * dirsin +
              p->ship->trail_emitters[g].y_engine * dircos +
              p->ship->trail_emitters[g].h_engine;
         spfx_trail_sample( p->trail[i++], p->solid->pos.x + dx, p->solid->pos.y + dy*M_SQRT1_2, mode, mode==MODE_NONE );
      }
}


/**
 * @brief Return true if the given trail_emitters index has a corresponding generated trail.
 */
static int pilot_trail_generated( Pilot* p, int generator )
{
   return !p->ship->trail_emitters[generator].trail_spec->nebula || cur_system->nebu_density>0;
}


/**
 * @brief Deletes a pilot.
 *
 *    @param p Pilot to delete.
 */
void pilot_delete( Pilot* p )
{
   Pilot *leader;
   PilotOutfitSlot* dockslot;

   /* Remove from parent's escort list */
   if (p->parent != 0) {
      leader = pilot_get(p->parent);
      if (leader != NULL)
         escort_rmList(leader, p->id);
   }

   /* Unmark as deployed if necessary */
   dockslot = pilot_getDockSlot( p );
   if ( dockslot != NULL )
   {
      dockslot->u.ammo.deployed--;
      p->dockpilot = 0;
      p->dockslot = -1;
   }

   /* Set flag to mark for deletion. */
   pilot_setFlag(p, PILOT_DELETE);
}


/**
 * @brief Handles pilot's hyperspace states.
 *
 *    @param p Pilot to handle hyperspace navigation.
 *    @param dt Current deltatick.
 */
static void pilot_hyperspace( Pilot* p, double dt )
{
   StarSystem *sys;
   double a, diff;
   int can_hyp;
   HookParam hparam;

   /* pilot is actually in hyperspace */
   if (pilot_isFlag(p, PILOT_HYPERSPACE)) {

      /* Time to play sound. */
      if ((p->id == PLAYER_ID) &&
            (p->ptimer < sound_getLength(snd_hypPowUpJump)) &&
            (p->timer[0] == -1.)) {
         p->timer[0] = -2.;
         player_soundPlay( snd_hypPowUpJump, 1 );
      }

      /* has jump happened? */
      if (p->ptimer < 0.) {
         pilot_setFlag( p, PILOT_HYP_END );
         pilot_setThrust( p, 0. );
         if (p->id == PLAYER_ID) /* player.p just broke hyperspace */
            player_setFlag( PLAYER_HOOK_HYPER );
         else {
            hparam.type        = HOOK_PARAM_JUMP;
            hparam.u.lj.srcid  = cur_system->id;
            hparam.u.lj.destid = cur_system->jumps[ p->nav_hyperspace ].targetid;

            /* Should be run before messing with delete flag. */
            pilot_runHookParam( p, PILOT_HOOK_JUMP, &hparam, 1 );

            pilot_delete(p);
         }
         return;
      }

      /* keep acceling - hyperspace uses much bigger accel */
      pilot_setThrust( p, HYPERSPACE_THRUST*p->solid->mass/p->thrust );
   }
   /* engines getting ready for the jump */
   else if (pilot_isFlag(p, PILOT_HYP_BEGIN)) {

      /* Make sure still within range. */
      can_hyp = space_canHyperspace( p );
      if (!can_hyp) {
         pilot_hyperspaceAbort( p );

         if (pilot_isPlayer(p))
            if (!player_isFlag(PLAYER_AUTONAV))
               player_message( _("#rStrayed too far from jump point: jump aborted.") );
      }
      else if (pilot_isFlag(p,PILOT_AFTERBURNER)) {
         pilot_hyperspaceAbort( p );

         if (pilot_isPlayer(p))
            if (!player_isFlag(PLAYER_AUTONAV))
               player_message( _("#rAfterburner active: jump aborted.") );
      }
      else {
         if (p->ptimer < 0.) { /* engines ready */
            p->ptimer = HYPERSPACE_FLY_DELAY * p->stats.jump_delay;
            pilot_setFlag(p, PILOT_HYPERSPACE);
            if (p->id == PLAYER_ID)
               p->timer[0] = -1.;
         }
      }
   }
   /* pilot is getting ready for hyperspace */
   else {
      /* Make sure still within range. */
      can_hyp = space_canHyperspace( p );
      if (!can_hyp) {
         pilot_hyperspaceAbort( p );

         if (pilot_isPlayer(p))
            if (!player_isFlag(PLAYER_AUTONAV))
               player_message( _("#rStrayed too far from jump point: jump aborted.") );
      }
      else {
         /* If the ship needs to charge up its hyperdrive, brake. */
         if (!p->stats.misc_instant_jump &&
               !pilot_isFlag(p, PILOT_HYP_BRAKE) && !pilot_isStopped(p))
            pilot_brake(p);
         /* face target */
         else {
            /* Done braking or no braking required. */
            pilot_setFlag( p, PILOT_HYP_BRAKE );
            pilot_setThrust( p, 0. );

            /* Face system headed to. */
            sys  = cur_system->jumps[p->nav_hyperspace].target;
            a    = ANGLE( sys->pos.x - cur_system->pos.x, sys->pos.y - cur_system->pos.y );
            diff = pilot_face( p, a );

            if (ABS(diff) < MAX_DIR_ERR) { /* we can now prepare the jump */
               if (jp_isFlag( &cur_system->jumps[p->nav_hyperspace], JP_EXITONLY )) {
                  WARN( _("Pilot '%s' trying to jump through exit-only jump from '%s' to '%s'"),
                        p->name, cur_system->name, sys->name );
               }
               else {
                  pilot_setTurn( p, 0. );
                  p->ptimer = HYPERSPACE_ENGINE_DELAY * !p->stats.misc_instant_jump;
                  pilot_setFlag(p, PILOT_HYP_BEGIN);
                  /* Player plays sound. */
                  if ((p->id == PLAYER_ID) && !p->stats.misc_instant_jump)
                     player_soundPlay( snd_hypPowUp, 1 );
               }
            }
         }
      }
   }

   if (pilot_isPlayer(p))
      player_updateSpecific( p, dt );
}


/**
 * @brief Stops the pilot from hyperspacing.
 *
 * Can only stop in preparation mode.
 *
 *    @param p Pilot to handle stop hyperspace.
 */
void pilot_hyperspaceAbort( Pilot* p )
{
   if (pilot_isFlag(p, PILOT_HYPERSPACE))
      return;

   if (pilot_isFlag(p, PILOT_HYP_BEGIN)) {
      /* Player plays sound. */
      if (p->id == PLAYER_ID) {
         player_soundStop();
         player_soundPlay( snd_hypPowDown, 1 );
      }
   }
   pilot_rmFlag(p, PILOT_HYP_BEGIN);
   pilot_rmFlag(p, PILOT_HYP_BRAKE);
   pilot_rmFlag(p, PILOT_HYP_PREP);
}


/**
 * @brief Attempts to start refueling the pilot's target.
 *
 *    @param p Pilot to try to start refueling.
 */
int pilot_refuelStart( Pilot *p )
{
   Pilot *target;

   /* Check to see if target exists, remove flag if not. */
   target = pilot_get(p->target);
   if (target == NULL) {
      pilot_rmFlag(p, PILOT_REFUELING);
      return 0;
   }

   /* Conditions are the same as boarding, except disabled. */
   if (vect_dist(&p->solid->pos, &target->solid->pos) >
         target->ship->gfx_space->sw * PILOT_SIZE_APPROX )
      return 0;
   else if ((pow2(VX(p->solid->vel)-VX(target->solid->vel)) +
            pow2(VY(p->solid->vel)-VY(target->solid->vel))) >
         (double)pow2(MAX_HYPERSPACE_VEL))
      return 0;

   /* Now start the boarding to refuel. */
   pilot_setFlag(p, PILOT_REFUELBOARDING);
   p->ptimer  = PILOT_REFUEL_TIME; /* Use timer to handle refueling. */
   return 1;
}


/**
 * @brief Has the pilot refuel its target.
 *
 *    @param p Pilot that is actively refueling.
 *    @param dt Current delta tick.
 */
static void pilot_refuel( Pilot *p, double dt )
{
   (void) dt;
   Pilot *target;

   /* Check to see if target exists, remove flag if not. */
   target = pilot_get(p->target);
   if (target == NULL) {
      pilot_rmFlag(p, PILOT_REFUELBOARDING);
      pilot_rmFlag(p, PILOT_REFUELING);
      return;
   }

   /* Match speeds. */
   p->solid->vel = target->solid->vel;

   /* Check to see if done. */
   if (p->ptimer < 0.) {
      /* Move fuel. */
      p->fuel       -= PILOT_REFUEL_QUANTITY;
      target->fuel   = MIN(target->fuel+PILOT_REFUEL_QUANTITY, target->fuel_max);

      pilot_rmFlag(p, PILOT_REFUELBOARDING);
      pilot_rmFlag(p, PILOT_REFUELING);
   }
}


/**
 * @brief Calculates the hyperspace delay for a pilot.
 *
 *    @param p Pilot to calculate hyperspace delay for.
 *    @return The hyperspace delay.
 */
ntime_t pilot_hyperspaceDelay( Pilot *p )
{
   int stu;
   stu = (int)(NT_PERIOD_SECONDS * p->stats.jump_delay);
   return ntime_create( 0, 0, stu );
}


/**
 * @brief Loops over pilot stack to remove an asteroid as target.
 *
 *    @param anchor Asteroid anchor the asteroid belongs to.
 *    @param asteroid Asteroid.
 */
void pilot_untargetAsteroid( int anchor, int asteroid )
{
   int i;
   for (i=0; i < array_size(pilot_stack); i++) {
      if ((pilot_stack[i]->nav_asteroid == asteroid) && (pilot_stack[i]->nav_anchor == anchor)) {
         pilot_stack[i]->nav_asteroid = -1;
         pilot_stack[i]->nav_anchor   = -1;
      }
   }
}


/**
 * @brief Checks to see how many of an outfit a pilot has.
 */
int pilot_numOutfit( const Pilot *p, const Outfit *o )
{
   int i, q;
   q = 0;
   for (i=0; i<array_size(p->outfits); i++) {
      if (p->outfits[i]->outfit == o)
         q++;
   }
   return q;
}


/**
 * @brief Checks to see if the pilot has at least a certain amount of credits.
 *
 *    @param p Pilot to check to see if they have enough credits.
 *    @param amount Amount to check for.
 *    @return 1 if they have enough, 0 otherwise.
 */
int pilot_hasCredits( Pilot *p, credits_t amount )
{
   if (amount < 0)
      return 1;
   return (amount <= p->credits);
}


/**
 * @brief Modifies the amount of credits the pilot has.
 *
 *    @param p Pilot to modify amount of credits of.
 *    @param amount Quantity of credits to give/take.
 *    @return Amount of credits the pilot has.
 */
credits_t pilot_modCredits( Pilot *p, credits_t amount )
{
   if (amount > 0) {
      if (CREDITS_MAX - p->credits <= amount)
         p->credits = CREDITS_MAX;
      else
         p->credits += amount;
   }
   else if (amount < 0) {
      /* ABS(CREDITS_MIN) doesn't work properly because it might be
       * -2,147,483,648, which ABS will try to convert to 2,147,483,648.
       * Problem is, that value would be represented like this in
       * binary:
       *
       * 10000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
       *
       * Which is actually -2,147,483,648, causing the condition
       * ABS(amount) >= p->credits to return false (since -2,147,483,648
       * is less than any amount of credits the player could have). */
      if ( (amount <= CREDITS_MIN) || (ABS(amount) >= p->credits) )
         p->credits = 0;
      else
         p->credits += amount;
   }
   return p->credits;
}


/**
 * @brief Initialize pilot.
 *
 *    @param pilot Pilot to initialize.
 *    @param ship Ship pilot will be flying.
 *    @param name Pilot's name, if NULL ship's name will be used.
 *    @param faction Faction of the pilot.
 *    @param ai Name of the AI profile to use for the pilot, or NULL to use the faction's.
 *    @param dir Initial direction to face (radians).
 *    @param pos Initial position.
 *    @param vel Initial velocity.
 *    @param flags Used for tweaking the pilot.
 *    @param dockpilot The pilot which launched this pilot (0 if N/A).
 *    @param dockslot The outfit slot which launched this pilot (-1 if N/A).
 */
static void pilot_init( Pilot* pilot, Ship* ship, const char* name, int faction, const char *ai,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const PilotFlags flags, unsigned int dockpilot, int dockslot )
{
   int i, j;
   PilotOutfitSlot *dslot, *slot;
   PilotOutfitSlot **pilot_list_ptr[] = { &pilot->outfit_structure, &pilot->outfit_utility, &pilot->outfit_weapon };
   ShipOutfitSlot *ship_list[] = { ship->outfit_structure, ship->outfit_utility, ship->outfit_weapon };

   /* Clear memory. */
   memset(pilot, 0, sizeof(Pilot));

   if (pilot_isFlagRaw(flags, PILOT_PLAYER)) /* Set player ID. TODO should probably be fixed to something better someday. */
      pilot->id = PLAYER_ID;
   else
      pilot->id = ++pilot_id; /* new unique pilot id based on pilot_id, can't be 0 */

   /* Defaults. */
   pilot->autoweap = 1;
   pilot->aimLines = 0;
   pilot->dockpilot = dockpilot;
   pilot->dockslot = dockslot;
   ss_statsInit( &pilot->intrinsic_stats );

   /* Basic information. */
   pilot->ship = ship;
   pilot->name = strdup( (name==NULL) ? ship->name : name );

   /* faction */
   pilot->faction = faction;

   /* solid */
   pilot->solid = solid_create(ship->mass, dir, pos, vel, SOLID_UPDATE_RK4);

   /* First pass to make sure requirements make sense. */
   pilot->armour = pilot->armour_max = 1.; /* hack to have full armour */
   pilot->shield = pilot->shield_max = 1.; /* ditto shield */
   pilot->energy = pilot->energy_max = 1.; /* ditto energy */
   pilot->fuel   = pilot->fuel_max   = 1; /* ditto fuel */
   pilot_calcStats(pilot);
   pilot->stress = 0.; /* No stress. */

   /* Allocate outfit memory. */
   pilot->outfits  = array_create( PilotOutfitSlot* );
   /* First pass copy data. */
   for (i=0; i<3; i++) {
      *pilot_list_ptr[i] = array_create_size( PilotOutfitSlot, array_size(ship_list[i]) );
      for (j=0; j<array_size(ship_list[i]); j++) {
         slot = &array_grow( pilot_list_ptr[i] );
         memset( slot, 0, sizeof(PilotOutfitSlot) );
         slot->id    = array_size(pilot->outfits);
         slot->sslot = &ship_list[i][j];
         array_push_back( &pilot->outfits, slot );
         if (pilot_list_ptr[i] != &pilot->outfit_weapon)
            slot->weapset = -1;
         if (slot->sslot->data != NULL)
            pilot_addOutfitRaw( pilot, slot->sslot->data, slot );
      }
   }
   array_shrink( &pilot->outfits );

   /* We must set the weapon auto in case some of the outfits had a default
    * weapon equipped. */
   if (!pilot_isFlagRaw(flags, PILOT_PLAYER))
      pilot_weaponAuto(pilot);

   /* cargo - must be set before calcStats */
   pilot->cargo_free = pilot->ship->cap_cargo; /* should get redone with calcCargo */

   /* Initialize heat. */
   pilot_heatReset( pilot );

   /* Set the pilot stats based on their ship and outfits */
   pilot_calcStats( pilot );

   /* Update dynamic electronic warfare (static should have already been done). */
   pilot_ewUpdateDynamic( pilot, 0. );

   /* Heal up the ship. */
   pilot->armour = pilot->armour_max;
   pilot->shield = pilot->shield_max;
   pilot->energy = pilot->energy_max;
   pilot->fuel   = pilot->fuel_max;

   /* Mark as deployed if needed */
   dslot = pilot_getDockSlot( pilot );
   if (dslot != NULL)
      dslot->u.ammo.deployed++;

   /* Safety check. */
#ifdef DEBUGGING
   const char *str = pilot_checkSpaceworthy( pilot );
   if (str != NULL) {
      DEBUG( _("Pilot '%s' failed safety check: %s"), pilot->name, str );
      for (i=0; i<array_size(pilot->outfits); i++) {
         if (pilot->outfits[i]->outfit != NULL)
            DEBUG(_("   [%d] %s"), i, _(pilot->outfits[i]->outfit->name) );
      }
   }
#endif /* DEBUGGING */

   /* set flags and functions */
   if (pilot_isFlagRaw(flags, PILOT_PLAYER)) {
      pilot->think            = player_think; /* players don't need to think! :P */
      pilot->update           = player_update; /* Players get special update. */
      pilot->render           = NULL; /* render will get called from player_think */
      pilot->render_overlay   = NULL;
      if (!pilot_isFlagRaw( flags, PILOT_EMPTY )) /* Sort of a hack. */
         player.p = pilot;
   }
   else {
      pilot->think            = ai_think;
      pilot->update           = pilot_update;
      pilot->render           = pilot_render;
      pilot->render_overlay   = pilot_renderOverlay;
   }

   /* Copy pilot flags. */
   pilot_copyFlagsRaw(pilot->flags, flags);

   /* Clear timers. */
   pilot_clearTimers(pilot);

   /* Update the x and y sprite positions. */
   gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,
         pilot->ship->gfx_space, pilot->solid->dir );

   /* Targets. */
   pilot_setTarget( pilot, pilot->id ); /* No target. */
   pilot->nav_planet       = -1;
   pilot->nav_hyperspace   = -1;
   pilot->nav_anchor       = -1;
   pilot->nav_asteroid     = -1;

   /* Check takeoff. */
   if (pilot_isFlagRaw( flags, PILOT_TAKEOFF )) {
      pilot->landing_delay = PILOT_TAKEOFF_DELAY * pilot->ship->dt_default;
      pilot->ptimer = pilot->landing_delay;
   }

   /* Create empty table for messages. */
   lua_newtable(naevL);
   pilot->messages = luaL_ref(naevL, LUA_REGISTRYINDEX);

   /* AI */
   if (ai == NULL)
      ai = faction_default_ai( faction );
   if (ai != NULL)
      ai_pinit( pilot, ai ); /* Must run before ai_create */
   pilot->shoot_indicator = 0;
}


/**
 * @brief Initialize pilot's trails according to the ship type and current system characteristics.
 */
static void pilot_init_trails( Pilot* p )
{
   int n;

   n = array_size(p->ship->trail_emitters);
   if (p->trail == NULL)
      p->trail = array_create_size( Trail_spfx*, n );

   for (int g=0; g<n; g++)
      if (pilot_trail_generated( p, g ))
         array_push_back( &p->trail, spfx_trail_create( p->ship->trail_emitters[g].trail_spec ) );
   array_shrink( &p->trail );
}


/**
 * @brief Creates a new pilot
 *
 * See pilot_init for parameters.
 *
 *    @return Pilot's id.
 *
 * @sa pilot_init
 */
unsigned int pilot_create( Ship* ship, const char* name, int faction, const char *ai,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const PilotFlags flags, unsigned int dockpilot, int dockslot )
{
   Pilot *dyn, **p;

   /* Allocate pilot memory. */
   dyn = malloc(sizeof(Pilot));
   if (dyn == NULL) {
      WARN(_("Unable to allocate memory"));
      return 0;
   }

   /* Set the pilot in the stack -- must be there before initializing */
   p = &array_grow( &pilot_stack );
   *p = dyn;

   /* Initialize the pilot. */
   pilot_init( dyn, ship, name, faction, ai, dir, pos, vel, flags, dockpilot, dockslot );

   /* Animated trail. */
   pilot_init_trails( dyn );

   /* Run Lua stuff. */
   pilot_outfitLInitAll( dyn );

   return dyn->id;
}


/**
 * @brief Creates a pilot without adding it to the stack.
 *
 *    @param ship Ship for the pilot to use.
 *    @param name Name of the pilot ship (NULL uses ship name).
 *    @param faction Faction of the ship.
 *    @param ai AI to use, or NULL to use the faction's.
 *    @param flags Flags for tweaking, PILOT_EMPTY is added.
 *    @return Pointer to the new pilot (not added to stack).
 */
Pilot* pilot_createEmpty( Ship* ship, const char* name,
      int faction, const char *ai, PilotFlags flags )
{
   Pilot* dyn;
   dyn = malloc(sizeof(Pilot));
   if (dyn == NULL) {
      WARN(_("Unable to allocate memory"));
      return 0;
   }
   pilot_setFlagRaw( flags, PILOT_EMPTY );
   pilot_init( dyn, ship, name, faction, ai, 0., NULL, NULL, flags, 0, 0 );
   return dyn;
}


/**
 * @brief Replaces the player's pilot with an alternate ship with the same ID.
 * @return The new pilot.
 */
Pilot* pilot_replacePlayer( Pilot* after )
{
   int i, j;
   i = pilot_getStackPos( PLAYER_ID );
   for (j=0; j<array_size(pilot_stack[i]->trail); j++)
      spfx_trail_remove( pilot_stack[i]->trail[j] );
   array_erase( &pilot_stack[i]->trail, array_begin(pilot_stack[i]->trail), array_end(pilot_stack[i]->trail) );
   pilot_stack[i] = after;
   pilot_init_trails( after );
   /* Run Lua stuff. */
   pilot_outfitLInitAll( after );
   return after;
}


/**
 * @brief Finds a spawn point for a pilot
 *
 *
 */
void pilot_choosePoint( Vector2d *vp, Planet **planet, JumpPoint **jump, int lf, int ignore_rules, int guerilla )
{
   int i, j, *ind;
   int *fact;
   double chance, limit;
   JumpPoint **validJumpPoints;
   JumpPoint *target;

   /* Initialize. */
   *planet = NULL;
   *jump   = NULL;
   vectnull( vp );

   /* Build landable planet table. */
   ind = array_create_size( int, array_size(cur_system->planets) );
   for (i=0; i<array_size(cur_system->planets); i++)
      if (planet_hasService(cur_system->planets[i],PLANET_SERVICE_INHABITED) &&
            !areEnemies(lf,cur_system->planets[i]->faction))
         array_push_back( &ind, i );

   /* Build jumpable jump table. */
   validJumpPoints = array_create_size( JumpPoint*, array_size(cur_system->jumps) );
   if (array_size(cur_system->jumps) > 0) {
      for (i=0; i<array_size(cur_system->jumps); i++) {
         /* The jump into the system must not be exit-only, and unless
          * ignore_rules is set, must also be non-hidden
          * (excepted if the pilot is guerilla) and have faction
          * presence matching the pilot's on the remote side.
          */
         target = cur_system->jumps[i].returnJump;

         limit = 0.;
         if (guerilla) {/* Test enemy presence on the other side. */
            fact = faction_getEnemies( lf );
            for (j=0; j<array_size(fact); j++)
               limit += system_getPresence( cur_system->jumps[i].target, fact[j] );
         }

         if (!jp_isFlag( target, JP_EXITONLY ) && (ignore_rules ||
               ( (!jp_isFlag( &cur_system->jumps[i], JP_HIDDEN ) || guerilla ) &&
               (system_getPresence( cur_system->jumps[i].target, lf ) > limit))))
            array_push_back(&validJumpPoints, target);
      }
   }

   /* Unusual case no landable nor presence, we'll just jump in randomly. */
   if (array_size(ind)==0 && array_size(validJumpPoints)==0) {
      if (guerilla) /* Guerilla ships are created far away in deep space. */
         vect_pset ( vp, 1.5*cur_system->radius, RNGF()*2*M_PI );
      else if (array_size(cur_system->jumps) > 0) {
         for (i=0; i<array_size(cur_system->jumps); i++)
            array_push_back(&validJumpPoints, cur_system->jumps[i].returnJump);
      }
      else {
         WARN(_("Creating pilot in system with no jumps nor planets to take off from!"));
         vectnull( vp );
      }
   }

   /* Calculate jump chance. */
   if (array_size(ind)>0 || array_size(validJumpPoints)>0) {
      chance = array_size(validJumpPoints);
      chance = chance / (chance + array_size(ind));

      /* Random jump in. */
      if ((RNGF() <= chance) && (validJumpPoints != NULL))
         *jump = validJumpPoints[ RNG_BASE(0,array_size(validJumpPoints)-1) ];
      /* Random take off. */
      else if (array_size(ind) != 0)
         *planet = cur_system->planets[ ind[ RNG_BASE(0, array_size(ind)-1) ] ];
   }

   /* Free memory allocated. */
   array_free( ind );
   array_free(validJumpPoints);
}


/**
 * @brief Frees and cleans up a pilot
 *
 *    @param p Pilot to free.
 */
void pilot_free( Pilot* p )
{
   int i;

   /* Clear up pilot hooks. */
   pilot_clearHooks(p);

   /* If hostile, must remove counter. */
   pilot_rmHostile(p);

   pilot_weapSetFree(p);

   array_free(p->outfits);
   array_free(p->outfit_structure);
   array_free(p->outfit_utility);
   array_free(p->outfit_weapon);

   pilot_cargoRmAll( p, 1 );

   /* Clean up data. */
   if (p->ai != NULL)
      ai_destroy(p); /* Must be destroyed first if applicable. */

   free(p->name);
   /* Case if pilot is the player. */
   if (player.p==p)
      player.p = NULL;
   solid_free(p->solid);
   free(p->mounted);

   escort_freeList(p);

   free(p->comm_msg);

   /* Free messages. */
   luaL_unref(naevL, p->messages, LUA_REGISTRYINDEX);

   /* Free animated trail. */
   for (i=0; i<array_size(p->trail); i++) {
      p->trail[i]->ontop = 0;
      spfx_trail_remove( p->trail[i] );
   }
   array_free(p->trail);

#ifdef DEBUGGING
   memset( p, 0, sizeof(Pilot) );
#endif /* DEBUGGING */

   free(p);
}


/**
 * @brief Destroys pilot from stack
 *
 *    @param p Pilot to destroy.
 */
void pilot_destroy(Pilot* p)
{
   int i;
   PilotOutfitSlot* dockslot;

   /* find the pilot */
   i = pilot_getStackPos(p->id);

   /* Stop all outfits. */
   pilot_outfitOffAll(p);

   /* Handle Lua outfits. */
   pilot_outfitLCleanup(p);

   /* Remove faction if necessary. */
   if (p->presence > 0) {
      system_rmCurrentPresence( cur_system, p->faction, p->presence );
      p->presence = 0;
   }

   /* Unmark as deployed if necessary */
   dockslot = pilot_getDockSlot( p );
   if ( dockslot != NULL ) {
      dockslot->u.ammo.deployed--;
      p->dockpilot = 0;
      p->dockslot = -1;
   }

   /* pilot is eliminated */
   pilot_free(p);
   array_erase( &pilot_stack, &pilot_stack[i], &pilot_stack[i+1] );
}


/**
 * @brief Initializes pilot stuff.
 */
void pilots_init (void)
{
   pilot_stack = array_create_size( Pilot*, PILOT_SIZE_MIN );
}


/**
 * @brief Frees the pilot stack.
 */
void pilots_free (void)
{
   int i;

   pilot_freeGlobalHooks();

   /* First pass to stop outfits. */
   for (i=0; i < array_size(pilot_stack); i++) {
      /* Stop all outfits. */
      pilot_outfitOffAll(pilot_stack[i]);
      /* Handle Lua outfits. */
      pilot_outfitLCleanup(pilot_stack[i]);
   }

   /* Free pilots. */
   for (i=0; i < array_size(pilot_stack); i++)
      pilot_free(pilot_stack[i]);
   array_free(pilot_stack);
   pilot_stack = NULL;
   player.p = NULL;
}


/**
 * @brief Cleans up the pilot stack - leaves the player
 *
 *    @param persist Do not remove persistent pilots.
 */
void pilots_clean( int persist )
{
   int i, persist_count=0;
   Pilot *p;

   /* First pass to stop outfits without clearing stuff - this can call all
    * sorts of Lua stuff. */
   for (i=0; i < array_size(pilot_stack); i++) {
      p = pilot_stack[i];
      if (p == player.p &&
          (persist && pilot_isFlag(p, PILOT_PERSIST)))
         continue;
      /* Stop all outfits. */
      pilot_outfitOffAll(p);
      /* Handle Lua outfits. */
      pilot_outfitLCleanup(p);
   }

   /* Here we actually clean up stuff. */
   for (i=0; i < array_size(pilot_stack); i++) {
      /* move player and persisted pilots to start */
      if (pilot_stack[i] == player.p ||
          (persist && pilot_isFlag(pilot_stack[i], PILOT_PERSIST))) {
         /* Have to swap the pilots so it gets properly freed. */
         p = pilot_stack[persist_count];
         pilot_stack[persist_count] = pilot_stack[i];
         pilot_stack[i] = p;
         p = pilot_stack[persist_count];
         /* Misc clean up. */
         p->lockons = 0; /* Clear lockons. */
         p->projectiles = 0; /* Clear projectiles. */
         pilot_clearTimers( p ); /* Reset timers. */
         /* Reset trails */
         for (int g=0; g<array_size(p->trail); g++)
            spfx_trail_remove( p->trail[g] );
         array_erase( &p->trail, array_begin(p->trail), array_end(p->trail) );
         /* All done. */
         persist_count++;
      }
      else /* rest get killed */
         pilot_free(pilot_stack[i]);
   }
   array_erase( &pilot_stack, &pilot_stack[persist_count], array_end(pilot_stack) );

   /* Clear global hooks. */
   pilots_clearGlobalHooks();
}


/**
 * @brief Updates pilot state which depends on the system (sensor range, nebula trails...)
 */
void pilots_newSystem (void)
{
   pilot_updateSensorRange();
   for (int i=0; i < array_size(pilot_stack); i++)
      pilot_init_trails( pilot_stack[i] );
}


/**
 * @brief Clears all the pilots except the player and clear-exempt pilots.
 */
void pilots_clear (void)
{
   int i;
   for (i=0; i < array_size(pilot_stack); i++)
      if (!pilot_isPlayer(pilot_stack[i])
            && !pilot_isFlag(pilot_stack[i], PILOT_NOCLEAR))
         pilot_delete( pilot_stack[i] );
}


/**
 * @brief Even cleans up the player.
 */
void pilots_cleanAll (void)
{
   pilots_clean(0);
   if (player.p != NULL) {
      pilot_free(player.p);
      player.p = NULL;
   }
   array_erase( &pilot_stack, array_begin(pilot_stack), array_end(pilot_stack) );
}


/**
 * @brief Updates all the pilots.
 *
 *    @param dt Delta tick for the update.
 */
void pilots_update( double dt )
{
   int i;
   Pilot *p;

   /* Now update all the pilots. */
   for (i=0; i<array_size(pilot_stack); i++) {
      p = pilot_stack[i];

      /* Destroy pilot and go on. */
      if (pilot_isFlag(p, PILOT_DELETE)) {
         pilot_destroy(p);
         i--; /* Must decrement iterator. */
         continue;
      }

      /* Invisible, not doing anything. */
      if (pilot_isFlag(p, PILOT_HIDE))
         continue;

      /* See if should think. */
      if ((p->think==NULL) || (p->ai==NULL))
         continue;
      if (pilot_isDisabled(p))
         continue;
      if (pilot_isFlag(p,PILOT_DEAD))
         continue;

      /* Ignore persisting pilots during simulation since they don't get cleared. */
      if (space_isSimulation() && (pilot_isFlag(p,PILOT_PERSIST)))
         continue;

      /* Hyperspace gets special treatment */
      if (pilot_isFlag(p, PILOT_HYP_PREP))
         pilot_hyperspace(p, dt);
      /* Entering hyperspace. */
      else if (pilot_isFlag(p, PILOT_HYP_END)) {
         if ((VMOD(p->solid->vel) < 2*solid_maxspeed( p->solid, p->speed, p->thrust) ) && (p->ptimer < 0.))
            pilot_rmFlag(p, PILOT_HYP_END);
      }
      /* Must not be boarding to think. */
      else if (!pilot_isFlag(p, PILOT_BOARDING) &&
            !pilot_isFlag(p, PILOT_REFUELBOARDING) &&
            /* Must not be landing nor taking off. */
            !pilot_isFlag(p, PILOT_LANDING) &&
            !pilot_isFlag(p, PILOT_TAKEOFF))
         p->think(p, dt);
   }

   /* Now update all the pilots. */
   for (i=0; i<array_size(pilot_stack); i++) {
      p = pilot_stack[i];

      /* Ignore. */
      if (pilot_isFlag(p, PILOT_DELETE))
         continue;

      /* Invisible, not doing anything. */
      if (pilot_isFlag(p, PILOT_HIDE))
         continue;

      /* Just update the pilot. */
      if (p->update) /* update */
         p->update( p, dt );
   }
}


/**
 * @brief Renders all the pilots.
 *
 *    @param dt Current delta tick.
 */
void pilots_render( double dt )
{
   int i;
   for (i=0; i<array_size(pilot_stack); i++) {

      /* Invisible, not doing anything. */
      if (pilot_isFlag(pilot_stack[i], PILOT_HIDE))
         continue;

      if (pilot_stack[i]->render != NULL) /* render */
         pilot_stack[i]->render(pilot_stack[i], dt);
   }
}


/**
 * @brief Renders all the pilots overlays.
 *
 *    @param dt Current delta tick.
 */
void pilots_renderOverlay( double dt )
{
   int i;
   for (i=0; i<array_size(pilot_stack); i++) {

      /* Invisible, not doing anything. */
      if (pilot_isFlag(pilot_stack[i], PILOT_HIDE))
         continue;

      if (pilot_stack[i]->render_overlay != NULL) /* render */
         pilot_stack[i]->render_overlay(pilot_stack[i], dt);
   }
}


/**
 * @brief Clears the pilot's timers.
 *
 *    @param pilot Pilot to clear timers of.
 */
void pilot_clearTimers( Pilot *pilot )
{
   int i, n;
   PilotOutfitSlot *o;

   /* Clear outfits first to not leave some outfits in dangling states. */
   pilot_outfitOffAll(pilot);

   pilot->ptimer     = 0.; /* Pilot timer. */
   pilot->tcontrol   = 0.; /* AI control timer. */
   pilot->stimer     = 0.; /* Shield timer. */
   pilot->dtimer     = 0.; /* Disable timer. */
   pilot->otimer     = 0.; /* Outfit timer. */
   for (i=0; i<MAX_AI_TIMERS; i++)
      pilot->timer[i] = 0.; /* Specific AI timers. */
   n = 0;
   for (i=0; i<array_size(pilot->outfits); i++) {
      o = pilot->outfits[i];
      o->timer    = 0.; /* Last used timer. */
      o->stimer   = 0.; /* State timer. */
      if (o->state != PILOT_OUTFIT_OFF) {
         o->state    = PILOT_OUTFIT_OFF; /* Set off. */
         n++;
      }
   }

   /* Must recalculate stats. */
   if (n > 0)
      pilot_calcStats( pilot );
}


/**
 * @brief Gets the relative size(shipmass) between the current pilot and the specified target
 *
 *    @param cur_pilot the current pilot
 *    @param p the pilot whose mass we will compare
 *    @return A number from 0 to 1 mapping the relative masses
 */
double pilot_relsize( const Pilot* cur_pilot, const Pilot* p )
{
   return (1 - 1/(1 + ((double)cur_pilot->solid->mass / (double)p->solid->mass)));
}

/**
 * @brief Gets the relative damage output(total DPS) between the current pilot and the specified target
 *
 *    @param cur_pilot Reference pilot to compare against.
 *    @param p The pilot whose dps we will compare
 *    @return The relative dps of p with respect to cur_pilot (0.5 is equal, 1 is p is infinitely stronger, 0 is t is infinitely stronger).
 */
double pilot_reldps( const Pilot* cur_pilot, const Pilot* p )
{
   int i;
   double DPSaccum_target = 0.;
   double DPSaccum_pilot = 0.;
   double delay_cache, damage_cache;
   Outfit *o;
   const Damage *dmg;

   for (i=0; i<array_size(p->outfit_weapon); i++) {
      o = p->outfit_weapon[i].outfit;
      if (o == NULL)
         continue;
      dmg = outfit_damage( o );
      if (dmg == NULL)
         continue;

      damage_cache   = dmg->damage;
      delay_cache    = outfit_delay( o );
      if ((damage_cache > 0) && (delay_cache > 0))
         DPSaccum_target += ( damage_cache/delay_cache );
   }

   for (i=0; i<array_size(cur_pilot->outfit_weapon); i++) {
      o = cur_pilot->outfit_weapon[i].outfit;
      if (o == NULL)
         continue;
      dmg = outfit_damage( o );
      if (dmg == NULL)
         continue;

      damage_cache   = dmg->damage;
      delay_cache    = outfit_delay( o );
      if ((damage_cache > 0) && (delay_cache > 0))
         DPSaccum_pilot += ( damage_cache/delay_cache );
   }

   if ((DPSaccum_target > 1e-6) && (DPSaccum_pilot > 1e-6))
      return DPSaccum_pilot / (DPSaccum_target + DPSaccum_pilot);
   else if (DPSaccum_pilot > 0.)
      return 1;
   return 0;
}

/**
 * @brief Gets the relative hp(combined shields and armour) between the current pilot and the specified target
 *
 *    @param cur_pilot Reference pilot.
 *    @param p the pilot whose shields/armour we will compare
 *    @return A number from 0 to 1 mapping the relative HPs (0.5 is equal, 1 is reference pilot is infinity, 0 is current pilot is infinity)
 */
double pilot_relhp( const Pilot* cur_pilot, const Pilot* p )
{
   double c_hp = cur_pilot -> armour_max + cur_pilot -> shield_max;
   double p_hp = p -> armour_max + p -> shield_max;
   return c_hp / (p_hp + c_hp);
}


/**
 * @brief Gets the price or worth of a pilot in credits.
 *
 *    @param p Pilot to get worth of.
 *    @return Worth of the pilot.
 */
credits_t pilot_worth( const Pilot *p )
{
   credits_t price;
   int i;

   /* Ship price is base price + outfit prices. */
   price = ship_basePrice( p->ship );
   for (i=0; i<array_size(p->outfits); i++) {
      if (p->outfits[i]->outfit == NULL)
         continue;
      /* Don't count unique outfits. */
      if (outfit_isProp(p->outfits[i]->outfit, OUTFIT_PROP_UNIQUE))
         continue;
      price += p->outfits[i]->outfit->price;
   }

   return price;
}


/**
 * @brief Sends a message
 *
 * @param p Pilot to send message
 * @param receiver Pilot to receive it
 * @param type Type of message.
 * @param idx Index of data on lua stack or 0
 */
void pilot_msg(Pilot *p, Pilot *receiver, const char *type, unsigned int idx)
{
   if (idx != 0)
      lua_pushvalue(naevL, idx); /* data */
   else
      lua_pushnil(naevL); /* data */

   lua_newtable(naevL); /* data, msg */

   lua_pushpilot(naevL, p->id); /* data, msg, sender */
   lua_rawseti(naevL, -2, 1); /* data, msg */

   lua_pushstring(naevL, type); /* data, msg, type */
   lua_rawseti(naevL, -2, 2); /* data, msg */

   lua_pushvalue(naevL, -2); /* data, msg, data */
   lua_rawseti(naevL, -2, 3); /* data, msg */

   lua_rawgeti(naevL, LUA_REGISTRYINDEX, receiver->messages); /* data, msg, messages */
   lua_pushvalue(naevL, -2); /* data, msg, messages, msg */
   lua_rawseti(naevL, -2, lua_objlen(naevL, -2)+1); /* data, msg, messages */
   lua_pop(naevL, 3); /*  */
}


/**
 * @brief Checks to see if the pilot has illegal stuf to a faction.
 *
 *    @param p Pilot to check.
 *    @param faction Faction to check.
 *    @return 1 if has illegal stuff 0 otherwise.
 */
int pilot_hasIllegal( const Pilot *p, int faction )
{
   int i;
   Commodity *c;
   /* Check commodities. */
   for (i=0; i<array_size(p->commodities); i++) {
      c = p->commodities[i].commodity;
      if (commodity_checkIllegal( c, faction )) {
         return 1;
      }
   }
   /* Nothing to see here sir. */
   return 0;
}

