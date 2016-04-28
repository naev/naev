/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot.c
 *
 * @brief Handles the pilot stuff.
 */


#include "pilot.h"

#include "naev.h"

#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "nxml.h"
#include "nstring.h"
#include "log.h"
#include "weapon.h"
#include "ndata.h"
#include "spfx.h"
#include "rng.h"
#include "hook.h"
#include "map.h"
#include "explosion.h"
#include "escort.h"
#include "music.h"
#include "player.h"
#include "gui.h"
#include "board.h"
#include "debris.h"
#include "ntime.h"
#include "ai.h"
#include "faction.h"
#include "font.h"
#include "land.h"
#include "land_outfits.h"
#include "land_shipyard.h"
#include "array.h"
#include "camera.h"
#include "damagetype.h"
#include "pause.h"


#define PILOT_CHUNK_MIN 128 /**< Minimum chunks to increment pilot_stack by */
#define PILOT_CHUNK_MAX 2048 /**< Maximum chunks to increment pilot_stack by */
#define CHUNK_SIZE      32 /**< Size to allocate memory by. */

/* ID Generators. */
static unsigned int pilot_id = PLAYER_ID; /**< Stack of pilot ids to assure uniqueness */


/* stack of pilot_nstack */
Pilot** pilot_stack = NULL; /**< Not static, used in player.c, weapon.c, pause.c, space.c and ai.c */
int pilot_nstack = 0; /**< same */
static int pilot_mstack = 0; /**< Memory allocated for pilot_stack. */


/* misc */
static double pilot_commTimeout  = 15.; /**< Time for text above pilot to time out. */
static double pilot_commFade     = 5.; /**< Time for text above pilot to fade out. */



/*
 * Prototypes
 */
/* Update. */
static void pilot_hyperspace( Pilot* pilot, double dt );
static void pilot_refuel( Pilot *p, double dt );
/* Clean up. */
static void pilot_dead( Pilot* p, unsigned int killer );
/* Targetting. */
static int pilot_validEnemy( const Pilot* p, const Pilot* target );
/* Misc. */
static void pilot_setCommMsg( Pilot *p, const char *s );
static int pilot_getStackPos( const unsigned int id );


/**
 * @brief Gets the pilot stack.
 */
