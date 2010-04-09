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


#define PILOT_CHUNK_MIN 128 /**< Maximum chunks to increment pilot_stack by */
#define PILOT_CHUNK_MAX 2048 /**< Minimum chunks to increment pilot_stack by */
#define CHUNK_SIZE      32 /**< Size to allocate memory by. */


/* ID Generators. */
static unsigned int pilot_id = PLAYER_ID; /**< Stack of pilot ids to assure uniqueness */
static unsigned int mission_cargo_id = 0; /**< ID generator for special mission cargo.
                                               Not guaranteed to be absolutely unique, 
                                               only unique for each pilot. */


/* stack of pilot_nstack */
Pilot** pilot_stack = NULL; /**< Not static, used in player.c, weapon.c, pause.c, space.c and ai.c */
int pilot_nstack = 0; /**< same */
static int pilot_mstack = 0; /**< Memory allocated for pilot_stack. */


/* misc */
static double sensor_curRange    = 0.; /**< Current base sensor range, used to calculate
                                         what is in range and what isn't. */
static double pilot_commTimeout  = 15.; /**< Time for text above pilot to time out. */
static double pilot_commFade     = 5.; /**< Time for text above pilot to fade out. */


/*
 * prototyes
 */
/* update. */
static int pilot_shootWeapon( Pilot* p, PilotOutfitSlot* w );
static void pilot_hyperspace( Pilot* pilot, double dt );
static void pilot_refuel( Pilot *p, double dt );
/* cargo. */
static int pilot_rmCargoRaw( Pilot* pilot, Commodity* cargo, int quantity, int cleanup );
static void pilot_calcCargo( Pilot* pilot );
static int pilot_addCargoRaw( Pilot* pilot, Commodity* cargo,
      int quantity, unsigned int id );
