/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file pilot_weapon.c
 *
 * @brief Handles pilot weapon sets which server to manage and interface with
 * active outfits..
 *
 * The basic approach is a flag and sweep operation. Outfits are flagged based
 * on what weapon sets they belong to when weapon sets change or weapons are
 * fired. Afterwards, the weapons will be updated every iteration to fire or
 * change states as necessary.
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
static int pilot_shootWeaponSetOutfit( Pilot* p, const Outfit *o, const Target *target, double time, int aim );
static void pilot_weapSetUpdateRange( const Pilot *p, PilotWeaponSet *ws );

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
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   /* Case no outfits. */
   if (ws->slots == NULL)
      return;

   /* Handle fire groups. */
   switch (ws->type) {
      case WEAPSET_TYPE_SWITCH:
         /* On press just change active weapon set to whatever is available. */
         if ((type > 0) && (array_size(ws->slots)>0)) {
            p->active_set = id;
            pilot_weapSetUpdateOutfits( p, ws );
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

         /* Turn them off. */
         if (ws->active)
            ws->active = 0;
         /* Turn them on. */
         else
            ws->active = WEAPSET_ACTIVE_ALL;
         break;

      case WEAPSET_TYPE_HOLD:
         /* Activation philosophy here is to turn on while pressed and off
          * when it's not held anymore. */

         /* Must not be disabled or cooling down. */
         if ((pilot_isDisabled(p)) || (pilot_isFlag(p, PILOT_COOLDOWN)))
            return;

         /* Clear change variables. */
         if (type > 0)
            ws->active = WEAPSET_ACTIVE_ALL;
         else if (type < 0)
            ws->active = 0;
         break;
   }

   pilot_weapSetUpdateOutfitState( p );
}

/**
 * @brief Updates the local state of all the pilot's outfits based on the weapon sets.
 */
void pilot_weapSetUpdateOutfitState( Pilot* p )
{
   int n;

   /* First pass to remove all dynamic flags. */
   for (int i=0; i<array_size(p->outfits); i++) {
      PilotOutfitSlot *pos = p->outfits[i];
      pos->flags &= ~PILOTOUTFIT_DYNAMIC_FLAGS;
   }

   /* Now mark all the outfits as on or off. */
   for (int i=0; i<PILOT_WEAPON_SETS; i++) {
      PilotWeaponSet *ws = &p->weapon_sets[i];
      if (ws->slots == NULL)
         continue;

      /* Only care if on. */
      if (!ws->active)
         continue;

      /* Keep on toggling on. */
      for (int j=0; j<array_size(ws->slots); j++) {
         PilotOutfitSlot *pos = p->outfits[ ws->slots[j].slotid ];
         if (pos->outfit == NULL)
            continue;
         if (!((1<<ws->slots[j].level) & ws->active))
            continue;
         pos->flags |= PILOTOUTFIT_ISON;
         if (ws->volley)
            pos->flags |= PILOTOUTFIT_VOLLEY;
         if (ws->inrange)
            pos->flags |= PILOTOUTFIT_INRANGE;
         if (ws->manual)
            pos->flags |= PILOTOUTFIT_MANUAL;
      }
   }

   /* Last pass figures out what to do. */
   n = 0;
   pilotoutfit_modified = 0;
   for (int i=0; i<array_size(p->outfits); i++) {
      PilotOutfitSlot *pos = p->outfits[i];
      const Outfit *o = pos->outfit;
      if (o == NULL)
         continue;
      if (!(pos->flags & PILOTOUTFIT_ACTIVE))
         continue;
      /* Ignore outfits handled by Lua. */
      if (pos->flags & PILOTOUTFIT_ISON_LUA)
         continue;

      /* Se whether to turn on or off. */
      if (pos->flags & PILOTOUTFIT_ISON) {
         if (pos->state == PILOT_OUTFIT_OFF)
            n += pilot_outfitOn( p, pos );
      }
      else {
         if (pos->state == PILOT_OUTFIT_ON)
            n += pilot_outfitOff( p, pos );
      }
   }

   /* Now update stats and shit as necessary. */
   if ((n > 0) || pilotoutfit_modified) {
      /* pilot_destealth should run calcStats already. */
      if (pilot_isFlag(p,PILOT_STEALTH) && (n>0))
         pilot_destealth( p );
      else
         pilot_calcStats( p );
   }
}

