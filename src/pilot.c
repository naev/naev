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
#include "gatherable.h"
#include "gui.h"
#include "hook.h"
#include "land.h"
#include "land_outfits.h"
#include "land_shipyard.h"
#include "log.h"
#include "map.h"
#include "music.h"
#include "nlua_pilotoutfit.h"
#include "nlua_vec2.h"
#include "nlua_outfit.h"
#include "ndata.h"
#include "nstring.h"
#include "ntime.h"
#include "nxml.h"
#include "pause.h"
#include "player.h"
#include "player_autonav.h"
#include "pilot_ship.h"
#include "quadtree.h"
#include "rng.h"
#include "weapon.h"

#define PILOT_SIZE_MIN 128 /**< Minimum chunks to increment pilot_stack by */

/* ID Generators. */
static unsigned int pilot_id = PLAYER_ID; /**< Stack of pilot ids to assure uniqueness */

/* stack of pilots */
static Pilot** pilot_stack = NULL; /**< All the pilots in space. (Player may have other Pilot objects, e.g. backup ships.) */
static Quadtree pilot_quadtree; /**< Quadtree for the pilots. */
static IntList pilot_qtquery; /**< Quadtree query. */
static int qt_init = 0;
/* A simple grid search procedure was used to determine the following parameters. */
static int qt_max_elem = 2;
static int qt_depth = 5;

/* misc */
static const double pilot_commTimeout  = 15.; /**< Time for text above pilot to time out. */
static const double pilot_commFade     = 5.; /**< Time for text above pilot to fade out. */

/*
 * Prototypes
 */
/* Create. */
static void pilot_init( Pilot* dest, const Ship* ship, const char* name, int faction,
      const double dir, const vec2* pos, const vec2* vel,
      const PilotFlags flags, unsigned int dockpilot, int dockslot );
/* Update. */
static void pilot_hyperspace( Pilot* pilot, double dt );
static void pilot_refuel( Pilot *p, double dt );
/* Clean up. */
static void pilot_erase( Pilot *p );
/* Misc. */
static void pilot_renderFramebufferBase( Pilot *p, GLuint fbo, double fw, double fh );
static int pilot_getStackPos( unsigned int id );
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
static int pilot_cmp( const void *ptr1, const void *ptr2 )
{
   const Pilot *p1, *p2;
   p1 = *((const Pilot**) ptr1);
   p2 = *((const Pilot**) ptr2);
   return p1->id - p2->id;
}

/**
 * @brief Gets the pilot's position in the stack.
 *
 *    @param id ID of the pilot to get.
 *    @return Position of pilot in stack or -1 if not found.
 */
