/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot_weapon.c
 *
 * @brief Handles pilot weapon sets.
 *
 * Cheat sheet: how this works (it's complicated).
 *
 * KEYPRESS
 * 1) weapSetPress
 * 2) weapSetFire
 * 2.1) Modifications get turned on/off
 * 2.2) Weapons go to shootWeaponSetOutfit
 *
 * UPDATE
 * 1) weapSetUpdate
 * 2.1) fire set => weapSetFire
 * 2.1.1) Modifications get turned on/off
 * 2.1.2) Weapons go to shootWeaponSetOutfit
 * 2.2) weapsetUpdateOutfits
 *
 * So to actually modify stuff, chances are you want to go to pilot_weapSetFire.
 */


#include "pilot.h"

#include "naev.h"

#include "nxml.h"

#include "log.h"
#include "array.h"
#include "weapon.h"
#include "escort.h"

#include "player.h"
#include "spfx.h"


/*
 * Prototypes.
 */
static void pilot_weapSetUpdateOutfits( Pilot* p, PilotWeaponSet *ws );
static PilotWeaponSet* pilot_weapSet( Pilot* p, int id );
static int pilot_weapSetFire( Pilot *p, PilotWeaponSet *ws, int level );
static int pilot_shootWeaponSetOutfit( Pilot* p, PilotWeaponSet *ws, Outfit *o, int level, double time );
static int pilot_shootWeapon( Pilot* p, PilotOutfitSlot* w, double time );
static void pilot_weapSetUpdateRange( PilotWeaponSet *ws );


/**
 * @brief Gets a weapon set from id.
 *
 *    @param p Pilot to get weapon set from.
 *    @param id ID of the weapon set.
 *    @return The weapon set matching id.
 */
static PilotWeaponSet* pilot_weapSet( Pilot* p, int id )
{
   return &p->weapon_sets[ id ];
}


/**
 * @brief Fires a weapon set.
 *
 *    @param p Pilot firing weaponsets.
 *    @param ws Weapon set to fire.
 *    @param level Level of the firing weapon set.
 */
static int pilot_weapSetFire( Pilot *p, PilotWeaponSet *ws, int level )
{
   int i, j, ret, s;
   Pilot *pt;
   AsteroidAnchor *field;
   Asteroid *ast;
   double time;
   Outfit *o;

   /* Case no outfits. */
   if (ws->slots == NULL)
      return 0;

   /* Fire. */
   ret    = 0;
   for (i=0; i<array_size(ws->slots); i++) {
      o = ws->slots[i].slot->outfit;

      /* Ignore NULL outfits. */
      if (o == NULL)
         continue;

      /* Only "active" outfits. */
      if ((level != -1) && (ws->slots[i].level != level))
         continue;

      /* Only run once for each weapon type in the group. */
      s = 0;
      for (j=0; j<i; j++) {
         /* Only active outfits. */
         if ((level != -1) && (ws->slots[j].level != level))
            continue;
         /* Found a match. */
         if (ws->slots[j].slot->outfit == o) {
            s = 1;
            break;
         }
      }
      if (s!=0)
         continue;

      /* Only "locked on" outfits. */
      if (outfit_isSeeker(o) &&
            (ws->slots[i].slot->u.ammo.lockon_timer > 0.))
         continue;

      /* If inrange is set we only fire at targets in range. */
      time = INFINITY;  /* With no target we just set time to infinity. */
      if (p->target != p->id){
         pt = pilot_get( p->target );
         if (pt != NULL)
            time = pilot_weapFlyTime( o, p, &pt->solid->pos, &pt->solid->vel);
         }
      /* Looking for a closer targeted asteroid */
      if (p->nav_asteroid != -1){
         field = &cur_system->asteroids[p->nav_anchor];
         ast = &field->asteroids[p->nav_asteroid];
         time = MIN( time, pilot_weapFlyTime( o, p, &ast->pos, &ast->vel) );
      }

      /* Only "inrange" outfits. */
      if ( ws->inrange && outfit_duration(o) < time)
         continue;

      /* Shoot the weapon of the weaponset. */
      ret += pilot_shootWeaponSetOutfit( p, ws, o, level, time );
   }

   return ret;
}


/**
 * @brief Useful function for AI, clears activeness of all weapon sets.
 */
void pilot_weapSetAIClear( Pilot* p )
{
   int i;
   PilotWeaponSet *ws;
   for (i=0; i<PILOT_WEAPON_SETS; i++) {
      ws = &p->weapon_sets[i];
      ws->active = 0;
   }
}


/**
 * @brief Handles a weapon set press.
 *
 *    @param p Pilot the weapon set belongs to.
 *    @param id ID of the weapon set.
 *    @param type Is +1 if it's a press or -1 if it's a release.
 */