/**
 * @brief Updates the pilot's weapon sets.
 *
 *    @param p Pilot to update.
 */
void pilot_weapSetUpdate( Pilot* p )
{
   int n, nweap, target_set;
   double time;
   Target wt;

   if (pilot_isFlag( p, PILOT_HYP_BEGIN))
      return;

   n = 0;
   nweap = 0;
   target_set = 0;
   pilotoutfit_modified = 0;
   for (int i=0; i<array_size(p->outfits); i++) {
      PilotOutfitSlot *pos = p->outfits[i];
      const Outfit *o = pos->outfit;
      int volley;
      if (o == NULL)
         continue;
      if (!(pos->flags & PILOTOUTFIT_ACTIVE))
         continue;
      /* Ignore outfits handled by Lua. */
      if (pos->flags & PILOTOUTFIT_ISON_LUA)
         continue;

      /* Turn on if off. */
      if (pos->flags & PILOTOUTFIT_ISON) {
         if (pos->state == PILOT_OUTFIT_OFF)
            n += pilot_outfitOn( p, pos );
      }
      else {
         if (pos->state == PILOT_OUTFIT_ON)
            n += pilot_outfitOff( p, pos );
      }

      /* Handle volley sets below. */
      if (!(pos->flags & PILOTOUTFIT_ISON))
         continue;
      if (pos->state!=PILOT_OUTFIT_ON)
         continue;
      if (!outfit_isWeapon(o))
         continue;

      /* @TODO Make beams not fire all at once. */
      volley = ((pos->flags & PILOTOUTFIT_VOLLEY) || outfit_isBeam(o));

      /* For non-volley mode we want to run once per outfit type. */
      if (!volley) {
         int s = 0;
         for (int j=0; j<i; j++) {
            PilotOutfitSlot *posj = p->outfits[j];
            if (posj->state!=PILOT_OUTFIT_ON)
               continue;
            /* Found a match. */
            if (posj->outfit == o) {
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

      /* Only "inrange" outfits.
       * XXX for simplicity we are using pilot position / velocity instead of mount point, which might be a bit off. */
      if ((pos->flags & PILOTOUTFIT_INRANGE) && !outfit_isFighterBay(o) && ((outfit_duration(o) * p->stats.launch_range < time) ||
               (!weapon_inArc( o, p, &wt, &p->solid.pos, &p->solid.vel, p->solid.dir, time))))
         continue;

      /* Shoot the weapon of the weaponset. */
      if (volley)
         nweap += pilot_shootWeapon( p, pos, &wt, time, !(pos->flags & PILOTOUTFIT_MANUAL) );
      else
         nweap += pilot_shootWeaponSetOutfit( p, o, &wt, time, !(pos->flags & PILOTOUTFIT_MANUAL) );
      n++;
   }

   /* Now update stats and shit as necessary. */
   if ((n > 0) || pilotoutfit_modified) {
      /* pilot_destealth should run calcStats already. */
      if (pilot_isFlag(p,PILOT_STEALTH) && (n>0))
         pilot_destealth( p );
      else
         pilot_calcStats( p );

      /* Firing stuff aborts active cooldown. */
      if (pilot_isFlag(p, PILOT_COOLDOWN) && (nweap>0))
         pilot_cooldownEnd(p, NULL);

      /* Trigger onshoot after stealth gets broken. */
      if (nweap > 0)
         pilot_outfitLOnshootany( p );
   }
}

/**
 * @brief Updates the outfits with their current weapon set level.
 */
static void pilot_weapSetUpdateOutfits( Pilot* p, PilotWeaponSet *ws )
{
   /* Make sure we have a valid active switched set. */
   const PilotWeaponSet *wsa = pilot_weapSet( p, p->active_set );
   if (wsa->type != WEAPSET_TYPE_SWITCH) {
      for (int i=0; i<PILOT_WEAPON_SETS; i++) {
         const PilotWeaponSet *wsi = pilot_weapSet( p, i );
         if (wsi->type == WEAPSET_TYPE_SWITCH) {
            p->active_set = i;
            wsa = pilot_weapSet( p, p->active_set );
            break;
         }
      }
   }

   /* Turn off switched sets. */
   for (int i=0; i<PILOT_WEAPON_SETS; i++) {
      PilotWeaponSet *wsi = pilot_weapSet( p, i );
      if (wsi->type != WEAPSET_TYPE_SWITCH)
         continue;
      wsi->active = 0;
   }

   /* Have to update slots potentially. */
   for (int i=0; i<array_size(p->outfits); i++) {
      PilotOutfitSlot *o = p->outfits[i];
      o->weapset = -1;
      for (int j=0; j<PILOT_WEAPON_SETS; j++) {
         if (pilot_weapSetCheck(p, j, o) != -1) {
            o->weapset = j;
            break;
         }
      }
   }

   /* Update range. */
   pilot_weapSetUpdateRange( p, ws );

   /* Just update levels of active weapon set.  */
   for (int i=0; i<array_size(p->outfits); i++)
      p->outfits[i]->level = -1;
   for (int i=0; i<array_size(wsa->slots); i++)
      p->outfits[ wsa->slots[i].slotid ]->level = wsa->slots[i].level;
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
   const PilotWeaponSet *ws = pilot_weapSet(p,id);
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
   ws->active = 0; /* Disable no matter what. */
   pilot_weapSetUpdateOutfits(p,ws);
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
   const PilotWeaponSet *ws = pilot_weapSet(p,id);
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
   const PilotWeaponSet *ws = pilot_weapSet(p,id);
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
   const PilotWeaponSet *ws = pilot_weapSet(p,id);
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
   static char setname[STRMAX_SHORT];
   const char *base, *type, *problem;
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   problem = type = base = NULL;

   switch (ws->type) {
      case WEAPSET_TYPE_SWITCH:
         type = p_("weapset","Switch");
         break;
      case WEAPSET_TYPE_TOGGLE:
         type = p_("weapset","Toggle");
         break;
      case WEAPSET_TYPE_HOLD:
         type = p_("weapset","Hold");
         break;
      default:
         type = p_("weapset","Unknown");
         break;
   }

   if (array_size(ws->slots)==0)
      base = _("Empty");
   else {
      const Outfit *o = NULL;
      int not_same = 0;
      int has_weap = 0;
      int has_util = 0;
      int has_stru = 0;
      for (int i=0; i<array_size(ws->slots); i++) {
         const PilotOutfitSlot *pos = p->outfits[ ws->slots[i].slotid ];
         if (pos->outfit==NULL) /* Ignore empty slots. */
            continue;
         if (!pilot_slotIsToggleable(pos)) /* Ignore non-active. */
            continue;
         if (o==NULL)
            o = pos->outfit;
         else if(o!=pos->outfit)
            not_same = 1;
         switch (pos->sslot->slot.type) {
            case OUTFIT_SLOT_STRUCTURE:
               has_stru++;
               break;
            case OUTFIT_SLOT_UTILITY:
               has_util++;
               break;
            case OUTFIT_SLOT_WEAPON:
               has_weap++;
               break;
            default:
               break;
         }
      }
      if (o==NULL)
         base = _("Empty");
      else if (not_same==0)
         base = _(o->name);
      else if (has_weap && !has_util && !has_stru)
         base = p_("weapset","Weapons");
      else if (!has_weap && has_util && !has_stru)
         base = p_("weapset","Utilities");
      else if (!has_weap && !has_util && has_stru)
         base = p_("weapset","Structurals");
      else
         base = p_("weapset","Mixed");

      /* Try to proactively detect issues with the weapon set. */
      if (!has_weap && (ws->type==WEAPSET_TYPE_SWITCH))
         problem = _("no weapons!");
   }

   if (problem!=NULL)
      snprintf( setname, sizeof(setname), p_("weapset", "#o%s - %s#0 [#r%s#0]"), type, base, problem );
   else
      snprintf( setname, sizeof(setname), p_("weapset", "%s - %s"), type, base );
   return setname;
}

/**
 * @brief Adds an outfit to a weapon set.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param o Outfit to add.
 *    @param level Level of the trigger.
 */
void pilot_weapSetAdd( Pilot* p, int id, const PilotOutfitSlot *o, int level )
{
   PilotWeaponSetOutfit *slot;
   PilotWeaponSet *ws = pilot_weapSet(p,id);

   /* Create if needed. */
   if (ws->slots == NULL)
      ws->slots = array_create( PilotWeaponSetOutfit );

   /* Check if already there. */
   for (int i=0; i<array_size(ws->slots); i++) {
      if (ws->slots[i].slotid != o->id)
         continue;
      ws->slots[i].level = level;
      pilot_weapSetUpdateOutfits( p, ws );
      return;
   }

   /* Add it. */
   slot        = &array_grow( &ws->slots );
   slot->level = level;
   slot->slotid= o->id;
   if (o->outfit!=NULL)
      slot->range2 = pow2(pilot_outfitRange( p, o->outfit ));
   else
      slot->range2 = 0.;
   pilot_weapSetUpdateOutfits( p, ws );
}

/**
 * @brief Removes a slot from a weapon set.
 *
 *    @param p Pilot who owns the weapon set.
 *    @param id ID of the weapon set.
 *    @param o Outfit to remove.
 */
void pilot_weapSetRm( Pilot* p, int id, const PilotOutfitSlot *o )
{
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   for (int i=0; i<array_size(ws->slots); i++) {
      if (ws->slots[i].slotid != o->id)
         continue;

      array_erase( &ws->slots, &ws->slots[i], &ws->slots[i+1] );
      pilot_weapSetUpdateOutfits( p, ws );
      return;
   }
}

/**
 * @brief Clears a weapon set.
 *
 *    @param p Pilot to clear weapon set.
 *    @param id Weapon set to clear.
 */
void pilot_weapSetClear( Pilot* p, int id )
{
   PilotWeaponSet *ws = pilot_weapSet(p,id);
   ws->type = WEAPSET_TYPE_SWITCH;
   array_free( ws->slots );
   ws->slots = NULL;

   /* Update if needed. */
   pilot_weapSetUpdateOutfits( p, ws );
}

/**
 * @brief Checks to see if a slot is in a weapon set.
 */
int pilot_weapSetInSet( Pilot* p, int id, const PilotOutfitSlot *o )
{
   const PilotWeaponSet *ws = pilot_weapSet(p,id);
   for (int i=0; i<array_size(ws->slots); i++) {
      /* Must match the current weapon. */
      if (ws->slots[i].slotid != o->id)
         continue;
      return ws->slots[i].level;
   }
   /* Not found. */
   return -1;
}

/**
 * @brief Checks to see if a slot is in a weapon set and usable.
 *
 *    @param p Pilot to check.
 *    @param id ID of the weapon set.
 *    @param o Outfit slot to check.
 *    @return The level to which it belongs (or -1 if it isn't set).
 */
int pilot_weapSetCheck( Pilot* p, int id, const PilotOutfitSlot *o )
{
   const PilotWeaponSet *ws = pilot_weapSet(p,id);
   for (int i=0; i<array_size(ws->slots); i++) {
      /* Must match the current weapon. */
      if (ws->slots[i].slotid != o->id)
         continue;
      /* Only weapons can be used as switch sets. */
      if ((o->outfit!= NULL) && !outfit_isWeapon(o->outfit) && (ws->type==WEAPSET_TYPE_SWITCH))
         continue;
      return ws->slots[i].level;
   }
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
      range = pilot_outfitRange( p, pos->outfit );
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
 *    @param primary Whether or not to shoot the primary.
 *    @param secondary Whether or not to shoot the secondary.
 *    @return The number of shots fired.
 */
int pilot_shoot( Pilot* p, int primary, int secondary )
{
   PilotWeaponSet *ws = pilot_weapSet( p, p->active_set );
   if (ws->type == WEAPSET_TYPE_SWITCH) {
      int old = ws->active;
      /* Set new state. */
      ws->active = 0;
      if (primary)
         ws->active |= WEAPSET_ACTIVE_PRIMARY;
      if (secondary)
         ws->active |= WEAPSET_ACTIVE_SECONDARY;
      /* Update state if something changed. */
      if (ws->active != old)
         pilot_weapSetUpdateOutfitState( p );
   }
   return 0;
}

/**
 * @brief Stops a beam outfit and sets delay as appropriate.
 *
 *    @param p Pilot that is firing.
 *    @param w Pilot's beam outfit.
 */
void pilot_stopBeam( const Pilot *p, PilotOutfitSlot *w )
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
   /*
   if ((w->outfit->lua_onshoot!= LUA_NOREF) &&
         !pilot_outfitLOnshoot( p, w, 0 ))
      return;
   */

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
static int pilot_shootWeaponSetOutfit( Pilot* p, const Outfit *o, const Target *target, double time, int aim )
{
   int is_launcher, is_bay;
   double rate_mod, energy_mod;
   int maxp, minh;
   double q, maxt;

   /* Stores if it is a launcher or bay. */
   is_launcher = outfit_isLauncher(o);
   is_bay = outfit_isFighterBay(o);

   /* Calculate rate modifier. */
   pilot_getRateMod( &rate_mod, &energy_mod, p, o );

   /* Find optimal outfit, coolest that can fire. */
   minh  = -1;
   maxt  = 0.;
   maxp  = -1;
   q     = 0.;
   for (int i=0; i<array_size(p->outfits); i++) {
      const PilotOutfitSlot *pos = p->outfits[i];

      /* Only matching outfits. */
      if (pos->outfit != o)
         continue;

      /* Must be on. */
      if (pos->state != PILOT_OUTFIT_ON)
         continue;

      /* Launcher only counts with ammo. */
      if ((is_launcher || is_bay) && (pos->u.ammo.quantity <= 0))
         continue;

      /* Get coolest that can fire. */
      if (pos->timer <= 0.) {
         if (is_launcher) {
            if ((minh < 0) || (p->outfits[ minh ]->u.ammo.quantity < pos->u.ammo.quantity))
               minh = i;
         }
         else {
            if ((minh < 0) || (p->outfits[ minh ]->heat_T > pos->heat_T))
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
   return pilot_shootWeapon( p, p->outfits[minh], target, time, aim );
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
      if ((aim >= 0) && (w->outfit->lua_onshoot != LUA_NOREF) &&
            !pilot_outfitLOnshoot( p, w ))
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
      if ((aim>=0) && (w->outfit->lua_onshoot != LUA_NOREF) &&
            !pilot_outfitLOnshoot( p, w ))
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
      //if ((w->outfit->u.lau.ai != AMMO_AI_UNGUIDED) && !((target->type==TARGET_PILOT) || (target->type==TARGET_ASTEROID)))
      if ((w->outfit->u.lau.ai != AMMO_AI_UNGUIDED) && (target->type!=TARGET_PILOT))
         return 0;

      /* Lua test. */
      if ((aim>=0) && (w->outfit->lua_onshoot != LUA_NOREF) &&
            !pilot_outfitLOnshoot( p, w ))
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
      if ((aim>=0) && (w->outfit->lua_onshoot != LUA_NOREF) &&
            !pilot_outfitLOnshoot( p, w ))
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
   pilot_weapSetType( p, 0, WEAPSET_TYPE_SWITCH ); /* All weaps. */
   pilot_weapSetType( p, 1, WEAPSET_TYPE_SWITCH ); /* Forwards. */
   pilot_weapSetType( p, 2, WEAPSET_TYPE_SWITCH ); /* Turrets. */
   pilot_weapSetType( p, 3, WEAPSET_TYPE_SWITCH ); /* All weaps. */
   pilot_weapSetType( p, 4, WEAPSET_TYPE_HOLD ); /* Seekers. */
   pilot_weapSetType( p, 5, WEAPSET_TYPE_HOLD ); /* Fighter bays. */
   pilot_weapSetType( p, 6, WEAPSET_TYPE_HOLD );
   pilot_weapSetType( p, 7, WEAPSET_TYPE_HOLD ); /* Afterburner. */
   pilot_weapSetType( p, 8, WEAPSET_TYPE_HOLD );
   pilot_weapSetType( p, 9, WEAPSET_TYPE_HOLD ); /* Turret seekers. */

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

   /* Update all outfits. */
   pilot_weaponSafe( p );
}

/**
 * @brief Gives the pilot a default weapon set.
 */
void pilot_weaponSetDefault( Pilot *p )
{
   int i;

   /* If current set isn't a fire group no need to worry. */
   if ((p->weapon_sets[ p->active_set ].type==WEAPSET_TYPE_SWITCH)
         && (array_size(p->weapon_sets[ p->active_set ].slots)>0)) {
      /* Update active weapon set. */
      pilot_weapSetUpdateOutfits( p, &p->weapon_sets[ p->active_set ] );
      return;
   }

   /* Find first fire group. */
   for (i=0; i<PILOT_WEAPON_SETS; i++) {
      const PilotWeaponSet *ws = &p->weapon_sets[i];
      if ((ws->type==WEAPSET_TYPE_SWITCH) && (array_size(ws->slots)>0))
         break;
   }

   /* Set active set to first if all fire groups or first non-fire group. */
   if (i >= PILOT_WEAPON_SETS) {
      /* Fallback to first switch group. */
      for (i=0; i<PILOT_WEAPON_SETS; i++) {
         const PilotWeaponSet *ws = &p->weapon_sets[i];
         if (ws->type==WEAPSET_TYPE_SWITCH)
            break;
      }
      p->active_set = (i>=PILOT_WEAPON_SETS) ? 0 : i;
   }
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

      /* Update range. */
      pilot_weapSetUpdateRange( p, ws );
   }

   /* Update active weapon set. */
   pilot_weapSetUpdateOutfits( p, &p->weapon_sets[ p->active_set ] );
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
   /* Disable Lua trigger. */
   o->flags &= ~PILOTOUTFIT_ISON_LUA;

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
      /*
      if ((o->outfit->lua_onshoot != LUA_NOREF) &&
            !pilot_outfitLOnshoot( p, o, 0 ))
         return 0;
      */
      /* Beams use stimer to represent minimum time until shutdown. */
      if (o->u.beamid>0) {
         /* Enforce minimum duration if set. */
         if (o->outfit->u.bem.min_duration > 0.) {

            o->stimer = o->outfit->u.bem.min_duration -
                  (o->outfit->u.bem.duration - o->timer);

            if (o->stimer > 0.)
               return 0;
         }
         beam_end( o->u.beamid );
         pilot_stopBeam(p, o); /* Sets the state. */
      }
      else
         o->state  = PILOT_OUTFIT_OFF;
   }
   else if (!(o->flags & PILOTOUTFIT_ACTIVE))
      /* Case of a mod we can't toggle. */
      return 0;
   else if (o->outfit->lua_ontoggle != LUA_NOREF) {
      int ret = pilot_outfitLOntoggle( p, o, 0 );
      if (ret && outfit_isWeapon(o->outfit))
         o->state = PILOT_OUTFIT_OFF;
   }
   else {
      o->stimer = outfit_cooldown( o->outfit );
      if (o->stimer < 0.)
         o->state  = PILOT_OUTFIT_OFF;
      else
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
   else if (o->outfit->lua_ontoggle != LUA_NOREF) {
      int ret = pilot_outfitLOntoggle( p, o, 1 );
      if (ret && outfit_isWeapon(o->outfit))
         o->state = PILOT_OUTFIT_ON;
      return ret;
   }
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
   if (p == NULL)
      return;

   if (pilot_isFlag(p, PILOT_HYP_PREP) || pilot_isFlag(p, PILOT_HYPERSPACE) ||
         pilot_isFlag(p, PILOT_LANDING) || pilot_isFlag(p, PILOT_TAKEOFF) ||
         pilot_isDisabled(p) || pilot_isFlag(p, PILOT_COOLDOWN))
      return;

   /* Not under manual control if is player. */
   if (pilot_isFlag( p, PILOT_MANUAL_CONTROL ) && pilot_isPlayer(p))
      return;

   /** @todo fancy effect? */
   if (p->afterburner == NULL)
      return;

   /* Needs at least enough energy to afterburn fo 0.5 seconds. */
   if (p->energy < p->afterburner->outfit->u.afb.energy*0.5)
      return;

   /* The afterburner only works if its efficiency is high enough. */
   if (pilot_heatEfficiencyMod( p->afterburner->heat_T, p->afterburner->outfit->overheat_min, p->afterburner->outfit->overheat_max ) < 0.1) {
      if (pilot_isPlayer(p))
         player_message(_("#r%s is overheating!#0"),_(p->afterburner->outfit->name));
      return;
   }

   /* Turn it on. */
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
      double afb_mod = MIN( 1., pilot_massFactor(player.p) );
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