/* clean up. */
void pilot_free( Pilot* p ); /* externed in player.c */
static void pilot_dead( Pilot* p );
/* misc */
static void pilot_setCommMsg( Pilot *p, const char *s );
static int pilot_getStackPos( const unsigned int id );
static void pilot_updateMass( Pilot *pilot );
extern int landtarget; /* From player.c  */


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
   d = 0.;
   for (i=0; i<pilot_nstack; i++) {
      /* Must not be bribed. */
      if ((pilot_stack[i]->faction == FACTION_PLAYER) && pilot_isFlag(p,PILOT_BRIBED))
         continue;

      if (!pilot_isFlag( pilot_stack[i], PILOT_INVISIBLE ) &&
            (areEnemies(p->faction, pilot_stack[i]->faction) || /* Enemy faction. */
               ((pilot_stack[i]->id == PLAYER_ID) && 
                  pilot_isFlag(p,PILOT_HOSTILE)))) { /* Hostile to player. */

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
 * @brief Get the nearest pilot to a pilot.
 *
 *    @param p Pilot to get his nearest pilot.
 *    @return The nearest pilot.
 */
unsigned int pilot_getNearestPilot( const Pilot* p )
{
   unsigned int tp;
   int i;
   double d, td;

   tp=PLAYER_ID;
   d=0;
   for (i=0; i<pilot_nstack; i++) {
      if (pilot_stack[i] == p)
         continue;

      /* Player doesn't select escorts. */
      if ((p->faction == FACTION_PLAYER) &&
            (pilot_stack[i]->faction == FACTION_PLAYER))
         continue;

      /* Shouldn't be disabled. */
      if (pilot_isDisabled(pilot_stack[i]))
         continue;

      /* Shouldn't be invisible. */
      if (pilot_isFlag( pilot_stack[i], PILOT_INVISIBLE ))
         continue;

      /* Must be in range. */
      if (!pilot_inRangePilot( p, pilot_stack[i] ))
         continue;

      td = vect_dist2(&pilot_stack[i]->solid->pos, &player.p->solid->pos);       
      if (((tp==PLAYER_ID) || (td < d))) {
         d = td;
         tp = pilot_stack[i]->id;
      }
   }
   return tp;
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
   p->solid->force_x = p->thrust * thrust;
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
 * @brief Check to see if a position is in range of the pilot.
 *
 *    @param p Pilot to check to see if position is in his sensor range.
 *    @param x X position to check.
 *    @param y Y position to check.
 *    @return 1 if the position is in range, 0 if it isn't.
 */
int pilot_inRange( const Pilot *p, double x, double y )
{
   double d;

   if (cur_system->interference == 0.)
      return 1;

   /* Get distance. */
   d = pow2(x-p->solid->pos.x) + pow2(y-p->solid->pos.y);

   if (d < sensor_curRange)
      return 1;

   return 0;
}


/**
 * @brief Check to see if a pilot is in sensor range of another.
 *
 *    @param p Pilot who is trying to check to see if other is in sensor range.
 *    @param target Target of p to check to see if is in sensor range.
 *    @return 1 if they are in range, 0 if they aren't.
 */
int pilot_inRangePilot( const Pilot *p, const Pilot *target )
{
   double d;

   if (cur_system->interference == 0.)
      return 1;

   /* Get distance. */
   d = vect_dist2( &p->solid->pos, &target->solid->pos );

   if (d < sensor_curRange)
      return 1;

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
   (void)p;
   (void)target;

   /* Always consider planets in range. */
   return 1;

#if 0
   double d;
   Planet *pnt;

   if (cur_system->interference == 0.)
      return 1;

   /* Get the planet. */
   pnt = cur_system->planets[target];

   /* Get distance. */
   d = vect_dist2( &p->solid->pos, &pnt->pos );

   if (d < sensor_curRange)
      return 1;

   return 0;
#endif
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
 * @brief Gets the quantity of a pilot outfit.
 *
 *    @param p Pilot to which the outfit belongs.
 *    @param w Outfit to check quantity of.
 *    @return The amount of the outfit the pilot has.
 */
int pilot_oquantity( Pilot* p, PilotOutfitSlot* w )
{
   return (outfit_isAmmo(w->outfit) && p->secondary) ?
      p->secondary->quantity : w->quantity ;
}


/**
 * @brief Makes the pilot shoot.
 *
 *    @param p The pilot which is shooting.
 *    @param type Indicate what shoot group to use.
 *           0 = all
 *           1 = turrets
 *           2 = forward
 *    @return The number of shots fired.
 */
int pilot_shoot( Pilot* p, int group )
{
   int i, ret;
   Outfit* o;

   if (!p->outfits) return 0; /* no outfits */

   ret = 0;
   for (i=0; i<p->outfit_nhigh; i++) { /* cycles through outfits to find primary weapons */
      o = p->outfit_high[i].outfit;

      if (o==NULL)
         continue;

      if (!outfit_isProp(o,OUTFIT_PROP_WEAP_SECONDARY) &&
            (outfit_isBolt(o) || outfit_isBeam(o) || outfit_isLauncher(o))) {

         /* Choose what to shoot dependent on type. */
         if ((group == 0) ||
               ((group == 1) && outfit_isTurret(o)) ||
               ((group == 2) && !outfit_isTurret(o))) {
            ret += pilot_shootWeapon( p, &p->outfit_high[i] );
         }
      }
   }

   return ret;
}


/**
 * @brief Makes the pilot shoot it's currently selected secondary weapon.
 *
 *    @param p The pilot which is to shoot.
 *    @return The number of shots fired.
 */
int pilot_shootSecondary( Pilot* p )
{
   int i, ret;

   /* No secondary weapon. */
   if (p->secondary == NULL)
      return 0;

   /* Fire all secondary weapon of same type. */
   ret = 0;
   for (i=0; i<p->outfit_nhigh; i++) {
      if (p->outfit_high[i].outfit == p->secondary->outfit)
         ret += pilot_shootWeapon( p, &p->outfit_high[i] );
   }

   return ret;
}


/**
 * @brief Have pilot stop shooting his weapon.
 *
 * Only really deals with beam weapons.
 *
 *    @param p Pilot that was shooting.
 *    @param secondary If weapon is secondary.
 */
void pilot_shootStop( Pilot* p, const int secondary )
{
   int i;
   Outfit* o;

   /* Non beam secondaries don't matter. */
   if (secondary && ((p->secondary == NULL) ||
            (p->secondary->outfit == NULL) ||
            !outfit_isBeam(p->secondary->outfit)))
      return;

   /* Iterate over them all. */
   for (i=0; i<p->outfit_nhigh; i++) {
      o = p->outfit_high[i].outfit;

      /* Must have outfit. */
      if (o==NULL)
         continue;
      /* Must be beam. */
      if (!outfit_isBeam(o))
         continue;

      if (secondary) {
         if (o == p->secondary->outfit) {
            if (p->outfit_high[i].u.beamid > 0) {
               beam_end( p->id, p->outfit_high[i].u.beamid );
               p->outfit_high[i].u.beamid = 0;
            }
         }
      }
      else {
         if (!outfit_isProp(o, OUTFIT_PROP_WEAP_SECONDARY)) {
            if (p->outfit_high[i].u.beamid > 0) {
               beam_end( p->id, p->outfit_high[i].u.beamid );
               p->outfit_high[i].u.beamid = 0;
            }
         }
      }
   }
}


/**
 * @brief Gets the mount position of a pilot.
 *
 * Position is relative to the pilot.
 *
 *    @param p Pilot to get mount position of.
 *    @param id ID of the mount.
 *    @param[out] v Position of the mount.
 *    @return 0 on success.
 */
int pilot_getMount( const Pilot *p, const PilotOutfitSlot *w, Vector2d *v )
{
   double a, x, y;
   double cm, sm;
   const ShipMount *m;

   /* Calculate the sprite angle. */
   a  = (double)(p->tsy * p->ship->gfx_space->sx + p->tsx);
   a *= p->ship->mangle;

   /* 2d rotation matrix
    * [ x' ]   [  cos  sin  ]   [ x ]
    * [ y' ] = [ -sin  cos  ] * [ y ]
    *
    * dir is inverted so that rotation is counter-clockwise.
    */
   m = &w->mount;
   cm = cos(-a);
   sm = sin(-a);
   x = m->x * cm + m->y * sm;
   y = m->x *-sm + m->y * cm;

   /* Correction for ortho perspective. */
   y *= M_SQRT1_2;

   /* Don't forget to add height. */
   y += m->h;

   /* Get the mount and add the player.p offset. */
   vect_cset( v, x, y );

   return 0;
}


/**
 * @brief Actually handles the shooting, how often the player.p can shoot and such.
 *
 *    @param p Pilot that is shooting.
 *    @param w Pilot's outfit to shoot.
 *    @return 0 if nothing was shot and 1 if something was shot.
 */
static int pilot_shootWeapon( Pilot* p, PilotOutfitSlot* w )
{
   Vector2d vp, vv;
   int i;
   PilotOutfitSlot *slot;
   int minp;
   double q, mint;
   int is_launcher;
   double rate_mod, energy_mod;

   /* check to see if weapon is ready */
   if (w->timer > 0.)
      return 0;

   /* See if is launcher. */
   is_launcher = outfit_isLauncher(w->outfit);

   /* Calculate rate modifier. */
   switch (w->outfit->type) {
      case OUTFIT_TYPE_BOLT:
         rate_mod   = 2. - p->stats.firerate_forward; /* invert. */
         energy_mod = p->stats.energy_forward;
         break;
      case OUTFIT_TYPE_TURRET_BOLT:
         rate_mod   = 2. - p->stats.firerate_turret; /* invert. */
         energy_mod = p->stats.energy_turret;
         break;

      default:
         rate_mod   = 1.;
         energy_mod = 1.;
         break;
   }

   /* Count the outfits and current one - only affects non-beam. */
   if (!outfit_isBeam(w->outfit)) {

      /* Calculate last time weapon was fired. */
      q     = 0.;
      minp  = -1;
      for (i=0; i<p->outfit_nhigh; i++) {
         slot = &p->outfit_high[i];

         /* No outfit. */
         if (slot->outfit == NULL)
            continue;

         /* Not what we are looking for. */
         if (outfit_delay(slot->outfit) != outfit_delay(w->outfit))
            continue;

         /* Launcher only counts with ammo. */
         if (is_launcher && ((w->u.ammo.outfit == NULL) || (w->u.ammo.quantity <= 0)))
            continue;

         /* Save some stuff. */
         if ((minp < 0) || (slot->timer > mint)) {
            minp = i;
            mint = slot->timer;
         }
         q++;
      }

      /* Q must be valid. */
      if (q == 0)
         return 0;

      /* Only fire if the last weapon to fire fired more than (q-1)/q ago. */
      if (mint > rate_mod * outfit_delay(w->outfit) * ((q-1) / q))
         return 0;
   }

   /* Get weapon mount position. */
   pilot_getMount( p, w, &vp );
   vp.x += p->solid->pos.x;
   vp.y += p->solid->pos.y;

   /* Modify velocity to take into account the rotation. */
   vect_cset( &vv, p->solid->vel.x + vp.x*p->solid->dir_vel,
         p->solid->vel.y + vp.y*p->solid->dir_vel );

   /*
    * regular bolt weapons
    */
   if (outfit_isBolt(w->outfit)) {
      
      /* enough energy? */
      if (outfit_energy(w->outfit)*energy_mod > p->energy)
         return 0;

      p->energy -= outfit_energy(w->outfit)*energy_mod;
      weapon_add( w->outfit, p->solid->dir,
            &vp, &p->solid->vel, p, p->target );
   }

   /*
    * Beam weapons.
    */
   else if (outfit_isBeam(w->outfit)) {

      /* Check if enough energy to last a second. */
      if (outfit_energy(w->outfit)*energy_mod > p->energy)
         return 0;

      /** @todo Handle warmup stage. */
      w->state = PILOT_OUTFIT_ON;
      w->u.beamid = beam_start( w->outfit, p->solid->dir,
            &vp, &p->solid->vel, p, p->target, w );
   }

   /*
    * missile launchers
    *
    * must be a secondary weapon
    */
   else if (outfit_isLauncher(w->outfit)) {

      /* Shooter can't be the target - sanity check for the player.p */
      if ((w->outfit->u.lau.ammo->u.amm.ai > 0) && (p->id==p->target))
         return 0;

      /* Must have ammo left. */
      if ((w->u.ammo.outfit == NULL) || (w->u.ammo.quantity <= 0))
         return 0;

      /* enough energy? */
      if (outfit_energy(w->u.ammo.outfit)*energy_mod > p->energy)
         return 0;

      p->energy -= outfit_energy(w->u.ammo.outfit)*energy_mod;
      weapon_add( w->u.ammo.outfit, p->solid->dir,
            &vp, &p->solid->vel, p, p->target );

      w->u.ammo.quantity -= 1; /* we just shot it */
      p->mass_outfit     -= w->u.ammo.outfit->mass;
      pilot_updateMass( p );
   }

   /*
    * Fighter bays.
    */
   else if (outfit_isFighterBay(w->outfit)) {

      /* Must have ammo left. */
      if ((w->u.ammo.outfit == NULL) || (w->u.ammo.quantity <= 0))
         return 0;

      /* Create the escort. */
      escort_create( p, w->u.ammo.outfit->u.fig.ship,
            &vp, &p->solid->vel, p->solid->dir, ESCORT_TYPE_BAY, 1 );

      w->u.ammo.quantity -= 1; /* we just shot it */
      p->mass_outfit     -= w->u.ammo.outfit->mass;
      w->u.ammo.deployed += 1; /* Mark as deployed. */
      pilot_updateMass( p );
   }

   else {
      WARN("Shooting unknown weapon type: %s", w->outfit->name);
   }

   /* Reset timer. */
   w->timer += rate_mod * outfit_delay( w->outfit );

   return 1;
}



/**
 * @brief Sets the pilot's secondary weapon.
 *
 *    @param p Pilot to set secondary weapon.
 *    @param i Index of the weapon to set to.
 */
void pilot_switchSecondary( Pilot* p, PilotOutfitSlot* w )
{
   pilot_shootStop( p, 1 );
   player.p->secondary = w;
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
      const DamageType dtype, const double damage )
{
   int mod, h;
   double damage_shield, damage_armour, knockback, dam_mod, dmg;
   Pilot *pshooter;

   /* Defaults. */
   pshooter = NULL;
   dam_mod  = 0.;
   dmg      = 0.;

   /* calculate the damage */
   outfit_calcDamage( &damage_shield, &damage_armour, &knockback, dtype, damage );

   /* Player breaks autonav. */
   if ((w != NULL) && (p->id == PLAYER_ID) &&
         !pilot_isFlag(player.p, PILOT_HYP_BEGIN) &&
         !pilot_isFlag(player.p, PILOT_HYPERSPACE))
      player_abortAutonav("Sustaining Damage");

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
   }
   /*
    * Armour takes the entire blow.
    */
   else if (p->armour > 0.) {
      dmg        = damage_armour;
      p->armour -= damage_armour;
   }

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
      pilot_runHook( p, PILOT_HOOK_DISABLE );
   }

   /* Officially dead. */
   if (p->armour <= 0.) {
      p->armour = 0.;
      dam_mod   = 0.;

      if (!pilot_isFlag(p, PILOT_DEAD)) {
         pilot_setFlag( p, PILOT_DISABLED ); /* Player isn't disabled so we must disable here. */
         pilot_dead(p);

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
 */
void pilot_dead( Pilot* p )
{
   if (pilot_isFlag(p,PILOT_DEAD))
      return; /* he's already dead */

   /* basically just set timers */
   if (p->id==PLAYER_ID)
      player_dead();
   p->timer[0] = 0.; /* no need for AI anymore */
   p->ptimer = 1. + sqrt(10*p->armour_max*p->shield_max) / 1500.;
   p->timer[1] = 0.; /* explosion timer */

   /* flag cleanup - fixes some issues */
   if (pilot_isFlag(p,PILOT_HYP_PREP))
      pilot_rmFlag(p,PILOT_HYP_PREP);
   if (pilot_isFlag(p,PILOT_HYP_BEGIN))
      pilot_rmFlag(p,PILOT_HYP_BEGIN);
   if (pilot_isFlag(p,PILOT_HYPERSPACE))
      pilot_rmFlag(p,PILOT_HYPERSPACE);

   /* PILOT R OFFICIALLY DEADZ0R */
   pilot_setFlag(p,PILOT_DEAD);

   /* Pilot must die before setting death flag and probably messing with other flags. */
   pilot_runHook( p, PILOT_HOOK_DEATH );
}


/**
 * @brief Tries to run a pilot hook if he has it.
 *
 *    @param p Pilot to run the hook.
 *    @param hook_type Type of hook to run.
 *    @return The number of hooks run.
 */
int pilot_runHook( Pilot* p, int hook_type )
{
   int i, run, ret;
   run = 0;
   for (i=0; i<p->nhooks; i++) {
      if (p->hooks[i].type == hook_type) {
         ret = hook_runID( p->hooks[i].id );
         if (ret)
            WARN("Pilot '%s' failed to run hook type %d", p->name, hook_type);
         run++;
      }
   }
   return run;
}


/**
 * @brief Docks the pilot on it's target pilot.
 *
 *    @param p Pilot that wants to dock.
 *    @param target Pilot to dock on.
 *    @param deployed Was pilot already deployed?
 *    @return 0 on successful docking.
 */
int pilot_dock( Pilot *p, Pilot *target, int deployed )
{
   int i;
   Outfit *o = NULL;

   /* Must be close. */
   if (vect_dist(&p->solid->pos, &target->solid->pos) >
         target->ship->gfx_space->sw * PILOT_SIZE_APROX )
      return -1;

   /* Cannot be going much faster. */
   if ((pow2(VX(p->solid->vel)-VX(target->solid->vel)) +
            pow2(VY(p->solid->vel)-VY(target->solid->vel))) >
         (double)pow2(MAX_HYPERSPACE_VEL))
      return -1;

   /* Check to see if target has an available bay. */
   for (i=0; i<target->noutfits; i++) {

      /* Must have outfit. */
      if (target->outfits[i]->outfit == NULL)
         continue;

      /* Must be fighter bay. */
      if (!outfit_isFighterBay(target->outfits[i]->outfit))
         continue;

      /* Must have deployed. */
      if (deployed && (target->outfits[i]->u.ammo.deployed <= 0))
         continue;

      o = outfit_ammo(target->outfits[i]->outfit);

      /* Try to add fighter. */
      if (outfit_isFighter(o) &&
            (strcmp(p->ship->name,o->u.fig.ship)==0)) {
         if (deployed)
            target->outfits[i]->u.ammo.deployed -= 1;
         break;
      }
   }
   if ((o==NULL) || (i >= target->noutfits))
      return -1;

   /* Add the pilot's outfit. */
   if (pilot_addAmmo(target, target->outfits[i], o, 1) != 1)
      return -1;

   /* Remove from pilot's escort list. */
   if (deployed) {
      for (i=0; i<target->nescorts; i++) {
         if ((target->escorts[i].type == ESCORT_TYPE_BAY) &&
               (target->escorts[i].id == p->id))
            break;
      }
      /* Not found as pilot's escorts. */
      if (i >= target->nescorts)
         return -1;
      /* Free if last pilot. */
      if (target->nescorts == 1) {
         free(target->escorts);
         target->escorts   = NULL;
         target->nescorts  = 0;
      }
      else {
         memmove( &target->escorts[i], &target->escorts[i+1],
               sizeof(Escort_t) * (target->nescorts-i-1) );
         target->nescorts--;
      }
   }

   /* Destroy the pilot. */
   pilot_delete(p);

   return 0;
}


/**
 * @brief Checks to see if the pilot has deployed ships.
 *
 *    @param p Pilot to see if has deployed ships.
 *    @return 1 if pilot has deployed ships, 0 otherwise.
 */
int pilot_hasDeployed( Pilot *p )
{
   int i;
   for (i=0; i<p->noutfits; i++) {
      if (p->outfits[i]->outfit == NULL)
         continue;
      if (outfit_isFighterBay(p->outfits[i]->outfit))
         if (p->outfits[i]->u.ammo.deployed > 0)
            return 1;
   }
   return 0;
}


/**
 * @brief Makes the pilot explosion.
 *    @param x X position of the pilot.
 *    @param y Y position of the pilot.
 *    @param radius Radius of the explosion.
 *    @param dtype Damage type of the explosion.
 *    @param damage Amount of damage by the explosion.
 *    @param parent The exploding pilot.
 */
void pilot_explode( double x, double y, double radius,
      DamageType dtype, double damage, const Pilot *parent )
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
         pilot_hit( p, &s, (parent!=NULL)?parent->id:0, dtype, damage );

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

         /* Display text. */
         gl_printRaw( NULL, x - p->comm_msgWidth/2. + SCREEN_W/2.,
               y + PILOT_SIZE_APROX*p->ship->gfx_space->sh/2. + SCREEN_H/2.,
               &c, p->comm_msg );
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

   /*
    * Update timers.
    */
   pilot->ptimer -= dt;
   pilot->tcontrol -= dt;
   for (i=0; i<MAX_AI_TIMERS; i++)
      if (pilot->timer[i] > 0.)
         pilot->timer[i] -= dt;
   for (i=0; i<pilot->noutfits; i++) {
      o = pilot->outfits[i];
      if (o->timer > 0.)
         o->timer -= dt;
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
            pilot_rmFlag(pilot,PILOT_LANDING);
            land( cur_system->planets[ landtarget ] );
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
         snprintf(buf, 16, "explosion%d", RNG(0,2));
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
               MAX(0., 2. * (a * (1. + sqrt(pilot->fuel + 1.) / 28.))),
               NULL, EXPL_MODE_SHIP );
         debris_add( pilot->solid->mass, pilot->ship->gfx_space->sw/2.,
               pilot->solid->pos.x, pilot->solid->pos.y,
               pilot->solid->vel.x, pilot->solid->vel.y );
         pilot_setFlag(pilot,PILOT_EXPLODED);

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
   else if (pilot->armour <= 0.) /* PWNED */
      pilot_dead(pilot); /* start death stuff */

   /* purpose fallthrough to get the movement like disabled */
   if (pilot_isDisabled(pilot)) {
      /* Do the slow brake thing */
      vect_pset( &pilot->solid->vel, /* slowly brake */
         VMOD(pilot->solid->vel) * (1. - dt*0.10),
         VANGLE(pilot->solid->vel) );
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
   pilot->shield += pilot->shield_regen * dt;
   if (pilot->shield > pilot->shield_max)
      pilot->shield = pilot->shield_max;

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
   if (pilot->solid->force_x > 0) {
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

   /* update the solid */
   pilot->solid->update( pilot->solid, dt );
   gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,  
         pilot->ship->gfx_space, pilot->solid->dir );

   if (!pilot_isFlag(pilot, PILOT_HYPERSPACE)) { /* limit the speed */

      /* pilot is afterburning */
      if (pilot_isFlag(pilot, PILOT_AFTERBURNER) && /* must have enough energy left */
               (pilot->energy > pilot->afterburner->outfit->u.afb.energy * dt)) {
         limit_speed( &pilot->solid->vel, /* limit is higher */
               pilot->speed * pilot->afterburner->outfit->u.afb.speed, dt );

         if (pilot->id == PLAYER_ID)
            spfx_shake( 0.75*SHAKE_DECAY * dt); /* shake goes down at quarter speed */

         pilot->energy -= pilot->afterburner->outfit->u.afb.energy * dt; /* energy loss */
      }
      else /* normal limit */
         limit_speed( &pilot->solid->vel, pilot->speed, dt );
   }
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
   JumpPoint *jp;

   /* pilot is actually in hyperspace */
   if (pilot_isFlag(p, PILOT_HYPERSPACE)) {

      /* Time to play sound. */
      if ((p->id == PLAYER_ID) &&
            (p->ptimer < sound_length(snd_hypPowUpJump)) &&
            (p->timer[0] == -1.)) {
         p->timer[0] = -2.;
         player_playSound( snd_hypPowUpJump, 1 );
      }

      /* has jump happened? */
      if (p->ptimer < 0.) {
         if (p->id == PLAYER_ID) { /* player.p just broke hyperspace */
            player_brokeHyperspace();
         }
         else {
            pilot_delete(p);
            pilot_runHook( p, PILOT_HOOK_JUMP ); /* Should be run before messing with delete flag. */
         }
         return;
      }

      /* keep acceling - hyperspace uses much bigger accel */
      pilot_setThrust( p, HYPERSPACE_THRUST*p->solid->mass/p->thrust );
   }
   /* engines getting ready for the jump */
   else if (pilot_isFlag(p, PILOT_HYP_BEGIN)) {

      if (p->ptimer < 0.) { /* engines ready */
         p->ptimer = HYPERSPACE_FLY_DELAY;
         pilot_setFlag(p, PILOT_HYPERSPACE);
         if (p->id == PLAYER_ID) {
            p->timer[0] = -1.;
         }
      }
   }
   /* pilot is getting ready for hyperspace */
   else {
      /* Make sure still within range. */
      jp = &cur_system->jumps[ p->nav_hyperspace ];
      if (jp->radius*jp->radius < vect_dist2( &p->solid->pos, &jp->pos ) ) {
         pilot_hyperspaceAbort( p );

         if (p == player.p) {
            if (!player_isFlag(PLAYER_AUTONAV))
               player_message( "\erStrayed too far from jump point: jump aborted." );
         }
      }
      else {

         /* brake */
         if (VMOD(p->solid->vel) > MIN_VEL_ERR) {
            diff = pilot_face( p, VANGLE(p->solid->vel) + M_PI );

            if (ABS(diff) < MAX_DIR_ERR)
               pilot_setThrust( p, 1. );
            else
               pilot_setThrust( p, 0. );

         }
         /* face target */
         else {

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
                  player_playSound( snd_hypPowUp, 1 );
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
   if (!pilot_isFlag(p, PILOT_HYPERSPACE)) {
      if (pilot_isFlag(p, PILOT_HYP_BEGIN)) {
         /* Player plays sound. */
         if (p->id == PLAYER_ID) {
            player_stopSound();
            player_playSound( snd_hypPowDown, 1 );
         }
      }
      pilot_rmFlag(p, PILOT_HYP_BEGIN);
      pilot_rmFlag(p, PILOT_HYP_PREP);
   }
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
 * @brief Adds an outfit to the pilot, ignoring CPU or other limits.
 *
 * @note Does not call pilot_calcStats().
 *
 *    @param pilot Pilot to add the outfit to.
 *    @param outfit Outfit to add to the pilot.
 *    @param s Slot to add ammo to.
 *    @return 0 on success.
 */
int pilot_addOutfitRaw( Pilot* pilot, Outfit* outfit, PilotOutfitSlot *s )
{
   /* Set the outfit. */
   s->outfit   = outfit;
   s->quantity = 1; /* Sort of pointless, but hey. */

   /* Set some default parameters. */
   s->timer    = 0.;

   /* Some per-case scenarios. */
   if (outfit_isFighterBay(outfit)) {
      s->u.ammo.outfit   = NULL;
      s->u.ammo.quantity = 0;
      s->u.ammo.deployed = 0;
   }
   if (outfit_isTurret(outfit)) { /* used to speed up AI */
      pilot_setFlag(pilot, PILOT_HASTURRET);
   }
   if (outfit_isBeam(outfit)) { /* Used to speed up some calculations. */
      s->u.beamid = -1;
      pilot_setFlag(pilot, PILOT_HASBEAMS);
   }
   if (outfit_isLauncher(outfit)) {
      s->u.ammo.outfit   = NULL;
      s->u.ammo.quantity = 0;
      s->u.ammo.deployed = 0; /* Just in case. */
   }

   return 0;
}


/**
 * @brief Tests to see if an outfit can be added.
 *
 *    @param pilot Pilot to add outfit to.
 *    @param outfit Outfit to add.
 *    @param s Slot adding outfit to.
 *    @param warn Whether or not should generate a warning.
 *    @return 0 if can add, -1 if can't.
 */
int pilot_addOutfitTest( Pilot* pilot, Outfit* outfit, PilotOutfitSlot *s, int warn )
{
   const char *str;

   /* See if slot has space. */
   if (s->outfit != NULL) {
      if (warn)
         WARN( "Pilot '%s': trying to add outfit '%s' to slot that already has an outfit",
               pilot->name, outfit->name );
      return -1;
   }
   else if ((outfit_cpu(outfit) > 0) &&
         (pilot->cpu < outfit_cpu(outfit))) {
      if (warn)
         WARN( "Pilot '%s': Not enough CPU to add outfit '%s'",
               pilot->name, outfit->name );
      return -1;
   }
   else if ((str = pilot_canEquip( pilot, s, outfit, 1)) != NULL) {
      if (warn)
         WARN( "Pilot '%s': Trying to add outfit but %s",
               pilot->name, str );
      return -1;
   }
   return 0;
}



/**
 * @brief Adds an outfit to the pilot.
 *
 *    @param pilot Pilot to add the outfit to.
 *    @param outfit Outfit to add to the pilot.
 *    @param s Slot to add ammo to.
 *    @return 0 on success.
 */
int pilot_addOutfit( Pilot* pilot, Outfit* outfit, PilotOutfitSlot *s )
{
   int ret;

   /* Test to see if outfit can be added. */
   ret = pilot_addOutfitTest( pilot, outfit, s, 1 );
   if (ret != 0)
      return -1;

   /* Add outfit. */
   ret = pilot_addOutfitRaw( pilot, outfit, s );

   /* Recalculate the stats */
   pilot_calcStats(pilot);

   return ret;
}


/**
 * @brief Removes an outfit from the pilot without doing any checks.
 *
 * @note Does not run pilot_calcStats().
 *
 *    @param pilot Pilot to remove the outfit from.
 *    @param s Slot to remove.
 *    @return 0 on success.
 */
int pilot_rmOutfitRaw( Pilot* pilot, PilotOutfitSlot *s )
{
   int ret;

   /* Remove the outfit. */
   ret         = (s->outfit==NULL);
   s->outfit   = NULL;

   /* Remove secondary and such if necessary. */
   if (pilot->secondary == s)
      pilot->secondary = NULL;
   if (pilot->afterburner == s)
      pilot->afterburner = NULL;

   return ret;
}


/**
 * @brief Removes an outfit from the pilot.
 *
 *    @param pilot Pilot to remove the outfit from.
 *    @param s Slot to remove.
 *    @return 0 on success.
 */
int pilot_rmOutfit( Pilot* pilot, PilotOutfitSlot *s )
{
   const char *str;
   int ret;

   str = pilot_canEquip( pilot, s, s->outfit, 0 );
   if (str != NULL) {
      WARN("Pilot '%s': Trying to remove outfit but %s",
            pilot->name, str );
      return -1;
   }

   ret = pilot_rmOutfitRaw( pilot, s );

   /* recalculate the stats */
   pilot_calcStats(pilot);

   return ret;
}


/**
 * @brief Pilot sanity check - makes sure stats are sane.
 *
 *    @param p Pilot to check.
 */
const char* pilot_checkSanity( Pilot *p )
{
   if (p->cpu < 0)
      return "Negative CPU";

   /* Movement. */
   if (p->thrust < 0)
      return "Negative Thrust";
   if (p->speed < 0)
      return "Negative Speed";
   if (p->turn < 0)
      return "Negative Turn";

   /* Health. */
   if (p->armour_max < 0)
      return "Negative Armour";
   if (p->armour_regen < 0)
      return "Negative Armour Regeneration";
   if (p->shield_max < 0)
      return "Negative Shield";
   if (p->shield_regen < 0)
      return "Negative Shield Regeneration";
   if (p->energy_max < 0)
      return "Negative Energy";
   if (p->energy_regen < 0)
      return "Negative Energy Regeneration";

   /* Misc. */
   if (p->fuel_max < 0)
      return "Negative Fuel Maximum";

   /* All OK. */
   return NULL;
}


/**
 * @brief Checks to see if can equip/remove an outfit from a slot.
 *
 *    @return NULL if can swap, or error message if can't.
 */
const char* pilot_canEquip( Pilot *p, PilotOutfitSlot *s, Outfit *o, int add )
{
   /* Just in case. */
   if ((p==NULL) || (o==NULL))
      return "Nothing selected.";

   /* Adding outfit. */
   if (add) {
      if ((outfit_cpu(o) > 0) && (p->cpu < outfit_cpu(o)))
         return "Insufficient CPU";

      /* Can't add more than one afterburner. */
      if (outfit_isAfterburner(o) &&
            (p->afterburner != NULL))
         return "Already have an afterburner";

      /* Must not drive some things negative. */
      if (outfit_isMod(o)) {
         /*
          * Movement.
          */
         if (((o->u.mod.thrust + o->u.mod.thrust_rel * p->ship->thrust) < 0) &&
               (fabs(o->u.mod.thrust + o->u.mod.thrust_rel * p->ship->thrust) > p->thrust))
            return "Insufficient thrust";
         if (((o->u.mod.speed + o->u.mod.speed_rel * p->ship->speed) < 0) &&
               (fabs(o->u.mod.speed + o->u.mod.speed_rel * p->ship->speed) > p->speed))
            return "Insufficient speed";
         if (((o->u.mod.turn + o->u.mod.turn_rel * p->ship->turn * p->ship->mass/p->solid->mass) < 0) &&
               (fabs(o->u.mod.turn + o->u.mod.turn_rel * p->ship->turn * p->ship->mass/p->solid->mass) > p->turn_base))
            return "Insufficient turn";

         /*
          * Health.
          */
         /* Max. */
         if ((o->u.mod.armour < 0) &&
               (fabs(o->u.mod.armour) > p->armour_max))
            return "Insufficient armour";
         if ((o->u.mod.armour_rel < 0.) &&
               (fabs(o->u.mod.armour_rel * p->ship->armour) > p->armour_max))
            return "Insufficient armour";
         if ((o->u.mod.shield < 0) &&
               (fabs(o->u.mod.shield) > p->shield_max))
            return "Insufficient shield";
         if ((o->u.mod.shield_rel < 0.) &&
               (fabs(o->u.mod.shield_rel * p->ship->shield) > p->shield_max))
            return "Insufficient shield";
         if ((o->u.mod.energy < 0) &&
               (fabs(o->u.mod.energy) > p->armour_max))
            return "Insufficient energy";
         if ((o->u.mod.energy_rel < 0.) &&
               (fabs(o->u.mod.energy_rel * p->ship->energy) > p->energy_max))
            return "Insufficient energy";
         /* Regen. */
         if ((o->u.mod.armour_regen < 0) &&
               (fabs(o->u.mod.armour_regen) > p->armour_regen))
            return "Insufficient energy regeneration";
         if ((o->u.mod.shield_regen < 0) &&
               (fabs(o->u.mod.shield_regen) > p->shield_regen))
            return "Insufficient shield regeneration";
         if ((o->u.mod.energy_regen < 0) &&
               (fabs(o->u.mod.energy_regen) > p->energy_regen))
            return "Insufficient energy regeneration";

         /* 
          * Misc.
          */
         if ((o->u.mod.fuel < 0) &&
               (fabs(o->u.mod.fuel) > p->fuel_max))
            return "Insufficient fuel";
         if ((o->u.mod.cargo < 0) &&
               (fabs(o->u.mod.cargo) > p->cargo_free))
            return "Insufficient cargo space";
      }
   }
   /* Removing outfit. */
   else {
      if ((outfit_cpu(o) < 0) && (p->cpu < fabs(outfit_cpu(o))))
         return "Lower CPU usage first";

      /* Must not drive some things negative. */
      if (outfit_isMod(o)) {
         /*
          * Movement.
          */
         if (((o->u.mod.thrust + o->u.mod.thrust_rel * p->ship->thrust) > 0) &&
               (o->u.mod.thrust + o->u.mod.thrust_rel * p->ship->thrust > p->thrust))
            return "Increase thrust first";
         if (((o->u.mod.speed + o->u.mod.speed_rel * p->ship->speed) > 0) &&
               (o->u.mod.speed + o->u.mod.speed_rel * p->ship->speed > p->speed))
            return "Increase speed first";
         if (((o->u.mod.turn + o->u.mod.turn_rel * p->ship->turn * p->ship->mass/p->solid->mass) > 0) &&
               (fabs(o->u.mod.turn + o->u.mod.turn_rel * p->ship->turn * p->ship->mass/p->solid->mass) > p->turn_base))
            return "Increase turn first";

         /*
          * Health.
          */
         /* Max. */
         if ((o->u.mod.armour > 0) &&
               (o->u.mod.armour > p->armour_max))
            return "Increase armour first";
         if ((o->u.mod.shield > 0) &&
               (o->u.mod.shield > p->shield_max))
            return "Increase shield first";
         if ((o->u.mod.energy > 0) &&
               (o->u.mod.energy > p->energy_max))
            return "Increase energy first";
         /* Regen. */
         if ((o->u.mod.armour_regen > 0) &&
               (o->u.mod.armour_regen > p->armour_regen))
            return "Lower energy usage first";
         if ((o->u.mod.shield_regen > 0) &&
               (o->u.mod.shield_regen > p->shield_regen))
            return "Lower shield usage first";
         if ((o->u.mod.energy_regen > 0) &&
               (o->u.mod.energy_regen > p->energy_regen))
            return "Lower energy usage first";

         /* 
          * Misc.
          */
         if ((o->u.mod.fuel > 0) &&
               (o->u.mod.fuel > p->fuel_max))
            return "Increase fuel first";
         if ((o->u.mod.cargo > 0) &&
               (o->u.mod.cargo > p->cargo_free))
            return "Increase free cargo space first";

      }
      else if (outfit_isFighterBay(o)) {
         if (s->u.ammo.deployed > 0)
            return "Recall the fighters first";
      }
   }

   /* Can equip. */
   return NULL;
}


/**
 * @brief Adds some ammo to the pilot stock.
 *
 *    @param pilot Pilot to add ammo to.
 *    @param s Slot to add ammo to.
 *    @param ammo Ammo to add.
 *    @param quantity Amount to add.
 *    @return Amount actually added.
 */
int pilot_addAmmo( Pilot* pilot, PilotOutfitSlot *s, Outfit* ammo, int quantity )
{
   int q;
   (void) pilot;

   /* Failure cases. */
   if (s->outfit == NULL) {
      WARN("Pilot '%s': Trying to add ammo to unequiped slot.", pilot->name );
      return 0;
   }
   else if (!outfit_isLauncher(s->outfit) && !outfit_isFighterBay(s->outfit)) {
      WARN("Pilot '%s': Trying to add ammo to non-launcher/fighterbay type outfit '%s'",
            pilot->name, s->outfit->name);
      return 0;
   }
   else if (!outfit_isAmmo(ammo) && !outfit_isFighter(ammo)) {
      WARN( "Pilot '%s': Trying to add non-ammo/fighter type outfit '%s' as ammo.",
            pilot->name, ammo->name );
      return 0;
   }
   else if (outfit_isLauncher(s->outfit) && outfit_isFighter(ammo)) {
      WARN("Pilot '%s': Trying to add fighter '%s' as launcher '%s' ammo",
            pilot->name, ammo->name, s->outfit->name );
      return 0;
   }
   else if (outfit_isFighterBay(s->outfit) && outfit_isAmmo(ammo)) {
      WARN("Pilot '%s': Trying to add ammo '%s' as fighter bay '%s' ammo",
            pilot->name, ammo->name, s->outfit->name );
      return 0;
   }
   else if ((s->u.ammo.outfit != NULL) && (s->u.ammo.quantity > 0) &&
         (s->u.ammo.outfit != ammo)) {
      WARN("Pilot '%s': Trying to add ammo to outfit that already has ammo.",
            pilot->name );
      return 0;
   }

   /* Set the ammo type. */
   s->u.ammo.outfit    = ammo;

   /* Add the ammo. */
   q                   = s->u.ammo.quantity; /* Amount have. */
   s->u.ammo.quantity += quantity;
   s->u.ammo.quantity  = MIN( outfit_amount(s->outfit) - s->u.ammo.deployed,
         s->u.ammo.quantity );
   q                   = s->u.ammo.quantity - q; /* Amount actually added. */
   pilot->mass_outfit += q * s->u.ammo.outfit->mass;
   pilot_updateMass( pilot );

   return q;
}


/**
 * @brief Removes some ammo from the pilot stock.
 *
 *    @param pilot Pilot to remove ammo from.
 *    @param s Slot to remove ammo from.
 *    @param quantity Amount to remove.
 *    @return Amount actually removed.
 */
int pilot_rmAmmo( Pilot* pilot, PilotOutfitSlot *s, int quantity )
{
   int q;
   (void) pilot;

   /* Failure cases. */
   if (s->outfit == NULL) {
      WARN("Pilot '%s': Trying to remove ammo from unequiped slot.", pilot->name );
      return 0;
   }
   else if (!outfit_isLauncher(s->outfit) && !outfit_isFighterBay(s->outfit)) {
      WARN("Pilot '%s': Trying to remove ammo from non-launcher/fighter bay type outfit '%s'",
            pilot->name, s->outfit->name);
      return 0;
   }

   /* No ammo already. */
   if (s->u.ammo.outfit == NULL) {
      return 0;
   }

   /* Remove ammo. */
   q                   = MIN( quantity, s->u.ammo.quantity );
   s->u.ammo.quantity -= q;
   pilot->mass_outfit -= q * s->u.ammo.outfit->mass;
   pilot_updateMass( pilot );
   /* We don't set the outfit to null so it "remembers" old ammo. */

   return q;
}


/**
 * @brief Gets all the outfits in nice text form.
 *    
 *    @param pilot Pilot to get the outfits from.
 */
char* pilot_getOutfits( Pilot* pilot )
{
   int i;
   char *buf;
   int p, len;

   len = 1024;

   buf = malloc(sizeof(char)*len);
   buf[0] = '\0';
   p = 0;
   for (i=1; i<pilot->noutfits; i++) {
      if (pilot->outfits[i]->outfit == NULL)
         continue;
      p += snprintf( &buf[p], len-p, (p==0) ? "%s" : ", %s",
            pilot->outfits[i]->outfit->name );
   }

   if (p==0)
      p += snprintf( &buf[p], len-p, "None" );

   return buf;
}


/**
 * @brief Recalculates the pilot's stats based on his outfits.
 *
 *    @param pilot Pilot to recalculate his stats.
 */
void pilot_calcStats( Pilot* pilot )
{
   int i;
   double q;
   double wrange, wspeed;
   int nweaps;
   Outfit* o;
   PilotOutfitSlot *slot;
   double ac, sc, ec, fc; /* temporary health coeficients to set */
   double arel, srel, erel; /* relative health bonuses. */
   ShipStats *s;
   int nfirerate_turret, nfirerate_forward;
   int njammers;

   /* Comfortability. */
   s = &pilot->stats;

   /*
    * set up the basic stuff
    */
   /* mass */
   pilot->solid->mass   = pilot->ship->mass;
   /* movement */
   pilot->thrust        = pilot->ship->thrust;
   pilot->turn_base     = pilot->ship->turn;
   pilot->speed         = pilot->ship->speed;
   /* cpu */
   pilot->cpu_max       = pilot->ship->cpu;
   pilot->cpu           = pilot->cpu_max;
   /* health */
   ac = pilot->armour / pilot->armour_max;
   sc = pilot->shield / pilot->shield_max;
   ec = pilot->energy / pilot->energy_max;
   fc = pilot->fuel   / pilot->fuel_max;
   pilot->armour_max    = pilot->ship->armour;
   pilot->shield_max    = pilot->ship->shield;
   pilot->fuel_max      = pilot->ship->fuel;
   pilot->armour_regen  = pilot->ship->armour_regen;
   pilot->shield_regen  = pilot->ship->shield_regen;
   /* Energy. */
   pilot->energy_max    = pilot->ship->energy;
   pilot->energy_regen  = pilot->ship->energy_regen;
   /* Jamming */
   pilot->jam_range     = 0.;
   pilot->jam_chance    = 0.;
   /* Stats. */
   memcpy( s, &pilot->ship->stats, sizeof(ShipStats) );

   /* cargo has to be reset */
   pilot_calcCargo(pilot);

   /*
    * now add outfit changes
    */
   nfirerate_forward = nfirerate_turret = 0;
   nweaps               = 0;
   wrange = wspeed      = 0.;
   pilot->mass_outfit   = 0.;
   njammers             = 0;
   pilot->jam_range     = 0.;
   pilot->jam_chance    = 0.;
   arel                 = 0.;
   srel                 = 0.;
   erel                 = 0.;
   for (i=0; i<pilot->noutfits; i++) {
      slot = pilot->outfits[i];
      o    = slot->outfit;

      if (o==NULL)
         continue;

      q = (double) slot->quantity;

      /* Subtract CPU. */
      pilot->cpu           -= outfit_cpu(o) * q;
      if (outfit_cpu(o) < 0.)
         pilot->cpu_max    -= outfit_cpu(o) * q;

      /* Add mass. */
      pilot->mass_outfit   += o->mass;

      if (outfit_isMod(o)) { /* Modification */
         /* movement */
         pilot->thrust        += o->u.mod.thrust * pilot->ship->mass * q;
         pilot->thrust        += o->u.mod.thrust_rel * pilot->ship->thrust * q;
         pilot->turn_base     += o->u.mod.turn * q;
         pilot->turn_base     += o->u.mod.turn_rel * pilot->ship->turn * q;
         pilot->speed         += o->u.mod.speed * q;
         pilot->speed         += o->u.mod.speed_rel * pilot->ship->speed * q;
         /* health */
         pilot->armour_max    += o->u.mod.armour * q;
         pilot->armour_regen  += o->u.mod.armour_regen * q;
         arel                 += o->u.mod.armour_rel * q;
         pilot->shield_max    += o->u.mod.shield * q;
         pilot->shield_regen  += o->u.mod.shield_regen * q;
         srel                 += o->u.mod.shield_rel * q;
         pilot->energy_max    += o->u.mod.energy * q;
         pilot->energy_regen  += o->u.mod.energy_regen * q;
         erel                 += o->u.mod.energy_rel * q;
         /* fuel */
         pilot->fuel_max      += o->u.mod.fuel * q;
         /* misc */
         pilot->cargo_free    += o->u.mod.cargo * q;
         pilot->mass_outfit   += o->u.mod.mass_rel * pilot->ship->mass * q;
         /*
          * Stats.
          */
         /* Fighter. */
         s->accuracy_forward  += o->u.mod.stats.accuracy_forward * q;
         s->damage_forward    += o->u.mod.stats.damage_forward * q;
         s->energy_forward    += o->u.mod.stats.energy_forward * q;
         if (o->u.mod.stats.firerate_forward != 0.) {
            s->firerate_forward  += o->u.mod.stats.firerate_forward * q;
            nfirerate_forward    += q;
         }
         /* Cruiser. */
         s->accuracy_turret   += o->u.mod.stats.accuracy_turret * q;
         s->damage_turret     += o->u.mod.stats.damage_turret * q;
         s->energy_turret     += o->u.mod.stats.energy_turret * q;
         if (o->u.mod.stats.firerate_turret != 0.) {
            s->firerate_turret   += o->u.mod.stats.firerate_turret * q;
            if (o->u.mod.stats.firerate_turret > 0.) /* Only modulate bonuses. */
               nfirerate_turret     += q;
         }
         /* Freighter. */
         s->jump_delay        += o->u.mod.stats.jump_delay * q;
      }
      else if (outfit_isAfterburner(o)) /* Afterburner */
         pilot->afterburner = pilot->outfits[i]; /* Set afterburner */
      else if (outfit_isJammer(o)) { /* Jammer */
         pilot->jam_range        += o->u.jam.range * q;
         pilot->jam_chance       += o->u.jam.chance * q;
         pilot->energy_regen     -= o->u.jam.energy * q;
         njammers                += q;;
      }
      if ((outfit_isWeapon(o) || outfit_isTurret(o)) && /* Primary weapon */
            !outfit_isProp(o,OUTFIT_PROP_WEAP_SECONDARY)) {
         nweaps++;
         wrange += outfit_range(o);
         wspeed += outfit_speed(o);
      }
      /* Add ammo mass. */
      if (outfit_ammo(o) != NULL) {
         if (slot->u.ammo.outfit != NULL)
            pilot->mass_outfit += slot->u.ammo.quantity * slot->u.ammo.outfit->mass;
      }
   }

   /* Set final energy tau. */
   pilot->energy_tau = pilot->energy_max / pilot->energy_regen;

   /* Set weapon range and speed */
   if (nweaps > 0) {
      pilot->weap_range = wrange / (double)nweaps;
      pilot->weap_speed = wspeed / (double)nweaps;
   }
   else {
      pilot->weap_range = 0.;
      pilot->weap_speed = 0.;
   }

   /*
    * Calculate jammers.
    *
    * Range is averaged.
    * Diminishing return on chance.
    *  chance = p * exp( -0.2 * (n-1) )
    *  1x 20% -> 20%
    *  2x 20% -> 32%
    *  2x 40% -> 65%
    *  6x 40% -> 88%
    */
   if (njammers > 1) {
      pilot->jam_range  /= (double)njammers;
      pilot->jam_chance *= exp( -0.2 * (double)(njammers-1) );
   }

   /* 
    * Normalize stats.
    */
   /* Fighter. */
   s->accuracy_forward  = s->accuracy_forward/100. + 1.;
   s->damage_forward    = s->damage_forward/100. + 1.;
   s->energy_forward    = s->energy_forward/100. + 1.;
   /* Fire rate:
    *  amount = p * exp( -0.15 * (n-1) )
    *  1x 15% -> 15%
    *  2x 15% -> 25.82%
    *  3x 15% -> 33.33%
    *  6x 15% -> 42.51%
    */
   s->firerate_forward  = s->firerate_forward/100.;
   if (nfirerate_forward > 0)
      s->firerate_forward *= exp( -0.15 * (double)(nfirerate_forward-1) );
   s->firerate_forward += 1.;
   /* Cruiser. */
   s->accuracy_turret   = s->accuracy_turret/100. + 1.;
   s->damage_turret     = s->damage_turret/100. + 1.;
   s->energy_turret     = s->energy_turret/100. + 1.;
   s->firerate_turret   = s->firerate_turret/100.;
   if (nfirerate_turret > 0)
      s->firerate_turret  *= exp( -0.15 * (double)(nfirerate_turret-1) );
   s->firerate_turret  += 1.;
   /* Freighter. */
   s->jump_delay        = s->jump_delay/100. + 1.;

   /* Increase health by relative bonuses. */
   pilot->armour_max += arel * pilot->ship->armour;
   pilot->shield_max += srel * pilot->ship->shield;
   pilot->energy_max += erel * pilot->ship->energy;

   /* Give the pilot his health proportion back */
   pilot->armour = ac * pilot->armour_max;
   pilot->shield = sc * pilot->shield_max;
   pilot->energy = ec * pilot->energy_max;
   pilot->fuel   = fc * pilot->fuel_max;

   /* Calculate mass. */
   pilot->solid->mass = pilot->ship->mass + pilot->mass_cargo + pilot->mass_outfit;

   /* Modulate by mass. */
   pilot_updateMass( pilot );
}


/**
 * @brief Updates the pilot stats after mass change.
 */
static void pilot_updateMass( Pilot *pilot )
{
   pilot->turn = pilot->turn_base * pilot->ship->mass / pilot->solid->mass;
}


/**
 * @brief Gets the pilot's free cargo space.
 *
 *    @param p Pilot to get the free space of.
 *    @return Free cargo space on pilot.
 */
int pilot_cargoFree( Pilot* p )
{
   return p->cargo_free;
}


/**
 * @brief Moves cargo from one pilot to another.
 *
 * At the end has dest have exactly the same cargo as src and leaves src with none.
 *
 *    @param dest Destination pilot.
 *    @param src Source pilot.
 *    @return 0 on success.
 */
int pilot_moveCargo( Pilot* dest, Pilot* src )
{
   int i;

   /* Nothing to copy, success! */
   if (src->ncommodities == 0)
      return 0;

   /* Check if it fits. */
   if (pilot_cargoUsed(src) > pilot_cargoFree(dest)) {
      WARN("Unable to copy cargo over from pilot '%s' to '%s'", src->name, dest->name );
      return -1;
   }

   /* Allocate new space. */
   i = dest->ncommodities;
   dest->ncommodities += src->ncommodities;
   dest->commodities   = realloc( dest->commodities,
         sizeof(PilotCommodity)*dest->ncommodities);
  
   /* Copy over. */
   memmove( &dest->commodities[0], &src->commodities[0],
         sizeof(PilotCommodity) * src->ncommodities);

   /* Clean src. */
   if (src->commodities != NULL)
      free(src->commodities);
   src->ncommodities = 0;
   src->commodities  = NULL;

   return 0;
}


/**
 * @brief Adds a cargo raw.
 *
 * Does not check if currently exists.
 *
 *    @param pilot Pilot to add cargo to.
 *    @param cargo Cargo to add.
 *    @param quantity Quantity to add.
 *    @param id Mission ID to add (0 in none).
 */
static int pilot_addCargoRaw( Pilot* pilot, Commodity* cargo,
      int quantity, unsigned int id )
{
   int i, f, q;

   q = quantity;

   /* If not mission cargo check to see if already exists. */
   if (id == 0) {
      for (i=0; i<pilot->ncommodities; i++)
         if (!pilot->commodities[i].id &&
               (pilot->commodities[i].commodity == cargo)) {

            /* Check to see how much to add. */
            f = pilot_cargoFree(pilot);
            if (f < quantity)
               q = f;

            /* Tweak results. */
            pilot->commodities[i].quantity += q;
            pilot->cargo_free              -= q;
            pilot->mass_cargo              += q;
            pilot->solid->mass             += q;
            pilot_updateMass( pilot );
            return q;
         }
   }

   /* Create the memory space. */
   pilot->commodities = realloc( pilot->commodities,
         sizeof(PilotCommodity) * (pilot->ncommodities+1));
   pilot->commodities[ pilot->ncommodities ].commodity = cargo;

   /* See how much to add. */
   f = pilot_cargoFree(pilot);
   if (f < quantity)
      q = f;

   /* Set parameters. */
   pilot->commodities[ pilot->ncommodities ].id       = id;
   pilot->commodities[ pilot->ncommodities ].quantity = q;

   /* Tweak pilot. */
   pilot->cargo_free    -= q;
   pilot->mass_cargo    += q;
   pilot->solid->mass   += q;
   pilot->ncommodities++;
   pilot_updateMass( pilot );

   return q;
}


/**
 * @brief Tries to add quantity of cargo to pilot.
 *
 *    @param pilot Pilot to add cargo to.
 *    @param cargo Cargo to add.
 *    @param quantity Quantity to add.
 *    @return Quantity actually added.
 */
int pilot_addCargo( Pilot* pilot, Commodity* cargo, int quantity )
{
   return pilot_addCargoRaw( pilot, cargo, quantity, 0 );
}


/**
 * @brief Gets how much cargo ship has on board.
 *
 *    @param pilot Pilot to get used cargo space of.
 *    @return The used cargo space by pilot.
 */
int pilot_cargoUsed( Pilot* pilot )
{
   int i, q;

   q = 0; 
   for (i=0; i<pilot->ncommodities; i++)
      q += pilot->commodities[i].quantity;

   return q;
}


/**
 * @brief Calculates how much cargo ship has left and such.
 *
 *    @param pilot Pilot to calculate free cargo space of.
 */
static void pilot_calcCargo( Pilot* pilot )
{
   pilot->mass_cargo  = pilot_cargoUsed( pilot );
   pilot->cargo_free  = pilot->ship->cap_cargo - pilot->mass_cargo;
   pilot->solid->mass = pilot->ship->mass + pilot->mass_cargo + pilot->mass_outfit;
   pilot_updateMass( pilot );
}


/**
 * @brief Adds special mission cargo, can't sell it and such.
 *
 *    @param pilot Pilot to add it to.
 *    @param cargo Commodity to add.
 *    @param quantity Quantity to add.
 *    @return The Mission Cargo ID of created cargo. 
 */
unsigned int pilot_addMissionCargo( Pilot* pilot, Commodity* cargo, int quantity )
{
   int i;
   unsigned int id, max_id;
   int q;
   q = quantity;

   /* Get ID. */
   id = ++mission_cargo_id;

   /* Check for collisions with pilot and set ID generator to the max. */
   max_id = 0;
   for (i=0; i<pilot->ncommodities; i++)
      if (pilot->commodities[i].id > max_id)
         max_id = pilot->commodities[i].id;
   if (max_id >= id) {
      mission_cargo_id = max_id;
      id = ++mission_cargo_id;
   }

   /* Add the cargo. */
   pilot_addCargoRaw( pilot, cargo, quantity, id );

   return id;
}


/**
 * @brief Removes special mission cargo based on id.
 *
 *    @param pilot Pilot to remove cargo from.
 *    @param cargo_id ID of the cargo to remove.
 *    @param jettison Should jettison the cargo?
 *    @return 0 on success (cargo removed).
 */
int pilot_rmMissionCargo( Pilot* pilot, unsigned int cargo_id, int jettison )
{
   int i;

   /* check if pilot has it */
   for (i=0; i<pilot->ncommodities; i++)
      if (pilot->commodities[i].id == cargo_id)
         break;
   if (i>=pilot->ncommodities)
      return 1; /* pilot doesn't have it */

   if (jettison)
      commodity_Jettison( pilot->id, pilot->commodities[i].commodity,
            pilot->commodities[i].quantity );

   /* remove cargo */
   pilot->cargo_free    += pilot->commodities[i].quantity;
   pilot->mass_cargo    -= pilot->commodities[i].quantity;
   pilot->solid->mass   -= pilot->commodities[i].quantity;
   pilot->ncommodities--;
   if (pilot->ncommodities <= 0) {
      if (pilot->commodities != NULL) {
         free( pilot->commodities );
         pilot->commodities   = NULL;
      }
      pilot->ncommodities  = 0;
   }
   else {
      memmove( &pilot->commodities[i], &pilot->commodities[i+1],
            sizeof(PilotCommodity) * (pilot->ncommodities-i) );
      pilot->commodities = realloc( pilot->commodities,
            sizeof(PilotCommodity) * pilot->ncommodities );
   }

   /* Update mass. */
   pilot_updateMass( pilot );

   return 0;
}


/**
 * @brief Tries to get rid of quantity cargo from pilot.  Can remove mission cargo.
 * 
 *    @param pilot Pilot to get rid of cargo.
 *    @param cargo Cargo to get rid of.
 *    @param quantity Amount of cargo to get rid of.
 *    @param cleanup Whether we're cleaning up or not (removes mission cargo).
 *    @return Amount of cargo gotten rid of.
 */
static int pilot_rmCargoRaw( Pilot* pilot, Commodity* cargo, int quantity, int cleanup )
{
   int i;
   int q;

   /* check if pilot has it */
   q = quantity;
   for (i=0; i<pilot->ncommodities; i++)
      if (pilot->commodities[i].commodity == cargo) {

         /* Must not be missino cargo unless cleaning up. */
         if (!cleanup && (pilot->commodities[i].id != 0))
            continue;

         if (quantity >= pilot->commodities[i].quantity) {
            q = pilot->commodities[i].quantity;

            /* remove cargo */
            pilot->ncommodities--;
            if (pilot->ncommodities <= 0) {
               if (pilot->commodities != NULL) {
                  free( pilot->commodities );
                  pilot->commodities   = NULL;
               }
               pilot->ncommodities  = 0;
            }
            else {
               memmove( &pilot->commodities[i], &pilot->commodities[i+1],
                     sizeof(PilotCommodity) * (pilot->ncommodities-i) );
               pilot->commodities = realloc( pilot->commodities,
                     sizeof(PilotCommodity) * pilot->ncommodities );
            }
         }
         else
            pilot->commodities[i].quantity -= q;
         pilot->cargo_free    += q;
         pilot->mass_cargo    -= q;
         pilot->solid->mass   -= q;
         pilot_updateMass( pilot );
         return q;
      }
   return 0; /* pilot didn't have it */
}

/**
 * @brief Tries to get rid of quantity cargo from pilot.
 * 
 *    @param pilot Pilot to get rid of cargo.
 *    @param cargo Cargo to get rid of.
 *    @param quantity Amount of cargo to get rid of.
 *    @return Amount of cargo gotten rid of.
 */
int pilot_rmCargo( Pilot* pilot, Commodity* cargo, int quantity )
{
   return pilot_rmCargoRaw( pilot, cargo, quantity, 0 );
}


/**
 * @brief Calculates the hyperspace delay for a pilot.
 *
 *    @param p Pilot to calculate hyperspace delay for.
 *    @return Average hyperspace delay (in STU).
 */
double pilot_hyperspaceDelay( Pilot *p )
{
   double val;

   /* Calculate jump delay. */
   val  = pow( p->solid->mass, 1./2.5 ) / 5.;

   /* Modulate by stats. */
   val *= p->stats.jump_delay;

   return val;
}


/**
 * @brief Adds a hook to the pilot.
 *
 *    @param pilot Pilot to add the hook to.
 *    @param type Type of the hook to add.
 *    @param hook ID of the hook to add.
 */
void pilot_addHook( Pilot *pilot, int type, unsigned int hook )
{
   pilot->nhooks++;
   pilot->hooks = realloc( pilot->hooks, sizeof(PilotHook) * pilot->nhooks );
   pilot->hooks[pilot->nhooks-1].type  = type;
   pilot->hooks[pilot->nhooks-1].id    = hook;
}


/**
 * @brief Removes a hook from all the pilots.
 *
 *    @param hook Hook to remove.
 */
void pilots_rmHook( unsigned int hook )
{
   int i, j;
   Pilot *p;

   for (i=0; i<pilot_nstack; i++) {
      p = pilot_stack[i];

      /* Must have hooks. */
      if (p->nhooks <= 0)
         continue;

      for (j=0; j<p->nhooks; j++) {

         /* Hook not found. */
         if (p->hooks[j].id != hook)
            continue;

         p->nhooks--;
         memmove( &p->hooks[j], &p->hooks[j+1], sizeof(PilotHook) * (p->nhooks-j) );
         j--; /* Dun like it but we have to keep iterator sane. */
      }
   }
}


/**
 * @brief Clears the pilots hooks.
 *
 *    @param p Pilot to clear his hooks.
 */
void pilot_clearHooks( Pilot *p )
{
   int i;

   if (p->nhooks <= 0)
      return;

   /* Remove the hooks. */
   for (i=0; i<p->nhooks; i++)
      hook_rm( p->hooks[i].id );

   /* Clear the hooks. */
   free(p->hooks);
   p->hooks  = NULL;
   p->nhooks = 0;
}


/**
 * @brief Checks to see if the pilot has at least a certain amount of credits.
 *
 *    @param p Pilot to check to see if he has enough credits.
 *    @param amount Amount to check for.
 *    @return 1 if he has enough, 0 otherwise.
 */
int pilot_hasCredits( Pilot *p, int amount )
{
   unsigned long ul;

   ul = (unsigned long) ABS(amount);

   return (ul <= p->credits);
}


/**
 * @brief Modifies the amount of credits the pilot has.
 *
 *    @param p Pilot to modify amount of credits of.
 *    @param amount QUantity of credits to give/take.
 *    @return Amount of credits the pilot has.
 */
unsigned long pilot_modCredits( Pilot *p, int amount )
{
   unsigned long ul;

   ul = (unsigned long) ABS(amount);

   if (amount > 0) {
      if (ULONG_MAX-p->credits < ul)
         p->credits = ULONG_MAX;
      else
         p->credits += ul;
   }
   else if (amount < 0) {
      if (ul > p->credits)
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

   /* Basic information. */
   pilot->ship = ship;
   pilot->name = strdup( (name==NULL) ? ship->name : name );

   /* faction */
   pilot->faction = faction;

   /* System fleet. */
   pilot->systemFleet = systemFleet;

   /* solid */
   pilot->solid = solid_create(ship->mass, dir, pos, vel);

   /* First pass to make sure requirements make sense. */
   pilot->armour = pilot->armour_max = 1.; /* hack to have full armour */
   pilot->shield = pilot->shield_max = 1.; /* ditto shield */
   pilot->energy = pilot->energy_max = 1.; /* ditto energy */
   pilot->fuel   = pilot->fuel_max   = 1.; /* ditto fuel */
   pilot_calcStats(pilot);

   /* Allocate outfit memory. */
   /* Slot types. */
   pilot->outfit_nlow    = ship->outfit_nlow;
   pilot->outfit_low     = calloc( ship->outfit_nlow, sizeof(PilotOutfitSlot) );
   pilot->outfit_nmedium = ship->outfit_nmedium;
   pilot->outfit_medium  = calloc( ship->outfit_nmedium, sizeof(PilotOutfitSlot) );
   pilot->outfit_nhigh   = ship->outfit_nhigh;
   pilot->outfit_high    = calloc( ship->outfit_nhigh, sizeof(PilotOutfitSlot) );
   /* Global. */
   pilot->noutfits = pilot->outfit_nlow + pilot->outfit_nmedium + pilot->outfit_nhigh;
   pilot->outfits  = calloc( pilot->noutfits, sizeof(PilotOutfitSlot*) );
   /* First pass copy data. */
   p = 0;
   for (i=0; i<pilot->outfit_nlow; i++) {
      pilot->outfit_low[i].slot = OUTFIT_SLOT_LOW;
      pilot->outfits[p] = &pilot->outfit_low[i];
      memcpy( &pilot->outfits[p]->mount, &ship->outfit_low[i].mount, sizeof(ShipMount) );
      p++;
   }
   for (i=0; i<pilot->outfit_nmedium; i++) {
      pilot->outfit_medium[i].slot = OUTFIT_SLOT_MEDIUM;
      pilot->outfits[p] = &pilot->outfit_medium[i];
      memcpy( &pilot->outfits[p]->mount, &ship->outfit_medium[i].mount, sizeof(ShipMount) );
      p++;
   }
   for (i=0; i<pilot->outfit_nhigh; i++) {
      pilot->outfit_high[i].slot = OUTFIT_SLOT_HIGH;
      pilot->outfits[p] = &pilot->outfit_high[i];
      memcpy( &pilot->outfits[p]->mount, &ship->outfit_high[i].mount, sizeof(ShipMount) );
      p++;
   }

   /* cargo - must be set before calcStats */
   pilot->cargo_free = pilot->ship->cap_cargo; /* should get redone with calcCargo */

   /* set the pilot stats based on his ship and outfits */
   pilot_calcStats(pilot);

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
      if (!pilot_isFlagRaw( flags, PILOT_EMPTY )) { /* Sort of a hack. */
         player.p = pilot;
         gui_load( pilot->ship->gui ); /* load the gui */
      }
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
   dest->outfit_nlow = src->outfit_nlow;
   dest->outfit_low  = malloc( sizeof(PilotOutfitSlot) * dest->outfit_nlow );
   memcpy( dest->outfit_low, src->outfit_low,
         sizeof(PilotOutfitSlot) * dest->outfit_nlow );
   dest->outfit_nmedium = src->outfit_nmedium;
   dest->outfit_medium  = malloc( sizeof(PilotOutfitSlot) * dest->outfit_nmedium );
   memcpy( dest->outfit_medium, src->outfit_medium,
         sizeof(PilotOutfitSlot) * dest->outfit_nmedium );
   dest->outfit_nhigh = src->outfit_nhigh;
   dest->outfit_high  = malloc( sizeof(PilotOutfitSlot) * dest->outfit_nhigh );
   memcpy( dest->outfit_high, src->outfit_high,
         sizeof(PilotOutfitSlot) * dest->outfit_nhigh );
   p = 0;
   for (i=0; i<dest->outfit_nlow; i++)
      dest->outfits[p++] = &dest->outfit_low[i];
   for (i=0; i<dest->outfit_nmedium; i++)
      dest->outfits[p++] = &dest->outfit_medium[i];
   for (i=0; i<dest->outfit_nhigh; i++)
      dest->outfits[p++] = &dest->outfit_high[i];
   dest->secondary   = NULL;
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
      pilot_addCargoRaw( dest, src->commodities[i].commodity,
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

   /* Free outfits. */
   if (p->outfits != NULL)
      free(p->outfits);
   if (p->outfit_low != NULL)
      free(p->outfit_low);
   if (p->outfit_medium != NULL)
      free(p->outfit_medium);
   if (p->outfit_high != NULL)
      free(p->outfit_high);

   /* Remove commodities. */
   while (p->commodities != NULL)
      pilot_rmCargoRaw( p, p->commodities[0].commodity,
            p->commodities[0].quantity, 1 );

   /* Free name and title. */
   if (p->name != NULL)
      free(p->name);
   if (p->title != NULL)
      free(p->title);

   /* Clean up data. */
   if (p->ai != NULL)
      ai_destroy(p); /* Must be destroyed first if applicable. */
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
   for (i=0; i < pilot_nstack; i++)
      /* we'll set player.p at privileged position */
      if ((player.p != NULL) && (pilot_stack[i] == player.p)) {
         pilot_stack[0] = player.p;
         pilot_stack[0]->lockons = 0; /* Clear lockons. */
      }
      else /* rest get killed */
         pilot_free(pilot_stack[i]);

   if (player.p != NULL) { /* set stack to 1 if pilot exists */
      pilot_nstack = 1;
      pilot_clearTimers( player.p ); /* Reset the player's timers. */
   }
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
 * @brief Updates the system's base sensor range.
 */
void pilot_updateSensorRange (void)
{
   /* Calculate the sensor sensor_curRange. */
   if (cur_system->interference == 0.)
      sensor_curRange = INFINITY;
   else if (cur_system->interference >= 999.)
      sensor_curRange = 0.; /* No range. */
   else {
      /* 0    ->    inf
       * 250  ->   1500.
       * 500  ->    750.
       * 750  ->    500.
       * 1000 ->    300. */
      sensor_curRange  = 375.;
      sensor_curRange /= (cur_system->interference / 1000.);
   }

   /* Speeds up calculations. */
   sensor_curRange = pow2(sensor_curRange);
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
   for ( i=0; i < pilot_nstack; i++ ) {
      p = pilot_stack[i];

      /* Destroy pilot and go on. */
      if (pilot_isFlag(p, PILOT_DELETE)) {
         pilot_destroy(p);
         continue;
      }

      /* Invisible, not doing anything. */
      if (pilot_isFlag(p, PILOT_INVISIBLE))
         continue;

      /* See if should think. */
      if (p->think && !pilot_isDisabled(p) && !pilot_isFlag(p,PILOT_DEAD)) {

         /* Hyperspace gets special treatment */
         if (pilot_isFlag(p, PILOT_HYP_PREP))
            pilot_hyperspace(p, dt);
         /* Entering hyperspace. */
         else if (pilot_isFlag(p, PILOT_HYP_END)) {
            if (VMOD(p->solid->vel) < 2*p->speed)
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