static int pilot_getStackPos( unsigned int id )
{
   const Pilot pid = { .id = id };
   const Pilot *pidptr = &pid;
   /* binary search */
   Pilot **pp = bsearch(&pidptr, pilot_stack, array_size(pilot_stack), sizeof(Pilot*), pilot_cmp);
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
unsigned int pilot_getNextID( unsigned int id, int mode )
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
unsigned int pilot_getPrevID( unsigned int id, int mode )
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
   /* Must be targetable. */
   if (!pilot_canTarget( target ))
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
int pilot_validEnemy( const Pilot* p, const Pilot* target )
{
   return pilot_validEnemyDist( p, target, NULL );
}

/**
 * @brief Same as pilot_validEnemy, but able to store the distance too.
 */
int pilot_validEnemyDist( const Pilot* p, const Pilot* target, double *dist )
{
   /* Shouldn't be disabled. */
   if (pilot_isDisabled(target))
      return 0;

   /* Shouldn't be invincible. */
   if (pilot_isFlag( target, PILOT_INVINCIBLE ))
      return 0;

   /* Shouldn't be landing or taking off. */
   if (pilot_isFlag( target, PILOT_LANDING) ||
         pilot_isFlag( target, PILOT_TAKEOFF ) ||
         pilot_isFlag( target, PILOT_NONTARGETABLE))
      return 0;

   /* Must be a valid target. */
   if (!pilot_validTarget( p, target ))
      return 0;

   /* Should either be hostile by faction or by player. */
   if (!pilot_areEnemies( p, target ))
      return 0;

   /* Must not be fuzzy. */
   if (pilot_inRangePilot( p, target, dist ) != 1)
      return 0;

   /* They're ok. */
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
   unsigned int tp = 0;
   double d = 0.;

   for (int i=0; i<array_size(pilot_stack); i++) {
      double td;

      if (!pilot_validEnemy( p, pilot_stack[i] ))
         continue;

      /* Check distance. */
      td = vec2_dist2(&pilot_stack[i]->solid.pos, &p->solid.pos);
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
   unsigned int tp = 0;
   double d = 0.;

   for (int i=0; i<array_size(pilot_stack); i++) {
      double td;

      if (!pilot_validEnemy( p, pilot_stack[i] ))
         continue;

      if (pilot_stack[i]->solid.mass < target_mass_LB || pilot_stack[i]->solid.mass > target_mass_UB)
         continue;

      /* Check distance. */
      td = vec2_dist2(&pilot_stack[i]->solid.pos, &p->solid.pos);
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
   unsigned int tp = 0;
   double current_heuristic_value = 10e3;

   for (int i=0; i<array_size(pilot_stack); i++) {
      double temp;
      Pilot *target = pilot_stack[i];

      if (!pilot_validEnemy( p, target ))
         continue;

      /* Check distance. */
      temp = range_factor *
               vec2_dist2( &target->solid.pos, &p->solid.pos )
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
   pilot_getNearestPos( p, &t, p->solid.pos.x, p->solid.pos.y, 0 );
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
   double relpower, ppower;
   /* TODO : all the parameters should be adjustable with arguments */

   relpower = 0;
   t = 0;

   /* Initialized to 0.25 which would mean equivalent power. */
   ppower = 0.5*0.5;

   for (int i=0; i<array_size(pilot_stack); i++) {
      double dx, dy, td, curpower;

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
      dx = pilot_stack[i]->solid.pos.x + 2*pilot_stack[i]->solid.vel.x -
           p->solid.pos.x - 2*p->solid.vel.x;
      dy = pilot_stack[i]->solid.pos.y + 2*pilot_stack[i]->solid.vel.y -
           p->solid.pos.y - 2*p->solid.vel.y;
      td = sqrt( pow2(dx) + pow2(dy) );
      if (td > 5e3)
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
   double d = 0.;

   *tp = PLAYER_ID;
   for (int i=0; i<array_size(pilot_stack); i++) {
      double td;

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
      td = pow2(x-pilot_stack[i]->solid.pos.x) + pow2(y-pilot_stack[i]->solid.pos.y);
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
   double a = ang + M_PI;

   *tp = PLAYER_ID;
   for (int i=0; i<array_size(pilot_stack); i++) {
      double rx, ry, ta;

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

      ta = atan2( p->solid.pos.y - pilot_stack[i]->solid.pos.y,
            p->solid.pos.x - pilot_stack[i]->solid.pos.x );
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
 *  abused all the time. Maximum iterations is 32 on a platform with 32 bit
 *  unsigned ints.
 *
 *    @param id ID of the pilot to get.
 *    @return The actual pilot who has matching ID or NULL if not found.
 */
Pilot* pilot_get( unsigned int id )
{
   const Pilot pid = { .id = id };
   const Pilot *pidptr = &pid;
   /* binary search */
   Pilot **pp = bsearch(&pidptr, pilot_stack, array_size(pilot_stack), sizeof(Pilot*), pilot_cmp);
   if ((pp==NULL) || (pilot_isFlag(*pp, PILOT_DELETE)))
      return NULL;
   return *pp;
}

/**
 * @brief Gets the target of a pilot using a fancy caching system.
 */
Pilot* pilot_getTarget( Pilot *p )
{
   Pilot *t;

   /* Case no target. */
   if (p->target==p->id)
      return NULL;

   t = p->ptarget;
   /* Return ptarget if exists and valid. */
   if (t != NULL) {
      if (pilot_isFlag( t, PILOT_DELETE )) {
         p->ptarget = NULL;
         t = NULL;
      }
      return t;
   }

   p->ptarget = pilot_get( p->target );
   return p->ptarget;
}

/**
 * @brief Sets the pilot's thrust.
 */
void pilot_setThrust( Pilot *p, double thrust )
{
   p->solid.thrust = p->thrust * thrust;
}

/**
 * @brief Sets the pilot's turn.
 */
void pilot_setTurn( Pilot *p, double turn )
{
   p->solid.dir_vel = p->turn * turn;
}

/**
 * @brief Checks to see if pilot is hostile to the player.
 *
 *    @param p Player to see if is hostile.
 *    @return 1 if pilot is hostile to the player.
 */
int pilot_isHostile( const Pilot *p )
{
   if (!pilot_isFriendly( p )
         && !pilot_isFlag( p, PILOT_BRIBED )
         && (pilot_isFlag( p, PILOT_HOSTILE ) ||
            areEnemies( FACTION_PLAYER, p->faction )))
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
 * @brief Like areAllies but for pilots.
 */
int pilot_areAllies( const Pilot *p, const Pilot *target )
{
   if (pilot_isWithPlayer(p)) {
      if (pilot_isFriendly(target))
         return 1;
      else if (pilot_isFlag( target, PILOT_HOSTILE ))
         return 0;
   }
   else if (pilot_isWithPlayer(target)) {
      if (pilot_isFriendly(p))
         return 1;
      else if (pilot_isFlag( p, PILOT_HOSTILE ))
         return 0;
   }
   else {
      if (areAllies( p->faction, target->faction ))
         return 1;
   }
   return 0;
}

/**
 * @brief Like areEnemies but for pilots.
 */
int pilot_areEnemies( const Pilot *p, const Pilot *target )
{
   if (pilot_isWithPlayer(p)) {
      if (pilot_isHostile(target))
         return 1;
      else if (pilot_isFlag( target, PILOT_FRIENDLY ))
         return 0;
      else if (pilot_isFlag(target, PILOT_BRIBED))
         return 0;
   }
   if (pilot_isWithPlayer(target)) {
      if (pilot_isHostile(p))
         return 1;
      else if (pilot_isFlag( p, PILOT_FRIENDLY ))
         return 0;
      else if (pilot_isFlag(p, PILOT_BRIBED))
         return 0;
   }
   else {
      if (areEnemies( p->faction, target->faction ))
         return 1;
   }
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
   if ((p->dockpilot != 0) && (p->dockslot != -1)) {
      Pilot *dockpilot = pilot_get(p->dockpilot);
      if (dockpilot != NULL)
         return dockpilot->outfits[p->dockslot];
   }
   p->dockpilot = 0;
   p->dockslot = -1;
   return NULL;
}

const IntList *pilot_collideQuery( int x1, int y1, int x2, int y2 )
{
   qt_query( &pilot_quadtree, &pilot_qtquery, x1, y1, x2, y2 );
   return &pilot_qtquery;
}

void pilot_collideQueryIL( IntList *il, int x1, int y1, int x2, int y2 )
{
   qt_query( &pilot_quadtree, il, x1, y1, x2, y2 );
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
   double diff = angle_diff( p->solid.dir, dir );
   double turn = CLAMP( -1., 1., -10.*diff );
   pilot_setTurn( p, -turn );

   return diff;
}

/**
 * @brief See if the pilot wants to use their reverse thrusters to brake.
 */
int pilot_brakeCheckReverseThrusters( const Pilot *p )
{
   if (p->stats.misc_reverse_thrust) {
      double diff, btime, ftime, vel, t;
      vel = MIN( VMOD(p->solid.vel), p->speed );
      t = vel / p->thrust * p->solid.mass;

      /* Calculate the time to face backward and apply forward thrust. */
      diff = angle_diff(p->solid.dir, VANGLE(p->solid.vel) + M_PI);
      btime = ABS(diff) / p->turn + t;

      /* Calculate the time to face forward and apply reverse thrust. */
      diff = angle_diff(p->solid.dir, VANGLE(p->solid.vel));
      ftime = ABS(diff) / p->turn + t / PILOT_REVERSE_THRUST;

      if (btime > ftime)
         return 1;
   }
   return 0;
}

/**
 * @brief Gets the minimum braking distance for the pilot.
 */
double pilot_minbrakedist( const Pilot *p )
{
   double vel = MIN( VMOD(p->solid.vel), p->speed );
   double thrust = p->thrust / p->solid.mass;
   double t = vel / thrust;
   if (pilot_brakeCheckReverseThrusters(p))
      return vel * t - 0.5 * PILOT_REVERSE_THRUST * thrust * t * t;
   return vel*(t+1.1*M_PI/p->turn) - 0.5 * thrust * t * t;
}

/**
 * @brief Causes the pilot to turn around and brake.
 *
 *    @param p Pilot to brake.
 *    @return 1 when braking has finished.
 */
int pilot_brake( Pilot *p )
{
   double dir, thrust, diff;
   int isstopped = pilot_isStopped(p);

   if (isstopped)
      return 1;

   if (pilot_brakeCheckReverseThrusters(p)) {
      dir    = VANGLE(p->solid.vel);
      thrust = -PILOT_REVERSE_THRUST;
   }
   else {
      dir    = VANGLE(p->solid.vel) + M_PI;
      thrust = 1.;
   }

   diff = pilot_face(p, dir);
   if (ABS(diff) < MAX_DIR_ERR && !isstopped)
      pilot_setThrust(p, thrust);
   else {
      pilot_setThrust(p, 0.);
      if (isstopped)
         return 1;
   }
   return 0;
}

/**
 * @brief Begins active cooldown, reducing hull and outfit temperatures.
 *
 *    @param p Pilot that should cool down.
 *    @param dochecks Whether or not to do the standard checks or cooling down automatically.
 */
void pilot_cooldown( Pilot *p, int dochecks )
{
   double heat_capacity, heat_mean;

   /* Brake if necessary. */
   if (dochecks && !pilot_isStopped(p)) {
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
   for (int i=0; i<array_size(p->outfits); i++) {
      PilotOutfitSlot *o = p->outfits[i];
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
         (1. + pow(MAX(heat_mean / CONST_SPACE_STAR_TEMP - 1.,0.), 1.25));
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
         player_message("#o%s",_("Active cooldown completed."));
      else {
         if (reason != NULL)
            player_message(_("#rActive cooldown aborted: %s!"), reason);
         else
            player_message("#r%s",_("Active cooldown aborted!"));
      }
      gui_cooldownEnd();
   }

   pilot_rmFlag(p, PILOT_COOLDOWN);

   /* Cooldown finished naturally, reset heat just in case. */
   if (p->ctimer < 0.) {
      pilot_heatReset( p );
      pilot_fillAmmo( p );
      pilot_outfitLCooldown(p, 1, 1, 0.);
   }
   else {
      pilot_outfitLCooldown(p, 1, 0, 0.);
   }
}

/**
 * @brief Returns the angle for a pilot to aim at another pilot
 *
 *    @param p Pilot that aims.
 *    @param pos Posiion of the target being aimed at.
 *    @param vel Velocity of the target being aimed at.
 */
double pilot_aimAngle( Pilot *p, const vec2* pos, const vec2* vel )
{
   double x, y;
   double t;
   vec2 tv, approach_vector, relative_location, orthoradial_vector;
   double dist;
   double speed;
   double radial_speed;
   double orthoradial_speed;

   /* Get the distance */
   dist = vec2_dist( &p->solid.pos, pos );

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
   vec2_cset(&approach_vector, VX(p->solid.vel) - VX(*vel), VY(p->solid.vel) - VY(*vel) );
   vec2_cset(&relative_location, VX(*pos) -  VX(p->solid.pos),  VY(*pos) - VY(p->solid.pos) );
   vec2_cset(&orthoradial_vector, VY(p->solid.pos) - VY(*pos), VX(*pos) -  VX(p->solid.pos) );

   radial_speed = vec2_dot(&approach_vector, &relative_location);
   radial_speed = radial_speed / VMOD(relative_location);

   orthoradial_speed = vec2_dot(&approach_vector, &orthoradial_vector);
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
   x = pos->x + vel->x*t - (p->solid.pos.x + p->solid.vel.x*t);
   y = pos->y + vel->y*t - (p->solid.pos.y + p->solid.vel.y*t);
   vec2_cset( &tv, x, y );

   return VANGLE(tv);
}

/**
 * @brief Marks pilot as hostile to player.
 *
 *    @param p Pilot to mark as hostile to player.
 */
void pilot_setHostile( Pilot* p )
{
   if (pilot_isFriendly( p ) || pilot_isFlag( p, PILOT_BRIBED )
         || !pilot_isFlag( p, PILOT_HOSTILE ))
      pilot_setFlag( p, PILOT_HOSTILE );
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
      return 'F';
   else if (pilot_isHostile(p))
      return 'H';
   else if (pilot_isFlag(p, PILOT_BRIBED))
      return 'N';
   return faction_getColourChar(p->faction);
}

/**
 * @brief Sets the overhead communication message of the pilot.
 */
void pilot_setCommMsg( Pilot *p, const char *s )
{
   free( p->comm_msg );
   p->comm_msg       = strdup( s );
   p->comm_msgWidth  = gl_printWidthRaw( NULL, s );
   p->comm_msgTimer  = pilot_commTimeout;
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
 */
void pilot_distress( Pilot *p, Pilot *attacker, const char *msg )
{
   int r;
   double d;

   /* Broadcast the message. */
   if (msg[0] != '\0')
      pilot_broadcast( p, msg, 0 );

   /* Use the victim's target if the attacker is unknown. */
   if (attacker == NULL)
      attacker = pilot_getTarget( p );

   /* Now proceed to see if player.p should incur faction loss because
    * of the broadcast signal. */

   /* Consider not in range at first. */
   r = 0;

   if (!pilot_isFlag(p, PILOT_DISTRESSED)) {
      /* Check if spob is in range. */
      for (int i=0; i<array_size(cur_system->spobs); i++) {
         if (spob_hasService(cur_system->spobs[i], SPOB_SERVICE_INHABITED) &&
               pilot_inRangeSpob(p, i) &&
               !areEnemies(p->faction, cur_system->spobs[i]->presence.faction)) {
            r = 1;
            break;
         }
      }
   }

   /* Now we must check to see if a pilot is in range. */
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *pi = pilot_stack[i];

      /* Skip if unsuitable. */
      if ((pi->ai == NULL) || (pi->id == p->id) ||
            (pilot_isFlag(pi, PILOT_DEAD)) ||
            (pilot_isFlag(pi, PILOT_DELETE)))
         continue;

      if (!pilot_inRangePilot(p, pi, NULL)) {
         /*
            * If the pilots are within sensor range of each other, send the
            * distress signal, regardless of electronic warfare hide values.
            */
         d = vec2_dist2( &p->solid.pos, &pi->solid.pos );
         if (d > pilot_sensorRange())
            continue;
      }

      /* Send AI the distress signal. */
      ai_getDistress( pi, p, attacker );

      /* Check if should take faction hit. */
      if (!pilot_isFlag(p, PILOT_DISTRESSED) &&
            !areEnemies(p->faction, pi->faction))
         r = 1;
   }

   /* Player only gets one faction hit per pilot. */
   if (!pilot_isFlag(p, PILOT_DISTRESSED)) {

      /* Modify faction, about 1 for a llama, 4.2 for a hawking */
      if (r && (attacker != NULL) && (pilot_isWithPlayer(attacker))) {
         if (p->ai == NULL)
            WARN(_("Pilot '%s' does not have an AI!"), p->name);
         else {
            double hit;
            lua_rawgeti( naevL, LUA_REGISTRYINDEX, p->lua_mem ); /* m */
            lua_getfield( naevL, -1, "distress_hit" );/* m, v */
            if (lua_isnil(naevL,-1))
               hit = (pow(p->base_mass, 0.2) - 1.);
            else if (lua_isnumber(naevL,-1))
               hit = lua_tonumber(naevL,-1);
            else {
               WARN(_("Pilot '%s' has non-number mem.distress_hit!"),p->name);
               hit = 0.;
            }
            lua_pop(naevL,2);
            faction_modPlayer( p->faction, -hit, "distress" );
         }
      }

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
   if (!pilot_isHostile(p))
      return;

   if (pilot_isFlag(p, PILOT_HOSTILE))
      pilot_rmFlag(p, PILOT_HOSTILE);

   /* Set "bribed" flag if faction has poor reputation */
   if (areEnemies( FACTION_PLAYER, p->faction ))
      pilot_setFlag(p, PILOT_BRIBED);
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

   if (pilot_inRangePilot(player.p, p, NULL) == -1)
      col = &cNeutral;
   else if (pilot_isDisabled(p) || pilot_isFlag(p,PILOT_DEAD))
      col = &cInert;
   else if (pilot_isFriendly(p))
      col = &cFriend;
   else if (pilot_isHostile(p))
      col = &cHostile;
   else
      col = &cNeutral;

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
   p->ptarget = NULL; /* Gets recomputed later in pilot_getTarget. */
   pilot_lockClear( p );

   /* Set the scan timer. */
   pilot_ewScanStart( p );

   /* Untarget asteroid (if any). */
   p->nav_anchor   = -1;
   p->nav_asteroid = -1;
}

/**
 * @brief Damages the pilot.
 *
 *    @param p Pilot that is taking damage.
 *    @param w Solid that is hitting pilot.
 *    @param pshooter Attacker that shot the pilot.
 *    @param dmg Damage being done.
 *    @param outfit Outfit doing the damage if applicable.
 *    @param lua_mem Lua reference to the Pilot Outfit's mem table, if applicable.
 *    @param reset Whether the shield timer should be reset.
 *    @return The real damage done.
 */
double pilot_hit( Pilot* p, const Solid* w, const Pilot *pshooter,
      const Damage *dmg, const Outfit *outfit, int lua_mem, int reset )
{
   int mod, shooter;
   double damage_shield, damage_armour, disable, knockback, dam_mod, ddmg, ddis, absorb, dmod, start;
   double tdshield, tdarmour;

   /* Invincible means no damage. */
   if (pilot_isFlag( p, PILOT_INVINCIBLE ) ||
         pilot_isFlag( p, PILOT_HIDE ) ||
         ((pshooter!=NULL) && pilot_isWithPlayer(pshooter) && pilot_isFlag( p, PILOT_INVINC_PLAYER )))
      return 0.;

   /* Defaults. */
   dam_mod        = 0.;
   ddmg           = 0.;
   ddis           = 0.;
   shooter        = (pshooter==NULL) ? 0 : pshooter->id;

   /* Calculate the damage. */
   absorb         = 1. - CLAMP( 0., 1., p->dmg_absorb - dmg->penetration );
   disable        = dmg->disable;
   dtype_calcDamage( &damage_shield, &damage_armour, absorb, &knockback, dmg, &p->stats );

   /*
    * Delay undisable if necessary. Amount varies with damage, as e.g. a
    * single Laser Cannon shot should not reset a Peacemaker's timer.
    */
   if (!pilot_isFlag(p, PILOT_DEAD) && (p->dtimer_accum > 0.))
      p->dtimer_accum -= MIN( pow(disable, 0.8), p->dtimer_accum );

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
      ddis = disable * (0.5 + (0.5 - ((start+p->shield) / p->shield_max) / 4.));
      p->stress += ddis;

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
      ddis = disable * (1. - dmod) * (0.5 + (0.5 - (start / p->shield_max / 4.)));
      p->stress += ddis;

      /* Reduce stress as armour is eaten away. */
      //p->stress  *= (p->armour - dmod * damage_armour) / p->armour;
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
      //p->stress  *= (p->armour - damage_armour) / p->armour;
      p->armour  -= damage_armour;
      ddis = disable;
      p->stress  += ddis;

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

   /* Can't undisable permanently disabled pilots through shooting. */
   if (pilot_isFlag( p, PILOT_DISABLED_PERM ))
      p->stress = p->armour;

   /* Some minor effects and stuff. */
   if (p->shield <= 0.) {
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
      player.ps.dmg_taken_shield += tdshield;
      player.ps.dmg_taken_armour += tdarmour;
   }
   /* TODO we might want to actually resolve shooter and check for
    * FACTION_PLAYER so that escorts also get counted... */
   else if (shooter == PLAYER_ID) {
      player.dmg_done_shield += tdshield;
      player.dmg_done_armour += tdarmour;
      player.ps.dmg_done_shield += tdshield;
      player.ps.dmg_done_armour += tdarmour;
   }

   if (w != NULL)
      /* knock back effect is dependent on both damage and mass of the weapon
       * should probably get turned into a partial conservative collision */
      vec2_cadd( &p->solid.vel,
            knockback * (w->vel.x * (dam_mod/9. + w->mass/p->solid.mass/6.)),
            knockback * (w->vel.y * (dam_mod/9. + w->mass/p->solid.mass/6.)) );

   /* On hit weapon effects. */
   if ((outfit!=NULL) && (outfit->lua_onimpact != LUA_NOREF)) {
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, lua_mem); /* mem */
      nlua_setenv(naevL, outfit->lua_env, "mem"); /* */

      /* Set up the function: onimpact( pshooter, p ) */
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, outfit->lua_onimpact); /* f */
      lua_pushpilot(naevL, shooter); /* f, p */
      lua_pushpilot(naevL, p->id);   /* f, p, p  */
      lua_pushvector(naevL, w->pos); /* f, p, p, x */
      lua_pushvector(naevL, w->vel); /* f, p, p, x, v */
      lua_pushoutfit(naevL, outfit); /* f, p, p, x, v, o */
      if (nlua_pcall( outfit->lua_env, 5, 0 )) {   /* */
         WARN( _("Pilot '%s''s outfit '%s' -> '%s':\n%s"), p->name, outfit->name, "onimpact", lua_tostring(naevL,-1) );
         lua_pop(naevL, 1);
      }
   }

   /* On hit Lua outfits activate. */
   pilot_outfitLOnhit( p, tdarmour, tdshield, shooter );

   /* Run disabled before death. Should be run after on hit effects in case of regen. */
   pilot_updateDisable(p, shooter);

   /* Officially dead, run after in case they are regenerated by outfit. */
   if (p->armour <= 0.) {
      p->armour = 0.;

      if (!pilot_isFlag(p, PILOT_DEAD)) {
         pilot_dead( p, shooter );

         /* adjust the combat rating based on pilot mass and ditto faction */
         if ((pshooter != NULL) && pilot_isWithPlayer(pshooter)) {

            /* About 6 for a llama, 52 for hawking. */
            mod = 2. * (pow(p->base_mass, 0.4) - 1.);

            /* Modify faction for him and friends. */
            faction_modPlayer( p->faction, -mod, "kill" );

            /* Note that player destroyed the ship. */
            player.ships_destroyed[p->ship->class]++;
            player.ps.ships_destroyed[p->ship->class]++;
         }
      }
   }

   return ddmg + ddis;
}

/**
 * @brief Handles pilot disabling. Set or unset the disable status depending on health and stress values.
 *
 *    @param p The pilot in question.
 *    @param shooter Attacker that shot the pilot.
 */
void pilot_updateDisable( Pilot* p, unsigned int shooter )
{
   if ((!pilot_isFlag(p, PILOT_DISABLED)) &&
       (!pilot_isFlag(p, PILOT_NODISABLE) || (p->armour <= 0.)) &&
       (p->armour <= p->stress)) { /* Pilot should be disabled. */
      HookParam hparam;

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

      /* Disabled ships don't use up presence. */
      if (p->presence > 0) {
         system_rmCurrentPresence( cur_system, p->faction, p->presence );
         p->presence = 0;
      }

      /* Set disable timer. This is the time the pilot will remain disabled. */
      /* 200 mass llama       => 46.78 s
       * 8000 mass peacemaker => 156 s
       */
      p->dtimer = 8. * pow( p->solid.mass, 1./3. );
      p->dtimer_accum = 0.;

      /* Disable active outfits. */
      if (pilot_outfitOffAll( p ) > 0)
         pilot_calcStats( p );

      pilot_setFlag( p, PILOT_DISABLED ); /* set as disabled */
      if (pilot_isPlayer( p ))
         player_message("#r%s",_("You have been disabled!"));

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

      /* Reset the accumulated disable time. */
      p->dtimer_accum = 0.;

      /* TODO: Make undisabled pilot use up presence again. */
      pilot_runHook( p, PILOT_HOOK_UNDISABLE );

      /* This is sort of a hack to make sure it gets reset... */
      if (pilot_isPlayer(p)) {
         player_autonavResetSpeed();
         player_message("#g%s",_("You have recovered control of your ship!"));
      }
   }
}

/**
 * @brief Pilot is dead, now will slowly explode.
 *
 *    @param p Pilot that just died.
 *    @param killer Pilot killer or 0 if invalid.
 */
void pilot_dead( Pilot* p, unsigned int killer )
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
   if (p->armour <= 0.) {
      if (p->parent == PLAYER_ID)
         player_message( _("#rShip under command '%s' was destroyed!#0"), p->name );
      /* PILOT R OFFICIALLY DEADZ0R */
      pilot_setFlag( p, PILOT_DEAD );

      /* Run Lua if applicable. */
      pilot_shipLExplodeInit( p );
   }
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
   double dist, rad2;
   Solid s; /* Only need to manipulate mass and vel. */
   Damage ddmg;
   const IntList *qt;
   int qx, qy, qr;

   rad2 = radius*radius;
   ddmg = *dmg;

   qx = round(x);
   qy = round(y);
   qr = ceil(radius);
   qt = pilot_collideQuery( qx-qr, qy-qr, qx+qr, qy+qr );
   for (int i=0; i<il_size(qt); i++) {
      Pilot *p = pilot_stack[ il_get( qt, i, 0 ) ];
      double rx, ry;

      /* Calculate a bit. */
      rx = p->solid.pos.x - x;
      ry = p->solid.pos.y - y;
      dist = pow2(rx) + pow2(ry);
      /* Take into account ship size. */
      dist -= pow2(p->ship->gfx_space->sw);
      dist = MAX(0,dist);

      /* Pilot is not hit. */
      if (dist > rad2)
         continue;

      /* Adjust damage based on distance. */
      ddmg.damage = dmg->damage * (1. - sqrt(dist / rad2));

      /* Impact settings. */
      s.mass =  pow2(dmg->damage) / 30.;
      s.vel.x = rx;
      s.vel.y = ry;

      /* Actual damage calculations. */
      pilot_hit( p, &s, parent, &ddmg, NULL, LUA_NOREF, 1 );

      /* Shock wave from the explosion. */
      if (p->id == PILOT_PLAYER)
         spfx_shake( pow2(ddmg.damage) / pow2(100.) );
   }
}

/**
 * @brief Renders a pilot to a framebuffer without effects.
 *
 *    @param p Pilot to render.
 *    @param fbo Framebuffer to render to.
 *    @param fw Framebuffer width.
 *    @param fh Framebuffer height.
 */
static void pilot_renderFramebufferBase( Pilot *p, GLuint fbo, double fw, double fh )
{
   glColour c = { 1., 1., 1., 1. };

   /* Add some transparency if stealthed. TODO better effect */
   if (!pilot_isPlayer(p) && pilot_isFlag(p, PILOT_STEALTH))
      c.a = 0.5;

   glBindFramebuffer( GL_FRAMEBUFFER, fbo );
   glClearColor( 0., 0., 0., 0. );

   if (p->ship->gfx_3d != NULL) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      /* TODO fix 3D rendering. */
   }
   else {
      double tx,ty;
      const glTexture *sa, *sb;
      mat4 tmpm;

      glClear( GL_COLOR_BUFFER_BIT );

      sa = p->ship->gfx_space;
      sb = p->ship->gfx_engine;

      /* texture coords */
      tx = sa->sw*(double)(p->tsx)/sa->w;
      ty = sa->sh*(sa->sy-(double)p->tsy-1)/sa->h;

      tmpm = gl_view_matrix;
      gl_view_matrix = mat4_ortho( 0., fw, 0, fh, -1., 1. );

      gl_renderTextureInterpolate( sa, sb,
            1.-p->engine_glow, 0., 0., sa->sw, sa->sh,
            tx, ty, sa->srw, sa->srh, &c );

      gl_view_matrix = tmpm;
   }

   glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);
   glClearColor( 0., 0., 0., 1. );
}

/**
 * @brief Renders a pilot to a framebuffer.
 *
 * @TODO Reduce duplicate code with pilot_render.
 *
 *    @param p Pilot to render.
 *    @param fbo Framebuffer to render to.
 *    @param fw Framebuffer width.
 *    @param fh Framebuffer height.
 */
void pilot_renderFramebuffer( Pilot *p, GLuint fbo, double fw, double fh )
{
   double x,y, w,h;
   Effect *e = NULL;

   /* Transform coordinates. */
   w = p->ship->gfx_space->sw;
   h = p->ship->gfx_space->sh;
   gl_gameToScreenCoords( &x, &y, p->solid.pos.x-w/2., p->solid.pos.y-h/2. );

   /* Render effects. */
   for (int i=0; i<array_size(p->effects); i++) {
   //for (int i=array_size(p->effects)-1; i>=0; i--) {
      Effect *eiter = &p->effects[i];
      if (eiter->data->program==0)
         continue;

      /* Only render one effect for now. */
      e = eiter;
      break;
   }

   /* Render normally. */
   if (e==NULL)
      pilot_renderFramebufferBase( p, fbo, fw, fh );
   /* Render effect single effect. */
   else {
      mat4 projection, tex_mat;
      const EffectData *ed = e->data;

      /* Render onto framebuffer. */
      pilot_renderFramebufferBase( p, gl_screen.fbo[2], gl_screen.nw, gl_screen.nh );

      glBindFramebuffer( GL_FRAMEBUFFER, fbo );

      glClearColor( 0., 0., 0., 0. );
      glClear( GL_COLOR_BUFFER_BIT );

      glUseProgram( ed->program );

      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, gl_screen.fbo_tex[2] );
      glUniform1i( ed->u_tex, 0 );

      glEnableVertexAttribArray( ed->vertex );
      gl_vboActivateAttribOffset( gl_squareVBO, ed->vertex, 0, 2, GL_FLOAT, 0 );

      projection = mat4_ortho( 0., fw, 0, fh, -1., 1. );
      mat4_scale( &projection, w, h, 1. );
      gl_uniformMat4(ed->projection, &projection);

      tex_mat = mat4_identity();
      mat4_scale( &tex_mat, w/(double)gl_screen.nw, h/(double)gl_screen.nh, 1. );
      gl_uniformMat4(ed->tex_mat, &tex_mat);

      glUniform3f( ed->dimensions, SCREEN_W, SCREEN_H, 1. );
      glUniform1f( ed->u_timer, e->timer );
      glUniform1f( ed->u_elapsed, e->elapsed );
      glUniform1f( ed->u_r, e->r );
      glUniform1f( ed->u_dir, p->solid.dir );

      /* Draw. */
      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

      /* Clear state. */
      glDisableVertexAttribArray( ed->vertex );

      glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);
      glClearColor( 0., 0., 0., 1. );
   }
}

/**
 * @brief Renders the pilot.
 *
 *    @param p Pilot to render.
 */
void pilot_render( Pilot *p )
{
   double scale, x,y, w,h, z;
   int inbounds = 1;
   Effect *e = NULL;
   glColour c = {.r=1., .g=1., .b=1., .a=1.};

   /* Don't render the pilot. */
   if (pilot_isFlag( p, PILOT_NORENDER ))
      return;

   /* Transform coordinates. */
   z = cam_getZoom();
   w = p->ship->gfx_space->sw;
   h = p->ship->gfx_space->sh;
   gl_gameToScreenCoords( &x, &y, p->solid.pos.x-w/2., p->solid.pos.y-h/2. );

   /* Check if inbounds */
   if ((x < -w) || (x > SCREEN_W+w) ||
         (y < -h) || (y > SCREEN_H+h))
      inbounds = 0;

   if (inbounds) {
      /* Render effects. */
      for (int i=0; i<array_size(p->effects); i++) {
      //for (int i=array_size(p->effects)-1; i>=0; i--) {
         Effect *eiter = &p->effects[i];
         if (eiter->data->program==0)
            continue;

         /* Only render one effect for now. */
         e = eiter;
         break;
      }

      /* Check if needs scaling. */
      if (pilot_isFlag( p, PILOT_LANDING ))
         scale = CLAMP( 0., 1., p->ptimer / p->landing_delay );
      else if (pilot_isFlag( p, PILOT_TAKEOFF ))
         scale = CLAMP( 0., 1., 1. - p->ptimer / p->landing_delay );
      else
         scale = 1.;

      /* Add some transparency if stealthed. TODO better effect */
      if (!pilot_isPlayer(p) && pilot_isFlag(p, PILOT_STEALTH))
         c.a = 0.5;

      /* Render normally. */
      if (e==NULL) {
         if (p->ship->gfx_3d != NULL) {
            /* 3d */
            object_renderSolidPart(p->ship->gfx_3d, &p->solid, "body", c.a, p->ship->gfx_3d_scale * scale);
            object_renderSolidPart(p->ship->gfx_3d, &p->solid, "engine", c.a * p->engine_glow, p->ship->gfx_3d_scale * scale);
         }
         else {
            gl_renderSpriteInterpolateScale( p->ship->gfx_space, p->ship->gfx_engine,
                  1.-p->engine_glow, p->solid.pos.x, p->solid.pos.y,
                  scale, scale, p->tsx, p->tsy, &c );
         }
      }
      /* Render effect single effect. */
      else {
         mat4 projection, tex_mat;
         const EffectData *ed = e->data;

         /* Render onto framebuffer. */
         pilot_renderFramebufferBase( p, gl_screen.fbo[2], gl_screen.nw, gl_screen.nh );

         glUseProgram( ed->program );

         glActiveTexture( GL_TEXTURE0 );
         glBindTexture( GL_TEXTURE_2D, gl_screen.fbo_tex[2] );
         glUniform1i( ed->u_tex, 0 );

         glEnableVertexAttribArray( ed->vertex );
         gl_vboActivateAttribOffset( gl_squareVBO, ed->vertex, 0, 2, GL_FLOAT, 0 );

         projection = gl_view_matrix;
         mat4_translate( &projection, x + (1.-scale)*z*w/2., y + (1.-scale)*z*h/2., 0. );
         mat4_scale( &projection, scale*z*w, scale*z*h, 1. );
         gl_uniformMat4(ed->projection, &projection);

         tex_mat = mat4_identity();
         mat4_scale( &tex_mat, w/(double)gl_screen.nw, h/(double)gl_screen.nh, 1. );
         gl_uniformMat4(ed->tex_mat, &tex_mat);

         glUniform3f( ed->dimensions, SCREEN_W, SCREEN_H, cam_getZoom() );
         glUniform1f( ed->u_timer, e->timer );
         glUniform1f( ed->u_elapsed, e->elapsed );
         glUniform1f( ed->u_r, e->r );
         glUniform1f( ed->u_dir, p->solid.dir );

         /* Draw. */
         glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

         /* Clear state. */
         glDisableVertexAttribArray( ed->vertex );
      }
   }

#ifdef DEBUGGING
   double dircos, dirsin;
   int debug_mark_emitter = debug_isFlag(DEBUG_MARK_EMITTER);
   vec2 v;
   if (debug_mark_emitter) {
      dircos = cos(p->solid.dir);
      dirsin = sin(p->solid.dir);
   }
#endif /* DEBUGGING */

   /* Re-draw backwards trails. */
   for (int i=0,g=0; g<array_size(p->ship->trail_emitters); g++) {
#ifdef DEBUGGING
      if (debug_mark_emitter) {
         /* Visualize the trail emitters. */
         v.x = p->ship->trail_emitters[g].x_engine * dircos -
              p->ship->trail_emitters[g].y_engine * dirsin;
         v.y = p->ship->trail_emitters[g].x_engine * dirsin +
              p->ship->trail_emitters[g].y_engine * dircos +
              p->ship->trail_emitters[g].h_engine;

         gl_gameToScreenCoords( &x, &y, p->solid.pos.x + v.x,
                                p->solid.pos.y + v.y*M_SQRT1_2 );
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
 */
void pilot_renderOverlay( Pilot* p )
{
   int playerdead;

   /* Don't render the pilot. */
   if (pilot_isFlag( p, PILOT_NORENDER ))
      return;

   playerdead = (player_isFlag(PLAYER_DESTROYED) || (player.p==NULL));

   /* Render the hailing graphic if needed. */
   if (!playerdead && pilot_isFlag( p, PILOT_HAILING )) {
      glTexture *ico_hail = gui_hailIcon();
      if (ico_hail != NULL) {
         int sx = (int)ico_hail->sx;

         /* Render. */
         gl_renderSprite( ico_hail,
               p->solid.pos.x + PILOT_SIZE_APPROX*p->ship->gfx_space->sw/2. + ico_hail->sw/4.,
               p->solid.pos.y + PILOT_SIZE_APPROX*p->ship->gfx_space->sh/2. + ico_hail->sh/4.,
               p->hail_pos % sx, p->hail_pos / sx, NULL );
      }
   }

   /* Text ontop if needed. */
   if (p->comm_msg != NULL) {
      double x, y, dx, dy;

      /* Coordinate translation. */
      gl_gameToScreenCoords( &x, &y, p->solid.pos.x, p->solid.pos.y );

      /* Display the text. */
      glColour c = {1., 1., 1., 1.};

      /* Colour. */
      if (p->comm_msgTimer - pilot_commFade < 0.)
         c.a = p->comm_msgTimer / pilot_commFade;

      /* Position to render at. */
      dx = x - p->comm_msgWidth/2.;
      dy = y + PILOT_SIZE_APPROX*p->ship->gfx_space->sh/2.;

      /* Background. */
      gl_renderRect( dx-2., dy-2., p->comm_msgWidth+4., gl_defFont.h+4., &cBlackHilight );

      /* Display text. */
      gl_printRaw( NULL, dx, dy, &c, -1., p->comm_msg );
   }

   /* Show health / friendlyness */
   if (conf.healthbars && !playerdead && !pilot_isPlayer(p) && !pilot_isFlag(p, PILOT_DEAD) && !pilot_isDisabled(p) &&
         (pilot_isFlag(p, PILOT_COMBAT) || (p->shield < p->shield_max))) {
      double x, y, w, h;

      /* Coordinate translation. */
      gl_gameToScreenCoords( &x, &y, p->solid.pos.x, p->solid.pos.y );

      w = p->ship->gfx_space->sw + 4.;
      h = p->ship->gfx_space->sh + 4.;

      /* Can do an inbounds check now. */
      if ((x < -w) || (x > SCREEN_W+w) ||
            (y < -h) || (y > SCREEN_H+h))
         return;

      w = PILOT_SIZE_APPROX * p->ship->gfx_space->sw;
      h = PILOT_SIZE_APPROX * p->ship->gfx_space->sh / 3.;

      glUseProgram( shaders.healthbar.program );
      glUniform2f( shaders.healthbar.dimensions, 5., h );
      glUniform1f( shaders.healthbar.paramf, (p->armour + p->shield) / (p->armour_max + p->shield_max) );
      gl_uniformColor( shaders.healthbar.paramv, (p->shield > 0.) ? &cShield : &cArmour );
      gl_renderShader( x + w/2. + 2.5, y, 5., h, 0., &shaders.healthbar, pilot_getColour(p), 1 );
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
   int cooling, nchg;
   Pilot *target;
   double a, px,py, vx,vy;
   double Q;

   /* Modify the dt with speedup. */
   dt *= pilot->stats.time_speedup;

   /* Check target validity. */
   target = pilot_getTarget( pilot );

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
   for (int i=0; i<MAX_AI_TIMERS; i++)
      if (pilot->timer[i] > 0.)
         pilot->timer[i] -= dt;
   if (pilot->comm_msg != NULL) {
      pilot->comm_msgTimer -= dt;
      if (pilot->comm_msgTimer < 0.) {
         free(pilot->comm_msg);
         pilot->comm_msg = NULL;
      }
   }
   if (pilot_isFlag( pilot, PILOT_HAILING )) {
      glTexture *ico_hail = gui_hailIcon();
      if (ico_hail != NULL) {
         int sx, sy;
         pilot->htimer -= dt;
         sx = (int)ico_hail->sx;
         sy = (int)ico_hail->sy;
         if (pilot->htimer < 0.) {
            pilot->htimer = 0.1;
            pilot->hail_pos++;
            pilot->hail_pos %= sx*sy;
         }
      }
   }
   /* Update heat. */
   a = -1.;
   Q = 0.;
   nchg = 0; /* Number of outfits that change state, processed at the end. */
   for (int i=0; i<array_size(pilot->outfits); i++) {
      PilotOutfitSlot *o = pilot->outfits[i];

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
      if (outfit_isLauncher(o->outfit) || outfit_isFighterBay(o->outfit)) {
         double ammo_threshold, reload_time;

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
            pilot_addAmmo( pilot, o, 1 );
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
      double stress_falloff = 0.3*sqrt(pilot->solid.mass); /* Should be about 38 seconds for a 300 mass ship with 200 armour, and 172 seconds for a 6000 mass ship with 4000 armour. */
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

   /* Damage effect. */
   if ((pilot->stats.damage > 0.) || (pilot->stats.disable > 0.)) {
      Damage dmg;
      dmg.type          = dtype_get("normal");
      dmg.damage        = pilot->stats.damage * dt;
      dmg.penetration   = 1.; /* Full penetration. */
      dmg.disable       = pilot->stats.disable * dt;
      pilot_hit( pilot, NULL, NULL, &dmg, NULL, LUA_NOREF, 0 );
   }

   /* Handle takeoff/landing. */
   if (pilot_isFlag(pilot,PILOT_TAKEOFF)) {
      if (pilot->ptimer < 0.) {
         pilot_rmFlag(pilot,PILOT_TAKEOFF);
         if (pilot_isFlag(pilot, PILOT_PLAYER)) {
            pilot_setFlag(pilot, PILOT_NONTARGETABLE);
            pilot->itimer = PILOT_PLAYER_NONTARGETABLE_TAKEOFF_DELAY;
         }
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

      if (pilot->ship->lua_explode_update != LUA_NOREF) {
         /* Run Lua if applicable. */
         pilot_shipLExplodeUpdate( pilot, dt );
      }
      else {
         /* pilot death sound */
         if (!pilot_isFlag(pilot,PILOT_DEATH_SOUND) &&
               (pilot->ptimer < 0.050)) {
            char buf[16];

            /* Play random explosion sound. */
            snprintf(buf, sizeof(buf), "explosion%d", RNG(0,2));
            sound_playPos( sound_get(buf), pilot->solid.pos.x, pilot->solid.pos.y,
                  pilot->solid.vel.x, pilot->solid.vel.y );

            pilot_setFlag(pilot,PILOT_DEATH_SOUND);
         }
         /* final explosion */
         else if (!pilot_isFlag(pilot,PILOT_EXPLODED) &&
               (pilot->ptimer < 0.200)) {
            Damage dmg;

            /* Damage from explosion. */
            a                 = sqrt(pilot->solid.mass);
            dmg.type          = dtype_get("explosion_splash");
            dmg.damage        = MAX(0., 2. * (a * (1. + sqrt(pilot->fuel + 1.) / 28.)));
            dmg.penetration   = 1.; /* Full penetration. */
            dmg.disable       = 0.;
            expl_explode( pilot->solid.pos.x, pilot->solid.pos.y,
                  pilot->solid.vel.x, pilot->solid.vel.y,
                  pilot->ship->gfx_space->sw/2./PILOT_SIZE_APPROX + a, &dmg, NULL, EXPL_MODE_SHIP );
            debris_add( pilot->solid.mass, pilot->ship->gfx_space->sw/2.,
                  pilot->solid.pos.x, pilot->solid.pos.y,
                  pilot->solid.vel.x, pilot->solid.vel.y );
            pilot_setFlag(pilot,PILOT_EXPLODED);
            pilot_runHook( pilot, PILOT_HOOK_EXPLODED );

            /* We do a check here in case the pilot was regenerated. */
            if (pilot_isFlag(pilot, PILOT_EXPLODED)) {
               /* Release cargo */
               for (int i=0; i<array_size(pilot->commodities); i++)
                  pilot_cargoJet( pilot, pilot->commodities[i].commodity,
                        pilot->commodities[i].quantity, 1 );
            }
         }
         /* reset random explosion timer */
         else if (pilot->timer[1] <= 0.) {
            unsigned int l;

            pilot->timer[1] = 0.08 * (pilot->ptimer - pilot->timer[1]) /
                  pilot->ptimer;

            /* random position on ship */
            a = RNGF()*2.*M_PI;
            px = VX(pilot->solid.pos) +  cos(a)*RNGF()*pilot->ship->gfx_space->sw/2.;
            py = VY(pilot->solid.pos) +  sin(a)*RNGF()*pilot->ship->gfx_space->sh/2.;
            vx = VX(pilot->solid.vel);
            vy = VY(pilot->solid.vel);

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
   }
   else if (pilot_isFlag(pilot, PILOT_NONTARGETABLE)) {
      pilot->itimer -= dt;
      if (pilot->itimer < 0.)
         pilot_rmFlag(pilot, PILOT_NONTARGETABLE);
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
            pilot_cooldown( pilot, 1 );
         else {
            /* Normal braking is done (we're below MIN_VEL_ERR), now sidestep
             * normal physics and bring the ship to a near-complete stop.
             */
            pilot->solid.speed_max = 0.;
            pilot->solid.update( &pilot->solid, dt );

            if (VMOD(pilot->solid.vel) < 1e-1) {
               vectnull( &pilot->solid.vel ); /* Forcibly zero velocity. */
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

      /* Regen fuel. */
      pilot->fuel = MIN( pilot->fuel_max, pilot->fuel + pilot->stats.fuel_regen * dt );

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
   }

   /* Update effects. */
   nchg += effect_update( &pilot->effects, dt );
   if (pilot_isFlag( pilot, PILOT_DELETE ))
      return; /* It's possible for effects to remove the pilot causing future Lua to be unhappy. */

   /* Must recalculate stats because something changed state. */
   if (nchg > 0)
      pilot_calcStats( pilot );

   /* purpose fallthrough to get the movement like disabled */
   if (pilot_isDisabled(pilot) || pilot_isFlag(pilot, PILOT_COOLDOWN)) {
      /* Do the slow brake thing */
      pilot->solid.speed_max = 0.;
      pilot_setThrust( pilot, 0. );
      pilot_setTurn( pilot, 0. );

      /* update the solid */
      pilot->solid.update( &pilot->solid, dt );

      gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,
            pilot->ship->gfx_space, pilot->solid.dir );

      /* Engine glow decay. */
      if (pilot->engine_glow > 0.) {
         pilot->engine_glow -= pilot->speed / pilot->thrust * dt * pilot->solid.mass;
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

   /* Pilot is boarding its target. Hack to match speeds. */
   if (pilot_isFlag(pilot, PILOT_BOARDING)) {
      if (target==NULL)
         pilot_rmFlag(pilot, PILOT_BOARDING);
      else {
         /* Match speeds. */
         pilot->solid.vel = target->solid.vel;

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
            double efficiency, thrust;

            if (pilot->id == PLAYER_ID)
               spfx_shake( 0.75*SPFX_SHAKE_DECAY * dt); /* shake goes down at quarter speed */
            efficiency = pilot_heatEfficiencyMod( pilot->afterburner->heat_T,
                  pilot->afterburner->outfit->u.afb.heat_base,
                  pilot->afterburner->outfit->u.afb.heat_cap );
            thrust = MIN( 1., pilot->afterburner->outfit->u.afb.mass_limit / pilot->solid.mass ) * efficiency;

            /* Adjust speed. Speed bonus falls as heat rises. */
            pilot->solid.speed_max = pilot->speed * (1. +
                  pilot->afterburner->outfit->u.afb.speed * thrust);

            /* Adjust thrust. Thrust bonus falls as heat rises. */
            pilot_setThrust(pilot, 1. + pilot->afterburner->outfit->u.afb.thrust * thrust);
         }
      }
      else
         pilot->solid.speed_max = pilot->speed;
   }
   else
      pilot->solid.speed_max = -1.; /* Disables max speed. */

   /* Set engine glow. */
   if (pilot->solid.thrust > 0.) {
      /*pilot->engine_glow += pilot->thrust / pilot->speed * dt;*/
      pilot->engine_glow += pilot->speed / pilot->thrust * dt * pilot->solid.mass;
      if (pilot->engine_glow > 1.)
         pilot->engine_glow = 1.;
   }
   else if (pilot->engine_glow > 0.) {
      pilot->engine_glow -= pilot->speed / pilot->thrust * dt * pilot->solid.mass;
      if (pilot->engine_glow < 0.)
         pilot->engine_glow = 0.;
   }

   /* Update the solid, must be run after limit_speed. */
   pilot->solid.update( &pilot->solid, dt );
   gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,
         pilot->ship->gfx_space, pilot->solid.dir );

   /* See if there is commodities to gather. */
   if (!pilot_isDisabled(pilot))
      gatherable_gather( pilot );

   /* Update the trail. */
   pilot_sample_trails( pilot, 0 );

   /* Update pilot Lua. */
   pilot_shipLUpdate( pilot, dt );

   /* Update outfits if necessary. */
   pilot->otimer += dt;
   while (pilot->otimer >= PILOT_OUTFIT_LUA_UPDATE_DT) {
      pilot_outfitLUpdate( pilot, PILOT_OUTFIT_LUA_UPDATE_DT );
      if (pilot_isFlag( pilot, PILOT_DELETE ))
         return; /* Same as with the effects, it is theoretically possible for the outfit to remove the pilot. */
      pilot->otimer -= PILOT_OUTFIT_LUA_UPDATE_DT;
   }
}

/**
 * @brief Updates the given pilot's trail emissions.
 *
 *    @param p Pilot to update trails of.
 *    @param none Indicates that the pilot should update trails but skip their position.
 */
void pilot_sample_trails( Pilot* p, int none )
{
   double d2, cx, cy, dircos, dirsin;
   TrailMode mode;

   /* Ignore for simulation. */
   if (!space_needsEffects())
      return;

   /* No trails to sample. */
   if (p->trail == NULL)
      return;

   /* Skip if far away (pretty heuristic-based but seems to work). */
   cam_getPos( &cx, &cy );
   d2 = pow2(cx-p->solid.pos.x) + pow2(cy-p->solid.pos.y);
   if (d2 > pow2( MAX(SCREEN_W,SCREEN_H) / conf.zoom_far * 2. ))
      return;

   dircos = cos(p->solid.dir);
   dirsin = sin(p->solid.dir);

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
   for (int i=0, g=0; g<array_size(p->ship->trail_emitters); g++) {
      double dx, dy, prod;

      if (!pilot_trail_generated( p, g ))
         continue;

      p->trail[i]->ontop = 0;
      if (!(p->ship->trail_emitters[g].always_under) && (dirsin > 0)) {
         /* See if the trail's front (tail) is in front of the ship. */
         prod = (trail_front( p->trail[i] ).x - p->solid.pos.x) * dircos +
                  (trail_front( p->trail[i] ).y - p->solid.pos.y) * dirsin;

         p->trail[i]->ontop = (prod < 0);
      }

      dx = p->ship->trail_emitters[g].x_engine * dircos -
            p->ship->trail_emitters[g].y_engine * dirsin;
      dy = p->ship->trail_emitters[g].x_engine * dirsin +
            p->ship->trail_emitters[g].y_engine * dircos +
            p->ship->trail_emitters[g].h_engine;
      spfx_trail_sample( p->trail[i++], p->solid.pos.x + dx, p->solid.pos.y + dy*M_SQRT1_2, mode, mode==MODE_NONE );
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
   PilotOutfitSlot* dockslot;

   /* Don't double delete, just in case. */
   if (pilot_isFlag( p, PILOT_DELETE ))
      return;

   /* If the pilot was deleted from Lua, we must run the explosion hook. */
   if ((p->ship->lua_explode_update != LUA_NOREF) && pilot_isFlag( p, PILOT_DEAD )) {
      pilot_setFlag( p, PILOT_EXPLODED );
      pilot_runHook( p, PILOT_HOOK_EXPLODED );
      if (!pilot_isFlag( p, PILOT_EXPLODED ))
         return;
   }

   /* Stop ship stuff. */
   pilot_shipLCleanup(p);

   /* Handle Lua outfits. */
   pilot_outfitOffAll(p);
   pilot_outfitLCleanup(p);

   /* Remove from parent's escort list */
   if (p->parent != 0) {
      Pilot *leader = pilot_get(p->parent);
      if (leader != NULL)
         escort_rmList(leader, p->id);
   }

   /* Remove faction if necessary. */
   if (p->presence > 0) {
      system_rmCurrentPresence( cur_system, p->faction, p->presence );
      p->presence = 0;
   }

   /* Unmark as deployed if necessary */
   dockslot = pilot_getDockSlot( p );
   if (dockslot != NULL) {
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
      pilot_setThrust( p, HYPERSPACE_THRUST*p->solid.mass/p->thrust );
   }
   /* engines getting ready for the jump */
   else if (pilot_isFlag(p, PILOT_HYP_BEGIN)) {

      /* Make sure still within range. */
      can_hyp = space_canHyperspace( p );
      if (!can_hyp) {
         pilot_hyperspaceAbort( p );

         if (pilot_isPlayer(p))
            if (!player_isFlag(PLAYER_AUTONAV))
               player_message( "#r%s", _("Strayed too far from jump point: jump aborted.") );
      }
      else if (pilot_isFlag(p,PILOT_AFTERBURNER)) {
         pilot_hyperspaceAbort( p );

         if (pilot_isPlayer(p))
            if (!player_isFlag(PLAYER_AUTONAV))
               player_message( "#r%s", _("Afterburner active: jump aborted.") );
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
               player_message( "#r%s", _("Strayed too far from jump point: jump aborted.") );
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
                  p->ptimer = HYPERSPACE_ENGINE_DELAY * p->stats.jump_warmup * !p->stats.misc_instant_jump;
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
   Pilot *target = pilot_getTarget( p );
   /* Check to see if target exists, remove flag if not. */
   if (target == NULL) {
      pilot_rmFlag(p, PILOT_REFUELING);
      return 0;
   }

   /* Conditions are the same as boarding, except disabled. */
   if (vec2_dist(&p->solid.pos, &target->solid.pos) >
         target->ship->gfx_space->sw * PILOT_SIZE_APPROX )
      return 0;
   else if (vec2_dist2( &p->solid.vel, &target->solid.vel ) > pow2(MAX_HYPERSPACE_VEL))
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
   /* Check to see if target exists, remove flag if not. */
   Pilot *target = pilot_getTarget( p );
   if (target == NULL) {
      pilot_rmFlag(p, PILOT_REFUELBOARDING);
      pilot_rmFlag(p, PILOT_REFUELING);
      return;
   }

   /* Match speeds. */
   p->solid.vel = target->solid.vel;

   /* Check to see if done. */
   if (p->ptimer < 0.) {
      /* Move fuel. */
      double amount  = MIN( p->fuel, p->refuel_amount );
      amount         = MIN( amount, target->fuel_max-target->fuel );
      p->fuel       -= amount;
      target->fuel  += amount;

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
ntime_t pilot_hyperspaceDelay( const Pilot *p )
{
   int stu = (int)(NT_PERIOD_SECONDS * p->stats.jump_delay);
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
   for (int i=0; i < array_size(pilot_stack); i++) {
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
   int q = 0;
   for (int i=0; i<array_size(p->outfits); i++) {
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
int pilot_hasCredits( const Pilot *p, credits_t amount )
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
 *    @param dir Initial direction to face (radians).
 *    @param pos Initial position.
 *    @param vel Initial velocity.
 *    @param flags Used for tweaking the pilot.
 *    @param dockpilot The pilot which launched this pilot (0 if N/A).
 *    @param dockslot The outfit slot which launched this pilot (-1 if N/A).
 */
static void pilot_init( Pilot* pilot, const Ship* ship, const char* name, int faction,
      const double dir, const vec2* pos, const vec2* vel,
      const PilotFlags flags, unsigned int dockpilot, int dockslot )
{
   PilotOutfitSlot *dslot;
   PilotOutfitSlot **pilot_list_ptr[] = { &pilot->outfit_structure, &pilot->outfit_utility, &pilot->outfit_weapon };
   ShipOutfitSlot *ship_list[] = { ship->outfit_structure, ship->outfit_utility, ship->outfit_weapon };

   /* Clear memory. */
   memset(pilot, 0, sizeof(Pilot));

   /* Defaults. */
   pilot->lua_mem = LUA_NOREF;
   pilot->lua_ship_mem = LUA_NOREF;
   pilot->autoweap = 1;
   pilot->aimLines = 0;
   pilot->dockpilot = dockpilot;
   pilot->parent = dockpilot; /* leader will default to mothership if exists. */
   pilot->dockslot = dockslot;

   /* Basic information. */
   pilot->ship = ship;
   pilot->name = strdup( (name==NULL) ? _(ship->name) : name );

   /* faction */
   pilot->faction = faction;

   /* solid */
   solid_init( &pilot->solid, ship->mass, dir, pos, vel, SOLID_UPDATE_RK4 );

   /* First pass to make sure requirements make sense. */
   pilot->armour = pilot->armour_max = 1.; /* hack to have full armour */
   pilot->shield = pilot->shield_max = 1.; /* ditto shield */
   pilot->energy = pilot->energy_max = 1.; /* ditto energy */
   pilot->fuel   = pilot->fuel_max   = 1.; /* ditto fuel */
   pilot_calcStats(pilot);
   pilot->stress = 0.; /* No stress. */

   /* Allocate outfit memory. */
   pilot->outfits  = array_create( PilotOutfitSlot* );
   /* First pass copy data. */
   for (int i=0; i<3; i++) {
      *pilot_list_ptr[i] = array_create_size( PilotOutfitSlot, array_size(ship_list[i]) );
      for (int j=0; j<array_size(ship_list[i]); j++) {
         PilotOutfitSlot *slot = &array_grow( pilot_list_ptr[i] );
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
   char message[STRMAX_SHORT];
   int notworthy = pilot_reportSpaceworthy( pilot, message, sizeof(message) );
   if (notworthy) {
      DEBUG( _("Pilot '%s' failed safety check: %s"), pilot->name, message );
      for (int i=0; i<array_size(pilot->outfits); i++) {
         if (pilot->outfits[i]->outfit != NULL)
            DEBUG(_("   [%d] %s"), i, _(pilot->outfits[i]->outfit->name) );
      }
   }
#endif /* DEBUGGING */

   /* Copy pilot flags. */
   pilot_copyFlagsRaw(pilot->flags, flags);

   /* Clear timers. */
   pilot_clearTimers(pilot);

   /* Update the x and y sprite positions. */
   gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,
         pilot->ship->gfx_space, pilot->solid.dir );

   /* Fill ammo. */
   pilot_fillAmmo( pilot );

   /* Targets. */
   pilot_setTarget( pilot, pilot->id ); /* No target. */
   pilot->nav_spob         = -1;
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

   pilot->shoot_indicator = 0;
}

/**
 * @brief Resets a pilot.
 *
 *    @param pilot Pilot to reset.
 */
void pilot_reset( Pilot* pilot )
{
   /* Clean up flag.s */
   for (int i=PILOT_NOCLEAR+1; i<PILOT_FLAGS_MAX; i++)
      pilot->flags[i] = 0;

   /* Initialize heat. */
   pilot_heatReset( pilot );

   /* Set the pilot stats based on their ship and outfits */
   pilot_calcStats( pilot );

   /* Update dynamic electronic warfare (static should have already been done). */
   pilot_ewUpdateDynamic( pilot, 0. );

   /* Clear timers. */
   pilot_clearTimers(pilot);

   /* Update the x and y sprite positions. */
   gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,
         pilot->ship->gfx_space, pilot->solid.dir );

   /* Heal up. */
   pilot_healLanded( pilot );

   /* Targets. */
   pilot_setTarget( pilot, pilot->id ); /* No target. */
   pilot->nav_spob         = -1;
   pilot->nav_hyperspace   = -1;
   pilot->nav_anchor       = -1;
   pilot->nav_asteroid     = -1;

   /* AI */
   pilot->shoot_indicator = 0;

   /* Run Lua stuff. */
   pilot_shipLInit( pilot );
   pilot_outfitLInitAll( pilot );
}

/**
 * @brief Initialize pilot's trails according to the ship type and current system characteristics.
 */
static void pilot_init_trails( Pilot* p )
{
   int n = array_size(p->ship->trail_emitters);
   if (p->trail == NULL)
      p->trail = array_create_size( Trail_spfx*, n );

   for (int g=0; g<n; g++)
      if (pilot_trail_generated( p, g ))
         array_push_back( &p->trail, spfx_trail_create( p->ship->trail_emitters[g].trail_spec ) );
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
unsigned int pilot_create( const Ship* ship, const char* name, int faction, const char *ai,
      const double dir, const vec2* pos, const vec2* vel,
      const PilotFlags flags, unsigned int dockpilot, int dockslot )
{
   /* Allocate pilot memory. */
   Pilot *p = malloc(sizeof(Pilot));
   if (p == NULL) {
      WARN(_("Unable to allocate memory"));
      return 0;
   }

   /* Set the pilot in the stack -- must be there before initializing */
   array_push_back( &pilot_stack, p );

   /* Initialize the pilot. */
   pilot_init( p, ship, name, faction, dir, pos, vel, flags, dockpilot, dockslot );

   /* Set the ID. */
   if (pilot_isFlagRaw(flags, PILOT_PLAYER)) { /* Set player ID. TODO should probably be fixed to something better someday. */
      p->id = PLAYER_ID;
      qsort( pilot_stack, array_size(pilot_stack), sizeof(Pilot*), pilot_cmp );
   }
   else
      p->id = ++pilot_id; /* new unique pilot id based on pilot_id, can't be 0 */

   /* Initialize AI if applicable. */
   if (ai == NULL)
      ai = faction_default_ai( faction );
   if (ai != NULL)
      ai_pinit( p, ai ); /* Must run before ai_create */

   /* Run Lua stuff. */
   pilot_shipLInit( p );
   pilot_outfitLInitAll( p );

   /* Animated trail. */
   pilot_init_trails( p );

   /* Pilot creation hook. */
   pilot_runHook( p, PILOT_HOOK_CREATION );

   return p->id;
}

/**
 * @brief Creates a pilot without adding it to the stack.
 *
 *    @param ship Ship for the pilot to use.
 *    @param name Name of the pilot ship (NULL uses ship name).
 *    @param faction Faction of the ship.
 *    @param flags Flags for tweaking.
 *    @return Pointer to the new pilot (not added to stack).
 */
Pilot* pilot_createEmpty( const Ship* ship, const char* name,
      int faction, PilotFlags flags )
{
   Pilot *dyn = malloc(sizeof(Pilot));
   if (dyn == NULL) {
      WARN(_("Unable to allocate memory"));
      return 0;
   }
   pilot_init( dyn, ship, name, faction, 0., NULL, NULL, flags, 0, 0 );
   return dyn;
}

/**
 * @brief Clones an existing pilot.
 *
 *    @param ref Reference pilot to be cloned.
 *    @return ID of the newly created clone.
 */
unsigned int pilot_clone( const Pilot *ref )
{
   Pilot *dyn, **p;
   PilotFlags pf;

   pilot_clearFlagsRaw( &pf );
   pilot_setFlagRaw( pf, PILOT_NO_OUTFITS );

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
   pilot_init( dyn, ref->ship, ref->name, ref->faction,
         ref->solid.dir, &ref->solid.pos, &ref->solid.vel, pf, 0, 0 );

   /* Add outfits over. */
   for (int i=0; i<array_size(ref->outfits); i++)
      if (ref->outfits[i]->outfit != NULL)
         pilot_addOutfitRaw( dyn, ref->outfits[i]->outfit, dyn->outfits[i] );
   for (int i=0; i<array_size(ref->outfit_intrinsic); i++)
      pilot_addOutfitIntrinsic( dyn, ref->outfit_intrinsic[i].outfit );

   /* Reset the pilot. */
   pilot_reset( dyn );

   return dyn->id;
}

/**
 * @brief Adds a pilot to the stack.
 */
unsigned int pilot_addStack( Pilot *p )
{
   p->id = ++pilot_id; /* new unique pilot id based on pilot_id, can't be 0 */
   pilot_setFlag( p, PILOT_NOFREE );

   array_push_back( &pilot_stack, p );

   /* Have to reset after adding to stack, as some Lua functions will run code on the pilot. */
   pilot_reset( p );

   /* Animated trail. */
   pilot_init_trails( p );

#if DEBUGGING
   for (int i=1; i<array_size(pilot_stack); i++)
      if (pilot_stack[i]==pilot_stack[i-1])
         WARN(_("Duplicate pilots on stack!"));
#endif /* DEBUGGING */

   return p->id;
}

/**
 * @brief Resets the trails for a pilot.
 */
void pilot_clearTrails( Pilot *p )
{
   for (int j=0; j<array_size(p->trail); j++)
      spfx_trail_remove( p->trail[j] );
   array_erase( &p->trail, array_begin(p->trail), array_end(p->trail) );
   pilot_init_trails( p );
}

/**
 * @brief Replaces the player's pilot with an alternate ship with the same ID.
 *
 *    @return The new pilot.
 */
Pilot* pilot_setPlayer( Pilot* after )
{
   int i = pilot_getStackPos( PLAYER_ID);
   int l = pilot_getStackPos( after->id );

   if (i < 0) { /* No existing player ID. */
      if (l < 0) /* No existing pilot, have to create. */
         array_push_back( &pilot_stack, after );
   }
   else { /* Player pilot already exists. */
      if (l >= 0)
         pilot_delete( pilot_stack[i] ); /* Both player and after are on stack. Remove player. */
      else
         pilot_stack[i] = after; /* after overwrites player. */
   }
   after->id = PLAYER_ID;
   qsort( pilot_stack, array_size(pilot_stack), sizeof(Pilot*), pilot_cmp );

   /* Set up stuff. */
   player.p = after;
   pilot_clearTrails( after );

   /* Initialize AI as necessary. */
   ai_pinit( after, "player" );

   /* Set player flag. */
   pilot_setFlag( after, PILOT_PLAYER );
   pilot_setFlag( after, PILOT_NOFREE );

   /* Run Lua stuff. */
   pilot_shipLInit( after );
   pilot_outfitLInitAll( after );

   return after;
}

/**
 * @brief Finds a spawn point for a pilot
 *
 *    @param[out] vp Position.
 *    @param[out] spob Spob chosen or NULL if not.
 *    @param[out] jump Jump chosen or NULL if not.
 *    @param lf Faction to choose point for.
 *    @param ignore_rules Whether or not to ignore all rules.
 *    @param guerilla Whether or not to spawn in deep space.
 */
void pilot_choosePoint( vec2 *vp, Spob **spob, JumpPoint **jump, int lf, int ignore_rules, int guerilla )
{
   int *ind;
   JumpPoint **validJumpPoints;

   /* Initialize. */
   *spob = NULL;
   *jump   = NULL;
   vectnull( vp );

   /* Build landable spob table. */
   ind = array_create_size( int, array_size(cur_system->spobs) );
   for (int i=0; i<array_size(cur_system->spobs); i++) {
      Spob *pnt = cur_system->spobs[i];
      if (spob_hasService( pnt, SPOB_SERVICE_INHABITED ) &&
            !areEnemies( lf, pnt->presence.faction ))
         array_push_back( &ind, i );
   }

   /* Build jumpable jump table. */
   validJumpPoints = array_create_size( JumpPoint*, array_size(cur_system->jumps) );
   if (array_size(cur_system->jumps) > 0) {
      for (int i=0; i<array_size(cur_system->jumps); i++) {
         /* The jump into the system must not be exit-only, and unless
          * ignore_rules is set, must also be non-hidden
          * (excepted if the pilot is guerilla) and have faction
          * presence matching the pilot's on the remote side.
          */
         JumpPoint *target = cur_system->jumps[i].returnJump;
         double limit = 0.;
         if (guerilla) {/* Test enemy presence on the other side. */
            const int *fact = faction_getEnemies( lf );
            for (int j=0; j<array_size(fact); j++)
               limit += system_getPresence( cur_system->jumps[i].target, fact[j] );
         }

         if (!jp_isFlag( target, JP_EXITONLY ) && (ignore_rules ||
               ((!jp_isFlag( &cur_system->jumps[i], JP_HIDDEN ) || guerilla) &&
               (system_getPresence( cur_system->jumps[i].target, lf ) > limit))))
            array_push_back(&validJumpPoints, target);
      }
   }

   /* Unusual case no landable nor presence, we'll just jump in randomly if possible. */
   if (array_size(ind)==0 && array_size(validJumpPoints)==0) {
      if (guerilla) /* Guerilla ships are created far away in deep space. */
         vec2_pset ( vp, 1.5*cur_system->radius, RNGF()*2*M_PI );
      else if (array_size(cur_system->jumps) > 0) {
         for (int i=0; i<array_size(cur_system->jumps); i++) {
            JumpPoint *jp = &cur_system->jumps[i];
            if (!jp_isFlag( jp->returnJump, JP_EXITONLY ))
               array_push_back(&validJumpPoints, jp->returnJump);
         }
      }
      else {
         WARN(_("Creating pilot in system with no jumps nor spobs to take off from!"));
         vectnull( vp );
      }
   }

   /* Calculate jump chance. */
   if (array_size(ind)>0 || array_size(validJumpPoints)>0) {
      double chance = array_size(validJumpPoints);
      chance = chance / (chance + array_size(ind));

      /* Random jump in. */
      if ((RNGF() <= chance) && (validJumpPoints != NULL))
         *jump = validJumpPoints[ RNG_BASE(0,array_size(validJumpPoints)-1) ];
      /* Random take off. */
      else if (array_size(ind) != 0)
         *spob = cur_system->spobs[ ind[ RNG_BASE(0, array_size(ind)-1) ] ];
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
void pilot_free( Pilot *p )
{
   /* Clear some useful things. */
   pilot_clearHooks(p);
   effect_cleanup( p->effects );
   p->effects = NULL;
   pilot_cargoRmAll( p, 1 );
   escort_freeList(p);

   /* If hostile, must remove counter. */
   pilot_rmHostile(p);

   /* Free animated trail. */
   for (int i=0; i<array_size(p->trail); i++)
      spfx_trail_remove( p->trail[i] );
   array_free(p->trail);
   p->trail = NULL;

   /* We don't actually free internals of the pilot once we cleaned up stuff. */
   if (pilot_isFlag( p, PILOT_NOFREE )) {
      p->id = 0; /* Invalidate ID. */
      return;
   }

   /* Clean up stats. */
   ss_free( p->ship_stats );
   ss_free( p->intrinsic_stats );

   lvar_freeArray( p->shipvar );

   pilot_weapSetFree(p);

   /* Clean up outfit slots. */
   for (int i=0; i<array_size(p->outfits); i++) {
      ss_free( p->outfits[i]->lua_stats );
   }
   array_free(p->outfits);
   array_free(p->outfit_structure);
   array_free(p->outfit_utility);
   array_free(p->outfit_weapon);
   array_free(p->outfit_intrinsic);

   /* Clean up data. */
   ai_destroy(p); /* Must be destroyed first if applicable. */

   free(p->name);
   /* Case if pilot is the player. */
   if (player.p==p) {
      player.p = NULL;
      player.ps.p = NULL;
   }
   //solid_free(p->solid);
   free(p->mounted);

   escort_freeList(p);

   free(p->comm_msg);

   /* Free messages. */
   luaL_unref(naevL, p->messages, LUA_REGISTRYINDEX);

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
static void pilot_erase( Pilot *p )
{
   int i = pilot_getStackPos( p->id );
   pilot_free(p);
   array_erase( &pilot_stack, &pilot_stack[i], &pilot_stack[i+1] );
}

/**
 * @brief Tries to remove a pilot from the stack.
 */
void pilot_stackRemove( Pilot *p )
{
   int i = pilot_getStackPos( p->id );
#ifdef DEBUGGING
   if (i < 0)
      WARN(_("Trying to remove non-existent pilot '%s' from stack!"), p->name);
#endif /* DEBUGGING */
   p->id = 0;
   array_erase( &pilot_stack, &pilot_stack[i], &pilot_stack[i+1] );
}

/**
 * @brief Initializes pilot stuff.
 */
void pilots_init (void)
{
   pilot_stack = array_create_size( Pilot*, PILOT_SIZE_MIN );
   il_create( &pilot_qtquery, 1 );
}

/**
 * @brief Frees the pilot stack.
 */
void pilots_free (void)
{
   pilot_freeGlobalHooks();

   /* First pass to stop outfits. */
   for (int i=0; i < array_size(pilot_stack); i++) {
      /* Stop ship stuff. */
      pilot_shipLCleanup( pilot_stack[i]);
      /* Stop all outfits. */
      pilot_outfitOffAll(pilot_stack[i]);
      /* Handle Lua outfits. */
      pilot_outfitLCleanup(pilot_stack[i]);
   }

   /* Free pilots. */
   for (int i=0; i < array_size(pilot_stack); i++)
      pilot_free(pilot_stack[i]);
   array_free(pilot_stack);
   pilot_stack = NULL;
   player.p = NULL;
   free( player.ps.acquired );
   memset( &player.ps, 0, sizeof(PlayerShip_t) );

   /* Clean up quadtree. */
   qt_destroy( &pilot_quadtree );
   il_destroy( &pilot_qtquery );
}

/**
 * @brief Cleans up the pilot stack - leaves the player
 *
 *    @param persist Do not remove persistent pilots.
 */
void pilots_clean( int persist )
{
   int persist_count = 0;

   /* First pass to stop outfits without clearing stuff - this can call all
    * sorts of Lua stuff. */
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *p = pilot_stack[i];
      if (p == player.p &&
          (persist && pilot_isFlag(p, PILOT_PERSIST)))
         continue;
      /* Stop all outfits. */
      pilot_outfitOffAll( p );
      /* Handle Lua outfits. */
      pilot_outfitLCleanup( p );
   }

   /* Here we actually clean up stuff. */
   for (int i=0; i<array_size(pilot_stack); i++) {
      /* move player and persisted pilots to start */
      if (!pilot_isFlag(pilot_stack[i], PILOT_DELETE) &&
            (pilot_stack[i] == player.p ||
             (persist && pilot_isFlag(pilot_stack[i], PILOT_PERSIST)))) {
         /* Have to swap the pilots so it gets properly freed. */
         Pilot *p = pilot_stack[persist_count];
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

   /* Init AI on the remaining pilots, has to be done here so the pilot_stack is consistent. */
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *p = pilot_stack[i];
      pilot_clearHooks(p);
      ai_cleartasks(p);
      ai_init(p);
   }

   /* Clear global hooks. */
   pilots_clearGlobalHooks();
}

/**
 * @brief Updates pilot state which depends on the system (sensor range, nebula trails...)
 */
void pilots_newSystem (void)
{
   double r = cur_system->radius * 1.1;

   pilot_updateSensorRange();
   for (int i=0; i < array_size(pilot_stack); i++)
      pilot_init_trails( pilot_stack[i] );

   if (qt_init)
      qt_destroy( &pilot_quadtree );
   qt_create( &pilot_quadtree, -r, -r, r, r, qt_max_elem, qt_depth );
   qt_init = 1;
}

/**
 * @brief Clears all the pilots except the player and clear-exempt pilots.
 */
void pilots_clear (void)
{
   for (int i=0; i < array_size(pilot_stack); i++)
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
      pilot_rmFlag( player.p, PILOT_NOFREE );
      pilot_free(player.p);
      player.p = NULL;
      free( player.ps.acquired );
      memset( &player.ps, 0, sizeof(PlayerShip_t) );
   }
   array_erase( &pilot_stack, array_begin(pilot_stack), array_end(pilot_stack) );
}

/**
 * @brief Purges pilots set for deletion.
 */
void pilots_updatePurge (void)
{
   /* Delete loop - this should be atomic or we get hook fuckery! */
   for (int i=array_size(pilot_stack)-1; i>=0; i--) {
      Pilot *p = pilot_stack[i];

      /* Clear target. */
      p->ptarget = NULL;

      /* Destroy pilot and go on. */
      if (pilot_isFlag(p, PILOT_DELETE))
         pilot_erase( p );
   }

   /* Second loop sets up quadtrees. */
   qt_clear( &pilot_quadtree ); /* Empty it. */
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *p = pilot_stack[i];
      int x, y, w2, h2;

      /* Ignore pilots being deleted. */
      if (pilot_isFlag(p, PILOT_DELETE))
         continue;

      /* Ignore hidden pilots. */
      if (pilot_isFlag(p, PILOT_HIDE))
         continue;

      x = round(p->solid.pos.x);
      y = round(p->solid.pos.y);
      w2 = ceil(p->ship->gfx_space->sw * 0.5);
      h2 = ceil(p->ship->gfx_space->sh * 0.5);
      qt_insert( &pilot_quadtree, i, x-w2, y-h2, x+w2, y+h2 );
   }
}

/**
 * @brief Updates all the pilots.
 *
 *    @param dt Delta tick for the update.
 */
void pilots_update( double dt )
{
   /* Have all the pilots think. */
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *p = pilot_stack[i];

      /* Invisible, not doing anything. */
      if (pilot_isFlag(p, PILOT_HIDE))
         continue;

      /* See if should think. */
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
         if ((VMOD(p->solid.vel) < 2*solid_maxspeed( &p->solid, p->speed, p->thrust) ) && (p->ptimer < 0.))
            pilot_rmFlag(p, PILOT_HYP_END);
      }
      /* Must not be boarding to think. */
      else if (!pilot_isFlag(p, PILOT_BOARDING) &&
            !pilot_isFlag(p, PILOT_REFUELBOARDING) &&
            /* Must not be landing nor taking off. */
            !pilot_isFlag(p, PILOT_LANDING) &&
            !pilot_isFlag(p, PILOT_TAKEOFF) &&
            /* Must not be jumping in. */
            !pilot_isFlag(p, PILOT_HYP_END)) {
         if (pilot_isFlag(p, PILOT_PLAYER))
            player_think( p, dt );
         else
            ai_think( p, dt );
      }
   }

   /* Now update all the pilots. */
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *p = pilot_stack[i];

      /* Ignore. */
      if (pilot_isFlag(p, PILOT_DELETE))
         continue;

      /* Invisible, not doing anything. */
      if (pilot_isFlag(p, PILOT_HIDE))
         continue;

      /* Just update the pilot. */
      if (pilot_isFlag( p, PILOT_PLAYER ))
         player_update( p, dt );
      else
         pilot_update( p, dt );
   }
}

/**
 * @brief Renders all the pilots.
 */
void pilots_render (void)
{
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *p = pilot_stack[i];

      /* Invisible, not doing anything. */
      if (pilot_isFlag(p, PILOT_HIDE) || pilot_isFlag(p, PILOT_DELETE))
         continue;

      if (!pilot_isFlag( p, PILOT_PLAYER ))
         pilot_render( p );
   }
}

/**
 * @brief Renders all the pilots overlays.
 */
void pilots_renderOverlay (void)
{
   for (int i=0; i<array_size(pilot_stack); i++) {
      Pilot *p = pilot_stack[i];

      /* Invisible, not doing anything. */
      if (pilot_isFlag(p, PILOT_HIDE) || pilot_isFlag(p, PILOT_DELETE))
         continue;

      if (!pilot_isFlag( p, PILOT_PLAYER ))
         pilot_renderOverlay( p );
   }
}

/**
 * @brief Clears the pilot's timers.
 *
 *    @param pilot Pilot to clear timers of.
 */
void pilot_clearTimers( Pilot *pilot )
{
   int n;

   /* Clear outfits first to not leave some outfits in dangling states. */
   pilot_outfitOffAll(pilot);

   pilot->ptimer     = 0.; /* Pilot timer. */
   pilot->tcontrol   = 0.; /* AI control timer. */
   pilot->stimer     = 0.; /* Shield timer. */
   pilot->dtimer     = 0.; /* Disable timer. */
   pilot->otimer     = 0.; /* Outfit timer. */
   for (int i=0; i<MAX_AI_TIMERS; i++)
      pilot->timer[i] = 0.; /* Specific AI timers. */
   n = 0;
   for (int i=0; i<array_size(pilot->outfits); i++) {
      PilotOutfitSlot *o = pilot->outfits[i];
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
   return (1. - 1./(1. + ((double)cur_pilot->solid.mass / (double)p->solid.mass)));
}

/**
 * @brief Calculates the dps and eps of a pilot.
 *
 *    @param p Pilot to compute stats of.
 *    @param[out] pdps DPS of pilot.
 *    @param[out] peps EPS of pilot.
 */
void pilot_dpseps( const Pilot *p, double *pdps, double *peps )
{
   double shots, dps=0., eps=0.;
   for (int i=0; i<array_size(p->outfits); i++) {
      const Damage *dmg;
      double mod_energy, mod_damage, mod_shots;
      const Outfit *o = p->outfits[i]->outfit;
      if (o==NULL)
         continue;
      switch (o->type) {
         case OUTFIT_TYPE_BOLT:
            mod_energy = p->stats.fwd_energy;
            mod_damage = p->stats.fwd_damage;
            mod_shots  = 1. / p->stats.fwd_firerate * (double)o->u.blt.shots;
            break;
         case OUTFIT_TYPE_TURRET_BOLT:
            mod_energy = p->stats.tur_energy;
            mod_damage = p->stats.tur_damage;
            mod_shots  = 1. / p->stats.tur_firerate * (double)o->u.blt.shots;
            break;
         case OUTFIT_TYPE_LAUNCHER:
         case OUTFIT_TYPE_TURRET_LAUNCHER:
            mod_energy = 1.;
            mod_damage = p->stats.launch_damage;
            mod_shots  = 1. / p->stats.launch_rate * (double)o->u.lau.shots;
            break;
         case OUTFIT_TYPE_BEAM:
         case OUTFIT_TYPE_TURRET_BEAM:
            /* Special case due to continuous fire. */
            if (o->type == OUTFIT_TYPE_BEAM) {
               mod_energy = p->stats.fwd_energy;
               mod_damage = p->stats.fwd_damage;
               mod_shots  = 1. / p->stats.fwd_firerate;
            }
            else {
               mod_energy = p->stats.tur_energy;
               mod_damage = p->stats.tur_damage;
               mod_shots  = 1. / p->stats.tur_firerate;
            }
            shots = outfit_duration(o);
            mod_shots = shots / (shots + mod_shots * outfit_delay(o));
            dps += mod_shots * mod_damage * outfit_damage(o)->damage;
            eps += mod_shots * mod_energy * outfit_energy(o);
            continue;

         default:
            continue;
      }
      shots = 1. / (mod_shots * outfit_delay(o));

      dmg   = outfit_damage(o);
      dps  += shots * mod_damage * dmg->damage;
      eps  += shots * mod_energy * MAX( outfit_energy(o), 0. );
   }
   if (pdps != NULL)
      *pdps = dps;
   if (peps != NULL)
      *peps = eps;
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
   double DPSaccum_target, DPSaccum_pilot;

   pilot_dpseps( p, &DPSaccum_target, NULL );
   pilot_dpseps( cur_pilot, &DPSaccum_pilot, NULL );

   if ((DPSaccum_target > 1e-6) && (DPSaccum_pilot > 1e-6))
      return DPSaccum_pilot / (DPSaccum_target + DPSaccum_pilot);
   else if (DPSaccum_pilot > 0.)
      return 1.;
   return 0.;
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
 *    @param count_unique Whether or not to count the cost of unique outfits.
 *    @return Worth of the pilot.
 */
credits_t pilot_worth( const Pilot *p, int count_unique )
{
   /* Ship price is base price + outfit prices. */
   credits_t price = ship_basePrice( p->ship );
   for (int i=0; i<array_size(p->outfits); i++) {
      if (p->outfits[i]->outfit == NULL)
         continue;
      /* Don't count unique outfits. */
      if (!count_unique && outfit_isProp(p->outfits[i]->outfit, OUTFIT_PROP_UNIQUE))
         continue;
      price += p->outfits[i]->outfit->price;
   }

   return price;
}

/**
 * @brief Sends a message
 *
 *    @param p Pilot to send message
 *    @param receiver Pilot to receive it
 *    @param type Type of message.
 *    @param idx Index of data on lua stack or 0
 */
void pilot_msg( Pilot *p, Pilot *receiver, const char *type, unsigned int idx )
{
   if (idx != 0)
      lua_pushvalue(naevL, idx); /* data */
   else
      lua_pushnil(naevL); /* data */

   lua_newtable(naevL); /* data, msg */

   if (p != NULL) {
      lua_pushpilot(naevL, p->id); /* data, msg, sender */
      lua_rawseti(naevL, -2, 1); /* data, msg */
   }

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
   /* Check commodities. */
   for (int i=0; i<array_size(p->commodities); i++) {
      const Commodity *c = p->commodities[i].commodity;
      if (commodity_checkIllegal( c, faction ))
         return 1;
   }
   /* Check outfits. */
   for (int i=0; i<array_size(p->outfits); i++) {
      const Outfit *o = p->outfits[i]->outfit;
      if ((o != NULL) && outfit_checkIllegal( o, faction ))
         return 1;
   }
   /* Nothing to see here sir. */
   return 0;
}

/**
 * @brief Sets the quad tree parameters. Can have significant impact on performance.
 *
 *    @param max_elem Maximum elements to add to a leaf before splitting.
 *    @param depth Maximum depth to use.
 */
void pilot_quadtreeParams( int max_elem, int depth )
{
   qt_max_elem = max_elem;
   qt_depth = depth;
}