void pilot_weapSetPress( Pilot* p, int id, int type )
{
   int i, l, on, n;
   PilotWeaponSet *ws;

   ws = pilot_weapSet(p,id);
   /* Case no outfits. */
   if (ws->slots == NULL)
      return;

   /* Handle fire groups. */
   switch (ws->type) {
      case WEAPSET_TYPE_CHANGE:
         /* On press just change active weapon set to whatever is available. */
         if (type > 0) {
            if (id != p->active_set)
               pilot_weapSetUpdateOutfits( p, ws );
            p->active_set = id;
         }
         break;

      case WEAPSET_TYPE_WEAPON:
         /* Activation philosophy here is to turn on while pressed and off
          * when it's not held anymore. */
         if (type > 0)
            ws->active = 1;
         else if (type < 0)
            ws->active = 0;
         break;

      case WEAPSET_TYPE_ACTIVE:
         /* The behaviour here is more complex. What we do is consider a group
          * to be entirely off if not all outfits are either on or cooling down.
          * In the case it's deemed to be off, all outfits that are off get turned
          * on, otherwise all outfits that are on are turrned to cooling down. */
         /* Only care about presses. */
         if (type < 0)
            break;

         /* Must not be disabled or cooling down. */
         if ((pilot_isDisabled(p)) || (pilot_isFlag(p, PILOT_COOLDOWN)))
            return;

         /* Decide what to do. */
         on = 1;
         l  = array_size(ws->slots);
         for (i=0; i<l; i++) {
            if (ws->slots[i].slot->state == PILOT_OUTFIT_OFF) {
               on = 0;
               break;
            }
         }

         /* Turn them off. */
         n = 0;
         if (on) {
            for (i=0; i<l; i++) {
               if (ws->slots[i].slot->state != PILOT_OUTFIT_ON)
                  continue;

               n += pilot_outfitOff( p, ws->slots[i].slot );
            }
         }
         /* Turn them on. */
         else {
            for (i=0; i<l; i++) {
               if (ws->slots[i].slot->state != PILOT_OUTFIT_OFF)
                  continue;
               if (outfit_isAfterburner(ws->slots[i].slot->outfit))
                  pilot_afterburn( p );
               else {
                  ws->slots[i].slot->state  = PILOT_OUTFIT_ON;
                  ws->slots[i].slot->stimer = outfit_duration( ws->slots[i].slot->outfit );
               }
               n++;
            }
         }
         /* Must recalculate stats. */
         if (n > 0)
            pilot_calcStats( p );

         break;
   }
}


/**
 * @brief Updates the pilot's weapon sets.
 *
 *    @param p Pilot to update.
 */
void pilot_weapSetUpdate( Pilot* p )
{
   PilotWeaponSet *ws;
   int i;

   /* Must not be doing hyperspace procedures. */
   if (pilot_isFlag( p, PILOT_HYP_BEGIN))
      return;

   for (i=0; i<PILOT_WEAPON_SETS; i++) {
      ws = &p->weapon_sets[i];
      if (ws->slots == NULL)
         continue;

      /* Weapons must get "fired" every turn. */
      if (ws->type == WEAPSET_TYPE_WEAPON) {
         if (ws->active)
            pilot_weapSetFire( p, ws, -1 );
      }
   }
}


/**
 * @brief Updates the outfits with their current weapon set level.
 */
static void pilot_weapSetUpdateOutfits( Pilot* p, PilotWeaponSet *ws )
{
   int i;

   for (i=0; i<p->noutfits; i++)
      p->outfits[i]->level = -1;

   if (ws->slots != NULL)
      for (i=0; i<array_size(ws->slots); i++)
         ws->slots[i].slot->level = ws->slots[i].level;
}


/**
 * @brief Checks the current weapon set type.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set to check.
 *    @return The type of the weapon set.
 */
int pilot_weapSetTypeCheck( Pilot* p, int id )
{
   PilotWeaponSet *ws;
   ws = pilot_weapSet(p,id);
   return ws->type;
}


/**
 * @brief Changes the weapon sets mode.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param fire Whether or not to enable fire mode.
 */
void pilot_weapSetType( Pilot* p, int id, int type )
{
   int i;
   PilotWeaponSet *ws;

   ws = pilot_weapSet(p,id);
   ws->type = type;

   /* Set levels just in case. */
   if (ws->slots == NULL)
      return;

   /* See if we must overwrite levels. */
   if ((ws->type == WEAPSET_TYPE_WEAPON) ||
         (ws->type == WEAPSET_TYPE_ACTIVE))
      for (i=0; i<array_size(ws->slots); i++)
         ws->slots[i].level = 0;
}


/**
 * @brief Checks the current weapon set inrange property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set to check.
 *    @return The inrange mode of the weapon set.
 */
int pilot_weapSetInrangeCheck( Pilot* p, int id )
{
   PilotWeaponSet *ws;
   ws = pilot_weapSet(p,id);
   return ws->inrange;
}


/**
 * @brief Changes the weapon set inrange property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param inrange Whether or not to only fire at stuff in range.
 */
void pilot_weapSetInrange( Pilot* p, int id, int inrange )
{
   PilotWeaponSet *ws;
   ws = pilot_weapSet(p,id);
   ws->inrange = inrange;
}


/**
 * @brief Gets the name of a weapon set.
 */
const char *pilot_weapSetName( Pilot* p, int id )
{
   PilotWeaponSet *ws;
   ws = pilot_weapSet(p,id);
   if ((ws->slots == NULL) || (array_size(ws->slots)==0))
      return _("Unused");
   switch (ws->type) {
      case WEAPSET_TYPE_CHANGE: return _("Weapons - Switched");  break;
      case WEAPSET_TYPE_WEAPON: return _("Weapons - Instant");   break;
      case WEAPSET_TYPE_ACTIVE: return _("Abilities - Toggled"); break;
   }
   return NULL;
}


/**
 * @brief Removes slots by type from the weapon set.
 */
void pilot_weapSetRmSlot( Pilot *p, int id, OutfitSlotType type )
{
   int i, n, l;
   PilotWeaponSet *ws;

   /* We must clean up the slots. */
   n  = 0; /* Number to remove. */
   ws = pilot_weapSet(p,id);
   if (ws->slots == NULL)
      return;
   l  = array_size(ws->slots);
   for (i=0; i<l; i++) {
      if (ws->slots->slot->sslot->slot.type != type)
         continue;

      /* Move down. */
      memmove( &ws->slots[i], &ws->slots[i+1], sizeof(PilotWeaponSetOutfit) * (l-i-1) );
      n++;
   }

   /* Remove surplus. */
   array_erase( &ws->slots, &ws->slots[l-n], &ws->slots[l] );
}


