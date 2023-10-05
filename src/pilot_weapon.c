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
/** @cond */
#include "naev.h"
/** @endcond */

#include "array.h"
#include "escort.h"
#include "log.h"
#include "nxml.h"
#include "nlua_pilot.h"
#include "nlua_pilotoutfit.h"
#include "pilot.h"
#include "player.h"
#include "spfx.h"
#include "weapon.h"

/*
 * Prototypes.
 */
static void pilot_weapSetUpdateOutfits( Pilot* p, PilotWeaponSet *ws );
static int pilot_weapSetFire( Pilot *p, PilotWeaponSet *ws, int level );
static int pilot_shootWeaponSetOutfit( Pilot* p, PilotWeaponSet *ws, const Outfit *o, int level, const Target *target, double time, int aim );
static void pilot_weapSetUpdateRange( const Pilot *p, PilotWeaponSet *ws );
unsigned int pilot_weaponSetShootStop( Pilot* p, PilotWeaponSet *ws, int level );

/**
 * @brief Gets a weapon set from id.

 *    @param p Pilot to get weapon set from.
 *    @param id ID of the weapon set.
 *    @return The weapon set matching id.
 */
PilotWeaponSet* pilot_weapSet( Pilot* p, int id )
{
   return &p->weapon_sets[ id ];
}

/**
 * @brief Fires a weapon set.
 *
 *    @param p Pilot firing weaponsets.
 *    @param ws Weapon set to fire.
 *    @param level Level of the firing weapon set.
 *    @return Number of weapons shot.
 */
static int pilot_weapSetFire( Pilot *p, PilotWeaponSet *ws, int level )
{
   int ret = 0;
   int isstealth = pilot_isFlag( p, PILOT_STEALTH );
   int target_set = 0;
   double time;
   Target wt;

   for (int i=0; i<array_size(ws->slots); i++) {
      PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];
      const Outfit *o = pos->outfit;

      /* Ignore NULL outfits. */
      if (o == NULL)
         continue;

      /* Only "active" outfits. */
      if ((level != -1) && (ws->slots[i].level != level))
         continue;

      /* Only weapons, TODO probably better check. */
      if (pos->sslot->slot.type!=OUTFIT_SLOT_WEAPON)
         continue;

      /* Only run once for each weapon type in the group if not volley. */
      if (!ws->volley) {
         int s = 0;
         for (int j=0; j<i; j++) {
            /* Only active outfits. */
            if ((level != -1) && (ws->slots[j].level != level))
               continue;
            /* Found a match. */
            if (p->outfits[ ws->slots[j].slotid ]->outfit == o) {
               s = 1;
               break;
            }
         }
         if (s!=0)
            continue;
      }

      /* Only "locked on" outfits. */
      if (outfit_isSeeker(o) && (pos->u.ammo.lockon_timer > 0.))
         continue;

      /* Lazy target setting. */
      if (!target_set) {
         pilot_weaponTarget( p, &wt );
         target_set = 1;
      }
      time = weapon_targetFlyTime( o, p, &wt );

      /* Only "inrange" outfits. */
      if (ws->inrange && outfit_duration(o) < time)
         continue;

      /* Shoot the weapon of the weaponset. */
      if (ws->volley)
         ret += pilot_shootWeapon( p, pos, &wt, time, !ws->manual );
      else
         ret += pilot_shootWeaponSetOutfit( p, ws, o, level, &wt, time, !ws->manual );
   }

   /* Destealth when attacking. */
   if (isstealth && (ret>0))
      pilot_destealth( p );

   /* Trigger onshoot after stealth gets broken. */
   if (ret > 0)
      pilot_outfitLOnshoot( p );

   return ret;
}

/**
 * @brief Useful function for AI, clears activeness of all weapon sets.
 */
