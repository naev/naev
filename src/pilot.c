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


#define PILOT_CHUNK     32 /**< Chunks to increment pilot_stack by */
#define CHUNK_SIZE      32 /**< Size to allocate memory by. */


/* ID Generators. */
static unsigned int pilot_id = PLAYER_ID; /**< Stack of pilot ids to assure uniqueness */
static unsigned int mission_cargo_id = 0; /**< ID generator for special mission cargo.
                                               Not guaranteed to be absolutely unique, 
                                               only unique for each pilot. */


/* stack of pilot_nstack */
Pilot** pilot_stack = NULL; /**< Not static, used in player.c, weapon.c, pause.c and ai.c */
int pilot_nstack = 0; /**< same */
static int pilot_mstack = 0; /**< Memory allocated for pilot_stack. */


/* misc */
static double sensor_curRange = 0.; /**< Current base sensor range, used to calculate
                                         what is in range and what isn't. */


/*
 * prototyes
 */
/* external */
extern void ai_getDistress( Pilot* p, const Pilot* distressed ); /**< from ai.c */
extern AI_Profile* ai_pinit( Pilot *p, const char *ai ); /**< from ai.c */
extern void ai_destroy( Pilot* p ); /**< from ai.c */
extern void ai_think( Pilot* pilot ); /**< from ai.c */
/* internal */
/* update. */
static void pilot_shootWeapon( Pilot* p, PilotOutfit* w );
static void pilot_hyperspace( Pilot* pilot );
static void pilot_refuel( Pilot *p, double dt );
static void pilot_update( Pilot* pilot, const double dt );
/* render */
void pilot_render( Pilot* pilot ); /* externed in player.c */
/* cargo. */
static int pilot_rmCargoRaw( Pilot* pilot, Commodity* cargo, int quantity, int cleanup );
static void pilot_calcCargo( Pilot* pilot );
static int pilot_addCargoRaw( Pilot* pilot, Commodity* cargo,
      int quantity, unsigned int id );
