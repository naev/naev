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

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "nxml.h"

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
#include "ai_extra.h"
#include "faction.h"
#include "font.h"
#include "land.h"
#include "land_outfits.h"
#include "land_shipyard.h"
#include "array.h"
#include "camera.h"


#define PILOT_CHUNK_MIN 128 /**< Maximum chunks to increment pilot_stack by */
#define PILOT_CHUNK_MAX 2048 /**< Minimum chunks to increment pilot_stack by */
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
 * prototyes
 */
/* update. */
static void pilot_hyperspace( Pilot* pilot, double dt );
static void pilot_refuel( Pilot *p, double dt );
/* clean up. */
static void pilot_dead( Pilot* p, unsigned int killer );
/* misc */
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
 * @brief Gets the nearest enemy to the pilot.
 *
 *    @param p Pilot to get his nearest enemy.
 *    @return ID of his nearest enemy.
 */
unsigned int pilot_getNearestEnemy( const Pilot* p )
{
   unsigned int tp;
   int i;
   double d, td;

   tp = 0;
   d  = 0.;
   for (i=0; i<pilot_nstack; i++) {
      /* Must not be dead. */
      if (pilot_isFlag( pilot_stack[i], PILOT_DELETE ) ||
            pilot_isFlag( pilot_stack[i], PILOT_DEAD))
         continue;

      /* Must not be bribed. */
      if ((pilot_stack[i]->faction == FACTION_PLAYER) && pilot_isFlag(p,PILOT_BRIBED))
         continue;

      /* Must not be invisible. */
      if (pilot_isFlag( pilot_stack[i], PILOT_INVISIBLE ))
         continue;

      /* Should either be hostile by faction or by player. */
      if (!(areEnemies( p->faction, pilot_stack[i]->faction) ||
               ((pilot_stack[i]->id == PLAYER_ID) &&
                pilot_isFlag(p,PILOT_HOSTILE))))
         continue;


      /* Shouldn't be disabled. */
      if (pilot_isDisabled(pilot_stack[i]))
         continue;

      /* Must be in range. */
      if (!pilot_inRangePilot( p, pilot_stack[i] ))
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
 *    @param p Pilot to get his nearest enemy.
 *    @param target_mass_LB the lower bound for target mass
 *    @param target_mass_UB the upper bound for target mass
 *    @return ID of his nearest enemy.
 */
unsigned int pilot_getNearestEnemy_size( const Pilot* p, int target_mass_LB, int target_mass_UB)
{
   unsigned int tp;
   int i;
   double d, td;

   tp = 0;
   d = 0.;
   for (i=0; i<pilot_nstack; i++) {
      /* Must not be bribed. */
      if ((pilot_stack[i]->faction == FACTION_PLAYER) && pilot_isFlag(p,PILOT_BRIBED))
         continue;

      if ((areEnemies(p->faction, pilot_stack[i]->faction) || /* Enemy faction. */
            ((pilot_stack[i]->id == PLAYER_ID) && 
               pilot_isFlag(p,PILOT_HOSTILE)))) { /* Hostile to player. */

         /*mass is in bounds*/
         if (pilot_stack[i]->solid->mass >= target_mass_LB && pilot_stack[i]->solid->mass <= target_mass_UB)
            continue;

         /* Shouldn't be disabled. */
         if (pilot_isDisabled(pilot_stack[i]))
            continue;

         /* Must be in range. */
         if (!pilot_inRangePilot( p, pilot_stack[i] ))
            continue;

         /* Check distance. */
         td = vect_dist2(&pilot_stack[i]->solid->pos, &p->solid->pos);
         if (!tp || (td < d)) {
            d = td;
            tp = pilot_stack[i]->id;
         }
      }
   }
   return tp;

}

/**
 * @brief Gets the nearest enemy to the pilot closest to the pilot whose mass is between LB and UB.
 *
 *    @param p Pilot to get his nearest enemy.
 *    @param mass_factor parameter for target mass (0-1, 0.5 = current mass)
 *    @param health_factor parameter for target shields/armor (0-1, 0.5 = current health)
 *    @param dps_factor parameter for target dps (0-1, 0.5 = current dps)
 *    @param range_factor weighting for range (typically >> 1)
 *    @return ID of his nearest enemy.
 */
unsigned int pilot_getNearestEnemy_heuristic(const Pilot* p, double mass_factor, double health_factor, double damage_factor, double range_factor)
{
   unsigned int tp;
   int i;
   double d, td, current_heuristic_value=10000, temp;

   tp = 0;
   d = 0.;
   for (i=0; i<pilot_nstack; i++) {
      /* Must not be bribed. */
      if ((pilot_stack[i]->faction == FACTION_PLAYER) && pilot_isFlag(p,PILOT_BRIBED))
         continue;

      if ((areEnemies(p->faction, pilot_stack[i]->faction) || /* Enemy faction. */
            ((pilot_stack[i]->id == PLAYER_ID) && 
               pilot_isFlag(p,PILOT_HOSTILE)))) { /* Hostile to player. */

         

         /* Shouldn't be disabled. */
         if (pilot_isDisabled(pilot_stack[i]))
            continue;

         /* Must be in range. */ 
         if (!pilot_inRangePilot( p, pilot_stack[i] ))
            continue;

         /* Check distance. */
         td = vect_dist2(&pilot_stack[i]->solid->pos, &p->solid->pos)* range_factor;
         temp = td+fabs( pilot_relsize(p, pilot_stack[i]) /*0.5*/-mass_factor) + fabs(pilot_relhp(p, pilot_stack[i]) /*0.5*/- health_factor) + fabs(pilot_reldps(p, pilot_stack[i]) /*0.5*/-damage_factor);
         
         if ((tp == 0) || (temp< current_heuristic_value)) {
            current_heuristic_value = temp;
            tp = pilot_stack[i]->id;
         }
      }
   }
   return tp;

}

/**
 * @brief Get the nearest pilot to a pilot.
 *
 *    @param p Pilot to get his nearest pilot.
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

      /* Shouldn't be invisible. */
      if (pilot_isFlag( pilot_stack[i], PILOT_INVISIBLE ))
         continue;

      /* Shouldn't be dead. */
      if (pilot_isFlag(pilot_stack[i], PILOT_DEAD) ||
            pilot_isFlag(pilot_stack[i], PILOT_DELETE))
         continue;

      /* Must be in range. */
      if (!pilot_inRangePilot( p, pilot_stack[i] ))
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
   a  = 10e10;
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

      /* Shouldn't be invisible. */
      if (pilot_isFlag( pilot_stack[i], PILOT_INVISIBLE ))
         continue;

      /* Shouldn't be dead. */
      if (pilot_isFlag(pilot_stack[i], PILOT_DEAD) ||
            pilot_isFlag(pilot_stack[i], PILOT_DELETE))
         continue;

      /* Must be in range. */
      if (!pilot_inRangePilot( p, pilot_stack[i] ))
         continue;

      /* Only allow selection if off-screen. */
      if (gui_onScreenPilot( &rx, &ry, pilot_stack[i] )) {
         continue;
      }

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
 *  abused all the time.  Maximum iterations is 32 on a platfom with 32 bit
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
 *    @param p Pilot to send distress signal.
 *    @param msg Message in distress signal.
 *    @param ignore_int Whether or not should ignore interference.
 */
void pilot_distress( Pilot *p, const char *msg, int ignore_int )
{
   int i, r;
   Pilot *t;

   /* Broadcast the message. */
   if (msg[0] != '\0')
      pilot_broadcast( p, msg, ignore_int );

   /* Get the target to see if it's the player. */
   t = pilot_get(p->target);

   /* Now proceed to see if player.p should incur faction loss because
    * of the broadcast signal. */

   /* Consider not in range at first. */
   r = 0;

   /* Check if planet is in range. */
   for (i=0; i<cur_system->nplanets; i++) {
      if (planet_hasService(cur_system->planets[i], PLANET_SERVICE_INHABITED) &&
            (!ignore_int && pilot_inRangePlanet(p, i)) &&
            !areEnemies(p->faction, cur_system->planets[i]->faction)) {
         r = 1;
         break;
      }
   }

   /* Now we must check to see if a pilot is in range. */
   for (i=0; i<pilot_nstack; i++) {
      if ((pilot_stack[i]->id != p->id) &&
            (!ignore_int && pilot_inRangePilot(p, pilot_stack[i]))) {

         /* Send AI the distress signal. */
         if (pilot_stack[i]->ai != NULL)
            ai_getDistress( pilot_stack[i], p );

         /* Check if should take faction hit. */
         if (!areEnemies(p->faction, pilot_stack[i]->faction))
            r = 1;
      }
   }

   /* Player only gets one faction hit per pilot. */
   if (!pilot_isFlag(p, PILOT_DISTRESSED)) {

      /* Modify faction, about 1 for a llama, 4.2 for a hawking */
      if ((t != NULL) && (t->faction == FACTION_PLAYER) && r)
         faction_modPlayer( p->faction, -(pow(p->ship->mass, 0.2) - 1.) );

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
   return (int)(p->fuel) / HYPERSPACE_FUEL;
}


/**
 * @brief Damages the pilot.
 *
 *    @param p Pilot that is taking damage.
 *    @param w Solid that is hitting pilot.
 *    @param shooter Attacker that shot the pilot.
 *    @param dtype Type of damage.
 *    @param damage Amount of damage.
 *    @return The real damage done.
 */
double pilot_hit( Pilot* p, const Solid* w, const unsigned int shooter,
      const DamageType dtype, const double damage, const double penetration )
{
   int mod, h;
   double damage_shield, damage_armour, knockback, dam_mod, dmg;
   double armour_start, absorb;
   Pilot *pshooter;
   HookParam hparam;

   /* Invincible means no damage. */
   if (pilot_isFlag( p, PILOT_INVINCIBLE) ||
         pilot_isFlag( p, PILOT_INVISIBLE))
      return 0.;

   /* Defaults. */
   pshooter = NULL;
   dam_mod  = 0.;
   dmg      = 0.;
   armour_start = p->armour;

   /* Calculate the damage. */
   absorb = 1. - CLAMP( 0., 1., p->dmg_absorb - penetration );
   outfit_calcDamage( &damage_shield, &damage_armour, &knockback, dtype, damage );
   damage_shield *= absorb;
   damage_armour *= absorb;

   /* Player breaks autonav. */
   if ((w != NULL) && (p->id == PLAYER_ID) &&
         !pilot_isFlag(player.p, PILOT_HYP_BEGIN) &&
         !pilot_isFlag(player.p, PILOT_HYPERSPACE))
      player_autonavAbort("Sustaining Damage");

   /*
    * EMP don't do damage if pilot is disabled.
    */
   if (pilot_isDisabled(p) && (dtype == DAMAGE_TYPE_EMP)) {
      dmg        = 0.;
      dam_mod    = 0.;
   }

   /*
    * Shields take entire blow.
    */
   else if (p->shield-damage_shield > 0.) {
      dmg        = damage_shield;
      p->shield -= damage_shield;
      dam_mod    = damage_shield/p->shield_max;
   }
   /*
    * Shields take part of the blow.
    */
   else if (p->shield > 0.) {
      dmg        = p->shield + (1. - p->shield/damage_shield) * damage_armour;
      p->armour -= (1. - p->shield/damage_shield) * damage_armour;
      p->shield  = 0.;
      dam_mod    = (damage_shield+damage_armour) /
                   ((p->shield_max+p->armour_max) / 2.);
      p->stimer  = 3.;
      p->sbonus  = 3.;
   }
   /*
    * Armour takes the entire blow.
    */
   else if (p->armour > 0.) {
      dmg        = damage_armour;
      p->armour -= damage_armour;
      p->stimer  = 3.;
      p->sbonus  = 3.;
   }

   /* EMP does not kill. */
   if ((dtype == DAMAGE_TYPE_EMP) && (p->armour < PILOT_DISABLED_ARMOR*p->ship->armour*0.75))
      p->armour = MIN( armour_start, PILOT_DISABLED_ARMOR*p->ship->armour*0.75);

   /* Disabled always run before dead to ensure crating boost. */
   if (!pilot_isFlag(p,PILOT_DISABLED) && (p != player.p) && (!pilot_isFlag(p,PILOT_NODISABLE) || (p->armour < 0.)) &&
         (p->armour < PILOT_DISABLED_ARMOR*p->ship->armour)) { /* disabled */

      /* If hostile, must remove counter. */
      h = (pilot_isHostile(p)) ? 1 : 0;
      pilot_rmHostile(p);
      if (h == 1) /* Horrible hack to make sure player.p can hit it if it was hostile. */
         /* Do not use pilot_setHostile here or music will change again. */
         pilot_setFlag(p,PILOT_HOSTILE);

      pshooter = pilot_get(shooter);
      if ((pshooter != NULL) && (pshooter->faction == FACTION_PLAYER)) {
         /* About 3 for a llama, 26 for hawking. */
         mod = pow(p->ship->mass,0.4) - 1.;

         /* Modify combat rating. */
         player.crating += 2*mod;
      }

      /* Remove faction if necessary. */
      if (p->presence > 0) {
         system_rmCurrentPresence( cur_system, p->faction, p->presence );
         p->presence = 0;
      }

      pilot_setFlag( p,PILOT_DISABLED ); /* set as disabled */
      /* Run hook */
      if (shooter > 0) {
         hparam.type       = HOOK_PARAM_PILOT;
         hparam.u.lp.pilot = shooter;
      }
      else
         hparam.type       = HOOK_PARAM_NIL;
      pilot_runHookParam( p, PILOT_HOOK_DISABLE, &hparam, 1 ); /* Already disabled. */
   }

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
            mod = 2*(pow(p->ship->mass,0.4) - 1.);

            /* Modify faction for him and friends. */
            faction_modPlayer( p->faction, -mod );
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

   return dmg;
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
      hparam.u.lp.pilot = killer;
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
 *    @param dtype Damage type of the explosion.
 *    @param damage Amount of damage by the explosion.
 *    @param penetration Damage penetration [0:1].
 *    @param parent The exploding pilot.
 */
void pilot_explode( double x, double y, double radius,
      DamageType dtype, double damage, 
      double penetration, const Pilot *parent )
{
   int i;
   double rx, ry;
   double dist, rad2;
   Pilot *p;
   Solid s; /* Only need to manipulate mass and vel. */

   rad2 = radius*radius;

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
         damage *= 1. - sqrt(dist / rad2);

         /* Impact settings. */
         s.mass =  pow2(damage) / 30.;
         s.vel.x = rx;
         s.vel.y = ry;

         /* Actual damage calculations. */
         pilot_hit( p, &s, (parent!=NULL)?parent->id:0, dtype, damage, penetration );

         /* Shock wave from the explosion. */
         if (p->id == PILOT_PLAYER)
            spfx_shake( pow2(damage) / pow2(100.) * SHAKE_MAX );
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
   int i;
   unsigned int l;
   Pilot *target;
   double a, px,py, vx,vy;
   char buf[16];
   PilotOutfitSlot *o;
   double Q;

   /*
    * Update timers.
    */
   pilot->ptimer   -= dt;
   pilot->tcontrol -= dt;
   pilot->stimer   -= dt;
   if (pilot->stimer <= 0.)
      pilot->sbonus   -= dt;
   Q = 0.;
   for (i=0; i<MAX_AI_TIMERS; i++)
      if (pilot->timer[i] > 0.)
         pilot->timer[i] -= dt;
   for (i=0; i<pilot->noutfits; i++) {
      o = pilot->outfits[i];
      if (o->outfit == NULL)
         continue;
      if (o->active) {
         if (o->timer > 0.)
            o->timer -= dt * pilot_heatFireRateMod( o->heat_T );
         Q  += pilot_heatUpdateSlot( pilot, o, dt );
      }
   }

   /* Global heat. */
   pilot_heatUpdateShip( pilot, Q, dt );

   /* Update electronic warfare. */
   pilot_ewUpdateDynamic( pilot );

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
      if (pilot->ptimer < 0.) { /* completely destroyed with final explosion */
         if (pilot->id==PLAYER_ID) /* player.p handled differently */
            player_destroyed();
         pilot_delete(pilot);
         return;
      }

      /* pilot death sound */
      if (!pilot_isFlag(pilot,PILOT_DEATH_SOUND) && (pilot->ptimer < 0.050)) {

         /* Play random explsion sound. */
         snprintf(buf, sizeof(buf), "explosion%d", RNG(0,2));
         sound_playPos( sound_get(buf), pilot->solid->pos.x, pilot->solid->pos.y,
               pilot->solid->vel.x, pilot->solid->vel.y );

         pilot_setFlag(pilot,PILOT_DEATH_SOUND);
      }
      /* final explosion */
      else if (!pilot_isFlag(pilot,PILOT_EXPLODED) && (pilot->ptimer < 0.200)) {

         /* Damage from explosion. */
         a = sqrt(pilot->solid->mass);
         expl_explode( pilot->solid->pos.x, pilot->solid->pos.y,
               pilot->solid->vel.x, pilot->solid->vel.y,
               pilot->ship->gfx_space->sw/2. + a,
               DAMAGE_TYPE_KINETIC,
               MAX(0., 2. * (a * (1. + sqrt(pilot->fuel + 1.) / 28.))), 1., /* 100% penetration. */
               NULL, EXPL_MODE_SHIP );
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
         if (RNGF() > 0.8) spfx_add( spfx_get("ExpM"), px, py, vx, vy, l );
         else spfx_add( spfx_get("ExpS"), px, py, vx, vy, l );
      }
   }
   else if (pilot->armour <= 0.) { /* PWNED */
      pilot_dead( pilot, 0 ); /* start death stuff - dunno who killed. */
   }

   /* purpose fallthrough to get the movement like disabled */
   if (pilot_isDisabled(pilot)) {
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

   /* regen shield */
   if (pilot->stimer <= 0.) {
      pilot->shield += pilot->shield_regen * dt;
      if (pilot->sbonus > 0.)
         pilot->shield += dt * (pilot->shield_regen * (pilot->sbonus / 1.5));
      if (pilot->shield > pilot->shield_max)
         pilot->shield = pilot->shield_max;
   }

   /* Update energy */
   if ((pilot->energy < 1.) && pilot_isFlag(pilot, PILOT_AFTERBURNER))
      pilot_rmFlag(pilot, PILOT_AFTERBURNER); /* Break afterburner */

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

   /* Player damage decay. */
   if (pilot->player_damage > 0.)
      pilot->player_damage -= dt * PILOT_HOSTILE_DECAY;
   else
      pilot->player_damage = 0.;

   /* check limits */
   if (pilot->energy > pilot->energy_max)
      pilot->energy = pilot->energy_max;

   /* Pilot is board/refueling.  Hack to match speeds. */
   if (pilot_isFlag(pilot, PILOT_REFUELBOARDING))
      pilot_refuel(pilot, dt);

   /* Pilot is boarding it's target.  Hack to match speeds. */
   if (pilot_isFlag(pilot, PILOT_BOARDING)) {
      target = pilot_get(pilot->target);
      if (target==NULL)
         pilot_rmFlag(pilot, PILOT_BOARDING);
      else {
         /* Match speeds. */
         vectcpy( &pilot->solid->vel, &target->solid->vel );

         /* See if boarding is finished. */
         if (pilot->ptimer < 0.)
            pilot_boardComplete(pilot);
      }
   }


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

   /* Update weapons. */
   pilot_weapSetUpdate( pilot );

   if (!pilot_isFlag(pilot, PILOT_HYPERSPACE)) { /* limit the speed */
      
      /* pilot is afterburning */
      if (pilot_isFlag(pilot, PILOT_AFTERBURNER) && /* must have enough energy left */
               (pilot->energy > pilot->afterburner->outfit->u.afb.energy * dt)) {
         pilot->solid->speed_max = pilot->speed +
               pilot->speed * pilot->afterburner->outfit->u.afb.speed *
               MIN( 1., pilot->afterburner->outfit->u.afb.mass_limit/pilot->solid->mass);

         if (pilot->id == PLAYER_ID)
            spfx_shake( 0.75*SHAKE_DECAY * dt); /* shake goes down at quarter speed */

         pilot->energy -= pilot->afterburner->outfit->u.afb.energy * dt; /* energy loss */
      }
      else /* normal limit */
         pilot->solid->speed_max = pilot->speed;
   }
   else
      pilot->solid->speed_max = -1.; /* Disables max speed. */

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
         if (p->id == PLAYER_ID) { /* player.p just broke hyperspace */
            player_setFlag( PLAYER_HOOK_HYPER );
         }
         else {
            pilot_runHook( p, PILOT_HOOK_JUMP ); /* Should be run before messing with delete flag. */
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

         if (p == player.p) {
            if (!player_isFlag(PLAYER_AUTONAV))
               player_message( "\erStrayed too far from jump point: jump aborted." );
         }
      }
      else {
         if (p->ptimer < 0.) { /* engines ready */
            p->ptimer = HYPERSPACE_FLY_DELAY;
            pilot_setFlag(p, PILOT_HYPERSPACE);
            if (p->id == PLAYER_ID) {
               p->timer[0] = -1.;
            }
         }
      }
   }
   /* pilot is getting ready for hyperspace */
   else {
      /* Make sure still within range. */
      can_hyp = space_canHyperspace( p );
      if (!can_hyp) {
         pilot_hyperspaceAbort( p );

         if (p == player.p) {
            if (!player_isFlag(PLAYER_AUTONAV))
               player_message( "\erStrayed too far from jump point: jump aborted." );
         }
      }
      else {

         /* brake */
         if (!pilot_isFlag(p, PILOT_HYP_BRAKE) && (VMOD(p->solid->vel) > MIN_VEL_ERR)) {
            diff = pilot_face( p, VANGLE(p->solid->vel) + M_PI );

            if (ABS(diff) < MAX_DIR_ERR)
               pilot_setThrust( p, 1. );
            else
               pilot_setThrust( p, 0. );
         }
         /* face target */
         else {
            /* Done braking. */
            pilot_setFlag( p, PILOT_HYP_BRAKE);
            pilot_setThrust( p, 0. );

            /* Face system headed to. */
            sys = cur_system->jumps[p->nav_hyperspace].target;
            a = ANGLE( sys->pos.x - cur_system->pos.x, sys->pos.y - cur_system->pos.y );
            diff = pilot_face( p, a );

            if (ABS(diff) < MAX_DIR_ERR) { /* we can now prepare the jump */
               pilot_setTurn( p, 0. );
               p->ptimer = HYPERSPACE_ENGINE_DELAY;
               pilot_setFlag(p, PILOT_HYP_BEGIN);
               /* Player plays sound. */
               if (p->id == PLAYER_ID) {
                  player_soundPlay( snd_hypPowUp, 1 );
               }
            }
         }
      }
   }

   if (p == player.p)
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
   return 1;
}


/**
 * @brief Has the pilot refuel it's target.
 *
 *    @param p Pilot that is actively refueling.
 *    @param dt Current delta tick.
 */
static void pilot_refuel( Pilot *p, double dt )
{
   Pilot *target;

   /* Check to see if target exists, remove flag if not. */
   target = pilot_get(p->target);
   if (target == NULL) {
      pilot_rmFlag(p, PILOT_REFUELBOARDING);
      pilot_rmFlag(p, PILOT_REFUELING);
      return;
   }

   /* Match speeds. */
   vectcpy( &p->solid->vel, &target->solid->vel );

   /* Move fuel. */
   p->fuel        -= PILOT_REFUEL_RATE*dt;
   target->fuel   += PILOT_REFUEL_RATE*dt;
   /* Stop refueling at max. */
   if (target->fuel > target->fuel_max) {
      p->ptimer      = -1.;
      target->fuel   = target->fuel_max;
   }

   /* Check to see if done. */
   if (p->ptimer < 0.) {
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
 *    @param p Pilot to check to see if he has enough credits.
 *    @param amount Amount to check for.
 *    @return 1 if he has enough, 0 otherwise.
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
 *    @param pilot Pilot to initialise.
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
   pilot->crew   = pilot->ship->crew;
   pilot_calcStats(pilot);

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
      pilot->outfit_structure[i].slot.type = OUTFIT_SLOT_STRUCTURE;
      pilot->outfit_structure[i].slot.size = ship->outfit_structure[i].slot.size;
      pilot->outfits[p] = &pilot->outfit_structure[i];
      memcpy( &pilot->outfits[p]->mount, &ship->outfit_structure[i].mount, sizeof(ShipMount) );
      p++;
   }
   for (i=0; i<pilot->outfit_nutility; i++) {
      pilot->outfit_utility[i].slot.type = OUTFIT_SLOT_UTILITY;
      pilot->outfit_utility[i].slot.size = ship->outfit_utility[i].slot.size;
      pilot->outfits[p] = &pilot->outfit_utility[i];
      memcpy( &pilot->outfits[p]->mount, &ship->outfit_utility[i].mount, sizeof(ShipMount) );
      p++;
   }
   for (i=0; i<pilot->outfit_nweapon; i++) {
      pilot->outfit_weapon[i].slot.type = OUTFIT_SLOT_WEAPON;
      pilot->outfit_weapon[i].slot.size = ship->outfit_weapon[i].slot.size;
      pilot->outfits[p] = &pilot->outfit_weapon[i];
      memcpy( &pilot->outfits[p]->mount, &ship->outfit_weapon[i].mount, sizeof(ShipMount) );
      p++;
   }
   /* Second pass set ID. */
   for (i=0; i<pilot->noutfits; i++)
      pilot->outfits[i]->id = i;

   /* cargo - must be set before calcStats */
   pilot->cargo_free = pilot->ship->cap_cargo; /* should get redone with calcCargo */

   /* set the pilot stats based on his ship and outfits */
   pilot_calcStats( pilot );

   /* Sanity check. */
#ifdef DEBUGGING
   const char *str = pilot_checkSanity( pilot );
   if (str != NULL)
      DEBUG( "Pilot '%s' failed sanity check: %s", pilot->name, str );
#endif /* DEBUGGING */

   /* set flags and functions */
   if (pilot_isFlagRaw(flags, PILOT_PLAYER)) {
      pilot->think            = player_think; /* players don't need to think! :P */
      pilot->update           = player_update; /* Players get special update. */
      pilot->render           = NULL; /* render will get called from player_think */
      pilot->render_overlay   = NULL;
      pilot_setFlag(pilot,PILOT_PLAYER); /* it is a player! */
      if (!pilot_isFlagRaw( flags, PILOT_EMPTY )) /* Sort of a hack. */
         player.p = pilot;
   }
   else {
      pilot->think            = ai_think;
      pilot->update           = pilot_update;
      pilot->render           = pilot_render;
      pilot->render_overlay   = pilot_renderOverlay;
   }

   /* Set enter hyperspace flag if needed. */
   if (pilot_isFlagRaw( flags, PILOT_HYP_END ))
      pilot_setFlag(pilot, PILOT_HYP_END);

   /* Escort stuff. */
   if (pilot_isFlagRaw( flags, PILOT_ESCORT )) {
      pilot_setFlag(pilot,PILOT_ESCORT);
      if (pilot_isFlagRaw( flags, PILOT_CARRIED ))
         pilot_setFlag(pilot,PILOT_CARRIED);
   }

   /* Clear timers. */
   pilot_clearTimers(pilot);

   /* Update the x and y sprite positions. */
   gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,
         pilot->ship->gfx_space, pilot->solid->dir );

   /* Targets. */
   pilot->target           = pilot->id; /* Self = no target. */
   pilot->nav_planet       = -1;
   pilot->nav_hyperspace   = -1;

   /* Check takeoff. */
   if (pilot_isFlagRaw( flags, PILOT_TAKEOFF )) {
      pilot_setFlag( pilot, PILOT_TAKEOFF );
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
   memcpy( dest, src, sizeof(Pilot) );

   /* Copy names. */
   if (src->name)
      dest->name = strdup(src->name);
   if (src->title)
      dest->title = strdup(src->title);

   /* Copy solid. */
   dest->solid = malloc(sizeof(Solid));
   memcpy( dest->solid, src->solid, sizeof(Solid) );

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
      pilot_cargoAddRaw( dest, src->commodities[i].commodity,
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
   else {
      pilot_nstack = 0;
   }

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
      if (!p->think)
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
   int i;
   PilotOutfitSlot *o;

   pilot->ptimer = 0.;
   pilot->tcontrol = 0.;
   for (i=0; i<MAX_AI_TIMERS; i++)
      pilot->timer[i] = 0.;
   for (i=0; i<pilot->noutfits; i++) {
      o = pilot->outfits[i];
      if (o->timer > 0.)
         o->timer = 0.;
   }
}


/**
 * @brief Updates the systemFleet of all pilots.
 *
 * @param index Index number that was deleted.
 */
void pilots_updateSystemFleet( const int deletedIndex ) {
   int i;

   for(i = 0; i < pilot_nstack; i++)
      if(pilot_stack[i]->systemFleet >= deletedIndex)
         pilot_stack[i]->systemFleet--;

   return;
}

/**
 * @brief Gets the relative size(shipmass) between the current pilot and the specified target
 *
 * @param p the pilot whose mass we will compare   
 *    @luareturn A number from 0 to 1 mapping the relative masses
 * relsize()
 */
double pilot_relsize(const Pilot* cur_pilot, const Pilot* p)
{
    /*double mass_map;
    
    mass_map = 1 - 1/(1 + ( (double) cur_pilot -> solid -> mass / (double) p->solid->mass );*/
    
    return (1 - 1/(1 + ( (double) cur_pilot -> solid -> mass / (double) p->solid->mass) ) );
    }

/**
 * @brief Gets the relative damage output(total DPS) between the current pilot and the specified target
 *
 * @param p the pilot whose dps we will compare   
 *    @return A number from 0 to 1 mapping the relative damage output
 * reldps()
 */
double pilot_reldps(const Pilot* cur_pilot, const Pilot* p)
{
    int i;

    int DPSaccum_target = 0, DPSaccum_pilot = 0;
    double delay_cache, damage_cache;

    for(i = 0; i < p->outfit_nweapon; i++)
    {
       /*DPSaccum_target += ( outfit_damage(p->outfit_weapon[i].outfit)/outfit_delay(p->outfit_weapon[i].outfit) );*/
       if(p->outfit_weapon[i].outfit){
       damage_cache = outfit_damage(p->outfit_weapon[i].outfit);
        delay_cache = outfit_delay(p->outfit_weapon[i].outfit);
        if(damage_cache > 0 && delay_cache > 0)
           DPSaccum_target += ( damage_cache/delay_cache );}

    }

    for(i = 0; i < cur_pilot->outfit_nweapon; i++)
    {
      
        /*DPSaccum_pilot += ( outfit_damage(cur_pilot->outfit_weapon[i].outfit)/outfit_delay(cur_pilot->outfit_weapon[i].outfit) );*/

        if(cur_pilot->outfit_weapon[i].outfit) {
        damage_cache = outfit_damage(cur_pilot->outfit_weapon[i].outfit);
        delay_cache = outfit_delay(cur_pilot->outfit_weapon[i].outfit);
        if(damage_cache > 0 && delay_cache > 0)
           DPSaccum_pilot += ( damage_cache/delay_cache );}

    }

    if(DPSaccum_target > 0 && DPSaccum_pilot > 0)
        return (1 - 1/(1 + ( (double) DPSaccum_pilot / (double) DPSaccum_target) ) );
    else if (DPSaccum_pilot > 0)
        return 1;
    else
        return 0;

}
    
/**
 * @brief Gets the relative hp(combined shields and armor) between the current pilot and the specified target
 *
 * @param p the pilot whose shields/armor we will compare   
 *    @return A number from 0 to 1 mapping the relative HPs
 * relhp() 
 */
double pilot_relhp(const Pilot* cur_pilot, const Pilot* p)
{
    return (1 - 1/(1 + ( (double) (cur_pilot -> armour_max + cur_pilot -> shield_max ) / (double) (p -> armour_max + p -> shield_max) ) ) );

    }