/**
 * @brief Adds an outfit to a weapon set.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param o Outfit to add.
 *    @param level Level of the trigger.
 */
void pilot_weapSetAdd( Pilot* p, int id, PilotOutfitSlot *o, int level )
{
   PilotWeaponSet *ws;
   PilotWeaponSetOutfit *slot;
   Outfit *oo;
   int i, j;
   double r;

   ws = pilot_weapSet(p,id);

   /* Make sure outfit is valid. */
   oo = o->outfit;
   if (oo == NULL)
      return;

   /* Make sure outfit type is weapon (or usable). */
   if (!outfit_isActive(oo))
      return;

   /* Create if needed. */
   if (ws->slots == NULL)
      ws->slots = array_create( PilotWeaponSetOutfit );

   /* Check if already there. */
   for (i=0; i<array_size(ws->slots); i++) {
      if (ws->slots[i].slot == o) {
         ws->slots[i].level = level;

         /* Update if needed. */
         if (id == p->active_set)
            pilot_weapSetUpdateOutfits( p, ws );
         return;
      }
   }

   /* Add it. */
   slot        = &array_grow( &ws->slots );
   slot->level = level;
   slot->slot  = o;
   r           = outfit_range(oo);
   if (r > 0)
      slot->range2 = pow2(r);

   /* Updated cached weapset. */
   o->weapset = -1;
   for (j=0; j<PILOT_WEAPON_SETS; j++) {
      if (pilot_weapSetCheck(p, j, o) != -1) {
         o->weapset = j;
         break;
      }
   }

   /* Update range. */
   pilot_weapSetUpdateRange( ws );

   /* Update if needed. */
   if (id == p->active_set)
      pilot_weapSetUpdateOutfits( p, ws );
}


/**
 * @brief Removes a slot from a weapon set.
 *
 *    @param p Pilot who owns the weapon set.
 *    @param id ID of the weapon set.
 *    @param o Outfit to remove.
 */
void pilot_weapSetRm( Pilot* p, int id, PilotOutfitSlot *o )
{
   PilotWeaponSet *ws;
   int i, j;

   /* Make sure it has slots. */
   ws = pilot_weapSet(p,id);
   if (ws->slots == NULL)
      return;

   /* Find the slot. */
   for (i=0; i<array_size(ws->slots); i++) {
      if (ws->slots[i].slot != o)
         continue;

      array_erase( &ws->slots, &ws->slots[i], &ws->slots[i+1] );

      /* Update range. */
      pilot_weapSetUpdateRange( ws );

      /* Update if needed. */
      if (id == p->active_set)
         pilot_weapSetUpdateOutfits( p, ws );

      /* Updated cached weapset. */
      o->weapset = -1;
      for (j=0; j<PILOT_WEAPON_SETS; j++) {
         if (pilot_weapSetCheck(p, j, o) != -1) {
            o->weapset = j;
            break;
         }
      }

      return;
   }
}


/**
 * @brief Checks to see if a slot is in a weapon set.
 *
 *    @param p Pilot to check.
 *    @param id ID of the weapon set.
 *    @param o Outfit slot to check.
 *    @return The level to which it belongs (or -1 if it isn't set).
 */
int pilot_weapSetCheck( Pilot* p, int id, PilotOutfitSlot *o )
{
   PilotWeaponSet *ws;
   int i;

   /* Make sure it has slots. */
   ws = pilot_weapSet(p,id);
   if (ws->slots == NULL)
      return -1;

   /* Find the slot. */
   for (i=0; i<array_size(ws->slots); i++)
      if (ws->slots[i].slot == o)
         return ws->slots[i].level;

   /* Not found. */
   return -1;
}


/**
 * @brief Updates the weapon range for a pilot weapon set.
 *
 *    @param ws Weapon Set to update range for.
 */
static void pilot_weapSetUpdateRange( PilotWeaponSet *ws )
{
   int i, lev;
   double range, speed;
   double range_accum[PILOT_WEAPSET_MAX_LEVELS];
   int range_num[PILOT_WEAPSET_MAX_LEVELS];
   double speed_accum[PILOT_WEAPSET_MAX_LEVELS];
   int speed_num[PILOT_WEAPSET_MAX_LEVELS];

   /* No slots. */
   if (ws->slots == NULL) {
      for (i=0; i<PILOT_WEAPSET_MAX_LEVELS; i++) {
         ws->range[i] = 0.;
         ws->speed[i] = 0.;
      }
      return;
   }

   /* Calculate ranges. */
   for (i=0; i<PILOT_WEAPSET_MAX_LEVELS; i++) {
      range_accum[i] = 0.;
      range_num[i]   = 0;
      speed_accum[i] = 0.;
      speed_num[i]   = 0;
   }
   for (i=0; i<array_size(ws->slots); i++) {
      if (ws->slots[i].slot->outfit == NULL)
         continue;

      /* Get level. */
      lev = ws->slots[i].level;
      if (lev >= PILOT_WEAPSET_MAX_LEVELS)
         continue;

      /* Empty Launchers aren't valid */
      if (outfit_isLauncher(ws->slots[i].slot->outfit) && ws->slots[i].slot->u.ammo.quantity <= 0)
         continue;

      /* Get range. */
      range = outfit_range(ws->slots[i].slot->outfit);
      if (range >= 0.) {
         /* Calculate. */
         range_accum[ lev ] += range;
         range_num[ lev ]++;
      }

      /* Get speed. */
      speed = outfit_speed(ws->slots[i].slot->outfit);
      if (speed >= 0.) {
         /* Calculate. */
         speed_accum[ lev ] += speed;
         speed_num[ lev ]++;
      }
   }

   /* Postprocess. */
   for (i=0; i<PILOT_WEAPSET_MAX_LEVELS; i++) {
      /* Postprocess range. */
      if (range_num[i] == 0)
         ws->range[i] = 0;
      else
         ws->range[i] = range_accum[i] / (double) range_num[i];

      /* Postprocess speed. */
      if (speed_num[i] == 0)
         ws->speed[i] = 0;
      else
         ws->speed[i] = speed_accum[i] / (double) speed_num[i];
   }
}