/* outfits. */
static int pilot_setOutfitMounts( Pilot *p, PilotOutfit* po, int o, int q );
static void pilot_setSecondary( Pilot* p, Outfit* o );
/* clean up. */
void pilot_free( Pilot* p ); /* externed in player.c */
static void pilot_dead( Pilot* p );
/* misc */
static int pilot_getStackPos( const unsigned int id );


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
   if (player == NULL)
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
         if (pilot_inRangePilot( player, pilot_stack[p] ))
            return pilot_stack[p]->id;
         p++;
      }
   }
   /* Get first hostile in range. */
   if (mode == 1) {
      while (p < pilot_nstack) {
         if (pilot_inRangePilot( player, pilot_stack[p] ) &&
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
   if (player == NULL)
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
         if (pilot_inRangePilot( player, pilot_stack[p] ))
            return pilot_stack[p]->id;
         p--;
      }
   }
   /* Get first hostile in range. */
   else if (mode == 1) {
      while (p >= 0) {
         if (pilot_inRangePilot( player, pilot_stack[p] ) &&
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
      if ((pilot_stack[i]->id == PLAYER_ID) && pilot_isFlag(p,PILOT_BRIBED))
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
   for (i=0; i<pilot_nstack; i++)
      if (pilot_stack[i] != p) {

         /* Shouldn't be disabled. */
         if (pilot_isDisabled(pilot_stack[i]))
            continue;

         /* Must be in range. */
         if (!pilot_inRangePilot( p, pilot_stack[i] ))
            continue;

         td = vect_dist2(&pilot_stack[i]->solid->pos, &player->solid->pos);       
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

   if (id==PLAYER_ID) return player; /* special case player */
  
   m = pilot_getStackPos(id);

   if (m==-1)
      return NULL;
   else
      return pilot_stack[m];
}


/**
 * @brief Checks to see if pilot is hostile to the player.
 *
 *    @param p Player to see if is hostile.
 *    @return 1 if pilot is hostile to the player.
 */
int pilot_isHostile( const Pilot *p )
{
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

   p->solid->dir_vel = 0.;
   if (turn)
      p->solid->dir_vel -= p->turn * turn;

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
      if (player_enemies == 0)
         music_choose("combat");

      player_enemies++;
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

   /* Get the target. */
   t = pilot_get(target);
   if (t == NULL)
      return;

   /* Must be in range. */
   if (!ignore_int && !pilot_inRangePilot( player, p ))
      return;

   /* Only really affects player atm. */
   if (target == PLAYER_ID)
      player_message( "Comm %s> \"%s\"", p->name, msg );
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
   /* Only display if player exists and is in range. */
   if (player==NULL)
      return;

   /* Check if should ignore interference. */
   if (!ignore_int && !pilot_inRangePilot( player, p ))
      return;

   player_message( "Broadcast %s> \"%s\"", p->name, msg );
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

   /* Now proceed to see if player should incur faction loss because
    * of the broadcast signal. */

   /* Consider not in range at first. */
   r = 0;

   /* Check if planet is in range. */
   for (i=0; i<cur_system->nplanets; i++) {
      if (planet_hasService(cur_system->planets[i], PLANET_SERVICE_BASIC) &&
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
         faction_modPlayer( p->faction, pow(p->ship->mass, 0.2) - 1. );

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
      player_enemies--;
      pilot_rmFlag(p, PILOT_HOSTILE);

      /* Change music back to ambient if no more enemies. */
      if (player_enemies == 0)
         music_choose("ambient");
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
int pilot_oquantity( Pilot* p, PilotOutfit* w )
{
   return (outfit_isAmmo(w->outfit) && p->secondary) ?
      p->secondary->quantity : w->quantity ;
}


/**
 * @brief Gets pilot's free weapon space.
 *
 *    @param p Pilot to get free space of.
 *    @return Free weapon space of the pilot.
 */
int pilot_freeSpace( Pilot* p )
{
   int i,s;
   
   s = p->ship->cap_weapon;
   for (i=0; i<p->noutfits; i++)
      s -= p->outfits[i].quantity * p->outfits[i].outfit->mass;
   
   return s;
}



/**
 * @brief Makes the pilot shoot.
 *
 *    @param p The pilot which is shooting.
 *    @param type Indicate what shoot group to use.
 *           0 = all
 *           1 = turrets
 *           2 = forward
 */
void pilot_shoot( Pilot* p, int group )
{
   int i;
   Outfit* o;

   if (!p->outfits) return; /* no outfits */

   for (i=0; i<p->noutfits; i++) { /* cycles through outfits to find primary weapons */
      o = p->outfits[i].outfit;
      if (!outfit_isProp(o,OUTFIT_PROP_WEAP_SECONDARY) &&
            (outfit_isBolt(o) || outfit_isBeam(o))) {

         /* Choose what to shoot dependent on type. */
         if ((group == 0) ||
               ((group == 1) && outfit_isTurret(o)) ||
               ((group == 2) && !outfit_isTurret(o)))
            pilot_shootWeapon( p, &p->outfits[i] );
      }
   }
}


/**
 * @brief Makes the pilot shoot it's currently selected secondary weapon.
 *
 *    @param p The pilot which is to shoot.
 */
void pilot_shootSecondary( Pilot* p )
{
   /* No secondary weapon. */
   if (!p->secondary)
      return;

   pilot_shootWeapon( p, p->secondary );
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

   if (!p->outfits) return; /* no outfits */

   if (!secondary) { /* primary weapons */

      for (i=0; i<p->noutfits; i++) { /* cycles through outfits to find primary weapons */
         o = p->outfits[i].outfit;
         if (!outfit_isProp(o,OUTFIT_PROP_WEAP_SECONDARY) &&
               outfit_isBeam(o)) /** @todo possibly make this neater. */
            if (p->outfits[i].beamid > 0) {
               beam_end( p->id, p->outfits[i].beamid );
               p->outfits[i].beamid = 0;
            }
      }
   }
   else { /* secondary weapon */
      
      if (p->secondary == NULL) return; /* No secondary weapon. */
      
      o = p->secondary->outfit;

      if (outfit_isBeam(o) && (p->secondary->beamid > 0)) {
         beam_end( p->id, p->secondary->beamid );
         p->secondary->beamid = 0;
      }
   }
}


/**
 * @brief Gets the mount position of a pilot.
 *
 *    @param p Pilot to get mount position of.
 *    @param id ID of the mount.
 *    @param[out] v Position of the mount.
 *    @return 0 on success.
 */
int pilot_getMount( Pilot *p, int id, Vector2d *v )
{
   double a;

   /* Calculate the sprite angle. */
   a  = (double)(p->tsy * p->ship->gfx_space->sx + p->tsx);
   a *= p->ship->mangle;

   /* Get the mount and add the player offset. */
   ship_getMount( p->ship, a, id, v );
   vect_cadd( v, p->solid->pos.x, p->solid->pos.y );

   return 0;
}


/**
 * @brief Actually handles the shooting, how often the player can shoot and such.
 *
 *    @param p Pilot that is shooting.
 *    @param w Pilot's outfit to shoot.
 */
static void pilot_shootWeapon( Pilot* p, PilotOutfit* w )
{
   int id;
   Vector2d vp, vv;

   /* check to see if weapon is ready */
   if (w->timer > 0.)
      return;

   /* Get weapon mount position. */
   if (w->mounts == NULL)
      id = 0;
   else if (outfit_isTurret(w->outfit))
      id = w->mounts[w->lastshot];
   pilot_getMount( p, id, &vp );

   /* Modify velocity to take into account the rotation. */
   vect_cset( &vv, p->solid->vel.x + vp.x*p->solid->dir_vel,
         p->solid->vel.y + vp.y*p->solid->dir_vel );

   /*
    * regular bolt weapons
    */
   if (outfit_isBolt(w->outfit)) {
      
      /* enough energy? */
      if (outfit_energy(w->outfit) > p->energy) return;

      p->energy -= outfit_energy(w->outfit);
      weapon_add( w->outfit, p->solid->dir,
            &vp, &p->solid->vel, p->id, p->target );
   }

   /*
    * Beam weapons.
    */
   else if (outfit_isBeam(w->outfit)) {

      /* Check if enough energy to last a second. */
      if (outfit_energy(w->outfit) > p->energy) return;

      /** @todo Handle warmup stage. */
      w->state = PILOT_OUTFIT_ON;
      w->beamid = beam_start( w->outfit, p->solid->dir,
            &vp, &p->solid->vel, p->id, p->target, id );
   }

   /*
    * missile launchers
    *
    * must be a secondary weapon
    */
   else if (outfit_isLauncher(w->outfit) && (w==p->secondary)) {

      /* Shooter can't be the target - sanity check for the player */
      if ((w->outfit->type != OUTFIT_TYPE_MISSILE_DUMB) && (p->id==p->target))
         return;

      /* Must have ammo left. */
      if ((p->ammo == NULL) || (p->ammo->quantity <= 0))
         return;

      /* enough energy? */
      if (outfit_energy(w->outfit) > p->energy)
         return;

      p->energy -= outfit_energy(w->outfit);
      weapon_add( p->ammo->outfit, p->solid->dir,
            &vp, &p->solid->vel, p->id, p->target );

      p->ammo->quantity -= 1; /* we just shot it */
      if (p->ammo->quantity <= 0) /* Out of ammo. */
         pilot_rmOutfit( p, p->ammo->outfit, 0 ); /* It'll set p->ammo to NULL */
   }

   /*
    * Fighter bays.
    *
    * Must be secondary weapon.
    */
   else if (outfit_isFighterBay(w->outfit) && (w==p->secondary)) {

      /* Must have ammo left. */
      if ((p->ammo == NULL) || (p->ammo->quantity <= 0))
         return;

      /* Create the escort. */
      escort_create( p->id, p->ammo->outfit->u.fig.ship,
            &vp, &p->solid->vel, 1 );

      p->ammo->quantity -= 1; /* we just shot it */
      if (p->ammo->quantity <= 0) /* Out of ammo. */
         pilot_rmOutfit( p, p->ammo->outfit, 0 ); /* It'll set p->ammo to NULL */
   }

   else {
      WARN("Shooting unknown weapon type: %s", w->outfit->name);
   }

   /* Reset timer. */
   w->timer += ((double)outfit_delay( w->outfit ) / (double)w->quantity)/1000.;

   /* Mark last updated. */
   w->lastshot++;
   if (w->lastshot >= w->quantity)
      w->lastshot = 0;
}



/**
 * @brief Sets the pilot's secondary weapon.
 *
 *    @param p Pilot to set secondary weapon.
 *    @param i Index of the weapon to set to.
 */
void pilot_switchSecondary( Pilot* p, int i )
{
   PilotOutfit *cur;

   cur = player->secondary;

   if ((i < 0) || (i >= player->noutfits))
      player->secondary = NULL;
   else
      player->secondary = &player->outfits[i];

   /* Check for weapon change. */
   if ((cur != NULL) && (player->secondary != cur)) {
      if (outfit_isBeam(cur->outfit) && (cur->beamid > 0)) {
         beam_end( p->id, cur->beamid );
         cur->beamid = 0;
      }
   }
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
         !pilot_isFlag(player, PILOT_HYP_BEGIN) &&
         !pilot_isFlag(player, PILOT_HYPERSPACE))
      player_abortAutonav("Sustaining Damage");

   /*
    * Shields take entire blow.
    */
   if (p->shield-damage_shield > 0.) {
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

   /* EMP don't kill. */
   if ((dtype == DAMAGE_TYPE_EMP) &&
         (p->armour < PILOT_DISABLED_ARMOR * p->armour_max))
      p->armour = PILOT_DISABLED_ARMOR * p->armour_max - 1.;

   /* Disabled always run before dead to ensure crating boost. */
   if (!pilot_isFlag(p,PILOT_DISABLED) && (p != player) &&
         (p->armour < PILOT_DISABLED_ARMOR*p->armour_max)) { /* disabled */

      /* If hostile, must remove counter. */
      h = (pilot_isHostile(p)) ? 1 : 0;
      pilot_rmHostile(p);
      if (h == 1) /* Horrible hack to make sure player can hit it if it was hostile. */
         pilot_setFlag(p, PILOT_HOSTILE);

      pshooter = pilot_get(shooter);
      if ((pshooter != NULL) && (pshooter->faction == FACTION_PLAYER)) {
         /* About 3 for a llama, 26 for hawking. */
         mod = pow(p->ship->mass,0.4) - 1.;

         /* Modify combat rating. */
         player_crating += 2*mod;
      }

      pilot_setFlag( p,PILOT_DISABLED ); /* set as disabled */
      /* run hook */
      pilot_runHook( p, PILOT_HOOK_DISABLE );
   }

   /* Officially dead. */
   if (p->armour <= 0.) {
      p->armour = 0.;
      dam_mod   = 0.;

      if (!pilot_isFlag(p, PILOT_DEAD)) {
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
            knockback * (w->vel.x * (dam_mod/6. + w->mass/p->solid->mass/6.)),
            knockback * (w->vel.y * (dam_mod/6. + w->mass/p->solid->mass/6.)) );

   return dmg;
}


/**
 * @brief Pilot is dead, now will slowly explode.
 *
 *    @param p Pilot that just died.
 */
void pilot_dead( Pilot* p )
{
   if (pilot_isFlag(p,PILOT_DEAD)) return; /* he's already dead */

   /* basically just set timers */
   if (p->id==PLAYER_ID)
      player_dead();
   p->timer[0] = 0.; /* no need for AI anymore */
   p->ptimer = 1. + sqrt(10*p->armour_max*p->shield_max) / 1000.;
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

   /* run hook if pilot has a death hook */
   pilot_runHook( p, PILOT_HOOK_DEATH );
}


/**
 * @brief Tries to run a pilot hook if he has it.
 *
 *    @param p Pilot to run the hook.
 *    @param hook_type Type of hook to run.
 */
void pilot_runHook( Pilot* p, int hook_type )
{
   int i;
   for (i=0; i<p->nhooks; i++)
      if (p->hooks[i].type == hook_type)
         hook_runID( p->hooks[i].id );
}


/**
 * @brief Sets the pilot's secondary weapon.
 *
 *    @param p Pilot to set secondary weapon.
 *    @param o Outfit to set as secondary. 
 */
static void pilot_setSecondary( Pilot* p, Outfit* o )
{
   int i;

   /* no need for ammo if there is no secondary */
   if (o == NULL) {
      p->secondary   = NULL;
      p->ammo        = NULL;
      return;
   }

   /* find which is the secondary and set ammo appropriately */
   for (i=0; i<p->noutfits; i++) {
      if (p->outfits[i].outfit == o) {
         p->secondary = &p->outfits[i];
         pilot_setAmmo( p );
         return;
      }
   }

   p->secondary   = NULL;
   p->ammo        = NULL;
}


/**
 * @brief Sets the pilot's ammo based on their secondary weapon.
 *
 *    @param p Pilot to set ammo.
 */
void pilot_setAmmo( Pilot* p )
{
   int i;
   Outfit *ammo;

   /* Weapon must use ammo. */
   if ((p->secondary == NULL) || (outfit_ammo(p->secondary->outfit)==NULL)) {
      p->ammo = NULL;
      return;
   }

   /* find the ammo and set it */
   ammo = outfit_ammo(p->secondary->outfit);
   for (i=0; i<p->noutfits; i++)
      if (p->outfits[i].outfit == ammo) {
         p->ammo = &p->outfits[i];
         return;
      }

   /* none found, so we assume it doesn't need ammo */
   p->ammo = NULL;
}


/**
 * @brief Gets the amount of ammo pilot has for a certain outfit.
 *
 *    @param p Pilot to get amount of ammo for.
 *    @param o Outfit to get ammo for.
 *    @return Amount of ammo for o on p.
 */
int pilot_getAmmo( Pilot* p, Outfit* o )
{
   int i;
   Outfit *ammo;

   /* Must be a launcher. */
   if (!outfit_isLauncher(o))
      return 0;

   /* Try to find the ammo. */
   ammo = o->u.lau.ammo;
   for (i=0; i<p->noutfits; i++)
      if (p->outfits[i].outfit == ammo)
         return p->outfits[i].quantity;

   /* Assume none. */
   return 0;
}


/**
 * @brief Sets the pilot's afterburner if he has one.
 *
 *    @param p Pilot to set afterburner.
 */
void pilot_setAfterburner( Pilot* p )
{
   int i;

   for (i=0; i<p->noutfits; i++)
      if (outfit_isAfterburner( p->outfits[i].outfit )) {
         p->afterburner = &p->outfits[i];
         return;
      }
   p->afterburner = NULL;
}


/**
 * @brief Docks the pilot on it's target pilot.
 *
 *    @param p Pilot that wants to dock.
 *    @param target Pilot to dock on.
 *    @return 0 on successful docking.
 */
int pilot_dock( Pilot *p, Pilot *target )
{
   int i;
   Outfit *o;

   /* Must be close. */
   if (vect_dist(&p->solid->pos, &target->solid->pos) > 30.)
      return -1;

   /* Cannot be going much faster. */
   if (vect_dist(&p->solid->vel, &target->solid->vel) > 2*MIN_VEL_ERR)
      return -1;

   /* Remove from pilot's escort list. */
   for (i=0; i<target->nescorts; i++) {
      if (target->escorts[i] == p->id)
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
            sizeof(unsigned int) * (target->nescorts-i-1) );
      target->nescorts--;
   }

   /* Check to see if target has an available bay. */
   for (i=0; i<target->noutfits; i++) {
      if (outfit_isFighterBay(target->outfits[i].outfit)) {
         o = outfit_ammo(target->outfits[i].outfit);
         if (outfit_isFighter(o) &&
               (strcmp(p->ship->name,o->u.fig.ship)==0))
            break;
      }
   }
   if (i >= target->noutfits)
      return -1;

   /* Add the pilot's outfit. */
   if (pilot_addOutfit(target, o, 1) != 1)
      return -1;

   /* Destroy the pilot. */
   pilot_setFlag(p,PILOT_DELETE);

   return 0;
}


/**
 * @brief Makes the pilot explosion.
 *    @param x X position of the pilot.
 *    @param y Y position of the pilot.
 *    @param radius Radius of the explosion.
 *    @param dtype Damage type of the explosion.
 *    @param damage Amount of damage by the explosion.
 *    @param parent ID of the pilot exploding.
 */
void pilot_explode( double x, double y, double radius,
      DamageType dtype, double damage, unsigned int parent )
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
      dist += pow2(p->ship->gfx_space->sw) + pow2(p->ship->gfx_space->sh);

      /* Pilot is hit. */
      if (dist < rad2) {

         /* Impact settings. */
         s.mass =  pow2(damage) * sqrt(rad2 - dist) / 30.;
         s.vel.x = rx;
         s.vel.y = ry;

         /* Actual damage calculations. */
         pilot_hit( p, &s, parent, dtype, damage );

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
 */
void pilot_render( Pilot* p )
{
   gl_blitSprite( p->ship->gfx_space,
         p->solid->pos.x, p->solid->pos.y,
         p->tsx, p->tsy, NULL );
}


/**
 * @brief Updates the pilot.
 *
 *    @param pilot Pilot to update.
 *    @param dt Current delta tick.
 */
static void pilot_update( Pilot* pilot, const double dt )
{
   int i;
   unsigned int l;
   Pilot *target;
   double a, px,py, vx,vy;
   char buf[16];
   PilotOutfit *o;

   /*
    * Update timers.
    */
   pilot->ptimer -= dt;
   pilot->tcontrol -= dt;
   for (i=0; i<MAX_AI_TIMERS; i++)
      pilot->timer[i] -= dt;
   for (i=0; i<pilot->noutfits; i++) {
      o = &pilot->outfits[i];
      if (o->timer > 0.)
         o->timer -= dt;
   }

   /* he's dead jim */
   if (pilot_isFlag(pilot,PILOT_DEAD)) {
      if (pilot->ptimer < 0.) { /* completely destroyed with final explosion */
         if (pilot->id==PLAYER_ID) /* player handled differently */
            player_destroyed();
         pilot_setFlag(pilot,PILOT_DELETE); /* will get deleted next frame */
         return;
      }

      /* pilot death sound */
      if (!pilot_isFlag(pilot,PILOT_DEATH_SOUND) && (pilot->ptimer < 0.050)) {
         
         /* Play random explsion sound. */
         snprintf(buf, 16, "explosion%d", RNG(0,2));
         sound_playPos( sound_get(buf), pilot->solid->pos.x, pilot->solid->pos.y );
         
         pilot_setFlag(pilot,PILOT_DEATH_SOUND);
      }
      /* final explosion */
      else if (!pilot_isFlag(pilot,PILOT_EXPLODED) && (pilot->ptimer < 0.200)) {

         /* Damage from explosion. */
         a = sqrt(pilot->solid->mass);
         expl_explode( pilot->solid->pos.x, pilot->solid->pos.y,
               pilot->solid->vel.x, pilot->solid->vel.y,
               pilot->ship->gfx_space->sw/2. + a,
               DAMAGE_TYPE_KINETIC, 2.*a - 20.,
               0, EXPL_MODE_SHIP );
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
      else if (pilot->timer[1] < 0.) {
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
      vectnull( &pilot->solid->force ); /* no more accel */
      pilot->solid->dir_vel = 0.; /* no more turning */

      /* update the solid */
      pilot->solid->update( pilot->solid, dt );
      gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,
            pilot->ship->gfx_space, pilot->solid->dir );
      return;
   }

   /* still alive */
   else if (pilot->armour < pilot->armour_max) /* regen armour */
      pilot->armour += pilot->armour_regen * dt;
   else /* regen shield */
      pilot->shield += pilot->shield_regen * dt;

   /* Update energy */
   if ((pilot->energy < 1.) && pilot_isFlag(pilot, PILOT_AFTERBURNER))
      pilot_rmFlag(pilot, PILOT_AFTERBURNER); /* Break efterburner */

   /*
    * Using RC circuit eenergy loading.
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
   if (pilot->armour > pilot->armour_max)
      pilot->armour = pilot->armour_max;
   if (pilot->shield > pilot->shield_max)
      pilot->shield = pilot->shield_max;
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

   /* update the solid */
   (*pilot->solid->update)( pilot->solid, dt );
   gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,  
         pilot->ship->gfx_space, pilot->solid->dir );

   if (!pilot_isFlag(pilot, PILOT_HYPERSPACE)) { /* limit the speed */

      /* pilot is afterburning */
      if (pilot_isFlag(pilot, PILOT_AFTERBURNER) && /* must have enough energy left */
               (pilot->energy > pilot->afterburner->outfit->u.afb.energy * dt)) {
         limit_speed( &pilot->solid->vel, /* limit is higher */
               pilot->speed * pilot->afterburner->outfit->u.afb.speed_perc + 
               pilot->afterburner->outfit->u.afb.speed_abs, dt );

         if (pilot->id == PLAYER_ID)
            spfx_shake( 0.75*SHAKE_DECAY * dt); /* shake goes down at quarter speed */

         pilot->energy -= pilot->afterburner->outfit->u.afb.energy * dt; /* energy loss */
      }
      else /* normal limit */
         limit_speed( &pilot->solid->vel, pilot->speed, dt );
   }
}


/**
 * @brief Handles pilot's hyperspace states.
 *
 *    @param p Pilot to handle hyperspace navigation.
 */
static void pilot_hyperspace( Pilot* p )
{
   double diff;

   /* pilot is actually in hyperspace */
   if (pilot_isFlag(p, PILOT_HYPERSPACE)) {

      /* has jump happened? */
      if (p->ptimer < 0.) {
         if (p == player) { /* player just broke hyperspace */
            player_brokeHyperspace();
         }
         else {
            pilot_runHook( p, PILOT_HOOK_JUMP );
            pilot_setFlag(p, PILOT_DELETE); /* set flag to delete pilot */
         }
         return;
      }

      /* keep acceling - hyperspace uses much bigger accel */
      vect_pset( &p->solid->force, HYPERSPACE_THRUST*p->solid->mass, p->solid->dir );
   }
   /* engines getting ready for the jump */
   else if (pilot_isFlag(p, PILOT_HYP_BEGIN)) {

      if (p->ptimer < 0.) { /* engines ready */
         p->ptimer = HYPERSPACE_FLY_DELAY;
         pilot_setFlag(p, PILOT_HYPERSPACE);
      }
   }
   /* pilot is getting ready for hyperspace */
   else {

      /* brake */
      if (VMOD(p->solid->vel) > MIN_VEL_ERR) {
         diff = pilot_face( p, VANGLE(p->solid->vel) + M_PI );

         if (ABS(diff) < MAX_DIR_ERR)
            vect_pset( &p->solid->force, p->thrust, p->solid->dir );

      }
      /* face target */
      else {

         vectnull( &p->solid->force ); /* stop accel */

         /* player should actually face the system he's headed to */
         if (p==player) diff = player_faceHyperspace();
         else diff = pilot_face( p, VANGLE(p->solid->pos) );

         if (ABS(diff) < MAX_DIR_ERR) { /* we can now prepare the jump */
            p->solid->dir_vel = 0.;
            p->ptimer = HYPERSPACE_ENGINE_DELAY;
            pilot_setFlag(p, PILOT_HYP_BEGIN);
         }
      }
   }
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
      if (pilot_isFlag(p, PILOT_HYP_BEGIN))
         pilot_rmFlag(p, PILOT_HYP_BEGIN);
      if (pilot_isFlag(p, PILOT_HYP_PREP))
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
 * @brief Sets the mount points for an outfit.
 *
 *    @param p Pilot to set mounts on.
 *    @param po Outfit to set mount points for.
 *    @param o Original number of outfits.
 *    @param q Outfits added.
 *    @return 0 on success;
 */
static int pilot_setOutfitMounts( Pilot *p, PilotOutfit* po, int o, int q )
{
   int i, n, k, min;

   /* Grow the memory. */
   po->mounts = realloc(po->mounts, (o+q) * sizeof(int));

   /* Has to be done for each outfit added. */
   for (n=o; n < o+q; n++) {

      /* Special case no ship mounts. */
      if (p->mounted == NULL) {
         po->mounts[n] = 0;
         continue;
      }

      /* Default to 0. */
      k     = 0;
      min   = INT_MAX;
      /* Find mount with fewest spots. */
      for (i=1; i<p->ship->nmounts; i++) {
         if (p->mounted[i] < min) {
            k     = i;
            min   = p->mounted[i];
         }
      }

      /* Add the mount point. */
      po->mounts[n] = k;
      p->mounted[k]++;
   }

   return 0;
}


/**
 * @brief Adds an outfit to the pilot.
 *
 *    @param pilot Pilot to add the outfit to.
 *    @param outfit Outfit to add to the pilot.
 *    @param quantity Amount of the outfit to add.
 *    @return Amount of the outfit added.
 */
int pilot_addOutfit( Pilot* pilot, Outfit* outfit, int quantity )
{
   int i;
   int o, q, free_space;
   Outfit *osec;
   PilotOutfit *po;

   free_space = pilot_freeSpace( pilot );
   q = quantity;

   /* special case if it's a map */
   if (outfit_isMap(outfit)) {
      if (pilot == player) /* Only player can get it. */
         map_map(NULL,outfit->u.map.radius);
      return 1; /* must return 1 for paying purposes */
   }
   /* special case if it's a license. */
   else if (outfit_isLicense(outfit)) {
      if (pilot == player) /* Only player can get it. */
         player_addLicense(outfit->name);
      return 1; /* must return 1 for paying purposes */
   }

   /* Mod quantity down if it doesn't fit */
   if (q*outfit->mass > free_space)
      q = free_space / outfit->mass;

   /* Can we actually add any? */
   if (q == 0)
      return 0;

   /* does outfit already exist? */
   for (i=0; i<pilot->noutfits; i++)
      if (pilot->outfits[i].outfit == outfit) {
         po = &pilot->outfits[i];
         o = po->quantity;
         po->quantity += q;
         /* can't be over max */
         if (po->quantity > outfit->max) {
            q -= po->quantity - outfit->max;
            po->quantity = outfit->max;
         }

         /* If it's a turret we need to find a mount spot for it. */
         if (outfit_isTurret(outfit))
            pilot_setOutfitMounts( pilot, po, o, q );

         /* recalculate the stats */
         pilot_calcStats(pilot);
         return q;
      }

   /* hacks in case it reallocs */
   osec = (pilot->secondary) ? pilot->secondary->outfit : NULL;
   /* no need for ammo since it's handled in setSecondary,
    * since pilot has only one afterburner it's handled at the end */

   /* grow the outfits */
   pilot->noutfits++;
   pilot->outfits = realloc(pilot->outfits, pilot->noutfits*sizeof(PilotOutfit));
   po = &pilot->outfits[pilot->noutfits-1];
   memset(po, 0, sizeof(PilotOutfit));
   po->outfit = outfit;
   po->quantity = q;

   /* can't be over max */
   if (po->quantity > outfit->max) {
      q -= po->quantity - outfit->max;
      po->quantity = outfit->max;
   }

   if (outfit_isTurret(outfit)) { /* used to speed up AI */
      /* If it's a turret we need to find a mount spot for it. */
      pilot_setOutfitMounts( pilot, po, 0, q );
      pilot_setFlag(pilot, PILOT_HASTURRET);
   }

   if (outfit_isBeam(outfit)) /* Used to speed up some calculations. */
      pilot_setFlag(pilot, PILOT_HASBEAMS);

   /* hack due to realloc possibility */
   pilot_setSecondary( pilot, osec );
   pilot_setAfterburner( pilot );

   /* recalculate the stats */
   pilot_calcStats(pilot);
   return q;
}


/**
 * @brief Removes an outfit from the pilot.
 *
 *    @param pilot Pilot to remove the outfit from.
 *    @param outfit Outfit to remove from the pilot.
 *    @param quantity Amount of outfits to remove from the pilot.
 *    @return Number of outfits removed from the pilot.
 */
int pilot_rmOutfit( Pilot* pilot, Outfit* outfit, int quantity )
{
   int i, j, q, c, o;
   Outfit *osec;
   PilotOutfit *po;

   c = (outfit_isMod(outfit)) ? outfit->u.mod.cargo : 0;
   q = quantity;
   for (i=0; i<pilot->noutfits; i++)
      if (pilot->outfits[i].outfit == outfit) {
         po = &pilot->outfits[i];

         /* Remove quantity. */
         o = po->quantity;
         po->quantity -= quantity;

         /* Calculate q. */
         if (po->quantity <= 0) {
            /* we didn't actually remove the full amount */
            q += po->quantity;
            po->quantity = 0.;
         }

         /* Remove from mount points. */
         if ((pilot->mounted != NULL) && (po->mounts != NULL)) {
            for (j=o-1; j >= po->quantity; j--) {
               if (po->mounts[j] != 0)
                  pilot->mounted[ po->mounts[j] ]--;
            }
         }

         /* Need to remove the outfit. */
         if (po->quantity <= 0) {

            /* hack in case it reallocs - can happen even when shrinking */
            osec = (pilot->secondary) ? pilot->secondary->outfit : NULL;

            /* free some memory if needed. */
            if (po->mounts != NULL) {
               free(po->mounts);
               po->mounts = NULL;
            }

            /* remove the outfit */
            pilot->noutfits--;
            if (pilot->noutfits <= 0) {
               if (pilot->outfits != NULL) {
                   free(pilot->outfits);
                   pilot->outfits = NULL;
               }
               pilot->noutfits = 0;
            }
            else {
               memmove( &pilot->outfits[i], &pilot->outfits[i+1],
                     sizeof(PilotOutfit) * (pilot->noutfits-i) );
               pilot->outfits = realloc( pilot->outfits,
                     sizeof(PilotOutfit) * (pilot->noutfits) );
            }

            /* set secondary  and afterburner */
            pilot_setSecondary( pilot, osec );
            pilot_setAfterburner( pilot );
         }
         pilot_calcStats(pilot); /* recalculate stats */
         return q;
      }
   WARN("Failure attempting to remove %d '%s' from pilot '%s'",
         quantity, outfit->name, pilot->name );
   return 0;
}


/**
 * @brief Gets all the outfits in nice text form.
 *    
 *    @param pilot Pilot to get the oufits from.
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
   /* first outfit */
   if (pilot->noutfits>0)
      p += snprintf( &buf[p], len-p, "%dx %s",
            pilot->outfits[0].quantity, pilot->outfits[0].outfit->name );
   else
      p += snprintf( &buf[p], len-p, "None" );
   /* rest of the outfits */
   for (i=1; i<pilot->noutfits; i++)
      p += snprintf( &buf[p], len-p, ", %dx %s",
            pilot->outfits[i].quantity, pilot->outfits[i].outfit->name );

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
   double ac, sc, ec, fc; /* temporary health coeficients to set */

   /*
    * set up the basic stuff
    */
   /* movement */
   pilot->thrust        = pilot->ship->thrust;
   pilot->turn          = pilot->ship->turn;
   pilot->speed         = pilot->ship->speed;
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

   /* cargo has to be reset */
   pilot_calcCargo(pilot);

   /*
    * now add outfit changes
    */
   nweaps = 0;
   wrange = wspeed = 0.;
   for (i=0; i<pilot->noutfits; i++) {
      o = pilot->outfits[i].outfit;
      q = (double) pilot->outfits[i].quantity;

      if (outfit_isMod(o)) { /* Modification */
         /* movement */
         pilot->thrust        += o->u.mod.thrust * q;
         pilot->turn          += o->u.mod.turn * q;
         pilot->speed         += o->u.mod.speed * q;
         /* health */
         pilot->armour_max    += o->u.mod.armour * q;
         pilot->armour_regen  += o->u.mod.armour_regen * q;
         pilot->shield_max    += o->u.mod.shield * q;
         pilot->shield_regen  += o->u.mod.shield_regen * q;
         pilot->energy_max    += o->u.mod.energy * q;
         pilot->energy_regen  += o->u.mod.energy_regen * q;
         /* fuel */
         pilot->fuel_max      += o->u.mod.fuel * q;
         /* misc */
         pilot->cargo_free    += o->u.mod.cargo * q;
      }
      else if (outfit_isAfterburner(o)) /* Afterburner */
         pilot->afterburner = &pilot->outfits[i]; /* Set afterburner */
      else if (outfit_isJammer(o)) { /* Jammer */
         if (pilot->jam_chance < o->u.jam.chance) { /* substitute */
            /** @todo make more jammers improve overall */
            pilot->jam_range = o->u.jam.range;
            pilot->jam_chance = o->u.jam.chance;
         }
         pilot->energy_regen -= o->u.jam.energy;
      }
      else if ((outfit_isWeapon(o) || outfit_isTurret(o)) && /* Primary weapon */
            !outfit_isProp(o,OUTFIT_PROP_WEAP_SECONDARY)) {
         nweaps++;
         wrange = MAX(wrange,outfit_range(o));
         wspeed += outfit_speed(o);
      }
   }

   /* Set final energy tau. */
   pilot->energy_tau = pilot->energy_max / pilot->energy_regen;

   /* Set weapon range and speed */
   pilot->weap_range = wrange; /* Range is max */
   pilot->weap_speed = wspeed / (double)nweaps;

   /* Give the pilot his health proportion back */
   pilot->armour = ac * pilot->armour_max;
   pilot->shield = sc * pilot->shield_max;
   pilot->energy = ec * pilot->energy_max;
   pilot->fuel = fc * pilot->fuel_max;
}


/**
 * @brief Gets the piilot's free cargo space.
 *
 *    @param p Pilot to get the the free space of.
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
   int i, q, f;

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
            pilot->solid->mass             += q;
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
   pilot->solid->mass   += q;
   pilot->ncommodities++;

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
   int q;

   q = pilot_cargoUsed( pilot );

   pilot->cargo_free = pilot->ship->cap_cargo - q; /* reduce space left */
   pilot->solid->mass = pilot->ship->mass + q; /* cargo affects weight */
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
   if (max_id > id)
      mission_cargo_id = max_id;
   id = ++mission_cargo_id;

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
   int i, q;

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
         pilot->solid->mass   -= q;
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
 * @brief Adds a hook to the pilot.
 *
 *    @param pilot Pilot to add the hook to.
 *    @param type Type of the hook to add.
 *    @param hook ID of the hook to add.
 */
void pilot_addHook( Pilot *pilot, int type, int hook )
{
   pilot->nhooks++;
   pilot->hooks = realloc( pilot->hooks, sizeof(PilotHook) * pilot->nhooks );
   pilot->hooks[pilot->nhooks-1].type  = type;
   pilot->hooks[pilot->nhooks-1].id    = hook;
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
      const unsigned int flags )
{
   ShipOutfit* so;

   /* Clear memory. */
   memset(pilot, 0, sizeof(Pilot));

   if (flags & PILOT_PLAYER) /* player is ID 0 */
      pilot->id = PLAYER_ID;
   else
      pilot->id = ++pilot_id; /* new unique pilot id based on pilot_id, can't be 0 */

   /* Basic information. */
   pilot->ship = ship;
   pilot->name = strdup( (name==NULL) ? ship->name : name );

   /* faction */
   pilot->faction = faction;

   /* solid */
   pilot->solid = solid_create(ship->mass, dir, pos, vel);

   /* mounts */
   if (ship->nmounts > 0)
      pilot->mounted = calloc( ship->nmounts, sizeof(int) );

   /* outfits */
   if (!(flags & PILOT_NO_OUTFITS)) {
      if (ship->outfit) {
         for (so=ship->outfit; so; so=so->next) {
            pilot_addOutfit( pilot, so->data, so->quantity );
         }
      }
   }

   /* cargo - must be set before calcStats */
   pilot->cargo_free = pilot->ship->cap_cargo; /* should get redone with calcCargo */

   /* set the pilot stats based on his ship and outfits */
   pilot->armour = pilot->armour_max = 1.; /* hack to have full armour */
   pilot->shield = pilot->shield_max = 1.; /* ditto shield */
   pilot->energy = pilot->energy_max = 1.; /* ditto energy */
   pilot->fuel   = pilot->fuel_max   = 1.; /* ditto fuel */
   pilot_calcStats(pilot);

   /* set flags and functions */
   if (flags & PILOT_PLAYER) {
      pilot->think = player_think; /* players don't need to think! :P */
      pilot->render = NULL; /* render will get called from player_think */
      pilot_setFlag(pilot,PILOT_PLAYER); /* it is a player! */
      if (!(flags & PILOT_EMPTY)) { /* sort of a hack */
         player = pilot;
         gui_load( pilot->ship->gui ); /* load the gui */
      }
   }
   else {
      pilot->think = ai_think;
      pilot->render = pilot_render;
   }
   /* Set enter hyperspace flag if needed. */
   if (flags & PILOT_HYP_END)
      pilot_setFlag(pilot, PILOT_HYP_END);

   /* all update the same way */
   pilot->update = pilot_update;

   /* Escort stuff. */
   if (flags & PILOT_ESCORT) {
      pilot_setFlag(pilot,PILOT_ESCORT);
      if (flags & PILOT_CARRIED)
         pilot_setFlag(pilot,PILOT_CARRIED);
   }

   /* AI */
   pilot->target = pilot->id; /* Self = no target. */
   if (ai != NULL)
      ai_pinit( pilot, ai ); /* Must run before ai_create */

   /* Update the x and y sprite positions. */
   gl_getSpriteFromDir( &pilot->tsx, &pilot->tsy,
         pilot->ship->gfx_space, pilot->solid->dir );
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
      const unsigned int flags )
{
   Pilot *dyn;
   
   dyn = malloc(sizeof(Pilot));
   if (dyn == NULL) {
      WARN("Unable to allocate memory");
      return 0;
   }
   pilot_init( dyn, ship, name, faction, ai, dir, pos, vel, flags );

   /* see if memory needs to grow */
   if (pilot_nstack+1 > pilot_mstack) { /* needs to grow */
      pilot_mstack += PILOT_CHUNK;
      pilot_stack = realloc( pilot_stack, pilot_mstack*sizeof(Pilot*) );
   }

   /* set the pilot in the stack */
   pilot_stack[pilot_nstack] = dyn;
   pilot_nstack++; /* there's a new pilot */

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
      int faction, const char *ai, const unsigned int flags )
{
   Pilot* dyn;
   dyn = malloc(sizeof(Pilot));
   if (dyn == NULL) {
      WARN("Unable to allocate memory");
      return 0;
   }
   pilot_init( dyn, ship, name, faction, ai, 0., NULL, NULL, flags | PILOT_EMPTY );
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
   int i;
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

   /* Copy mountpoints. */
   if (src->mounted != NULL) {
      dest->mounted = malloc( sizeof(int)*src->ship->nmounts );
      memcpy( dest->mounted, src->mounted, sizeof(int)*src->ship->nmounts );
   }

   /* Hooks get cleared. */
   dest->hooks          = NULL;
   dest->nhooks         = 0;

   /* Copy has no escorts. */
   dest->escorts        = NULL;
   dest->nescorts       = 0;

   /* AI is not copied. */
   dest->task           = NULL;

   /* Set pointers and friends to NULL. */
   /* Outfits. */
   dest->outfits        = NULL;
   dest->noutfits       = 0;
   dest->secondary      = NULL;
   dest->ammo           = NULL;
   dest->afterburner    = NULL;
   /* Commodities. */
   dest->commodities    = NULL;
   dest->ncommodities   = 0;
   /* Calculate stats. */
   pilot_calcStats(dest);

   /* Copy outfits. */
   for (i=0; i<src->noutfits; i++)
      pilot_addOutfit( dest, src->outfits[i].outfit,
            src->outfits[i].quantity );

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
   if (p->hooks) {
      for (i=0; i<p->nhooks; i++)
         hook_rm( p->hooks[i].id );
      free(p->hooks);
   }

   /* If hostile, must remove counter. */
   pilot_rmHostile(p);

   /* Remove outfits. */
   while (p->outfits != NULL)
      pilot_rmOutfit( p, p->outfits[0].outfit, p->outfits[0].quantity );

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
   if (player==p)
      player = NULL;
   solid_free(p->solid);
   if (p->mounted != NULL)
      free(p->mounted);
   if (p->escorts)
      free(p->escorts);

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
   player = NULL;
   pilot_nstack = 0;
}


/**
 * @brief Cleans up the pilot stack - leaves the player.
 */
void pilots_clean (void)
{
   int i;
   for (i=0; i < pilot_nstack; i++)
      /* we'll set player at privileged position */
      if ((player != NULL) && (pilot_stack[i] == player)) {
         pilot_stack[0] = player;
         pilot_stack[0]->lockons = 0; /* Clear lockons. */
      }
      else /* rest get killed */
         pilot_free(pilot_stack[i]);

   if (player != NULL) { /* set stack to 1 if pilot exists */
      pilot_nstack = 1;
      pilot_clearTimers( player ); /* Reset the player's timers. */
   }
}


/**
 * @brief Even cleans up the player.
 */
void pilots_cleanAll (void)
{
   pilots_clean();
   if (player != NULL) {
      pilot_free(player);
      player = NULL;
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

      /* See if should think. */
      if (p->think && !pilot_isDisabled(p)) {

         /* Hyperspace gets special treatment */
         if (pilot_isFlag(p, PILOT_HYP_PREP))
            pilot_hyperspace(p);
         /* Entering hyperspace. */
         else if (pilot_isFlag(p, PILOT_HYP_END)) {
            if (VMOD(p->solid->vel) < 2*p->speed)
               pilot_rmFlag(p, PILOT_HYP_END);
         }
         /* Must not be boarding to think. */
         else if (!pilot_isFlag(p, PILOT_BOARDING) &&
               !pilot_isFlag(p, PILOT_REFUELBOARDING))
            p->think(p);

      }

      /* Just update the pilot. */
      if (p->update) { /* update */
         p->update( p, dt );
      }
   }
}


/**
 * @brief Renders all the pilots.
 */
void pilots_render (void)
{
   int i;
   for (i=0; i<pilot_nstack; i++) {
      if (player == pilot_stack[i]) continue; /* skip player */
      if (pilot_stack[i]->render != NULL) /* render */
         pilot_stack[i]->render(pilot_stack[i]);
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
   PilotOutfit *o;

   pilot->ptimer = 0.;
   pilot->tcontrol = 0.;
   for (i=0; i<MAX_AI_TIMERS; i++)
      pilot->timer[i] = 0.;
   for (i=0; i<pilot->noutfits; i++) {
      o = &pilot->outfits[i];
      if (o->timer > 0.)
         o->timer = 0.;
   }
}