void pilot_weapSetAIClear( Pilot* p )
{
   for (int i=0; i<PILOT_WEAPON_SETS; i++) {
      PilotWeaponSet *ws = &p->weapon_sets[i];
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
   int l, on, n;
   int isstealth = pilot_isFlag( p, PILOT_STEALTH );
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   /* Case no outfits. */
   if (ws->slots == NULL)
      return;

   /* Handle fire groups. */
   switch (ws->type) {
      case WEAPSET_TYPE_SWITCH:
         /* On press just change active weapon set to whatever is available. */
         if ((type > 0) && (array_size(ws->slots)>0)) {
            if (id != p->active_set)
               pilot_weapSetUpdateOutfits( p, ws );
            p->active_set = id;
         }
         break;

      case WEAPSET_TYPE_TOGGLE:
         /* The behaviour here is more complex. What we do is consider a group
          * to be entirely off if not all outfits are either on or cooling down.
          *  In the case it's deemed to be off, all outfits that are off get turned
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
         for (int i=0; i<l; i++) {
            PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];
            if (pos->state == PILOT_OUTFIT_OFF) {
               on = 0;
               break;
            }
         }

         /* Clear change variables. */
         n = 0;
         pilotoutfit_modified = 0;

         /* Turn them off. */
         if (on) {
            ws->active = 0;

            /* Weapons are weird still. */
            n += pilot_weaponSetShootStop( p, ws, -1 );

            for (int i=0; i<l; i++) {
               PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];
               if (pos->state != PILOT_OUTFIT_ON)
                  continue;

               n += pilot_outfitOff( p, pos );
            }
         }
         /* Turn them on. */
         else {
            ws->active = 1;

            /* Weapons are weird still :/ */
            n += pilot_weapSetFire( p, ws, -1 );

            for (int i=0; i<l; i++) {
               PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];
               if (pos->state != PILOT_OUTFIT_OFF)
                  continue;

               n += pilot_outfitOn( p, pos );
            }
         }
         /* Must recalculate stats. */
         if ((n > 0) || pilotoutfit_modified) {
            /* pilot_destealth should run calcStats already. */
            if (isstealth && (type>0))
               pilot_destealth( p );
            else
               pilot_calcStats( p );
         }
         break;

      case WEAPSET_TYPE_HOLD:
         /* Activation philosophy here is to turn on while pressed and off
          * when it's not held anymore. */

         /* Must not be disabled or cooling down. */
         if ((pilot_isDisabled(p)) || (pilot_isFlag(p, PILOT_COOLDOWN)))
            return;

         /* Clear change variables. */
         l  = array_size(ws->slots);
         n = 0;
         pilotoutfit_modified = 0;

         if (type > 0) {
            ws->active = 1;

            /* Weapons are weird still :/ */
            n += pilot_weapSetFire( p, ws, -1 );

            /* Turn on outfits. */
            for (int i=0; i<l; i++) {
               PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];
               if (pos->state != PILOT_OUTFIT_OFF)
                  continue;

               n += pilot_outfitOn( p, pos );
            }
         }
         else if (type < 0) {
            ws->active = 0;
            n += pilot_weaponSetShootStop( p, ws, -1 ); /* De-activate weapon set. */

            /* Weapons are weird still. */
            pilot_weaponSetShootStop( p, ws, -1 );

            /* Turn off outfits. */
            for (int i=0; i<l; i++) {
               PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];
               if (pos->state != PILOT_OUTFIT_ON)
                  continue;

               n += pilot_outfitOff( p, pos );
            }
         }

         /* Must recalculate stats. */
         if ((n > 0) || pilotoutfit_modified) {
            /* pilot_destealth should run calcStats already. */
            if (isstealth && (type>0))
               pilot_destealth( p );
            else
               pilot_calcStats( p );
         }
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
   if (pilot_isFlag( p, PILOT_HYP_BEGIN))
      return;

   for (int i=0; i<PILOT_WEAPON_SETS; i++) {
      PilotWeaponSet *ws = &p->weapon_sets[i];
      if (ws->slots == NULL)
         continue;

      /* Weapons must get "fired" every turn. */
      if (ws->active)
         pilot_weapSetFire( p, ws, -1 );
   }
}

/**
 * @brief Updates the outfits with their current weapon set level.
 */
static void pilot_weapSetUpdateOutfits( Pilot* p, PilotWeaponSet *ws )
{
   for (int i=0; i<array_size(p->outfits); i++)
      p->outfits[i]->level = -1;

   for (int i=0; i<array_size(ws->slots); i++)
      p->outfits[ ws->slots[i].slotid ]->level = ws->slots[i].level;
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
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   return ws->type;
}

/**
 * @brief Changes the weapon sets mode.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param type The mode (a WEAPSET_TYPE constant).
 */
void pilot_weapSetType( Pilot* p, int id, WeaponSetType type )
{
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   ws->type = type;
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
   PilotWeaponSet *ws = pilot_weapSet(p,id);
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
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   ws->inrange = inrange;
}

/**
 * @brief Checks the current weapon set manual property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set to check.
 *    @return The manual mode of the weapon set.
 */
int pilot_weapSetManualCheck( Pilot *p, int id )
{
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   return ws->manual;
}

/**
 * @brief Changes the weapon set manual property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param manual Whether or not to have manual aiming.
 */
void pilot_weapSetManual( Pilot* p, int id, int manual )
{
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   ws->manual = manual;
}

/**
 * @brief Checks the current weapon set volley property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set to check.
 *    @return The volley mode of the weapon set.
 */
int pilot_weapSetVolleyCheck( Pilot *p, int id )
{
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   return ws->volley;
}

/**
 * @brief Changes the weapon set volley property.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param volley Whether or not to have volley aiming.
 */
void pilot_weapSetVolley( Pilot* p, int id, int volley )
{
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   ws->volley = volley;
}

/**
 * @brief Gets the name of a weapon set.
 */
const char *pilot_weapSetName( Pilot* p, int id )
{
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   if (array_size(ws->slots)==0)
      return _("Unused");
   switch (ws->type) {
      case WEAPSET_TYPE_SWITCH: return _("Weapons - Switch");  break;
      case WEAPSET_TYPE_TOGGLE: return _("Outfits - Toggle"); break;
      case WEAPSET_TYPE_HOLD: return _("Outfits - Hold"); break;
   }
   return NULL;
}

/**
 * @brief Removes slots by type from the weapon set.
 */