/**
 * @brief Gets the range of the current pilot weapon set.
 *
 *    @param p Pilot to get the range of.
 *    @param id ID of weapon set to get the range of.
 *    @param Level of the weapons to get the range of (-1 for all).
 */
double pilot_weapSetRange( Pilot* p, int id, int level )
{
   PilotWeaponSet *ws;
   int i;
   double range;

   ws = pilot_weapSet(p,id);
   if (level < 0) {
      range = 0;
      for (i=0; i<PILOT_WEAPSET_MAX_LEVELS; i++)
         range += ws->range[i];
   }
   else
      range = ws->range[ level ];

   return range;
}


/**
 * @brief Gets the speed of the current pilot weapon set.
 *
 *    @param p Pilot to get the speed of.
 *    @param id ID of weapon set to get the speed of.
 *    @param Level of the weapons to get the speed of (-1 for all).
 */
double pilot_weapSetSpeed( Pilot* p, int id, int level )
{
   PilotWeaponSet *ws;
   int i;
   double speed;

   ws = pilot_weapSet(p,id);
   if (level < 0) {
      speed = 0;
      for (i=0; i<PILOT_WEAPSET_MAX_LEVELS; i++)
         speed += ws->speed[i];
   }
   else
      speed = ws->speed[ level ];

   return speed;
}


/**
 * @brief Cleans up a weapon set.
 *
 *    @param p Pilot who owns the weapon set.
 *    @param id ID of the weapon set to clean up.
 */
void pilot_weapSetCleanup( Pilot* p, int id )
{
   PilotWeaponSet *ws;

   ws = pilot_weapSet(p,id);

   if (ws->slots != NULL)
      array_free( ws->slots );
   ws->slots = NULL;

   /* Update range. */
   pilot_weapSetUpdateRange( ws );
}


/**
 * @brief Frees a pilot's weapon sets.
 */
void pilot_weapSetFree( Pilot* p )
{
   int i;
   for (i=0; i<PILOT_WEAPON_SETS; i++)
      pilot_weapSetCleanup( p, i );
}



/**
 * @brief Lists the items in a pilot weapon set.
 *
 *    @param p Pilot who owns the weapon set.
 *    @param id ID of the weapon set.
 *    @param[out] n Number of elements in the list.
 *    @return The array of pilot weaponset outfits.
 */
PilotWeaponSetOutfit* pilot_weapSetList( Pilot* p, int id, int *n )
{
   PilotWeaponSet *ws;

   ws = pilot_weapSet(p,id);
   if (ws->slots == NULL) {
      *n = 0;
      return NULL;
   }

   *n = array_size(ws->slots);
   return ws->slots;
}


/**
 * @brief Makes the pilot shoot.
 *
 *    @param p The pilot which is shooting.
 *    @param level Level of the shot.
 *    @return The number of shots fired.
 */
int pilot_shoot( Pilot* p, int level )
{
   PilotWeaponSet *ws;
   int ret;

   /* Get active set. */
   ws = pilot_weapSet( p, p->active_set );

   /* Fire weapons. */
   if (ws->type == WEAPSET_TYPE_CHANGE) { /* Must be a change set or a weaponset. */
      ret = pilot_weapSetFire( p, ws, level );

      /* Firing weapons aborts active cooldown. */
      if (pilot_isFlag(p, PILOT_COOLDOWN) && ret)
         pilot_cooldownEnd(p, NULL);

      return ret;
   }

   return 0;
}


/**
 * @brief Have pilot stop shooting their weapon.
 *
 * Only really deals with beam weapons.
 *
 *    @param p Pilot that was shooting.
 *    @param level Level of the shot.
 */
void pilot_shootStop( Pilot* p, int level )
{
   int i, recalc;
   PilotWeaponSet *ws;
   PilotOutfitSlot *slot;

   /* Get active set. */
   ws = pilot_weapSet( p, p->active_set );

   /* Case no outfits. */
   if (ws->slots == NULL)
      return;

   /* Stop all beams. */
   recalc = 0;
   for (i=0; i<array_size(ws->slots); i++) {
      slot = ws->slots[i].slot;

      /* Must have associated outfit. */
      if (ws->slots[i].slot->outfit == NULL)
         continue;

      /* Must match level. */
      if ((level != -1) && (ws->slots[i].level != level))
         continue;

      /* Only handle beams. */
      if (!outfit_isBeam(slot->outfit)) {
         /* Turn off the state. */
         if (outfit_isMod( slot->outfit )) {
            slot->state = PILOT_OUTFIT_OFF;
            recalc = 1;
         }
         continue;
      }

      /* Stop beam. */
      if (ws->slots[i].slot->u.beamid > 0) {
         /* Enforce minimum duration if set. */
         if (slot->outfit->u.bem.min_duration > 0.) {

            slot->stimer = slot->outfit->u.bem.min_duration -
                  (slot->outfit->u.bem.duration - slot->timer);

            if (slot->stimer > 0.)
               continue;
         }

         beam_end( p->id, slot->u.beamid );
         pilot_stopBeam(p, slot);
      }
   }

   /* Must recalculate. */
   if (recalc)
      pilot_calcStats( p );
}