Pilot** pilot_getAll( int *n )
{
   *n = pilot_nstack;
   return pilot_stack;
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
   int l,m,h;
   l = 0;
   h = pilot_nstack-1;
   while (l <= h) {
      m = (l+h) >> 1; /* for impossible overflow returning neg value */
      if (pilot_stack[m]->id > id) h = m-1;
      else if (pilot_stack[m]->id < id) l = m+1;
      else return m;
   }

   /* Not found. */
   return -1;
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
   if ((m == (pilot_nstack-1)) || (m == -1))
      return PLAYER_ID;

   /* Get first one in range. */
   p = m+1;
   if (mode == 0) {
      while (p < pilot_nstack) {
         if (((pilot_stack[p]->faction != FACTION_PLAYER) ||
                  pilot_isDisabled(pilot_stack[p])) &&
               !pilot_isFlag( pilot_stack[p], PILOT_INVISIBLE ) &&
               pilot_inRangePilot( player.p, pilot_stack[p] ))
            return pilot_stack[p]->id;
         p++;
      }
   }
   /* Get first hostile in range. */
   if (mode == 1) {
      while (p < pilot_nstack) {
         if ((pilot_stack[p]->faction != FACTION_PLAYER) &&
               !pilot_isFlag( pilot_stack[p], PILOT_INVISIBLE ) &&
               pilot_inRangePilot( player.p, pilot_stack[p] ) &&
               (pilot_isFlag(pilot_stack[p],PILOT_HOSTILE) ||
                  areEnemies( FACTION_PLAYER, pilot_stack[p]->faction)))
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
      p = pilot_nstack-1;
   else
      p = m-1;

   /* Get first one in range. */
   if (mode == 0) {
      while (p >= 0) {
         if (((pilot_stack[p]->faction != FACTION_PLAYER) ||
                  (pilot_isDisabled(pilot_stack[p]))) &&
               !pilot_isFlag( pilot_stack[p], PILOT_INVISIBLE ) &&
               pilot_inRangePilot( player.p, pilot_stack[p] ))
            return pilot_stack[p]->id;
         p--;
      }
   }
   /* Get first hostile in range. */
   else if (mode == 1) {
      while (p >= 0) {
         if ((pilot_stack[p]->faction != FACTION_PLAYER) &&
               !pilot_isFlag( pilot_stack[p], PILOT_INVISIBLE ) &&
               pilot_inRangePilot( player.p, pilot_stack[p] ) &&
               (pilot_isFlag(pilot_stack[p],PILOT_HOSTILE) ||
                  areEnemies( FACTION_PLAYER, pilot_stack[p]->faction)))
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
 *    @param p Reference pilot.
 *    @param target Pilot to see if is a valid target of the reference.
 *    @return 1 if it is valid, 0 otherwise.
 */
int pilot_validTarget( const Pilot* p, const Pilot* target )
{
   /* Must not be dead. */
   if (pilot_isFlag( target, PILOT_DELETE ) ||
         pilot_isFlag( target, PILOT_DEAD))
      return 0;

   /* Must not be invisible. */
   if (pilot_isFlag( target, PILOT_INVISIBLE ))
      return 0;

   /* Must be in range. */
   if (!pilot_inRangePilot( p, target ))
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
   /* Must not be bribed. */
   if ((target->faction == FACTION_PLAYER) && pilot_isFlag(p,PILOT_BRIBED))
      return 0;

   /* Should either be hostile by faction or by player. */
   if (!(areEnemies( p->faction, target->faction) ||
            ((target->id == PLAYER_ID) &&
             pilot_isFlag(p,PILOT_HOSTILE))))
      return 0;

   /* Shouldn't be disabled. */
   if (pilot_isDisabled(target))
      return 0;

   /* Must be a valid target. */
   if (!pilot_validTarget( p, target ))
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
   for (i=0; i<pilot_nstack; i++) {

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
unsigned int pilot_getNearestEnemy_size( const Pilot* p, double target_mass_LB, double target_mass_UB)
{
   unsigned int tp;
   int i;
   double d, td;

   tp = 0;
   d  = 0.;
   for (i=0; i<pilot_nstack; i++) {

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
 *    @param dps_factor parameter for target dps (0-1, 0.5 = current dps)
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
   for (i=0; i<pilot_nstack; i++) {
      target = pilot_stack[i];

      if (!pilot_validEnemy( p, target ))
         continue;

      /* Check distance. */
      temp = range_factor *
               vect_dist2( &target->solid->pos, &p->solid->pos )
            + fabs( pilot_relsize( p, target ) - mass_factor)
            + fabs( pilot_relhp(   p, target ) - health_factor)
            + fabs( pilot_reldps(  p, target ) - damage_factor);

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
   for (i=0; i<pilot_nstack; i++) {

      /* Must not be self. */
      if (pilot_stack[i] == p)
         continue;

      /* Player doesn't select escorts (unless disabled is active). */
      if (!disabled && (p->faction == FACTION_PLAYER) &&
            (pilot_stack[i]->faction == FACTION_PLAYER))
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
   for (i=0; i<pilot_nstack; i++) {

      /* Must not be self. */
      if (pilot_stack[i] == p)
         continue;

      /* Player doesn't select escorts (unless disabled is active). */
      if (!disabled && (p->faction == FACTION_PLAYER) &&
            (pilot_stack[i]->faction == FACTION_PLAYER))
         continue;

      /* Shouldn't be disabled. */
      if (!disabled && pilot_isDisabled(pilot_stack[i]))
         continue;

      /* Must be a valid target. */
      if (!pilot_validTarget( p, pilot_stack[i] ))
         continue;

      /* Must be in range. */
      if (!pilot_inRangePilot( p, pilot_stack[i] ))
         continue;

      /* Only allow selection if off-screen. */
      if (gui_onScreenPilot( &rx, &ry, pilot_stack[i] ))
         continue;

      ta = atan2( p->solid->pos.y - pilot_stack[i]->solid->pos.y,
            p->solid->pos.x - pilot_stack[i]->solid->pos.x );
      if ( ABS(angle_diff(ang, ta)) < ABS(angle_diff(ang, a))) {
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
   if (pilot_isFlag(p, PILOT_FRIENDLY))
      return 0;
   if (pilot_isFlag(p, PILOT_HOSTILE) ||
         areEnemies(FACTION_PLAYER,p->faction))
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
   if (pilot_isFlag(p, PILOT_HOSTILE))
      return 0;
   if (pilot_isFlag(p, PILOT_FRIENDLY) ||
         areAllies(FACTION_PLAYER,p->faction))
      return 1;
   return 0;
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
 *    @param Pilot to brake.
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
 *    @param Pilot that should cool down.
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
      player_message("\epActive cooldown engaged.");

   /* Disable active outfits. */
   if (pilot_outfitOffAll( p ) > 0)
      pilot_calcStats( p );

   /* Calculate the ship's overall heat. */
   heat_capacity = p->heat_C;
   heat_mean = p->heat_T * p->heat_C;
   for (i=0; i<p->noutfits; i++) {
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
   p->ctimer = p->cdelay;
   p->heat_start = p->heat_T;
   pilot_setFlag(p, PILOT_COOLDOWN);
}


/**
 * @brief Terminates active cooldown.
 *
 *    @param Pilot to stop cooling.
 *    @param Reason for the termination.
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
         player_message("\epActive cooldown completed.");
      else {
         if (reason != NULL)
            player_message("\erActive cooldown aborted: %s!", reason);
         else
            player_message("\erActive cooldown aborted!");
      }
   }

   pilot_rmFlag(p, PILOT_COOLDOWN);

   /* Cooldown finished naturally, reset heat just in case. */
   if (p->ctimer < 0.)
      pilot_heatReset( p );
}


/**
 * @brief Marks pilot as hostile to player.
 *
 *    @param p Pilot to mark as hostile to player.
 */
void pilot_setHostile( Pilot* p )
{
   if (!pilot_isFlag(p, PILOT_HOSTILE)) {
      /* Time to play combat music. */
      if (player.enemies == 0)
         music_choose("combat");

      player.enemies++;
      pilot_setFlag(p, PILOT_HOSTILE);
   }
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
   else if (pilot_isFlag(p, PILOT_BRIBED))
      return 'N';
   else if (pilot_isFlag(p, PILOT_HOSTILE))
      return 'H';
   return faction_getColourChar(p->faction);
}


/**
 * @brief Sets the overhead communication message of the pilot.
 */
static void pilot_setCommMsg( Pilot *p, const char *s )
{
   /* Free previous message. */
   if (p->comm_msg != NULL)
      free(p->comm_msg);

   /* Duplicate the message. */
   p->comm_msg       = strdup(s);
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
   if (!ignore_int && !pilot_inRangePilot( player.p, p ))
      return;

   /* Only really affects player.p atm. */
   if (target == PLAYER_ID) {
      c = pilot_getFactionColourChar( p );
      player_message( "\e%cComm %s>\e0 \"%s\"", c, p->name, msg );

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
   if (!ignore_int && !pilot_inRangePilot( player.p, p ))
      return;

   c = pilot_getFactionColourChar( p );
   player_message( "\e%cBroadcast %s>\e0 \"%s\"", c, p->name, msg );

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
      for (i=0; i<cur_system->nplanets; i++) {
         if (planet_hasService(cur_system->planets[i], PLANET_SERVICE_INHABITED) &&
               (!ignore_int && pilot_inRangePlanet(p, i)) &&
               !areEnemies(p->faction, cur_system->planets[i]->faction)) {
            r = 1;
            break;
         }
      }
   }

   /* Now we must check to see if a pilot is in range. */
   for (i=0; i<pilot_nstack; i++) {
      /* Skip if unsuitable. */
      if ((pilot_stack[i]->ai == NULL) || (pilot_stack[i]->id == p->id) ||
            (pilot_isFlag(pilot_stack[i], PILOT_DEAD)) ||
            (pilot_isFlag(pilot_stack[i], PILOT_DELETE)))
         continue;

      if (!ignore_int) {
         if (!pilot_inRangePilot(p, pilot_stack[i])) {
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
      if ((attacker != NULL) && (attacker->faction == FACTION_PLAYER) && r)
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
   if (pilot_isFlag(p, PILOT_HOSTILE)) {
      if (!pilot_isDisabled(p))
         player.enemies--;
      pilot_rmFlag(p, PILOT_HOSTILE);

      /* Change music back to ambient if no more enemies. */
      if (player.enemies <= 0) {
         music_choose("ambient");
         player.enemies = 0;
      }
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
   return (int)floor(p->fuel / p->fuel_consumption);
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

   if (pilot_inRangePilot(player.p, p) == -1) col = &cMapNeutral;
   else if (pilot_isDisabled(p) || pilot_isFlag(p,PILOT_DEAD)) col = &cInert;
   else if (pilot_isFlag(p,PILOT_BRIBED)) col = &cNeutral;
   else if (pilot_isHostile(p)) col = &cHostile;
   else if (pilot_isFriendly(p)) col = &cFriend;
   else col = faction_getColour(p->faction);

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
   Pilot *pshooter;

   /* Invincible means no damage. */
   if (pilot_isFlag( p, PILOT_INVINCIBLE) ||
         pilot_isFlag( p, PILOT_INVISIBLE))
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
   }
   /*
    * Shields take part of the blow.
    */
   else if (p->shield > 0.) {
      start      = p->shield;
      dmod        = (1. - p->shield/damage_shield);
      ddmg        = p->shield + dmod * damage_armour;
      p->shield   = 0.;

      /* Leak some disabling damage through the remaining bit of shields. */
      p->stress += disable * (1. - dmod) * (.5 + (.5 - (start / p->shield_max / 4.)));

      /* Reduce stress as armour is eaten away. */
      p->stress  *= (p->armour - dmod * damage_armour) / p->armour;
      p->armour  -= dmod * damage_armour;
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
   }

   /* Ensure stress never exceeds remaining armour. */
   if (p->stress > p->armour)
      p->stress = p->armour;

   /* Disabled always run before dead to ensure combat rating boost. */
   pilot_updateDisable(p, shooter);

   /* Do not let pilot die. */
   if (pilot_isFlag( p, PILOT_NODEATH ))
      p->armour = 1.;

   /* Officially dead. */
   if (p->armour <= 0.) {
      p->armour = 0.;
      dam_mod   = 0.;

      if (!pilot_isFlag(p, PILOT_DEAD)) {
         pilot_dead( p, shooter );

         /* adjust the combat rating based on pilot mass and ditto faction */
         if (pshooter == NULL)
            pshooter = pilot_get(shooter);
         if ((pshooter != NULL) && (pshooter->faction == FACTION_PLAYER)) {

            /* About 6 for a llama, 52 for hawking. */
            mod = 2 * (pow(p->base_mass, 0.4) - 1.);

            /* Modify faction for him and friends. */
            faction_modPlayer( p->faction, -mod, "kill" );
         }
      }
   }

   /* Some minor effects and stuff. */
   else if (p->shield <= 0.) {
      dam_mod = damage_armour/p->armour_max;

      if (p->id == PLAYER_ID) /* a bit of shaking */
         spfx_shake( SHAKE_MAX*dam_mod );
   }


   if (w != NULL)
      /* knock back effect is dependent on both damage and mass of the weapon
       * should probably get turned into a partial conservative collision */
      vect_cadd( &p->solid->vel,
            knockback * (w->vel.x * (dam_mod/9. + w->mass/p->solid->mass/6.)),
            knockback * (w->vel.y * (dam_mod/9. + w->mass/p->solid->mass/6.)) );

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
   int mod, h;
   Pilot *pshooter;
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

      /* Clear hyperspace flags. */
      pilot_rmFlag(p, PILOT_HYP_PREP);
      pilot_rmFlag(p, PILOT_HYP_BEGIN);
      pilot_rmFlag(p, PILOT_HYP_BRAKE);
      pilot_rmFlag(p, PILOT_HYPERSPACE);

      /* If hostile, must remove counter. */
      h = (pilot_isHostile(p)) ? 1 : 0;
      pilot_rmHostile(p);
      if (h == 1) /* Horrible hack to make sure player.p can hit it if it was hostile. */
         /* Do not use pilot_setHostile here or music will change again. */
         pilot_setFlag(p,PILOT_HOSTILE);

      /* Modify player combat rating if applicable. */
      /* TODO: Base off something more sensible than mass. */
      pshooter = pilot_get(shooter);
      if ((pshooter != NULL) && (pshooter->faction == FACTION_PLAYER)) {
         /* About 3 for a llama, 26 for hawking. */
         mod = pow(p->base_mass, 0.4) - 1.;

         /* Modify combat rating. */
         player.crating += 2*mod;
      }

      /* Disabled ships don't use up presence. */
      if (p->presence > 0) {
         system_rmCurrentPresence( cur_system, p->faction, p->presence );
         p->presence = 0;
      }

      /* Set disable timer. This is the time the pilot will remain disabled. */
      /* 50 armour llama        => 53.18s
       * 5000 armour peacemaker => 168.18s
       */
      p->dtimer = 20. * pow( p->armour, 0.25 );
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
         pilot_setFlag(p, PILOT_DISABLED_PERM ); /* Set as permanently disabled, since the disable was script-induced. */
      }
      pilot_runHookParam( p, PILOT_HOOK_DISABLE, &hparam, 1 ); /* Already disabled. */
   }
   else if (pilot_isFlag(p, PILOT_DISABLED) && (p->armour > p->stress)) { /* Pilot is disabled, but shouldn't be. */
      pilot_rmFlag( p, PILOT_DISABLED ); /* Undisable. */
      pilot_rmFlag( p, PILOT_DISABLED_PERM ); /* Clear perma-disable flag if necessary. */
      pilot_rmFlag( p, PILOT_BOARDING ); /* Can get boarded again. */

      /* Reset the accumulated disable time. */
      p->dtimer_accum = 0.;

      /* TODO: Make undisabled pilot use up presence again. */
      pilot_runHook( p, PILOT_HOOK_UNDISABLE );

      /* This is sort of a hack to make sure it gets reset... */
      if (p->id==PLAYER_ID)
         pause_setSpeed( player_isFlag(PLAYER_DOUBLESPEED) ? 2. : 1. );
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

   /* Pilot must die before setting death flag and probably messing with other flags. */
   if (killer > 0) {
      hparam.type       = HOOK_PARAM_PILOT;
      hparam.u.lp       = killer;
   }
   else
      hparam.type       = HOOK_PARAM_NIL;
   pilot_runHookParam( p, PILOT_HOOK_DEATH, &hparam, 1 );

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

   for (i=0; i<pilot_nstack; i++) {
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
            spfx_shake( pow2(ddmg.damage) / pow2(100.) * SHAKE_MAX );
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
   (void) dt;
   double scalew, scaleh;

   /* Check if needs scaling. */
   if (pilot_isFlag( p, PILOT_LANDING )) {
      scalew = CLAMP( 0., 1., p->ptimer / PILOT_LANDING_DELAY );
      scaleh = scalew;
   }
   else if (pilot_isFlag( p, PILOT_TAKEOFF )) {
      scalew = CLAMP( 0., 1., 1. - p->ptimer / PILOT_TAKEOFF_DELAY );
      scaleh = scalew;
   }
   else {
      scalew = 1.;
      scaleh = 1.;
   }

   /* Base ship. */
   gl_blitSpriteInterpolateScale( p->ship->gfx_space, p->ship->gfx_engine,
         1.-p->engine_glow, p->solid->pos.x, p->solid->pos.y,
         scalew, scaleh,
         p->tsx, p->tsy, NULL );
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
               p->solid->pos.x + PILOT_SIZE_APROX*p->ship->gfx_space->sw/2. + ico_hail->sw/4.,
               p->solid->pos.y + PILOT_SIZE_APROX*p->ship->gfx_space->sh/2. + ico_hail->sh/4.,
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
         dy = y + PILOT_SIZE_APROX*p->ship->gfx_space->sh/2.;

         /* Background. */
         gl_renderRect( dx-2., dy-2., p->comm_msgWidth+4., gl_defFont.h+4., &cBlackHilight );

         /* Display text. */
         gl_printRaw( NULL, dx, dy, &c, p->comm_msg );
      }
   }
}


/**
 * @brief Updates the pilot.
 *
 *    @param pilot Pilot to update.
 *    @param dt Current delta tick.
 */
void pilot_update( Pilot* pilot, const double dt )
{
   int i, cooling, nchg;
   unsigned int l;
   Pilot *target;
   double a, px,py, vx,vy;
   char buf[16];
   PilotOutfitSlot *o;
   double Q;
   Damage dmg;
   double stress_falloff;
   double efficiency, thrust;

   /* Check target sanity. */
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
   for (i=0; i<pilot->noutfits; i++) {
      o = pilot->outfits[i];

      /* Picky about our outfits. */
      if (o->outfit == NULL)
         continue;
      if (!o->active)
         continue;

      /* Handle firerate timer. */
      if (o->timer > 0.)
         o->timer -= dt * pilot_heatFireRateMod( o->heat_T );

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
   pilot_ewUpdateDynamic( pilot );

   /* Update stress. */
   if (!pilot_isFlag(pilot, PILOT_DISABLED)) { /* Case pilot is not disabled. */
      stress_falloff = 4.; /* TODO: make a function of the pilot's ship and/or its outfits. */
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
            pilot_setFlag(pilot,PILOT_DELETE);
         return;
      }
   }
   /* he's dead jim */
   else if (pilot_isFlag(pilot,PILOT_DEAD)) {

      /* pilot death sound */
      if (!pilot_isFlag(pilot,PILOT_DEATH_SOUND) &&
            (pilot->ptimer < 0.050)) {

         /* Play random explosion sound. */
         nsnprintf(buf, sizeof(buf), "explosion%d", RNG(0,2));
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
               pilot->ship->gfx_space->sw/2. + a, &dmg, NULL, EXPL_MODE_SHIP );
         debris_add( pilot->solid->mass, pilot->ship->gfx_space->sw/2.,
               pilot->solid->pos.x, pilot->solid->pos.y,
               pilot->solid->vel.x, pilot->solid->vel.y );
         pilot_setFlag(pilot,PILOT_EXPLODED);
         pilot_runHook( pilot, PILOT_HOOK_EXPLODED );

         /* Release cargo */
         for (i=0; i<pilot->ncommodities; i++)
            commodity_Jettison( pilot->id, pilot->commodities[i].commodity,
                  pilot->commodities[i].quantity );
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
         l = (pilot->id==PLAYER_ID) ? SPFX_LAYER_FRONT : SPFX_LAYER_BACK;
         if (RNGF() > 0.8)
            spfx_add( spfx_get("ExpM"), px, py, vx, vy, l );
         else
            spfx_add( spfx_get("ExpS"), px, py, vx, vy, l );
      }

      /* completely destroyed with final explosion */
      if (pilot->ptimer < 0.) {
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

      return;
   }

   /* Pilot is still alive */
   pilot->armour += pilot->armour_regen * dt;
   if (pilot->armour > pilot->armour_max)
      pilot->armour = pilot->armour_max;

   /* Regen shield */
   if (pilot->stimer <= 0.) {
      pilot->shield += pilot->shield_regen * dt;
      if (pilot->sbonus > 0.)
         pilot->shield += dt * (pilot->shield_regen * (pilot->sbonus / 1.5));
      if (pilot->shield > pilot->shield_max)
         pilot->shield = pilot->shield_max;
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
   }

   /* Must recalculate stats because something changed state. */
   if (nchg > 0)
      pilot_calcStats( pilot );

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
            if (pilot->id == PLAYER_ID) spfx_shake( 0.75*SHAKE_DECAY * dt); /* shake goes down at quarter speed */
            efficiency = pilot_heatEfficiencyMod( pilot->afterburner->heat_T,
                  pilot->afterburner->outfit->u.afb.heat_base,
                  pilot->afterburner->outfit->u.afb.heat_cap );
            thrust = MIN( 1., pilot->afterburner->outfit->u.afb.mass_limit / pilot->solid->mass) * efficiency;

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
}

/**
 * @brief Deletes a pilot.
 *
 *    @param p Pilot to delete3.
 */
void pilot_delete( Pilot* p )
{
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
            (p->ptimer < sound_length(snd_hypPowUpJump)) &&
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
               player_message( "\erStrayed too far from jump point: jump aborted." );
      }
      else if (pilot_isFlag(p,PILOT_AFTERBURNER)) {
         pilot_hyperspaceAbort( p );

         if (pilot_isPlayer(p))
            if (!player_isFlag(PLAYER_AUTONAV))
               player_message( "\erAfterburner active: jump aborted." );
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
               player_message( "\erStrayed too far from jump point: jump aborted." );
      }
      else {
         /* If the ship needs to charge up its hyperdrive, brake. */
         if (!p->stats.misc_instant_jump &&
               !pilot_isFlag(p, PILOT_HYP_BRAKE) && !pilot_isStopped(p))
            pilot_brake(p);
         /* face target */
         else {
            /* Done braking or no braking required. */
            pilot_setFlag( p, PILOT_HYP_BRAKE);
            pilot_setThrust( p, 0. );

            /* Face system headed to. */
            sys  = cur_system->jumps[p->nav_hyperspace].target;
            a    = ANGLE( sys->pos.x - cur_system->pos.x, sys->pos.y - cur_system->pos.y );
            diff = pilot_face( p, a );

            if (ABS(diff) < MAX_DIR_ERR) { /* we can now prepare the jump */
               if (jp_isFlag( &cur_system->jumps[p->nav_hyperspace], JP_EXITONLY )) {
                  WARN( "Pilot '%s' trying to jump through exit-only jump from '%s' to '%s'",
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
         target->ship->gfx_space->sw * PILOT_SIZE_APROX )
      return 0;
   else if ((pow2(VX(p->solid->vel)-VX(target->solid->vel)) +
            pow2(VY(p->solid->vel)-VY(target->solid->vel))) >
         (double)pow2(MAX_HYPERSPACE_VEL))
      return 0;

   /* Now start the boarding to refuel. */
   pilot_setFlag(p, PILOT_REFUELBOARDING);
   p->ptimer  = PILOT_REFUEL_TIME; /* Use timer to handle refueling. */
   p->pdata   = PILOT_REFUEL_QUANTITY;
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
   Pilot *target;
   double amount;
   int jumps;

   /* Check to see if target exists, remove flag if not. */
   target = pilot_get(p->target);
   if (target == NULL) {
      pilot_rmFlag(p, PILOT_REFUELBOARDING);
      pilot_rmFlag(p, PILOT_REFUELING);
      return;
   }

   /* Match speeds. */
   p->solid->vel = target->solid->vel;

   amount = CLAMP( 0., p->pdata, PILOT_REFUEL_RATE * dt);
   p->pdata -= amount;

   /* Move fuel. */
   p->fuel        -= amount;
   target->fuel   += amount;
   /* Stop refueling at max. */
   if (target->fuel > target->fuel_max) {
      p->ptimer      = -1.;
      target->fuel   = target->fuel_max;
   }

   /* Check to see if done. */
   if (p->ptimer < 0.) {
      /* Counteract accumulated floating point error by rounding up
       * if pilots have > 99.99% of a jump worth of fuel.
       */

      jumps = pilot_getJumps(p);
      if ((p->fuel / p->fuel_consumption - jumps) > 0.9999)
         p->fuel = p->fuel_consumption * (jumps + 1);

      jumps = pilot_getJumps(target);
      if ((target->fuel / target->fuel_consumption - jumps) > 0.9999)
         target->fuel = target->fuel_consumption * (jumps + 1);

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
   stu = (int)(NT_STP_STU * p->stats.jump_delay);
   return ntime_create( 0, 0, stu );
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
   uint64_t ul;

   ul = (uint64_t) ABS(amount);

   if (amount > 0) {
      if (CREDITS_MAX-p->credits < (credits_t)ul)
         p->credits = CREDITS_MIN;
      else
         p->credits += ul;
   }
   else if (amount < 0) {
      if ((credits_t)ul > p->credits)
         p->credits = 0;
      else
         p->credits -= ul;
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
 *    @param ai Name of the AI profile to use for the pilot.
 *    @param dir Initial direction to face (radians).
 *    @param pos Initial position.
 *    @param vel Initial velocity.
 *    @param flags Used for tweaking the pilot.
 */
void pilot_init( Pilot* pilot, Ship* ship, const char* name, int faction, const char *ai,
      const double dir, const Vector2d* pos, const Vector2d* vel,
      const PilotFlags flags, const int systemFleet )
{
   int i, p;

   /* Clear memory. */
   memset(pilot, 0, sizeof(Pilot));

   if (pilot_isFlagRaw(flags, PILOT_PLAYER)) /* Set player ID, should probably be fixed to something sane someday. */
      pilot->id = PLAYER_ID;
   else
      pilot->id = ++pilot_id; /* new unique pilot id based on pilot_id, can't be 0 */

   /* Defaults. */
   pilot->autoweap = 1;

   /* Basic information. */
   pilot->ship = ship;
   pilot->name = strdup( (name==NULL) ? ship->name : name );

   /* faction */
   pilot->faction = faction;

   /* System fleet. */
   pilot->systemFleet = systemFleet;

   /* solid */
   pilot->solid = solid_create(ship->mass, dir, pos, vel, SOLID_UPDATE_RK4);

   /* First pass to make sure requirements make sense. */
   pilot->armour = pilot->armour_max = 1.; /* hack to have full armour */
   pilot->shield = pilot->shield_max = 1.; /* ditto shield */
   pilot->energy = pilot->energy_max = 1.; /* ditto energy */
   pilot->fuel   = pilot->fuel_max   = 1.; /* ditto fuel */
   pilot_calcStats(pilot);
   pilot->stress = 0.; /* No stress. */

   /* Allocate outfit memory. */
   /* Slot types. */
   pilot->outfit_nstructure = ship->outfit_nstructure;
   pilot->outfit_structure = calloc( ship->outfit_nstructure, sizeof(PilotOutfitSlot) );
   pilot->outfit_nutility  = ship->outfit_nutility;
   pilot->outfit_utility   = calloc( ship->outfit_nutility, sizeof(PilotOutfitSlot) );
   pilot->outfit_nweapon   = ship->outfit_nweapon;
   pilot->outfit_weapon    = calloc( ship->outfit_nweapon, sizeof(PilotOutfitSlot) );
   /* Global. */
   pilot->noutfits = pilot->outfit_nstructure + pilot->outfit_nutility + pilot->outfit_nweapon;
   pilot->outfits  = calloc( pilot->noutfits, sizeof(PilotOutfitSlot*) );
   /* First pass copy data. */
   p = 0;
   for (i=0; i<pilot->outfit_nstructure; i++) {
      pilot->outfits[p] = &pilot->outfit_structure[i];
      pilot->outfits[p]->sslot = &ship->outfit_structure[i];
      if (ship->outfit_structure[i].data != NULL)
         pilot_addOutfitRaw( pilot, ship->outfit_structure[i].data, pilot->outfits[p] );
      p++;
   }
   for (i=0; i<pilot->outfit_nutility; i++) {
      pilot->outfits[p] = &pilot->outfit_utility[i];
      pilot->outfits[p]->sslot = &ship->outfit_utility[i];
      if (ship->outfit_utility[i].data != NULL)
         pilot_addOutfitRaw( pilot, ship->outfit_utility[i].data, pilot->outfits[p] );
      p++;
   }
   for (i=0; i<pilot->outfit_nweapon; i++) {
      pilot->outfits[p] = &pilot->outfit_weapon[i];
      pilot->outfits[p]->sslot = &ship->outfit_weapon[i];
      if (ship->outfit_weapon[i].data != NULL)
         pilot_addOutfitRaw( pilot, ship->outfit_weapon[i].data, pilot->outfits[p] );
      p++;
   }
   /* Second pass set ID. */
   for (i=0; i<pilot->noutfits; i++)
      pilot->outfits[i]->id = i;

   /* cargo - must be set before calcStats */
   pilot->cargo_free = pilot->ship->cap_cargo; /* should get redone with calcCargo */

   /* Initialize heat. */
   pilot_heatReset( pilot );

   /* set the pilot stats based on their ship and outfits */
   pilot_calcStats( pilot );

   /* Heal up the ship. */
   pilot->armour = pilot->armour_max;
   pilot->shield = pilot->shield_max;
   pilot->energy = pilot->energy_max;
   pilot->fuel   = pilot->fuel_max;

   /* Sanity check. */
#ifdef DEBUGGING
   const char *str = pilot_checkSpaceworthy( pilot );
   if (str != NULL)
      DEBUG( "Pilot '%s' failed sanity check: %s", pilot->name, str );
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

   /* Check takeoff. */
   if (pilot_isFlagRaw( flags, PILOT_TAKEOFF )) {
      pilot->ptimer = PILOT_TAKEOFF_DELAY;
   }

   /* AI */
   if (ai != NULL)
      ai_pinit( pilot, ai ); /* Must run before ai_create */
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
      const PilotFlags flags, const int systemFleet )
{
   Pilot *dyn;

   /* Allocate pilot memory. */
   dyn = malloc(sizeof(Pilot));
   if (dyn == NULL) {
      WARN("Unable to allocate memory");
      return 0;
   }

   /* See if memory needs to grow */
   if (pilot_nstack+1 > pilot_mstack) { /* needs to grow */
      if (pilot_mstack == 0)
         pilot_mstack = PILOT_CHUNK_MIN;
      else
         pilot_mstack += MIN( pilot_mstack, PILOT_CHUNK_MAX );
      pilot_stack = realloc( pilot_stack, pilot_mstack*sizeof(Pilot*) );
   }

   /* Set the pilot in the stack -- must be there before initializing */
   pilot_stack[pilot_nstack] = dyn;
   pilot_nstack++; /* there's a new pilot */

   /* Initialize the pilot. */
   pilot_init( dyn, ship, name, faction, ai, dir, pos, vel, flags, systemFleet );

   return dyn->id;
}


/**
 * @brief Creates a pilot without adding it to the stack.
 *
 *    @param ship Ship for the pilot to use.
 *    @param name Name of the pilot ship (NULL uses ship name).
 *    @param faction Faction of the ship.
 *    @param ai AI to use.
 *    @param flags Flags for tweaking, PILOT_EMPTY is added.
 *    @return Pointer to the new pilot (not added to stack).
 */
Pilot* pilot_createEmpty( Ship* ship, const char* name,
      int faction, const char *ai, PilotFlags flags )
{
   Pilot* dyn;
   dyn = malloc(sizeof(Pilot));
   if (dyn == NULL) {
      WARN("Unable to allocate memory");
      return 0;
   }
   pilot_setFlagRaw( flags, PILOT_EMPTY );
   pilot_init( dyn, ship, name, faction, ai, 0., NULL, NULL, flags, -1 );
   return dyn;
}


/**
 * @brief Copies src pilot to dest.
 *
 *    @param src Pilot to copy.
 *    @return Copy of src.
 */
Pilot* pilot_copy( Pilot* src )
{
   int i, p;
   Pilot *dest = malloc(sizeof(Pilot));

   /* Copy data over, we'll have to reset all the pointers though. */
   *dest = *src;

   /* Copy names. */
   if (src->name)
      dest->name = strdup(src->name);
   if (src->title)
      dest->title = strdup(src->title);

   /* Copy solid. */
   dest->solid = malloc(sizeof(Solid));
   *dest->solid = *src->solid;

   /* Copy outfits. */
   dest->noutfits = src->noutfits;
   dest->outfits  = malloc( sizeof(PilotOutfitSlot*) * dest->noutfits );
   dest->outfit_nstructure = src->outfit_nstructure;
   dest->outfit_structure  = malloc( sizeof(PilotOutfitSlot) * dest->outfit_nstructure );
   memcpy( dest->outfit_structure, src->outfit_structure,
         sizeof(PilotOutfitSlot) * dest->outfit_nstructure );
   dest->outfit_nutility = src->outfit_nutility;
   dest->outfit_utility  = malloc( sizeof(PilotOutfitSlot) * dest->outfit_nutility );
   memcpy( dest->outfit_utility, src->outfit_utility,
         sizeof(PilotOutfitSlot) * dest->outfit_nutility );
   dest->outfit_nweapon = src->outfit_nweapon;
   dest->outfit_weapon  = malloc( sizeof(PilotOutfitSlot) * dest->outfit_nweapon );
   memcpy( dest->outfit_weapon, src->outfit_weapon,
         sizeof(PilotOutfitSlot) * dest->outfit_nweapon );
   p = 0;
   for (i=0; i<dest->outfit_nstructure; i++)
      dest->outfits[p++] = &dest->outfit_structure[i];
   for (i=0; i<dest->outfit_nutility; i++)
      dest->outfits[p++] = &dest->outfit_utility[i];
   for (i=0; i<dest->outfit_nweapon; i++)
      dest->outfits[p++] = &dest->outfit_weapon[i];
   dest->afterburner = NULL;

   /* Hooks get cleared. */
   dest->hooks          = NULL;
   dest->nhooks         = 0;

   /* Copy has no escorts. */
   dest->escorts        = NULL;
   dest->nescorts       = 0;

   /* AI is not copied. */
   dest->task           = NULL;

   /* Set pointers and friends to NULL. */
   /* Commodities. */
   dest->commodities    = NULL;
   dest->ncommodities   = 0;
   /* Calculate stats. */
   pilot_calcStats(dest);

   /* Copy commodities. */
   for (i=0; i<src->ncommodities; i++)
      pilot_cargoAdd( dest, src->commodities[i].commodity,
            src->commodities[i].quantity, src->commodities[i].id );

   return dest;
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

   /* Free weapon sets. */
   pilot_weapSetFree(p);

   /* Free outfits. */
   if (p->outfits != NULL)
      free(p->outfits);
   if (p->outfit_structure != NULL)
      free(p->outfit_structure);
   if (p->outfit_utility != NULL)
      free(p->outfit_utility);
   if (p->outfit_weapon != NULL)
      free(p->outfit_weapon);

   /* Remove commodities. */
   while (p->commodities != NULL)
      pilot_cargoRmRaw( p, p->commodities[0].commodity,
            p->commodities[0].quantity, 1 );

   /* Clean up data. */
   if (p->ai != NULL)
      ai_destroy(p); /* Must be destroyed first if applicable. */

   /* Free name and title. */
   if (p->name != NULL)
      free(p->name);
   if (p->title != NULL)
      free(p->title);
   /* Case if pilot is the player. */
   if (player.p==p)
      player.p = NULL;
   solid_free(p->solid);
   if (p->mounted != NULL)
      free(p->mounted);

   /* Free escorts. */
   for (i=0; i<p->nescorts; i++)
      free(p->escorts[i].ship);
   if (p->escorts)
      free(p->escorts);

   /* Free comm message. */
   if (p->comm_msg != NULL)
      free(p->comm_msg);

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

   /* find the pilot */
   for (i=0; i < pilot_nstack; i++)
      if (pilot_stack[i]==p)
         break;

   /* Remove faction if necessary. */
   if (p->presence > 0) {
      system_rmCurrentPresence( cur_system, p->faction, p->presence );
      p->presence = 0;
   }

   /* pilot is eliminated */
   pilot_free(p);
   pilot_nstack--;

   /* copy other pilots down */
   memmove(&pilot_stack[i], &pilot_stack[i+1], (pilot_nstack-i)*sizeof(Pilot*));
}


/**
 * @brief Frees the pilot stack.
 */
void pilots_free (void)
{
   int i;

   pilot_freeGlobalHooks();

   /* Free pilots. */
   for (i=0; i < pilot_nstack; i++)
      pilot_free(pilot_stack[i]);
   free(pilot_stack);
   pilot_stack = NULL;
   player.p = NULL;
   pilot_nstack = 0;
}


/**
 * @brief Cleans up the pilot stack - leaves the player.
 */
void pilots_clean (void)
{
   int i;
   for (i=0; i < pilot_nstack; i++) {
      /* we'll set player.p at privileged position */
      if ((player.p != NULL) && (pilot_stack[i] == player.p)) {
         pilot_stack[0] = player.p;
         pilot_stack[0]->lockons = 0; /* Clear lockons. */
      }
      else /* rest get killed */
         pilot_free(pilot_stack[i]);
   }

   if (player.p != NULL) { /* set stack to 1 if pilot exists */
      pilot_nstack = 1;
      pilot_clearTimers( player.p ); /* Reset the player's timers. */
   }
   else
      pilot_nstack = 0;

   /* Clear global hooks. */
   pilots_clearGlobalHooks();
}


/**
 * @brief Clears all the pilots except the player.
 */
void pilots_clear (void)
{
   int i;
   for (i=0; i < pilot_nstack; i++)
      if (!pilot_isPlayer( pilot_stack[i] ))
         pilot_delete( pilot_stack[i] );
}


/**
 * @brief Even cleans up the player.
 */
void pilots_cleanAll (void)
{
   pilots_clean();
   if (player.p != NULL) {
      pilot_free(player.p);
      player.p = NULL;
   }
   pilot_nstack = 0;
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
   for (i=0; i<pilot_nstack; i++) {
      p = pilot_stack[i];

      /* Destroy pilot and go on. */
      if (pilot_isFlag(p, PILOT_DELETE)) {
         pilot_destroy(p);
         i--; /* Must decrement iterator. */
         continue;
      }

      /* Invisible, not doing anything. */
      if (pilot_isFlag(p, PILOT_INVISIBLE))
         continue;

      /* See if should think. */
      if ((p->think==NULL) || (p->ai==NULL))
         continue;
      if (pilot_isDisabled(p))
         continue;
      if (pilot_isFlag(p,PILOT_DEAD))
         continue;

      /* Hyperspace gets special treatment */
      if (pilot_isFlag(p, PILOT_HYP_PREP))
         pilot_hyperspace(p, dt);
      /* Entering hyperspace. */
      else if (pilot_isFlag(p, PILOT_HYP_END)) {
         if (VMOD(p->solid->vel) < 2*solid_maxspeed( p->solid, p->speed, p->thrust) )
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
   for (i=0; i<pilot_nstack; i++) {
      p = pilot_stack[i];

      /* Ignore. */
      if (pilot_isFlag(p, PILOT_DELETE))
         continue;

      /* Invisible, not doing anything. */
      if (pilot_isFlag(p, PILOT_INVISIBLE))
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
   for (i=0; i<pilot_nstack; i++) {

      /* Invisible, not doing anything. */
      if (pilot_isFlag(pilot_stack[i], PILOT_INVISIBLE))
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
   for (i=0; i<pilot_nstack; i++) {

      /* Invisible, not doing anything. */
      if (pilot_isFlag(pilot_stack[i], PILOT_INVISIBLE))
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

   pilot->ptimer     = 0.; /* Pilot timer. */
   pilot->tcontrol   = 0.; /* AI control timer. */
   pilot->stimer     = 0.; /* Shield timer. */
   pilot->dtimer     = 0.; /* Disable timer. */
   for (i=0; i<MAX_AI_TIMERS; i++)
      pilot->timer[i] = 0.; /* Specific AI timers. */
   n = 0;
   for (i=0; i<pilot->noutfits; i++) {
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
 * @brief Updates the systemFleet of all pilots.
 *
 * @param index Index number that was deleted.
 */
void pilots_updateSystemFleet( const int deletedIndex )
{
   int i;

   for(i = 0; i < pilot_nstack; i++)
      if(pilot_stack[i]->systemFleet >= deletedIndex)
         pilot_stack[i]->systemFleet--;

   return;
}

/**
 * @brief Gets the relative size(shipmass) between the current pilot and the specified target
 *
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
 *    @param p the pilot whose dps we will compare
 *    @return A number from 0 to 1 mapping the relative damage output
 */
double pilot_reldps( const Pilot* cur_pilot, const Pilot* p )
{
   int i;
   int DPSaccum_target = 0, DPSaccum_pilot = 0;
   double delay_cache, damage_cache;
   Outfit *o;
   const Damage *dmg;

   for (i=0; i<p->outfit_nweapon; i++) {
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

   for (i=0; i<cur_pilot->outfit_nweapon; i++) {
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

   if ((DPSaccum_target > 0) && (DPSaccum_pilot > 0))
      return (1 - 1 / (1 + ((double)DPSaccum_pilot / (double)DPSaccum_target)) );
   else if (DPSaccum_pilot > 0)
      return 1;
   else
      return 0;
}

/**
 * @brief Gets the relative hp(combined shields and armour) between the current pilot and the specified target
 *
 *    @param p the pilot whose shields/armour we will compare
 *    @return A number from 0 to 1 mapping the relative HPs
 */
double pilot_relhp( const Pilot* cur_pilot, const Pilot* p )
{
   return (1 - 1 / (1 + ((double)(cur_pilot -> armour_max + cur_pilot -> shield_max) /
         (double)(p -> armour_max + p -> shield_max))));
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
   for (i=0; i<p->noutfits; i++) {
      if (p->outfits[i]->outfit == NULL)
         continue;
      price += p->outfits[i]->outfit->price;
   }

   return price;
}