void pilot_weapSetRmSlot( Pilot *p, int id, OutfitSlotType type )
{
   int n, l;
   PilotWeaponSet *ws;

   /* We must clean up the slots. */
   n  = 0; /* Number to remove. */
   ws = pilot_weapSet(p,id);
   if (ws->slots == NULL)
      return;
   l  = array_size(ws->slots);
   for (int i=0; i<l; i++) {
      PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];

      if (pos->sslot->slot.type != type)
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
   PilotWeaponSetOutfit *slot;
   const Outfit *oo;
   double r;
   PilotWeaponSet *ws = pilot_weapSet(p,id);

   /* Make sure outfit is valid. */
   oo = o->outfit;
   if (oo == NULL)
      return;

   /* Make sure outfit type is weapon (or usable). */
   if (!pilot_slotIsToggleable(o))
      return;

   /* Create if needed. */
   if (ws->slots == NULL)
      ws->slots = array_create( PilotWeaponSetOutfit );

   /* Check if already there. */
   for (int i=0; i<array_size(ws->slots); i++) {
      if (ws->slots[i].slotid != o->id)
         continue;

      ws->slots[i].level = level;

      /* Update if needed. */
      if (id == p->active_set)
         pilot_weapSetUpdateOutfits( p, ws );
      return;
   }

   /* Add it. */
   slot        = &array_grow( &ws->slots );
   slot->level = level;
   slot->slotid= o->id;
   r           = outfit_range(oo);
   if (r > 0)
      slot->range2 = pow2(r);

   /* Updated cached weapset. */
   o->weapset = -1;
   for (int j=0; j<PILOT_WEAPON_SETS; j++) {
      if (pilot_weapSetCheck(p, j, o) != -1) {
         o->weapset = j;
         break;
      }
   }

   /* Update range. */
   pilot_weapSetUpdateRange( p, ws );

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
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   for (int i=0; i<array_size(ws->slots); i++) {
      if (ws->slots[i].slotid != o->id)
         continue;

      array_erase( &ws->slots, &ws->slots[i], &ws->slots[i+1] );

      /* Update range. */
      pilot_weapSetUpdateRange( p, ws );

      /* Update if needed. */
      if (id == p->active_set)
         pilot_weapSetUpdateOutfits( p, ws );

      /* Updated cached weapset. */
      o->weapset = -1;
      for (int j=0; j<PILOT_WEAPON_SETS; j++) {
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
int pilot_weapSetCheck( Pilot* p, int id, const PilotOutfitSlot *o )
{
   const PilotWeaponSet *ws = pilot_weapSet(p,id);
   for (int i=0; i<array_size(ws->slots); i++)
      if (ws->slots[i].slotid == o->id)
         return ws->slots[i].level;

   /* Not found. */
   return -1;
}

/**
 * @brief Update the weapon sets given pilot stat changes.
 *
 *    @param p Pilot to update.
 */
void pilot_weapSetUpdateStats( Pilot *p )
{
   for (int i=0; i<PILOT_WEAPON_SETS; i++)
      pilot_weapSetUpdateRange( p, &p->weapon_sets[i] );
}

/**
 * @brief Updates the weapon range for a pilot weapon set.
 *
 *    @param p Pilot whos weapon set is being updated.
 *    @param ws Weapon Set to update range for.
 */
static void pilot_weapSetUpdateRange( const Pilot *p, PilotWeaponSet *ws )
{
   int lev;
   double range, speed;
   double range_accum[PILOT_WEAPSET_MAX_LEVELS];
   int range_num[PILOT_WEAPSET_MAX_LEVELS];
   double speed_accum[PILOT_WEAPSET_MAX_LEVELS];
   int speed_num[PILOT_WEAPSET_MAX_LEVELS];

   /* Calculate ranges. */
   for (int i=0; i<PILOT_WEAPSET_MAX_LEVELS; i++) {
      range_accum[i] = 0.;
      range_num[i]   = 0;
      speed_accum[i] = 0.;
      speed_num[i]   = 0;
   }
   for (int i=0; i<array_size(ws->slots); i++) {
      PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];
      if (pos->outfit == NULL)
         continue;

      /* Get level. */
      lev = ws->slots[i].level;
      if (lev >= PILOT_WEAPSET_MAX_LEVELS)
         continue;

      /* Empty Launchers aren't valid */
      if (outfit_isLauncher(pos->outfit) && (pos->u.ammo.quantity <= 0))
         continue;

      /* Get range. */
      range = outfit_range(pos->outfit);
      if (outfit_isLauncher(pos->outfit))
         range *= p->stats.launch_range;
      if (range >= 0.) {
         /* Calculate. */
         range_accum[ lev ] += range;
         range_num[ lev ]++;
      }

      /* Get speed. */
      speed = outfit_speed(pos->outfit);
      if (speed >= 0.) {
         /* Calculate. */
         speed_accum[ lev ] += speed;
         speed_num[ lev ]++;
      }
   }

   /* Postprocess. */
   for (int i=0; i<PILOT_WEAPSET_MAX_LEVELS; i++) {
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
 *    @param level Level of the weapons to get the range of (-1 for all).
 */
double pilot_weapSetRange( Pilot* p, int id, int level )
{
   double range;
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   if (level < 0) {
      range = 0.;
      for (int i=0; i<PILOT_WEAPSET_MAX_LEVELS; i++)
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
 *    @param level Level of the weapons to get the speed of (-1 for all).
 */
double pilot_weapSetSpeed( Pilot* p, int id, int level )
{
   double speed;
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   if (level < 0) {
      speed = 0.;
      for (int i=0; i<PILOT_WEAPSET_MAX_LEVELS; i++)
         speed += ws->speed[i];
   }
   else
      speed = ws->speed[ level ];

   return speed;
}

/**
 * @brief Gets the ammo of the current pilot weapon set.
 *
 *    @param p Pilot to get the speed of.
 *    @param id ID of weapon set to get the speed of.
 *    @param level Level of the weapons to get the speed of (-1 for all).
 */
double pilot_weapSetAmmo( Pilot* p, int id, int level )
{
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   double ammo = 0.;
   int nammo = 0;
   for (int i=0; i<array_size(ws->slots); i++) {
      int amount;
      PilotOutfitSlot *s = p->outfits[ ws->slots[i].slotid ];
      if ((level >= 0) && (ws->slots[i].level != level))
         continue;
      amount = pilot_maxAmmoO( p, s->outfit );
      if (amount > 0) {
         ammo += (double)s->u.ammo.quantity / (double)amount;
         nammo++;
      }
   }
   return (nammo==0) ? 0. : ammo / (double)nammo;
}

/**
 * @brief Cleans up a weapon set.
 *
 *    @param p Pilot who owns the weapon set.
 *    @param id ID of the weapon set to clean up.
 */
void pilot_weapSetCleanup( Pilot* p, int id )
{
   PilotWeaponSet *ws = pilot_weapSet(p,id);

   array_free( ws->slots );
   ws->slots = NULL;

   /* Update range. */
   pilot_weapSetUpdateRange( p, ws );
}

/**
 * @brief Frees a pilot's weapon sets.
 */
void pilot_weapSetFree( Pilot* p )
{
   for (int i=0; i<PILOT_WEAPON_SETS; i++)
      pilot_weapSetCleanup( p, i );
}

/**
 * @brief Lists the items in a pilot weapon set.
 *
 *    @param p Pilot who owns the weapon set.
 *    @param id ID of the weapon set.
 *    @return The array (array.h) of pilot weaponset outfits.
 */
PilotWeaponSetOutfit* pilot_weapSetList( Pilot* p, int id )
{
   return pilot_weapSet( p, id )->slots;
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
   PilotWeaponSet *ws = pilot_weapSet( p, p->active_set );

   /* Fire weapons. */
   if (ws->type == WEAPSET_TYPE_SWITCH) { /* Must be a change set or a weaponset. */
      int ret = pilot_weapSetFire( p, ws, level );

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
   PilotWeaponSet *ws = pilot_weapSet( p, p->active_set );

   /* Stop and see if must recalculate. */
   if (pilot_weaponSetShootStop( p, ws, level ))
      pilot_calcStats( p );
}

/**
 * @brief Have pilot stop shooting a given weaponset.
 *
 * Only really deals with beam weapons.
 *
 *    @param p Pilot that was shooting.
 *    @param level Level of the shot.
 *    @param ws Weapon Set to stop shooting
 *    @return 1 if some outfit has been deactivated
 */
unsigned int pilot_weaponSetShootStop( Pilot* p, PilotWeaponSet *ws, int level )
{
   /* Stop all beams. */
   int recalc = 0;
   for (int i=0; i<array_size(ws->slots); i++) {
      PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];

      /* Must have associated outfit. */
      if (pos->outfit == NULL)
         continue;

      /* Must match level. */
      if ((level != -1) && (ws->slots[i].level != level))
         continue;

      /* Only handle beams. */
      if (!outfit_isBeam(pos->outfit)) {
         /* Turn off the state. */
         if (outfit_isMod( pos->outfit )) {
            pos->state = PILOT_OUTFIT_OFF;
            recalc = 1;
         }
         continue;
      }

      /* Stop beam. */
      if (pos->u.beamid > 0) {
         /* Enforce minimum duration if set. */
         if (pos->outfit->u.bem.min_duration > 0.) {

            pos->stimer = pos->outfit->u.bem.min_duration -
                  (pos->outfit->u.bem.duration - pos->timer);

            if (pos->stimer > 0.)
               continue;
         }

         beam_end( pos->u.beamid );
         pilot_stopBeam(p, pos);
      }
   }

   return recalc;
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

   /* Lua test to stop beam. */
   if ((w->outfit->lua_ontoggle != LUA_NOREF) &&
         !pilot_outfitLOntoggle( p, w, 0 ))
      return;

   /* Calculate rate modifier. */
   pilot_getRateMod( &rate_mod, &energy_mod, p, w->outfit );

   /* Beam duration used. Compensate for the fact it's duration might have
    * been shortened by heat. */
   used = w->outfit->u.bem.duration - w->timer*(1.-pilot_heatAccuracyMod(w->heat_T));

   w->timer = rate_mod * (used / w->outfit->u.bem.duration) * outfit_delay( w->outfit );
   w->u.beamid = 0;
   w->state = PILOT_OUTFIT_OFF;
}

/**
 * @brief Computes an estimation of ammo flying time
 *
 *    @param o the weapon to shoot.
 *    @param parent Parent of the weapon.
 *    @param pos Target's position.
 *    @param vel Target's velocity.
 */
double pilot_weapFlyTime( const Outfit *o, const Pilot *parent, const vec2 *pos, const vec2 *vel )
{
   vec2 approach_vector, relative_location, orthoradial_vector;
   double speed, radial_speed, orthoradial_speed, dist, t;

   dist = vec2_dist( &parent->solid.pos, pos );

   /* Beam weapons */
   if (outfit_isBeam(o)) {
      if (dist > o->u.bem.range)
         return INFINITY;
      return 0.;
   }

   /* A bay doesn't have range issues */
   if (outfit_isFighterBay(o))
      return 0.;

   /* Missiles use absolute velocity while bolts and unguided rockets use relative vel */
   if (outfit_isLauncher(o) && o->u.lau.ai != AMMO_AI_UNGUIDED)
      vec2_cset( &approach_vector, - vel->x, - vel->y );
   else
      vec2_cset( &approach_vector, VX(parent->solid.vel) - vel->x,
            VY(parent->solid.vel) - vel->y );

   speed = outfit_speed(o);

   /* Get the vector : shooter -> target */
   vec2_cset( &relative_location, pos->x - VX(parent->solid.pos),
         pos->y - VY(parent->solid.pos) );

   /* Get the orthogonal vector */
   vec2_cset(&orthoradial_vector, VY(parent->solid.pos) - pos->y,
         pos->x -  VX(parent->solid.pos) );

   radial_speed = vec2_dot( &approach_vector, &relative_location );
   radial_speed = radial_speed / VMOD(relative_location);

   orthoradial_speed = vec2_dot(&approach_vector, &orthoradial_vector);
   orthoradial_speed = orthoradial_speed / VMOD(relative_location);

   if ( ((speed*speed - VMOD(approach_vector)*VMOD(approach_vector)) != 0) && (speed*speed - orthoradial_speed*orthoradial_speed) > 0)
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
 * @brief Gets the weapon target of a pilot.
 *
 *    @param p Pilot to get weapon targot of.
 *    @param[out] wt Weapon target structure set up.
 *    @return The pilot pointer if applicable.
 */
Pilot *pilot_weaponTarget( Pilot *p, Target *wt )
{
   Pilot *pt = pilot_getTarget( p );
   if (pt != NULL) {
      wt->type = TARGET_PILOT;
      wt->u.id = pt->id;
      return pt;
   }
   else if (p->nav_asteroid != -1) {
      wt->type = TARGET_ASTEROID;
      wt->u.ast.anchor = p->nav_anchor;
      wt->u.ast.asteroid = p->nav_asteroid;
      return NULL;
   }
   wt->type = TARGET_NONE;
   return NULL;
}

/**
 * @brief Calculates and shoots the appropriate weapons in a weapon set matching an outfit.
 */
static int pilot_shootWeaponSetOutfit( Pilot* p, PilotWeaponSet *ws, const Outfit *o, int level, const Target *target, double time, int aim )
{
   int ret;
   int is_launcher, is_bay;
   double rate_mod, energy_mod;
   int maxp, minh;
   double q, maxt;

   /* Store number of shots. */
   ret = 0;

   /** @TODO Make beams not fire all at once. */
   if (outfit_isBeam(o)) {
      for (int i=0; i<array_size(ws->slots); i++) {
         PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];
         if (pos->outfit == o && (level == -1 || level == ws->slots[i].level)) {
            ret += pilot_shootWeapon( p, pos, target, 0., aim );
            pos->inrange = ws->inrange; /* State if the weapon has to be turn off when out of range. */
         }
      }
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
   for (int i=0; i<array_size(ws->slots); i++) {
      PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];

      /* Only matching outfits. */
      if (pos->outfit != o)
         continue;

      /* Only match levels. */
      if ((level != -1) && (ws->slots[i].level != level))
         continue;

      /* Launcher only counts with ammo. */
      if ((is_launcher || is_bay) && (pos->u.ammo.quantity <= 0))
         continue;

      /* Get coolest that can fire. */
      if (pos->timer <= 0.) {
         if (is_launcher) {
            if ((minh < 0) || (p->outfits[ ws->slots[minh].slotid ]->u.ammo.quantity < pos->u.ammo.quantity))
               minh = i;
         }
         else {
            if ((minh < 0) || (p->outfits[ ws->slots[minh].slotid ]->heat_T > pos->heat_T))
               minh = i;
         }
      }

      /* Save some stuff. */
      if ((maxp < 0) || (pos->timer > maxt)) {
         maxp = i;
         maxt = pos->timer;
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
   ret += pilot_shootWeapon( p, p->outfits[ ws->slots[minh].slotid ], target, time, aim );

   return ret;
}

/**
 * @brief Actually handles the shooting, how often the player.p can shoot and such.
 *
 *    @param p Pilot that is shooting.
 *    @param w Pilot's outfit to shoot.
 *    @param target Target shooting at.
 *    @param time Expected flight time.
 *    @param aim Whether or not to aim, if negative, indicates that it is automatically shot.
 *    @return 0 if nothing was shot and 1 if something was shot.
 */
int pilot_shootWeapon( Pilot *p, PilotOutfitSlot *w, const Target *target, double time, int aim )
{
   vec2 vp, vv;
   double rate_mod, energy_mod;
   double energy;

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

   /* Modify velocity to take into account the rotation. */
   vec2_cset( &vv, p->solid.vel.x - vp.y*p->solid.dir_vel,
         p->solid.vel.y + vp.x*p->solid.dir_vel );

   /* Get absolute weapon mount position. */
   vp.x += p->solid.pos.x;
   vp.y += p->solid.pos.y;

   /* Regular bolt weapons. */
   if (outfit_isBolt(w->outfit)) {
      /* enough energy? */
      if (outfit_energy(w->outfit)*energy_mod > p->energy)
         return 0;

      /* Lua test. */
      if ((aim >= 0) && (w->outfit->lua_ontoggle != LUA_NOREF) &&
            !pilot_outfitLOntoggle( p, w, 1 ))
         return 0;

      energy      = outfit_energy(w->outfit)*energy_mod;
      p->energy  -= energy;
      pilot_heatAddSlot( p, w );
      if (!outfit_isProp( w->outfit, OUTFIT_PROP_SHOOT_DRY )) {
         for (int i=0; i<w->outfit->u.blt.shots; i++) {
            weapon_add( w, NULL, p->solid.dir,
                  &vp, &vv, p, target, time, aim );
         }
      }
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

      /* Lua test. */
      if ((aim>=0) && (w->outfit->lua_ontoggle != LUA_NOREF) &&
            !pilot_outfitLOntoggle( p, w, 1 ))
         return 0;

      /** @todo Handle warmup stage. */
      w->state = PILOT_OUTFIT_ON;
      if (!outfit_isProp( w->outfit, OUTFIT_PROP_SHOOT_DRY )) {
         w->u.beamid = beam_start( w, p->solid.dir,
               &vp, &p->solid.vel, p, target, aim );
      }

      w->timer = w->outfit->u.bem.duration;

      return 1; /* Return early due to custom timer logic. */
   }

   /*
    * Missile launchers
    */
   else if (outfit_isLauncher(w->outfit)) {
      Target wt;
      /* Must have ammo left. */
      if (w->u.ammo.quantity <= 0)
         return 0;

      /* enough energy? */
      if (outfit_energy(w->outfit)*energy_mod > p->energy)
         return 0;

      /* Shooter can't be the target - safety check for the player.p */
      if (target == NULL) {
         pilot_weaponTarget( p, &wt );
         target = &wt;
      }
      if ((w->outfit->u.lau.ai != AMMO_AI_UNGUIDED) && (target->type!=TARGET_PILOT))
         return 0;

      /* Lua test. */
      if ((aim>=0) && (w->outfit->lua_ontoggle != LUA_NOREF) &&
            !pilot_outfitLOntoggle( p, w, 1 ))
         return 0;

      energy      = outfit_energy(w->outfit)*energy_mod;
      p->energy  -= energy;
      pilot_heatAddSlot( p, w );
      if (!outfit_isProp( w->outfit, OUTFIT_PROP_SHOOT_DRY )) {
         for (int i=0; i<w->outfit->u.lau.shots; i++)
            weapon_add( w, NULL, p->solid.dir,
                  &vp, &vv, p, target, time, aim );
      }

      pilot_rmAmmo( p, w, 1 );

      /* Make the AI aware a seeker has been shot */
      if (outfit_isSeeker(w->outfit))
         p->shoot_indicator = 1;

      /* If last ammo was shot, update the range */
      if (w->u.ammo.quantity <= 0) {
         for (int j=0; j<PILOT_WEAPON_SETS; j++)
            pilot_weapSetUpdateRange( p, &p->weapon_sets[j] );
      }
   }

   /*
    * Fighter bays.
    */
   else if (outfit_isFighterBay(w->outfit)) {
      int dockslot = -1;

      /* Must have ammo left. */
      if (w->u.ammo.quantity <= 0)
         return 0;

      /* Lua test. */
      if ((aim>=0) && (w->outfit->lua_ontoggle != LUA_NOREF) &&
            !pilot_outfitLOntoggle( p, w, 1 ))
         return 0;

      /* Get index of outfit slot */
      for (int j=0; j<array_size(p->outfits); j++) {
         if (p->outfits[j] == w)
            dockslot = j;
      }

      /* Create the escort. */
      if (!outfit_isProp( w->outfit, OUTFIT_PROP_SHOOT_DRY ))
         escort_create( p, w->outfit->u.bay.ship,
               &vp, &p->solid.vel, p->solid.dir, ESCORT_TYPE_BAY, 1, dockslot );

      w->u.ammo.quantity -= 1; /* we just shot it */
      p->mass_outfit     -= w->outfit->u.bay.ship_mass;
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
 *    @param o Pilot's outfit.
 */
void pilot_getRateMod( double *rate_mod, double* energy_mod,
      const Pilot* p, const Outfit *o )
{
   switch (o->type) {
      case OUTFIT_TYPE_BOLT:
      case OUTFIT_TYPE_BEAM:
         *rate_mod   = 1. / p->stats.fwd_firerate; /* Invert. */
         *energy_mod = p->stats.fwd_energy;
         break;
      case OUTFIT_TYPE_TURRET_BOLT:
      case OUTFIT_TYPE_TURRET_BEAM:
         *rate_mod   = 1. / p->stats.tur_firerate; /* Invert. */
         *energy_mod = p->stats.tur_energy;
         break;

      case OUTFIT_TYPE_LAUNCHER:
      case OUTFIT_TYPE_TURRET_LAUNCHER:
         *rate_mod   = 1. / p->stats.launch_rate;
         *energy_mod = 1.;
         break;

      case OUTFIT_TYPE_FIGHTER_BAY:
         *rate_mod   = 1. / p->stats.fbay_rate;
         *energy_mod = 1.;
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
   for (int i=0; i<PILOT_WEAPON_SETS; i++) {
      PilotWeaponSet *ws = pilot_weapSet( p, i );
      array_erase( &ws->slots, array_begin(ws->slots), array_end(ws->slots) );
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
   int level, id;

   /* Clear weapons. */
   pilot_weaponClear( p );

   /* Set modes. */
   pilot_weapSetType( p, 0, WEAPSET_TYPE_SWITCH );
   pilot_weapSetType( p, 1, WEAPSET_TYPE_SWITCH );
   pilot_weapSetType( p, 2, WEAPSET_TYPE_SWITCH );
   pilot_weapSetType( p, 3, WEAPSET_TYPE_SWITCH );
   pilot_weapSetType( p, 4, WEAPSET_TYPE_TOGGLE );
   pilot_weapSetType( p, 5, WEAPSET_TYPE_TOGGLE );
   pilot_weapSetType( p, 6, WEAPSET_TYPE_HOLD );
   pilot_weapSetType( p, 7, WEAPSET_TYPE_HOLD );
   pilot_weapSetType( p, 8, WEAPSET_TYPE_HOLD );
   pilot_weapSetType( p, 9, WEAPSET_TYPE_TOGGLE );

   /* All should be inrange. */
   if (!pilot_isPlayer(p))
      for (int i=0; i<PILOT_WEAPON_SETS; i++) {
         pilot_weapSetInrange( p, i, 1 );
         /* Update range and speed (at 0)*/
         pilot_weapSetUpdateRange( p, &p->weapon_sets[i] );
      }

   /* Iterate through all the outfits. */
   for (int i=0; i<array_size(p->outfits); i++) {
      PilotOutfitSlot *slot = p->outfits[i];
      const Outfit *o       = slot->outfit;

      /* Must be non-empty, and a weapon or active outfit. */
      if ((o == NULL) || !outfit_isActive(o)) {
         slot->level   = -1; /* Clear level. */
         slot->weapset = -1;
         continue;
      }

      /* Set level based on secondary flag. */
      level  = outfit_isSecondary(o);

      /* Manually defined group preempts others. */
      if (o->group) {
         id    = o->group;
      }
      /* Bolts and beams. */
      else if (outfit_isBolt(o) || outfit_isBeam(o) ||
            (outfit_isLauncher(o) && !outfit_isSeeker(o))) {
         id    = outfit_isTurret(o) ? 2 : 1;
      }
      /* Seekers. */
      else if (outfit_isLauncher(o) && outfit_isSeeker(o)) {
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
      else if (id == 4) { /* Seekers */
         pilot_weapSetAdd( p, 0, slot, level ); /* Also get added to 'All'. */
         if (outfit_isTurret(o))
            pilot_weapSetAdd( p, 9, slot, level ); /* Also get added to 'Turreted Seekers'. */
      }
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
   if (p->weapon_sets[ p->active_set ].type == WEAPSET_TYPE_SWITCH) {
      /* Update active weapon set. */
      pilot_weapSetUpdateOutfits( p, &p->weapon_sets[ p->active_set ] );
      return;
   }

   /* Find first fire group. */
   for (i=0; i<PILOT_WEAPON_SETS; i++)
      if (p->weapon_sets[i].type == WEAPSET_TYPE_SWITCH)
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
 * @brief Sets the weapon set as safe.
 *
 *    @param p Pilot to set weapons as safe.
 */
void pilot_weaponSafe( Pilot *p )
{
   for (int j=0; j<PILOT_WEAPON_SETS; j++) {
      PilotWeaponSet *ws = &p->weapon_sets[j];
      int l = array_size(ws->slots);
      int n = 0;
      for (int i=0; i<l; i++) {
         PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];
         if (pos->outfit != NULL)
            continue;

         /* Move down. */
         memmove( &ws->slots[i], &ws->slots[i+1], sizeof(PilotWeaponSetOutfit) * (l-i-1) );
         n++;
      }
      /* Remove surplus. */
      if (n > 0)
         array_erase( &ws->slots, &ws->slots[l-n], &ws->slots[l] );

      /* See if we must overwrite levels. */
      if ((ws->type == WEAPSET_TYPE_TOGGLE) ||
            (ws->type == WEAPSET_TYPE_HOLD))
         for (int i=0; i<array_size(ws->slots); i++)
            ws->slots[i].level = 0;

      /* Update range. */
      pilot_weapSetUpdateRange( p, ws );
   }
}

/**
 * @brief Disables a given active outfit.
 *
 * @param p Pilot whose outfit we are disabling.
 * @param o Outfit to disable.
 * @return Whether the outfit was actually disabled.
 */
int pilot_outfitOff( Pilot *p, PilotOutfitSlot *o )
{
   /* Must be equipped, not disabled, not cooling down. */
   if (o->outfit == NULL || (pilot_isDisabled(p)) || (pilot_isFlag(p, PILOT_COOLDOWN)))
      return 0;

   if (outfit_isAfterburner( o->outfit )) { /* Afterburners */
      if ((o->outfit->lua_ontoggle != LUA_NOREF) &&
            !pilot_outfitLOntoggle( p, o, 0 ))
         return 0;
      pilot_afterburnOver( p );
   }
   else if (outfit_isBeam( o->outfit )) {
      if ((o->outfit->lua_ontoggle != LUA_NOREF) &&
            !pilot_outfitLOntoggle( p, o, 0 ))
         return 0;
      /* Beams use stimer to represent minimum time until shutdown. */
      if (o->u.beamid>0) {
         beam_end( o->u.beamid );
         pilot_stopBeam(p, o);
      }
   }
   else if (!o->active)
      /* Case of a mod we can't toggle. */
      return 0;
   else if (o->outfit->lua_ontoggle != LUA_NOREF)
      /* TODO toggle Lua outfit. */
      return pilot_outfitLOntoggle( p, o, 0 );
   else {
      o->stimer = outfit_cooldown( o->outfit );
      o->state  = PILOT_OUTFIT_COOLDOWN;
   }

   return 1;
}

/**
 * @brief Enable a given active outfit.
 *
 * @param p Pilot whose outfit we are enabling.
 * @param o Outfit to enable.
 * @return Whether the outfit was actually enabled.
 */
int pilot_outfitOn( Pilot *p, PilotOutfitSlot *o )
{
   if (o->outfit == NULL)
      return 0;
   if (outfit_isAfterburner(o->outfit))
      pilot_afterburn( p );
   else if (o->outfit->lua_ontoggle != LUA_NOREF)
      /* TODO toggle Lua outfit. */
      return pilot_outfitLOntoggle( p, o, 1 );
   else {
      o->state  = PILOT_OUTFIT_ON;
      o->stimer = outfit_duration( o->outfit ) * p->stats.cooldown_mod;
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
   int nchg = 0;
   for (int i=0; i<array_size(p->outfits); i++) {
      PilotOutfitSlot *o = p->outfits[i];
      /* Picky about our outfits. */
      if (o->outfit == NULL)
         continue;
      if (o->state == PILOT_OUTFIT_ON)
         nchg += pilot_outfitOff( p, o );
   }
   return (nchg > 0);
}

/**
 * @brief Activate the afterburner.
 */
void pilot_afterburn( Pilot *p )
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
      pilot_destealth( p ); /* No afterburning stealth. */

      /* @todo Make this part of a more dynamic activated outfit sound system. */
      sound_playPos(p->afterburner->outfit->u.afb.sound_on,
            p->solid.pos.x, p->solid.pos.y, p->solid.vel.x, p->solid.vel.y);
   }

   if (pilot_isPlayer(p)) {
      afb_mod = MIN( 1., player.p->afterburner->outfit->u.afb.mass_limit / player.p->solid.mass );
      spfx_shake( afb_mod * player.p->afterburner->outfit->u.afb.rumble );
   }
}

/**
 * @brief Deactivates the afterburner.
 */
void pilot_afterburnOver( Pilot *p )
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
            p->solid.pos.x, p->solid.pos.y, p->solid.vel.x, p->solid.vel.y);
   }
}

/**
 * @brief Copies a weapon set over.
 */
void ws_copy( PilotWeaponSet dest[PILOT_WEAPON_SETS], const PilotWeaponSet src[PILOT_WEAPON_SETS] )
{
   ws_free( dest );
   memcpy( dest, src, sizeof(PilotWeaponSet)*PILOT_WEAPON_SETS );
   for (int i=0; i<PILOT_WEAPON_SETS; i++)
      dest[i].slots = array_copy( PilotWeaponSetOutfit, src[i].slots );
}

/**
 * @brief Frees a weapon set.
 */
void ws_free( PilotWeaponSet ws[PILOT_WEAPON_SETS] )
{
   for (int i=0; i<PILOT_WEAPON_SETS; i++) {
      array_free( ws[i].slots );
      ws[i].slots = NULL;
   }
}