/**
 * @brief Stops a beam outfit and sets delay as appropriate.
 *
 *    @param p Pilot that is firing.
 *    @param w Pilot's beam outfit.
 */
void pilot_stopBeam( Pilot *p, PilotOutfitSlot *w )
{
   double rate_mod, energy_mod, used;

   /* There's nothing to do if the beam isn't active. */
   if (w->u.beamid == 0)
      return;

  /* Safeguard against a nasty race condition. */
  if (w->outfit == NULL) {
      w->u.beamid = 0;
     return;
  }

   /* Calculate rate modifier. */
   pilot_getRateMod( &rate_mod, &energy_mod, p, w->outfit );

   /* Beam duration used. */
   used = w->outfit->u.bem.duration - w->timer;

   w->timer = rate_mod * (used / w->outfit->u.bem.duration) * outfit_delay( w->outfit );
   w->u.beamid = 0;
}


/**
 * @brief Computes an estimation of ammo flying time
 *
 *    @param w the weapon that shoot
 *    @param parent Parent of the weapon
 *    @param target Target of the weapon
 */
double pilot_weapFlyTime( Outfit *o, Pilot *parent, Vector2d *pos, Vector2d *vel)
{
   Vector2d approach_vector, relative_location, orthoradial_vector;
   double speed, radial_speed, orthoradial_speed, dist, t;

   dist = vect_dist( &parent->solid->pos, pos );

   /* Beam weapons */
   if (outfit_isBeam(o))
      {
      if (dist > o->u.bem.range)
         return INFINITY;
      return 0.;
      }

   /* A bay doesn't have range issues */
   if (outfit_isFighterBay(o))
      return 0.;

   /* Rockets use absolute velocity while bolt use relative vel */
   if (outfit_isLauncher(o))
         vect_cset( &approach_vector, - vel->x, - vel->y );
   else
         vect_cset( &approach_vector, VX(parent->solid->vel) - vel->x,
               VY(parent->solid->vel) - vel->y );

   speed = outfit_speed(o);

   /* Get the vector : shooter -> target */
   vect_cset( &relative_location, pos->x - VX(parent->solid->pos),
         pos->y - VY(parent->solid->pos) );

   /* Get the orthogonal vector */
   vect_cset(&orthoradial_vector, VY(parent->solid->pos) - pos->y,
         pos->x -  VX(parent->solid->pos) );

   radial_speed = vect_dot( &approach_vector, &relative_location );
   radial_speed = radial_speed / VMOD(relative_location);

   orthoradial_speed = vect_dot(&approach_vector, &orthoradial_vector);
   orthoradial_speed = orthoradial_speed / VMOD(relative_location);

   if( ((speed*speed - VMOD(approach_vector)*VMOD(approach_vector)) != 0) && (speed*speed - orthoradial_speed*orthoradial_speed) > 0)
      t = dist * (sqrt( speed*speed - orthoradial_speed*orthoradial_speed ) - radial_speed) /
            (speed*speed - VMOD(approach_vector)*VMOD(approach_vector));
   else
      return INFINITY;

   /* if t < 0, try the other solution */
   if (t < 0)
      t = - dist * (sqrt( speed*speed - orthoradial_speed*orthoradial_speed ) + radial_speed) /
            (speed*speed - VMOD(approach_vector)*VMOD(approach_vector));

   /* if t still < 0, no solution */
   if (t < 0)
      return INFINITY;

   return t;
}


/**
 * @brief Calculates and shoots the appropriate weapons in a weapon set matching an outfit.
 */
static int pilot_shootWeaponSetOutfit( Pilot* p, PilotWeaponSet *ws, Outfit *o, int level, double time )
{
   int i, ret;
   int is_launcher, is_bay;
   double rate_mod, energy_mod;
   PilotOutfitSlot *w;
   int maxp, minh;
   double q, maxt;

   /* Store number of shots. */
   ret = 0;

   /** @TODO Make beams not fire all at once. */
   if (outfit_isBeam(o)) {
      for (i=0; i<array_size(ws->slots); i++)
         if (ws->slots[i].slot->outfit == o)
            ret += pilot_shootWeapon( p, ws->slots[i].slot, 0 );
      return ret;
   }

   /* Stores if it is a launcher. */
   is_launcher = outfit_isLauncher(o);

   is_bay = outfit_isFighterBay(o);

   /* Calculate rate modifier. */
   pilot_getRateMod( &rate_mod, &energy_mod, p, o );

   /* Find optimal outfit, coolest that can fire. */
   minh  = -1;
   maxt  = 0.;
   maxp  = -1;
   q     = 0.;
   for (i=0; i<array_size(ws->slots); i++) {
      /* Only matching outfits. */
      if (ws->slots[i].slot->outfit != o)
         continue;

      /* Only match levels. */
      if ((level != -1) && (ws->slots[i].level != level))
         continue;

      /* Simplicity. */
      w = ws->slots[i].slot;

      /* Launcher only counts with ammo. */
      if ((is_launcher || is_bay) && ((w->u.ammo.outfit == NULL) || (w->u.ammo.quantity <= 0)))
         continue;

      /* Get coolest that can fire. */
      if (w->timer <= 0.) {
         if (is_launcher) {
            if ((minh < 0) || (ws->slots[minh].slot->u.ammo.quantity < w->u.ammo.quantity))
               minh = i;
         }
         else {
            if ((minh < 0) || (ws->slots[minh].slot->heat_T > w->heat_T))
               minh = i;
         }
      }

      /* Save some stuff. */
      if ((maxp < 0) || (w->timer > maxt)) {
         maxp = i;
         maxt = w->timer;
      }
      q += 1.;
   }

   /* No weapon can fire. */
   if (minh < 0)
      return 0;

   /* Only fire if the last weapon to fire fired more than (q-1)/q ago. */
   if (maxt > rate_mod * outfit_delay(o) * ((q-1.) / q))
      return 0;

   /* Shoot the weapon. */
   ret += pilot_shootWeapon( p, ws->slots[minh].slot, time );

   return ret;
}


/**
 * @brief Actually handles the shooting, how often the player.p can shoot and such.
 *
 *    @param p Pilot that is shooting.
 *    @param w Pilot's outfit to shoot.
 *    @param time Expected flight time.
 *    @return 0 if nothing was shot and 1 if something was shot.
 */
static int pilot_shootWeapon( Pilot* p, PilotOutfitSlot* w, double time )
{
   Vector2d vp, vv;
   double rate_mod, energy_mod;
   double energy;
   int j;

   /* Make sure weapon has outfit. */
   if (w->outfit == NULL)
      return 0;

   /* Reset beam shut-off if needed. */
   if (outfit_isBeam(w->outfit) && w->outfit->u.bem.min_duration)
      w->stimer = INFINITY;

   /* check to see if weapon is ready */
   if (w->timer > 0.)
      return 0;

   /* Calculate rate modifier. */
   pilot_getRateMod( &rate_mod, &energy_mod, p, w->outfit );

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

      energy      = outfit_energy(w->outfit)*energy_mod;
      p->energy  -= energy;
      pilot_heatAddSlot( p, w );
      weapon_add( w->outfit, w->heat_T, p->solid->dir,
            &vp, &p->solid->vel, p, p->target, time );
   }

   /*
    * Beam weapons.
    */
   else if (outfit_isBeam(w->outfit)) {
      /* Don't fire if the existing beam hasn't been destroyed yet. */
      if (w->u.beamid > 0)
         return 0;

      /* Check if enough energy to last a second. */
      if (outfit_energy(w->outfit)*energy_mod > p->energy)
         return 0;

      /** @todo Handle warmup stage. */
      w->state = PILOT_OUTFIT_ON;
      w->u.beamid = beam_start( w->outfit, p->solid->dir,
            &vp, &p->solid->vel, p, p->target, w );

      w->timer = w->outfit->u.bem.duration;

      return 1; /* Return early due to custom timer logic. */
   }

   /*
    * missile launchers
    *
    * must be a secondary weapon
    */
   else if (outfit_isLauncher(w->outfit)) {

      /* Shooter can't be the target - sanity check for the player.p */
      if ((w->outfit->u.lau.ammo->u.amm.ai != AMMO_AI_DUMB) && (p->id==p->target))
         return 0;

      /* Must have ammo left. */
      if ((w->u.ammo.outfit == NULL) || (w->u.ammo.quantity <= 0))
         return 0;

      /* enough energy? */
      if (outfit_energy(w->u.ammo.outfit)*energy_mod > p->energy)
         return 0;

      energy      = outfit_energy(w->u.ammo.outfit)*energy_mod;
      p->energy  -= energy;
      pilot_heatAddSlot( p, w );
      weapon_add( w->outfit, w->heat_T, p->solid->dir,
            &vp, &p->solid->vel, p, p->target, time );

      w->u.ammo.quantity -= 1; /* we just shot it */
      p->mass_outfit     -= w->u.ammo.outfit->mass;
      p->solid->mass     -= w->u.ammo.outfit->mass;

      pilot_updateMass( p );

      /* If last ammo was shot, update the range */
      if (w->u.ammo.quantity <= 0) {
         for (j=0; j<PILOT_WEAPON_SETS; j++)
            pilot_weapSetUpdateRange( &p->weapon_sets[j] );
      }
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
   else
      WARN(_("Shooting unknown weapon type: %s"), w->outfit->name);


   /* Reset timer. */
   w->timer += rate_mod * outfit_delay( w->outfit );

   return 1;
}


/**
 * @brief Gets applicable fire rate and energy modifications for a pilot's weapon.
 *
 *    @param[out] rate_mod Fire rate multiplier.
 *    @param[out] energy_mod Energy use multiplier.
 *    @param p Pilot who owns the outfit.
 *    @param w Pilot's outfit.
 */
void pilot_getRateMod( double *rate_mod, double* energy_mod,
      Pilot* p, Outfit *o )
{
   switch (o->type) {
      case OUTFIT_TYPE_BOLT:
         *rate_mod   = 2. - p->stats.fwd_firerate; /* Invert. */
         *energy_mod = p->stats.fwd_energy;
         break;
      case OUTFIT_TYPE_TURRET_BOLT:
         *rate_mod   = 2. - p->stats.tur_firerate; /* Invert. */
         *energy_mod = p->stats.tur_energy;
         break;

      default:
         *rate_mod   = 1.;
         *energy_mod = 1.;
         break;
   }
}


/**
 * @brief Clears the pilots weapon settings.
 *
 *    @param p Pilot whose weapons we're clearing.
 */
void pilot_weaponClear( Pilot *p )
{
   int i;
   PilotWeaponSet *ws;

   for (i=0; i<PILOT_WEAPON_SETS; i++) {
      ws = pilot_weapSet( p, i );
      if (ws->slots != NULL)
         array_erase( &ws->slots, &ws->slots[0], &ws->slots[ array_size(ws->slots) ] );
   }
}


/**
 * @brief Tries to automatically set and create the pilot's weapon set.
 *
 * Weapon set 0 is for all weapons. <br />
 * Weapon set 1 is for forward weapons. Ammo using weapons are secondaries. <br />
 * Weapon set 2 is for turret weapons. Ammo using weapons are secondaries. <br />
 * Weapon set 3 is for all weapons. Forwards are primaries and turrets are secondaries. <br />
 * Weapon set 4 is for seeking weapons. High payload variants are secondaries. <br />
 * Weapon set 5 is for fighter bays. <br />
 *
 *    @param p Pilot to automagically generate weapon lists.
 */
void pilot_weaponAuto( Pilot *p )
{
   PilotOutfitSlot *slot;
   Outfit *o;
   int i, level, id;

   /* Clear weapons. */
   pilot_weaponClear( p );

   /* Set modes. */
   pilot_weapSetType( p, 0, WEAPSET_TYPE_CHANGE );
   pilot_weapSetType( p, 1, WEAPSET_TYPE_CHANGE );
   pilot_weapSetType( p, 2, WEAPSET_TYPE_CHANGE );
   pilot_weapSetType( p, 3, WEAPSET_TYPE_CHANGE );
   pilot_weapSetType( p, 4, WEAPSET_TYPE_WEAPON );
   pilot_weapSetType( p, 5, WEAPSET_TYPE_WEAPON );
   pilot_weapSetType( p, 6, WEAPSET_TYPE_ACTIVE );
   pilot_weapSetType( p, 7, WEAPSET_TYPE_ACTIVE );
   pilot_weapSetType( p, 8, WEAPSET_TYPE_ACTIVE );
   pilot_weapSetType( p, 9, WEAPSET_TYPE_ACTIVE );

   /* All should be inrange. */
   if (!pilot_isPlayer(p))
      for (i=0; i<PILOT_WEAPON_SETS; i++){
         pilot_weapSetInrange( p, i, 1 );
         /* Update range and speed (at 0)*/
         pilot_weapSetUpdateRange( &p->weapon_sets[i] );
      }

   /* Iterate through all the outfits. */
   for (i=0; i<p->noutfits; i++) {
      slot = p->outfits[i];
      o    = slot->outfit;

      /* Must be non-empty, and a weapon or active outfit. */
      if ((o == NULL) || !outfit_isActive(o)) {
         slot->level   = -1; /* Clear level. */
         slot->weapset = -1;
         continue;
      }

      /* Manually defined group preempts others. */
      if (o->group) {
         id    = o->group;
      }
      /* Bolts and beams. */
      else if (outfit_isBolt(o) || outfit_isBeam(o) ||
            (outfit_isLauncher(o) && !outfit_isSeeker(o->u.lau.ammo))) {
         id    = outfit_isTurret(o) ? 2 : 1;
      }
      /* Seekers. */
      else if (outfit_isLauncher(o) && outfit_isSeeker(o->u.lau.ammo)) {
         id    = 4;
      }
      /* Fighter bays. */
      else if (outfit_isFighterBay(o)) {
         id    = 5;
      }
      /* Ignore rest. */
      else {
         slot->level = -1;
         continue;
      }

      /* Set level based on secondary flag. */
      level = outfit_isSecondary(o);

      /* Add to its base group. */
      pilot_weapSetAdd( p, id, slot, level );

      /* Also add another copy to another group. */
      if (id == 1) { /* Forward. */
         pilot_weapSetAdd( p, 0, slot, level ); /* Also get added to 'All'. */
         pilot_weapSetAdd( p, 3, slot, 0 );     /* Also get added to 'Fwd/Tur'. */
      }
      else if (id == 2) { /* Turrets. */
         pilot_weapSetAdd( p, 0, slot, level ); /* Also get added to 'All'. */
         pilot_weapSetAdd( p, 3, slot, 1 );     /* Also get added to 'Fwd/Tur'. */
      }
      else if (id == 4) /* Seekers */
         pilot_weapSetAdd( p, 0, slot, level ); /* Also get added to 'All'. */
   }

   /* Update active weapon set. */
   pilot_weapSetUpdateOutfits( p, &p->weapon_sets[ p->active_set ] );
}


/**
 * @brief Gives the pilot a default weapon set.
 */
void pilot_weaponSetDefault( Pilot *p )
{
   int i;

   /* If current set isn't a fire group no need to worry. */
   if (p->weapon_sets[ p->active_set ].type != WEAPSET_TYPE_CHANGE) {
      /* Update active weapon set. */
      pilot_weapSetUpdateOutfits( p, &p->weapon_sets[ p->active_set ] );
      return;
   }

   /* Find first fire group. */
   for (i=0; i<PILOT_WEAPON_SETS; i++)
      if (p->weapon_sets[i].type == WEAPSET_TYPE_CHANGE)
         break;

   /* Set active set to first if all fire groups or first non-fire group. */
   if (i >= PILOT_WEAPON_SETS)
      p->active_set = 0;
   else
      p->active_set = i;

   /* Update active weapon set. */
   pilot_weapSetUpdateOutfits( p, &p->weapon_sets[ p->active_set ] );
}


/**
 * @brief Sets the weapon set as sane.
 *
 *    @param p Pilot to set weapons as sane.
 */
void pilot_weaponSane( Pilot *p )
{
   int i, j;
   int n, l;
   PilotWeaponSet *ws;

   for (j=0; j<PILOT_WEAPON_SETS; j++) {
      ws = &p->weapon_sets[j];
      if (ws->slots == NULL)
         continue;

      l = array_size(ws->slots);
      n = 0;
      for (i=0; i<l; i++) {
         if (ws->slots[i].slot->outfit != NULL)
            continue;

         /* Move down. */
         memmove( &ws->slots[i], &ws->slots[i+1], sizeof(PilotWeaponSetOutfit) * (l-i-1) );
         n++;
      }
      /* Remove surplus. */
      if (n > 0)
         array_erase( &ws->slots, &ws->slots[l-n], &ws->slots[l] );

      /* See if we must overwrite levels. */
      if ((ws->type == WEAPSET_TYPE_WEAPON) ||
            (ws->type == WEAPSET_TYPE_ACTIVE))
         for (i=0; i<array_size(ws->slots); i++)
            ws->slots[i].level = 0;
   }

   /* Update range. */
   pilot_weapSetUpdateRange( ws );
}

/**
 * @brief Disables a given active outfit.
 *
 * @param p Pilot whose outfit we are disabling.
 * @return Whether the outfit was actually disabled.
 */
int pilot_outfitOff( Pilot *p, PilotOutfitSlot *o )
{
   /* Must not be disabled or cooling down. */
   if ((pilot_isDisabled(p)) || (pilot_isFlag(p, PILOT_COOLDOWN)))
      return 0;

   if (outfit_isAfterburner( o->outfit )) /* Afterburners */
      pilot_afterburnOver( p );
   else if (outfit_isBeam( o->outfit )) {
      /* Beams use stimer to represent minimum time until shutdown. */
      o->stimer = -1;
   }
   else {
      o->stimer = outfit_cooldown( o->outfit );
      o->state  = PILOT_OUTFIT_COOLDOWN;
   }

   return 1;
}

/**
 * @brief Disables all active outfits for a pilot.
 *
 * @param p Pilot whose outfits we are disabling.
 * @return Whether any outfits were actually disabled.
 */
int pilot_outfitOffAll( Pilot *p )
{
   PilotOutfitSlot *o;
   int nchg;
   int i;

   nchg = 0;
   for (i=0; i<p->noutfits; i++) {
      o = p->outfits[i];
      /* Picky about our outfits. */
      if (o->outfit == NULL)
         continue;
      if (!o->active)
         continue;
      if (o->state == PILOT_OUTFIT_ON)
         nchg += pilot_outfitOff( p, o );
   }
   return (nchg > 0);
}

/**
 * @brief Activate the afterburner.
 */
void pilot_afterburn (Pilot *p)
{
   double afb_mod;

   if (p == NULL)
      return;

   if (pilot_isFlag(p, PILOT_HYP_PREP) || pilot_isFlag(p, PILOT_HYPERSPACE) ||
         pilot_isFlag(p, PILOT_LANDING) || pilot_isFlag(p, PILOT_TAKEOFF) ||
         pilot_isDisabled(p) || pilot_isFlag(p, PILOT_COOLDOWN))
      return;

   /* Not under manual control if is player. */
   if (pilot_isFlag( p, PILOT_MANUAL_CONTROL ) && pilot_isFlag( p, PILOT_PLAYER ))
      return;

   /** @todo fancy effect? */
   if (p->afterburner == NULL)
      return;

   /* The afterburner only works if its efficiency is high enough. */
   if (pilot_heatEfficiencyMod( p->afterburner->heat_T,
         p->afterburner->outfit->u.afb.heat_base,
         p->afterburner->outfit->u.afb.heat_cap ) < 0.3)
      return;

   if (p->afterburner->state == PILOT_OUTFIT_OFF) {
      p->afterburner->state  = PILOT_OUTFIT_ON;
      p->afterburner->stimer = outfit_duration( p->afterburner->outfit );
      pilot_setFlag(p,PILOT_AFTERBURNER);
      pilot_calcStats( p );

      /* @todo Make this part of a more dynamic activated outfit sound system. */
      sound_playPos(p->afterburner->outfit->u.afb.sound_on,
            p->solid->pos.x, p->solid->pos.y, p->solid->vel.x, p->solid->vel.y);
   }

   if (pilot_isPlayer(p)) {
      afb_mod = MIN( 1., player.p->afterburner->outfit->u.afb.mass_limit / player.p->solid->mass );
      spfx_shake( afb_mod * player.p->afterburner->outfit->u.afb.rumble * SHAKE_MAX );
   }
}


/**
 * @brief Deactivates the afterburner.
 */
void pilot_afterburnOver (Pilot *p)
{
   if (p == NULL)
      return;
   if (p->afterburner == NULL)
      return;

   if (p->afterburner->state == PILOT_OUTFIT_ON) {
      p->afterburner->state  = PILOT_OUTFIT_OFF;
      pilot_rmFlag(p,PILOT_AFTERBURNER);
      pilot_calcStats( p );

      /* @todo Make this part of a more dynamic activated outfit sound system. */
      sound_playPos(p->afterburner->outfit->u.afb.sound_off,
            p->solid->pos.x, p->solid->pos.y, p->solid->vel.x, p->solid->vel.y);
   }
}
