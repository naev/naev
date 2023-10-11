/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file pilot_outfit.c
 *
 * @brief Handles pilot outfits.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "array.h"
#include "escort.h"
#include "gui.h"
#include "log.h"
#include "difficulty.h"
#include "nstring.h"
#include "nxml.h"
#include "outfit.h"
#include "pause.h"
#include "pilot.h"
#include "player.h"
#include "slots.h"
#include "space.h"
#include "nlua.h"
#include "nlua_pilot.h"
#include "nlua_pilotoutfit.h"
#include "nlua_outfit.h"

/*
 * Prototypes.
 */
static int pilot_hasOutfitLimit( const Pilot *p, const char *limit );
static void pilot_calcStatsSlot( Pilot *pilot, PilotOutfitSlot *slot );
static const char *outfitkeytostr( OutfitKey key );

/**
 * @brief Updates the lockons on the pilot's launchers
 *
 *    @param p Pilot being updated.
 *    @param o Slot being updated.
 *    @param t Pilot that is currently the target of p (or NULL if not applicable).
 *    @param a Angle to update if necessary. Should be initialized to -1 before the loop.
 *    @param dt Current delta tick.
 */
void pilot_lockUpdateSlot( Pilot *p, PilotOutfitSlot *o, Pilot *t, double *a, double dt )
{
   double max, old;
   double x,y, ang, arc;
   int locked;

   /* No target. */
   if (t == NULL)
      return;

   /* Nota  seeker. */
   if (!outfit_isSeeker(o->outfit))
      return;

   /* Check arc. */
   arc = o->outfit->u.lau.arc;
   if (arc > 0.) {

      /* We use an external variable to set and update the angle if necessary. */
      if (*a < 0.) {
         x     = t->solid.pos.x - p->solid.pos.x;
         y     = t->solid.pos.y - p->solid.pos.y;
         ang   = ANGLE( x, y );
         *a    = FABS( angle_diff( ang, p->solid.dir ) );
      }

      /* Decay if not in arc. */
      if (*a > arc) {
         /* Limit decay to the lockon time for this launcher. */
         max = o->outfit->u.lau.lockon;

         /* When a lock is lost, immediately gain half the lock timer.
          * This is meant as an incentive for the aggressor not to lose the lock,
          * and for the target to try and break the lock. */
         old = o->u.ammo.lockon_timer;
         o->u.ammo.lockon_timer += dt;
         if ((old <= 0.) && (o->u.ammo.lockon_timer > 0.))
            o->u.ammo.lockon_timer += o->outfit->u.lau.lockon / 2.;

         /* Cap at max. */
         if (o->u.ammo.lockon_timer > max)
            o->u.ammo.lockon_timer = max;

         /* Out of arc. */
         o->u.ammo.in_arc = 0;
         return;
      }
   }

   /* In arc. */
   o->u.ammo.in_arc = 1;
   locked = (o->u.ammo.lockon_timer < 0.);

   /* Lower timer. When the timer reaches zero, the lock is established. */
   max = -o->outfit->u.lau.lockon/3.;
   if (o->u.ammo.lockon_timer > max) {
      /* Targetting is linear and can't be faster than the time specified (can be slower though). */
      double mod = pilot_ewWeaponTrack( p, t, o->outfit->u.lau.trackmin, o->outfit->u.lau.trackmax );
      o->u.ammo.lockon_timer -= dt * mod / p->stats.launch_lockon;

      /* Cap at -max/3. */
      if (o->u.ammo.lockon_timer < max)
         o->u.ammo.lockon_timer = max;

      /* Trigger lockon hook. */
      if (!locked && (o->u.ammo.lockon_timer < 0.))
         pilot_runHook( p, PILOT_HOOK_LOCKON );
   }
}

/**
 * @brief Clears pilot's missile lockon timers.
 *
 *    @param p Pilot to clear missile lockon timers.
 */
void pilot_lockClear( Pilot *p )
{
   for (int i=0; i<array_size(p->outfits); i++) {
      PilotOutfitSlot *o = p->outfits[i];
      if (o->outfit == NULL)
         continue;
      if (!outfit_isSeeker(o->outfit))
         continue;

      /* Clear timer. */
      o->u.ammo.lockon_timer = o->outfit->u.lau.lockon;

      /* Clear arc. */
      o->u.ammo.in_arc = 0;
   }
}

/**
 * @brief Gets the mount position of a pilot.
 *
 * Position is relative to the pilot.
 *
 *    @param p Pilot to get mount position of.
 *    @param w Slot of the mount.
 *    @param[out] v Position of the mount.
 *    @return 0 on success.
 */
int pilot_getMount( const Pilot *p, const PilotOutfitSlot *w, vec2 *v )
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
   m = &w->sslot->mount;
   cm = cos(-a);
   sm = sin(-a);
   x = m->x * cm + m->y * sm;
   y = m->x *-sm + m->y * cm;

   /* Correction for ortho perspective. */
   y *= M_SQRT1_2;

   /* Don't forget to add height. */
   y += m->h;

   /* Get the mount and add the player.p offset. */
   vec2_cset( v, x, y );

   return 0;
}

/**
 * @brief Docks the pilot on its target pilot.
 *
 *    @param p Pilot that wants to dock.
 *    @param target Pilot to dock on.
 *    @return 0 on successful docking.
 */
int pilot_dock( Pilot *p, Pilot *target )
{
   int i;
   PilotOutfitSlot* dockslot;

   /* Must belong to target */
   if (p->dockpilot != target->id)
      return -1;

   /* Must have a dockslot */
   dockslot = pilot_getDockSlot( p );
   if (dockslot == NULL)
      return -1;

   /* Must be close. */
   if (vec2_dist(&p->solid.pos, &target->solid.pos) >
         target->ship->gfx_space->sw * PILOT_SIZE_APPROX )
      return -1;

   /* Cannot be going much faster. */
   if (vec2_dist2( &p->solid.vel, &target->solid.vel ) > pow2(MAX_HYPERSPACE_VEL))
      return -1;

   /* Grab dock ammo */
   i = p->dockslot;

   /* Try to add fighter. */
   dockslot->u.ammo.deployed--;
   p->dockpilot = 0;
   p->dockslot = -1;

   /* Add the pilot's outfit. */
   if (pilot_addAmmo(target, target->outfits[i], 1) != 1)
      return -1;

   /* Remove from pilot's escort list. */
   for (i=0; i<array_size(target->escorts); i++) {
      if ((target->escorts[i].type == ESCORT_TYPE_BAY) &&
            (target->escorts[i].id == p->id))
         break;
   }
   /* Not found as pilot's escorts. */
   if (i >= array_size(target->escorts))
      return -1;
   /* Free if last pilot. */
   if (array_size(target->escorts) == 1)
      escort_freeList(target);
   else
      escort_rmListIndex(target, i);

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
int pilot_hasDeployed( const Pilot *p )
{
   for (int i=0; i<array_size(p->outfits); i++) {
      if (p->outfits[i]->outfit == NULL)
         continue;
      if (outfit_isFighterBay(p->outfits[i]->outfit))
         if (p->outfits[i]->u.ammo.deployed > 0)
            return 1;
   }
   return 0;
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
int pilot_addOutfitRaw( Pilot* pilot, const Outfit* outfit, PilotOutfitSlot *s )
{
   const Outfit *o;

   /* Set the outfit. */
   s->state    = PILOT_OUTFIT_OFF;
   s->outfit   = outfit;

   /* Set some default parameters. */
   s->timer    = 0.;

   /* Some per-case scenarios. */
   if (outfit_isFighterBay(outfit)) {
      s->u.ammo.quantity = 0;
      s->u.ammo.deployed = 0;
      pilot->nfighterbays++;
   }
   else if (outfit_isTurret(outfit)) /* used to speed up AI */
      pilot->nturrets++;
   else if (outfit_isBolt(outfit))
      pilot->ncannons++;
   else if (outfit_isAfterburner(outfit))
      pilot->nafterburners++;
   if (outfit_isLauncher(outfit)) {
      s->u.ammo.quantity = 0;
      s->u.ammo.deployed = 0; /* Just in case. */
   }

   if (outfit_isBeam(outfit)) { /* Used to speed up some calculations. */
      s->u.beamid = 0;
      pilot->nbeams++;
   }

   /* Check if active. */
   o = s->outfit;
   s->active = outfit_isActive(o);

   /* Update heat. */
   pilot_heatCalcSlot( s );

   /* Disable lua for now. */
   s->lua_mem = LUA_NOREF;
   ss_free( s->lua_stats ); /* Just in case. */
   s->lua_stats = NULL;

   /* Initialize if active thingy if necessary. */
   pilot_outfitLAdd( pilot, s );

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
int pilot_addOutfitTest( Pilot* pilot, const Outfit* outfit, PilotOutfitSlot *s, int warn )
{
   const char *str;

   /* See if slot has space. */
   if (s->outfit != NULL) {
      if (warn)
         WARN( _("Pilot '%s': trying to add outfit '%s' to slot that already has an outfit"),
               pilot->name, outfit->name );
      return -1;
   }
   else if ((outfit_cpu(outfit) < 0) &&
         (pilot->cpu < ABS( outfit_cpu(outfit) ))) {
      if (warn)
         WARN( _("Pilot '%s': Not enough CPU to add outfit '%s'"),
               pilot->name, outfit->name );
      return -1;
   }
   else if ((str = pilot_canEquip( pilot, s, outfit)) != NULL) {
      if (warn)
         WARN( _("Pilot '%s': Trying to add outfit but %s"),
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
int pilot_addOutfit( Pilot* pilot, const Outfit* outfit, PilotOutfitSlot *s )
{
   /* Test to see if outfit can be added. */
   int ret = pilot_addOutfitTest( pilot, outfit, s, 1 );
   if (ret != 0)
      return -1;

   /* Add outfit. */
   ret = pilot_addOutfitRaw( pilot, outfit, s );

   /* Recalculate the stats */
   pilot_calcStats(pilot);

   return ret;
}

/**
 * @brief Adds an outfit as an intrinsic slot.
 */
int pilot_addOutfitIntrinsic( Pilot *pilot, const Outfit *outfit )
{
   PilotOutfitSlot *s;
   int ret;

   if (!outfit_isMod(outfit)) {
      WARN(_("Instrinsic outfits must be modifiers!"));
      return -1;
   }

   if (pilot->outfit_intrinsic==NULL)
      pilot->outfit_intrinsic = array_create( PilotOutfitSlot );

   s = &array_grow( &pilot->outfit_intrinsic );
   memset( s, 0, sizeof(PilotOutfitSlot) );
   ret = pilot_addOutfitRaw( pilot, outfit, s );
   if (pilot->id > 0 && ret==0)
      pilot_outfitLInit( pilot, s );

   return ret;
}

/**
 * @brief Removes an outfit from an intrinsic slot.
 */
int pilot_rmOutfitIntrinsic( Pilot *pilot, const Outfit *outfit )
{
   int ret = 0;
   for (int i=0; i<array_size(pilot->outfit_intrinsic); i++) {
      PilotOutfitSlot *s = &pilot->outfit_intrinsic[i];
      if (s->outfit != outfit)
         continue;
      ret = pilot_rmOutfitRaw( pilot, s );
      array_erase( &pilot->outfit_intrinsic, s, s+1 );
      break;
   }
   /* Recalculate the stats */
   if (ret)
      pilot_calcStats(pilot);
   return ret;
}

/**
 * @brief Gets how many copies of an intrinsic a pilot has.
 */
int pilot_hasIntrinsic( const Pilot *pilot, const Outfit *outfit )
{
   int ret = 0;
   for (int i=0; i<array_size(pilot->outfit_intrinsic); i++) {
      PilotOutfitSlot *s = &pilot->outfit_intrinsic[i];
      if (s->outfit != outfit)
         continue;
      ret++;
   }
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

   /* Force turn off if necessary. */
   if (s->state==PILOT_OUTFIT_ON)
      pilot_outfitOff( pilot, s );

   /* Run remove hook if necessary. */
   pilot_outfitLRemove( pilot, s );

   /* Decrement counters if necessary. */
   if (s->outfit != NULL) {
      if (outfit_isTurret(s->outfit))
         pilot->nturrets--;
      else if (outfit_isBolt(s->outfit))
         pilot->ncannons--;
      else if (outfit_isAfterburner(s->outfit))
         pilot->nafterburners--;
      else if (outfit_isFighterBay(s->outfit))
         pilot->nfighterbays--;
      if (outfit_isBeam(s->outfit))
         pilot->nbeams--;
   }

   /* Remove the outfit. */
   ret         = (s->outfit==NULL);
   s->outfit   = NULL;
   s->weapset  = -1;

   /* Remove secondary and such if necessary. */
   if (pilot->afterburner == s)
      pilot->afterburner = NULL;

   /* Clear Lua if necessary. */
   if (s->lua_mem != LUA_NOREF) {
      luaL_unref( naevL, LUA_REGISTRYINDEX, s->lua_mem );
      s->lua_mem = LUA_NOREF;
   }

   /* Clean up stats. */
   ss_free( s->lua_stats );
   s->lua_stats = NULL;

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
   int ret;
   const char *str = pilot_canEquip( pilot, s, NULL );
   if (str != NULL) {
      WARN(_("Pilot '%s': Trying to remove outfit but %s"),
            pilot->name, str );
      return -1;
   }

   ret = pilot_rmOutfitRaw( pilot, s );

   /* recalculate the stats */
   pilot_calcStats(pilot);

   return ret;
}

/**
 * @brief Pilot slot safety check - makes sure stats are safe.
 *
 *    @param p Pilot to check.
 *    @return 0 if a slot doesn't fit, !0 otherwise.
 */
int pilot_slotsCheckSafety( const Pilot *p )
{
   for (int i=0; i<array_size(p->outfits); i++)
      if ((p->outfits[i]->outfit != NULL) &&
            !outfit_fitsSlot( p->outfits[i]->outfit, &p->outfits[i]->sslot->slot ))
         return 0;
   return 1;
}

/**
 * @brief Pilot required (core) slot filled check - makes sure they are filled.
 *
 *    @param p Pilot to check.
 *    @return 0 if a slot is missing, !0 otherwise.
 */
int pilot_slotsCheckRequired( const Pilot *p )
{
   for (int i=0; i<array_size(p->outfits); i++)
      if (p->outfits[i]->sslot->required && p->outfits[i]->outfit == NULL)
         return 0;
   return 1;
}

/**
 * @brief Pilot safety check - makes sure stats are safe.
 *
 *    @param p Pilot to check.
 *    @return The reason why the pilot is not safe (or NULL if safe).
 */
int pilot_isSpaceworthy( const Pilot *p )
{
   return !pilot_reportSpaceworthy( p, NULL, 0 );
}

/**
 * @brief Pilot safety report - makes sure stats are safe.
 *
 *    @param p Pilot to check.
 *    @param buf Buffer to fill.
 *    @param bufSize Size of the buffer.
 *    @return Number of issues encountered.
 */
int pilot_reportSpaceworthy( const Pilot *p, char *buf, int bufSize )
{
#define SPACEWORTHY_CHECK(cond,msg) \
   if (cond) { ret++; \
      if (buf != NULL) { \
         if (pos > 0) \
            pos += scnprintf( &buf[pos], bufSize-pos, "\n" ); \
         pos += scnprintf( &buf[pos], bufSize-pos, (msg) ); } }
   int pos = 0;
   int ret = 0;

   /* Core Slots */
   SPACEWORTHY_CHECK( !pilot_slotsCheckRequired(p), _("!! Not All Core Slots are equipped") );
   /* CPU. */
   SPACEWORTHY_CHECK( p->cpu < 0, _("!! Insufficient CPU") );

   /* Movement. */
   SPACEWORTHY_CHECK( p->accel < 0,  _("!! Insufficient Accel") );
   SPACEWORTHY_CHECK( p->speed < 0,  _("!! Insufficient Speed") );
   SPACEWORTHY_CHECK( p->turn < 0,   _("!! Insufficient Turn") );

   /* Health. */
   SPACEWORTHY_CHECK( p->armour < 0.,       _("!! Insufficient Armour") );
   SPACEWORTHY_CHECK( p->armour_regen < 0., _("!! Insufficient Armour Regeneration") );
   SPACEWORTHY_CHECK( p->shield < 0.,       _("!! Insufficient Shield") );
   SPACEWORTHY_CHECK( p->shield_regen < 0., _("!! Insufficient Shield Regeneration") );
   SPACEWORTHY_CHECK( p->energy_max < 0.,   _("!! Insufficient Energy") );
   SPACEWORTHY_CHECK( (p->energy_regen <= 0.) && (p->energy_max > 0.), _("!! Insufficient Energy Regeneration") );

   /* Misc. */
   SPACEWORTHY_CHECK( p->fuel_max < 0,         _("!! Insufficient Fuel Maximum") );
   SPACEWORTHY_CHECK( p->fuel_consumption < 0, _("!! Insufficient Fuel Consumption") );
   SPACEWORTHY_CHECK( p->cargo_free < 0,       _("!! Insufficient Free Cargo Space") );
   SPACEWORTHY_CHECK( p->crew < 0,             _("!! Insufficient Crew") );

   /* No need to mess with the string. */
   if (buf==NULL)
      return ret;

   /* Buffer is full, lets write that there is more then what's copied */
   if (pos > bufSize-1) {
      buf[bufSize-4]='.';
      buf[bufSize-3]='.';
      buf[bufSize-2]='.';
      /* buf[bufSize-1]='\0'; already done for us */
   }
   else if (pos == 0) {
      /* String is empty so no errors encountered */
      pos += scnprintf( buf, bufSize, _("Spaceworthy"));
      if (ship_isFlag(p->ship, SHIP_NOPLAYER))
         pos += scnprintf( &buf[pos], bufSize-pos, "\n#o%s#0", _("Escort only") );
      if (ship_isFlag(p->ship, SHIP_NOESCORT))
         pos += scnprintf( &buf[pos], bufSize-pos, "\n#o%s#0", _("Lead ship only") );
   }

   return ret;
}
#undef SPACEWORTHY_CHECK

/**
 * @brief Checks to see if a pilot has an outfit with a specific outfit type.
 *
 *    @param p Pilot to check.
 *    @param limit Outfit (limiting) type to check.
 *    @return the amount of outfits of this type the pilot has.
 */
static int pilot_hasOutfitLimit( const Pilot *p, const char *limit )
{
   for (int i = 0; i<array_size(p->outfits); i++) {
      const Outfit *o = p->outfits[i]->outfit;
      if (o == NULL)
         continue;
      if ((o->limit != NULL) && (strcmp(o->limit,limit)==0))
         return 1;
   }
   return 0;
}

/**
 * @brief Checks to see if can equip/remove an outfit from a slot.
 *
 *    @param p Pilot to check if can equip.
 *    @param s Slot being checked to see if it can equip/remove an outfit.
 *    @param o Outfit to check (NULL if being removed).
 *    @return NULL if can swap, or error message if can't.
 */
const char* pilot_canEquip( const Pilot *p, const PilotOutfitSlot *s, const Outfit *o )
{
   /* Just in case. */
   if ((p==NULL) || (s==NULL))
      return _("Nothing selected.");

   if (o!=NULL) {
      /* Check slot type. */
      if (!outfit_fitsSlot( o, &s->sslot->slot ))
         return _("Does not fit slot.");
      /* Check outfit limit. */
      if ((o->limit != NULL) && pilot_hasOutfitLimit( p, o->limit ))
         return _("Already have an outfit of this type installed");
      /* Check to see if already equipped unique. */
      if (outfit_isProp(o,OUTFIT_PROP_UNIQUE) && (pilot_numOutfit(p,o)>0))
         return _("Can only install unique outfit once.");
   }
   else {
      /* Check fighter bay. */
      if ((o==NULL) && (s!=NULL) && (s->u.ammo.deployed > 0))
         return _("Recall the fighters first");
   }

   return NULL;
}

/**
 * @brief Adds some ammo to the pilot stock.
 *
 *    @param pilot Pilot to add ammo to.
 *    @param s Slot to add ammo to.
 *    @param quantity Amount to add.
 *    @return Amount actually added.
 */
int pilot_addAmmo( Pilot* pilot, PilotOutfitSlot *s, int quantity )
{
   int q, max;

   /* Failure cases. */
   if (s->outfit == NULL) {
      WARN(_("Pilot '%s': Trying to add ammo to unequipped slot."), pilot->name );
      return 0;
   }
   else if (!outfit_isLauncher(s->outfit) && !outfit_isFighterBay(s->outfit))
      return 0;

   /* Add the ammo. */
   max                 = pilot_maxAmmoO(pilot,s->outfit) - s->u.ammo.deployed;
   q                   = s->u.ammo.quantity; /* Amount have. */
   s->u.ammo.quantity += quantity;
   s->u.ammo.quantity  = MIN( max, s->u.ammo.quantity );
   q                   = s->u.ammo.quantity - q; /* Amount actually added. */
   pilot->mass_outfit += q * outfit_ammoMass( s->outfit );
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

   /* Failure cases. */
   if (!outfit_isLauncher(s->outfit) && !outfit_isFighterBay(s->outfit))
      return 0;
   else if (s->outfit == NULL) {
      WARN(_("Pilot '%s': Trying to remove ammo from unequipped slot."), pilot->name );
      return 0;
   }

   /* Remove ammo. */
   q                   = MIN( quantity, s->u.ammo.quantity );
   s->u.ammo.quantity -= q;
   pilot->mass_outfit -= q * outfit_ammoMass( s->outfit );
   pilot_updateMass( pilot );
   /* We don't set the outfit to null so it "remembers" old ammo. */

   return q;
}

/**
 * @brief Gets the number of ammo units on the ship
 *
 *    @param pilot Pilot to count the ammo on
 *    @@return The integer count of ammo units on pilot
 */
int pilot_countAmmo( const Pilot *pilot )
{
   int nammo = 0;
   for (int i=0; i<array_size(pilot->outfits); i++) {
      const Outfit* outfit;
      PilotOutfitSlot* po = pilot->outfits[i];
      if (po == NULL)
         continue;
      outfit = po->outfit;
      if (outfit == NULL)
         continue;
      if (!outfit_isLauncher(po->outfit))
         continue;
      nammo += po->u.ammo.quantity;
   }
   return nammo;
}

/**
 * @brief The maximum amount of ammo the pilot's current ship can hold.
 *
 *    @param pilot Pilot to get the count from
 *    @@return An integer, the max amount of ammo that can be held.
 */
int pilot_maxAmmo( const Pilot *pilot )
{
   int max = 0;
   for (int i=0; i<array_size(pilot->outfits); i++) {
      const Outfit* outfit;
      PilotOutfitSlot* po = pilot->outfits[i];
      if (po == NULL)
         continue;
      outfit = po->outfit;
      if (outfit == NULL)
         continue;
      if (!outfit_isLauncher(outfit))
         continue;
      max += outfit->u.lau.amount;
   }
   max = round( (double)max * pilot->stats.ammo_capacity );
   return max;
}

/**
 * @brief Gets the maximum available ammo for a pilot for a specific outfit.
 */
int pilot_maxAmmoO( const Pilot *p, const Outfit *o )
{
   int max;
   if (o==NULL)
      return 0;
   else if (outfit_isLauncher(o))
      max = round( (double)o->u.lau.amount * p->stats.ammo_capacity );
   else if (outfit_isFighterBay(o))
      max = round( (double)o->u.bay.amount * p->stats.fbay_capacity );
   else
      max = 0;
   return max;
}

/**
 * @brief Fills pilot's ammo completely.
 *
 *    @param pilot Pilot to add ammo to.
 */
void pilot_fillAmmo( Pilot* pilot )
{
   for (int i=0; i<array_size(pilot->outfits); i++) {
      int ammo_threshold;
      const Outfit *o = pilot->outfits[i]->outfit;

      /* Must be valid outfit. */
      if (o == NULL)
         continue;

      /* Initial (raw) ammo threshold */
      ammo_threshold = pilot_maxAmmoO( pilot, o );

      /* Adjust for deployed fighters if needed */
      if (outfit_isFighterBay( o ))
         ammo_threshold -= pilot->outfits[i]->u.ammo.deployed;

      /* Add ammo. */
      pilot_addAmmo( pilot, pilot->outfits[i],
         ammo_threshold - pilot->outfits[i]->u.ammo.quantity );
   }
}

/**
 * @brief Computes the stats for a pilot's slot.
 */
static void pilot_calcStatsSlot( Pilot *pilot, PilotOutfitSlot *slot )
{
   const Outfit *o = slot->outfit;
   ShipStats *s = &pilot->stats;

   /* Outfit must exist. */
   if (o==NULL)
      return;

   /* Modify CPU. */
   pilot->cpu           += outfit_cpu(o);

   /* Add mass. */
   pilot->mass_outfit   += o->mass;

   /* Keep a separate counter for required (core) outfits. */
   if (sp_required( o->slot.spid ))
      pilot->base_mass += o->mass;

   /* Add ammo mass. */
   if (outfit_isLauncher(o))
      pilot->mass_outfit += slot->u.ammo.quantity * o->u.lau.ammo_mass;
   else if (outfit_isFighterBay(o))
      pilot->mass_outfit += slot->u.ammo.quantity * o->u.bay.ship_mass;

   if (outfit_isAfterburner(o)) /* Afterburner */
      pilot->afterburner = slot; /* Set afterburner */

   /* Lua mods apply their stats. */
   if (slot->lua_mem != LUA_NOREF)
      ss_statsMergeFromList( &pilot->stats, slot->lua_stats );

   /* Has update function. */
   if (o->lua_update != LUA_NOREF)
      pilot->outfitlupdate = 1;

   /* Apply modifications. */
   if (outfit_isMod(o)) { /* Modification */
      /* Active outfits must be on to affect stuff. */
      if (slot->active && !(slot->state==PILOT_OUTFIT_ON))
         return;
      /* Add stats. */
      ss_statsMergeFromList( s, o->stats );

   }
   else if (outfit_isAfterburner(o)) { /* Afterburner */
      /* Active outfits must be on to affect stuff. */
      if (slot->active && !(slot->state==PILOT_OUTFIT_ON))
         return;
      /* Add stats. */
      ss_statsMergeFromList( s, o->stats );
      pilot_setFlag( pilot, PILOT_AFTERBURNER ); /* We use old school flags for this still... */
      pilot->energy_loss += pilot->afterburner->outfit->u.afb.energy; /* energy loss */
   }
   else {
      /* Always add stats for non mod/afterburners. */
      ss_statsMergeFromList( s, o->stats );
   }
}

/**
 * @brief Recalculates the pilot's stats based on his outfits.
 *
 *    @param pilot Pilot to recalculate his stats.
 */
void pilot_calcStats( Pilot* pilot )
{
   double ac, sc, ec, tm; /* temporary health coefficients to set */
   ShipStats *s;

   /*
    * Set up the basic stuff
    */
   /* mass */
   pilot->solid.mass    = pilot->ship->mass;
   pilot->base_mass     = pilot->solid.mass;
   /* cpu */
   pilot->cpu           = 0.;
   /* movement */
   pilot->accel_base    = pilot->ship->accel;
   pilot->turn_base     = pilot->ship->turn;
   pilot->speed_base    = pilot->ship->speed;
   /* crew */
   pilot->crew          = pilot->ship->crew;
   /* cargo */
   pilot->cap_cargo     = pilot->ship->cap_cargo;
   /* fuel_consumption. */
   pilot->fuel_consumption = pilot->ship->fuel_consumption;
   /* health */
   ac = (pilot->armour_max > 0.) ? pilot->armour / pilot->armour_max : 0.;
   sc = (pilot->shield_max > 0.) ? pilot->shield / pilot->shield_max : 0.;
   ec = (pilot->energy_max > 0.) ? pilot->energy / pilot->energy_max : 0.;
   pilot->armour_max    = pilot->ship->armour;
   pilot->shield_max    = pilot->ship->shield;
   pilot->fuel_max      = pilot->ship->fuel;
   pilot->armour_regen  = pilot->ship->armour_regen;
   pilot->shield_regen  = pilot->ship->shield_regen;
   /* Absorption. */
   pilot->dmg_absorb    = pilot->ship->dmg_absorb;
   /* Energy. */
   pilot->energy_max    = pilot->ship->energy;
   pilot->energy_regen  = pilot->ship->energy_regen;
   pilot->energy_loss   = 0.; /* Initially no net loss. */
   /* Misc. */
   pilot->outfitlupdate = 0;
   /* Stats. */
   s = &pilot->stats;
   tm = s->time_mod;
   *s = pilot->ship->stats_array;

   /* Player gets difficulty applied. */
   if (pilot_isPlayer(pilot))
      difficulty_apply( s );

   /* Now add outfit changes */
   pilot->mass_outfit   = 0.;
   for (int i=0; i<array_size(pilot->outfit_intrinsic); i++)
      pilot_calcStatsSlot( pilot, &pilot->outfit_intrinsic[i] );
   for (int i=0; i<array_size(pilot->outfits); i++)
      pilot_calcStatsSlot( pilot, pilot->outfits[i] );

   /* Merge stats. */
   ss_statsMergeFromList( &pilot->stats, pilot->ship_stats );
   ss_statsMergeFromList( &pilot->stats, pilot->intrinsic_stats );

   /* Compute effects. */
   effect_compute( &pilot->stats, pilot->effects );

   /* Apply system effects. */
   ss_statsMergeFromList( &pilot->stats, cur_system->stats );

   /* Apply stealth malus. */
   if (pilot_isFlag(pilot, PILOT_STEALTH)) {
      s->accel_mod   *= 0.8;
      s->turn_mod    *= 0.8;
      s->speed_mod   *= 0.5;
   }

   /*
    * Absolute increases.
    */
   /* Movement. */
   pilot->accel_base   += s->accel;
   pilot->turn_base    += s->turn * M_PI / 180.;
   pilot->speed_base   += s->speed;
   /* Health. */
   pilot->armour_max   += s->armour;
   pilot->armour_regen += s->armour_regen;
   pilot->shield_max   += s->shield;
   pilot->shield_regen += s->shield_regen;
   pilot->energy_max   += s->energy;
   pilot->energy_regen += s->energy_regen;
   /* Misc. */
   pilot->fuel_max     += s->fuel;
   pilot->cap_cargo    += s->cargo;

   /*
    * Relative increases.
    */
   /* Movement. */
   pilot->accel_base  *= s->accel_mod;
   pilot->turn_base    *= s->turn_mod;
   pilot->speed_base   *= s->speed_mod;
   /* Health. */
   pilot->armour_max   *= s->armour_mod;
   pilot->armour_regen *= s->armour_regen_mod;
   pilot->shield_max   *= s->shield_mod;
   pilot->shield_regen *= s->shield_regen_mod;
   pilot->energy_max   *= s->energy_mod;
   pilot->energy_regen *= s->energy_regen_mod;
   /* Enforce health to be at least 0 after mods, so that something like -1000% would just set it to 0 instead of negative. */
   pilot->armour_regen = MAX( 0., pilot->armour_regen );
   pilot->shield_regen = MAX( 0., pilot->shield_regen );
   pilot->energy_regen = MAX( 0., pilot->energy_regen );
   /* cpu */
   pilot->cpu_max       = (int)floor((float)(pilot->ship->cpu + s->cpu_max)*s->cpu_mod);
   pilot->cpu          += pilot->cpu_max; /* CPU is negative, this just sets it so it's based off of cpu_max. */
   /* Misc. */
   pilot->crew          = pilot->crew * s->crew_mod + s->crew;
   pilot->fuel_max     *= s->fuel_mod;
   pilot->cap_cargo    *= s->cargo_mod;
   s->engine_limit     *= s->engine_limit_rel;

   /* Set maximum speed. */
   if (!pilot_isFlag( pilot, PILOT_AFTERBURNER ))
      pilot->solid.speed_max = pilot->speed_base;

   /*
    * Flat increases.
    */
   pilot->armour_regen -= s->armour_regen_malus;
   pilot->shield_regen -= s->shield_regen_malus;
   pilot->energy_regen -= s->energy_regen_malus;
   pilot->energy_loss  += s->energy_loss;
   pilot->dmg_absorb    = CLAMP( 0., 1., pilot->dmg_absorb + s->absorb );

   /* Give the pilot his health proportion back */
   pilot->armour = ac * pilot->armour_max;
   pilot->shield = sc * pilot->shield_max;
   pilot->energy = ec * pilot->energy_max;

   /* Dump excess fuel */
   pilot->fuel   = MIN( pilot->fuel, pilot->fuel_max );

   /* Set final energy tau. */
   if (pilot->energy_regen > 0.)
      pilot->energy_tau = pilot->energy_max / pilot->energy_regen;
   else
      pilot->energy_tau = 1.;

   /* Cargo has to be reset. */
   pilot_cargoCalc(pilot); /* Calls pilot_updateMass. */

   /* Calculate the heat. */
   pilot_heatCalc( pilot );

   /* Update GUI as necessary. */
   gui_setGeneric( pilot );

   /* Update weapon set range. */
   pilot_weapSetUpdateStats( pilot );

   /* In case the time_mod has changed. */
   if (pilot_isPlayer(pilot) && (tm != s->time_mod))
      player_resetSpeed();
}

/**
 * @brief Cures the pilot as if he was landed.
 */
void pilot_healLanded( Pilot *pilot )
{
   pilot->armour = pilot->armour_max;
   pilot->shield = pilot->shield_max;
   pilot->energy = pilot->energy_max;

   pilot->stress = 0.;
   pilot->stimer = 0.;
   pilot->sbonus = 0.;

   pilot_fillAmmo( pilot );

   for (int i=0; i<array_size(pilot->escorts); i++) {
      Escort_t *e = &pilot->escorts[i];
      Pilot *pe = pilot_get( e->id );

      if (pe != NULL)
         pilot_healLanded( pe );
   }
}

/**
 * @brief Gets the outfit slot by name.
 */
PilotOutfitSlot *pilot_getSlotByName( Pilot *pilot, const char *name )
{
   for (int i=0; i<array_size(pilot->outfits); i++) {
      PilotOutfitSlot *s = pilot->outfits[i];
      if ((s->sslot->name!=NULL) && (strcmp(s->sslot->name,name)==0))
         return s;
   }
   return NULL;
}

/**
 * @brief Gets the factor at which speed gets worse.
 */
double pilot_massFactor( const Pilot *pilot )
{
   double mass = pilot->solid.mass;
   if ((pilot->stats.engine_limit > 0.) && (mass > pilot->stats.engine_limit)) {
      double f = (mass-pilot->stats.engine_limit) / pilot->stats.engine_limit;
      return 1./(1.+f+f+4.*pow(f,3.));
   }
   return 1.;
}

/**
 * @brief Updates the pilot stats after mass change.
 *
 *    @param pilot Pilot to update his mass.
 */
void pilot_updateMass( Pilot *pilot )
{
   double factor;

   /* Recompute effective mass if something changed. */
   pilot->solid.mass = MAX( pilot->stats.mass_mod*pilot->ship->mass + pilot->stats.cargo_inertia*pilot->mass_cargo + pilot->mass_outfit, 0.);

   /* Set and apply limit. */
   factor = pilot_massFactor( pilot );
   pilot->accel   = factor * pilot->accel_base;
   pilot->turn    = factor * pilot->turn_base;
   pilot->speed   = factor * pilot->speed_base;

   /* limit the maximum speed if limiter is active */
   if (pilot_isFlag(pilot, PILOT_HASSPEEDLIMIT)) {
      pilot->speed = pilot->speed_limit - pilot->accel / 3.;
      /* Speed must never go negative. */
      if (pilot->speed < 0.) {
         /* If speed DOES go negative, we have to lower accel. */
         pilot->accel = 3. * pilot->speed_limit;
         pilot->speed = 0.;
      }
   }
   /* Need to recalculate electronic warfare mass change. */
   pilot_ewUpdateStatic( pilot );

   /* Update ship stuff. */
   if (pilot_isPlayer(pilot))
      gui_setShip();
}

/**
 * @brief Checks to see if a slot has an active outfit that can be toggleable.
 *
 *    @param o Outfit slot to check.
 *    @return 1 if can toggle, 0 otherwise.
 */
int pilot_slotIsToggleable( const PilotOutfitSlot *o )
{
   const Outfit *oo;
   if (!o->active)
      return 0;

   oo = o->outfit;
   if (oo == NULL)
      return 0;
   if (!outfit_isToggleable(oo))
      return 0;

   return 1;
}

/**
 * @brief Wrapper that does all the work for us.
 */
static void pilot_outfitLRun( Pilot *p, void (*const func)( const Pilot *p, PilotOutfitSlot *po, const void *data ), const void *data )
{
   pilotoutfit_modified = 0;
   for (int i=0; i<array_size(p->outfits); i++) {
      PilotOutfitSlot *po = p->outfits[i];
      if (po->outfit==NULL)
         continue;
      func( p, po, data );
   }
   for (int i=0; i<array_size(p->outfit_intrinsic); i++) {
      PilotOutfitSlot *po = &p->outfit_intrinsic[i];
      if (po->outfit==NULL)
         continue;
      func( p, po, data );
   }
   /* Recalculate if anything changed. */
   if (pilotoutfit_modified)
      pilot_calcStats( p );
}
static void outfitLRunWarning( const Pilot *p, const Outfit *o, const char *name, const char *error )
{
   WARN( _("Pilot '%s''s outfit '%s' -> '%s':\n%s"), p->name, o->name, name, error );
}

/**
 * @brief Sets up the outfit memory for a slot.
 */
static int pilot_outfitLmem( PilotOutfitSlot *po, nlua_env env )
{
   int oldmem;
   /* Create the memory if necessary and initialize stats. */
   if (po->lua_mem == LUA_NOREF) {
      lua_newtable(naevL); /* mem */
      po->lua_mem = luaL_ref(naevL,LUA_REGISTRYINDEX); /* */
   }
   /* Get old memory. */
   nlua_getenv( naevL, env, "mem" ); /* oldmem */
   oldmem = luaL_ref( naevL, LUA_REGISTRYINDEX ); /* */
   /* Set the memory. */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->lua_mem); /* mem */
   nlua_setenv( naevL, env, "mem" ); /* */
   return oldmem;
}

/**
 * @brief Cleans up the outfit memory for a slot.
 */
static void pilot_outfitLunmem( nlua_env env, int oldmem )
{
   lua_rawgeti( naevL, LUA_REGISTRYINDEX, oldmem );
   nlua_setenv( naevL, env, "mem"); /* pm */
   luaL_unref( naevL, LUA_REGISTRYINDEX, oldmem );
}

static const char* pilot_outfitLDescExtra( const Pilot *p, const Outfit *o )
{
   static char descextra[STRMAX];
   const char *de;
   if (o->lua_descextra == LUA_NOREF)
      return o->desc_extra;

   /* Set up the function: init( p, po ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, o->lua_descextra); /* f */
   if ((p != NULL) && (p->id > 0)) /* Needs valid ID. */
      lua_pushpilot( naevL, p->id ); /* f, p */
   else
      lua_pushnil( naevL ); /* f, p */
   lua_pushoutfit( naevL, o ); /* f, p, o */
   if (nlua_pcall( o->lua_env, 2, 1 )) { /* */
      outfitLRunWarning( p, o, "descextra", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
      descextra[0] = '\0';
      return descextra;
   }
   de = luaL_checkstring( naevL, -1 );
   strncpy( descextra, de, sizeof(descextra)-1 );
   lua_pop( naevL, 1 );
   return descextra;
}

/**
 * @brief Gets the description of an outfit for a given pilot.
 *
 * Note: the returned string can get overwritten by subsequent calls.
 *
 *    @param p Pilot to get the outfit description of (or NULL for no pilot).
 *    @param o Outfit to get description of.
 *    @return The description of the outfit.
 */
const char* pilot_outfitDescription( const Pilot *p, const Outfit *o )
{
   static char o_description[STRMAX];
   const char *de = pilot_outfitLDescExtra( p, o );
   if (de == NULL)
      return _(o->desc_raw);
   snprintf( o_description, sizeof(o_description), "%s\n%s", _(o->desc_raw), de );
   return o_description;

}

/**
 * @brief Gets the summary of an outfit for a give pilot.
 *
 * Note: the returned string can get overwritten by subsequent calls.
 *
 *    @param p Pilot to get the outfit summary of (or NULL for no pilot).
 *    @param o Outfit to get summary of.
 *    @param withname Whether or not to show the name too.
 *    @return The summary of the outfit.
 */
const char* pilot_outfitSummary( const Pilot *p, const Outfit *o, int withname )
{
   static char o_summary[STRMAX];
   const char *de = pilot_outfitLDescExtra( p, o );
   if (de == NULL) {
      if (withname)
         snprintf( o_summary, sizeof(o_summary), "%s\n%s", _(o->name), o->summary_raw );
      else
         snprintf( o_summary, sizeof(o_summary), "%s", o->summary_raw );
   }
   else {
      if (withname)
         snprintf( o_summary, sizeof(o_summary), "%s\n%s\n%s", _(o->name), o->summary_raw, de );
      else
         snprintf( o_summary, sizeof(o_summary), "%s\n%s", o->summary_raw, de );
   }
   return o_summary;
}

/**
 * @brief Runs the pilot's Lua outfits init script.
 *
 *    @param pilot Pilot to run Lua outfits for.
 */
void pilot_outfitLInitAll( Pilot *pilot )
{
   pilotoutfit_modified = 0;
   for (int i=0; i<array_size(pilot->outfits); i++)
      pilot_outfitLInit( pilot, pilot->outfits[i] );
   for (int i=0; i<array_size(pilot->outfit_intrinsic); i++)
      pilot_outfitLInit( pilot, &pilot->outfit_intrinsic[i] );
   /* Recalculate if anything changed. */
   if (pilotoutfit_modified)
      pilot_calcStats( pilot );
}

/**
 * @brief Outfit is added to a ship.
 */
int pilot_outfitLAdd( Pilot *pilot, PilotOutfitSlot *po )
{
   int oldmem;

   if (po->outfit==NULL)
      return 0;
   if (po->outfit->lua_onadd == LUA_NOREF)
      return 0;

   /* Create the memory if necessary and initialize stats. */
   oldmem = pilot_outfitLmem( po, po->outfit->lua_env );

   /* Set up the function: init( p, po ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_onadd); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po); /* f, p, po */
   if (nlua_pcall( po->outfit->lua_env, 2, 0 )) { /* */
      outfitLRunWarning( pilot, po->outfit, "onadd", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
      pilot_outfitLunmem( po->outfit->lua_env, oldmem );
      return -1;
   }
   pilot_outfitLunmem( po->outfit->lua_env, oldmem );
   return 1;
}

/**
 * @brief Outfit is removed froma ship.
 */
int pilot_outfitLRemove( Pilot *pilot, PilotOutfitSlot *po )
{
   int oldmem;

   if (po->outfit==NULL)
      return 0;
   if (po->outfit->lua_onremove == LUA_NOREF)
      return 0;

   /* Create the memory if necessary and initialize stats. */
   oldmem = pilot_outfitLmem( po, po->outfit->lua_env );

   /* Set up the function: init( p, po ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_onremove); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po); /* f, p, po */
   if (nlua_pcall( po->outfit->lua_env, 2, 0 )) { /* */
      outfitLRunWarning( pilot, po->outfit, "onremove", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
      pilot_outfitLunmem( po->outfit->lua_env, oldmem );
      return -1;
   }
   pilot_outfitLunmem( po->outfit->lua_env, oldmem );
   return 1;
}

/**
 * @brief Runs the pilot's Lua outfits init script for an outfit.
 *
 *    @param pilot Pilot to run Lua outfits for.
 *    @param po Pilot outfit to check.
 *    @return 0 if nothing was done, 1 if script was run, and -1 on error.
 */
int pilot_outfitLInit( Pilot *pilot, PilotOutfitSlot *po )
{
   int lua_init, oldmem;
   nlua_env lua_env;

   if (po->outfit==NULL)
      return 0;

   if (po->outfit->lua_env==LUA_NOREF)
      return 0;

   lua_init = po->outfit->lua_init;
   lua_env = po->outfit->lua_env;

   /* Create the memory if necessary and initialize stats. */
   oldmem = pilot_outfitLmem( po, lua_env );

   if (lua_init == LUA_NOREF) {
      pilot_outfitLunmem( po->outfit->lua_env, oldmem );
      return 0;
   }

   /* Set up the function: init( p, po ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, lua_init); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po); /* f, p, po */
   if (nlua_pcall( lua_env, 2, 0 )) { /* */
      outfitLRunWarning( pilot, po->outfit, "init", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
      pilot_outfitLunmem( po->outfit->lua_env, oldmem );
      return -1;
   }
   pilot_outfitLunmem( po->outfit->lua_env, oldmem );
   return 1;
}

static void outfitLUpdate( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   double dt;
   int oldmem;
   if (po->outfit->lua_update == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;

   /* The data. */
   dt = *(double*)data;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: update( p, po, dt ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_update); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   lua_pushnumber(naevL, dt);       /* f, p, po, dt */
   if (nlua_pcall( env, 3, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "update", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Runs the pilot's Lua outfits update script.
 *
 *    @param pilot Pilot to run Lua outfits for.
 *    @param dt Delta-tick from last time it was run.
 */
void pilot_outfitLUpdate( Pilot *pilot, double dt )
{
   if (!pilot->outfitlupdate)
      return;
   pilot_outfitLRun( pilot, outfitLUpdate, &dt );
}

static void outfitLOutofenergy( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   (void) data;
   int oldmem;

   if (po->outfit->lua_outofenergy == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: outofenergy( p, po ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_outofenergy); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   if (nlua_pcall( env, 2, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "outofenergy", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Handles when the pilot runs out of energy.
 *
 *    @param pilot Pilot that ran out of energy.
 */
void pilot_outfitLOutfofenergy( Pilot *pilot )
{
   pilot_outfitLRun( pilot, outfitLOutofenergy, NULL );
}

struct OnhitData {
   double armour;
   double shield;
   unsigned int attacker;
};
static void outfitLOnhit( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   double armour, shield;
   unsigned int attacker;
   const struct OnhitData *odat;
   int oldmem;

   if (po->outfit->lua_onhit == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;

   /* Data. */
   odat = (const struct OnhitData*)data;
   armour = odat->armour;
   shield = odat->shield;
   attacker = odat->attacker;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: onhit( p, po, armour, shield ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_onhit); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   lua_pushnumber(naevL, armour );  /* f, p, po, a */
   lua_pushnumber(naevL, shield );  /* f, p, po, a, s */
   lua_pushpilot(naevL, attacker);  /* f, p, po, a, s, attacker */
   if (nlua_pcall( env, 5, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "onhit", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Runs the pilot's Lua outfits onhit script.
 *
 *    @param pilot Pilot to run Lua outfits for.
 *    @param armour Armour amage taken by pilot.
 *    @param shield Shield amage taken by pilot.
 *    @param attacker The attacker that hit the pilot.
 */
void pilot_outfitLOnhit( Pilot *pilot, double armour, double shield, unsigned int attacker )
{
   const struct OnhitData data = { .armour = armour, .shield = shield, .attacker = attacker };
   pilot_outfitLRun( pilot, outfitLOnhit, &data );
}

/**
 * @brief Handle the manual toggle of an outfit.
 *
 *    @param pilot Pilot to toggle outfit of.
 *    @param po Outfit to be toggling.
 *    @param on Whether to toggle on or off.
 *    @return 1 if was able to toggle it, 0 otherwise.
 */
int pilot_outfitLOntoggle( Pilot *pilot, PilotOutfitSlot *po, int on )
{
   nlua_env env = po->outfit->lua_env;
   int ret, oldmem;
   pilotoutfit_modified = 0;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: ontoggle( p, po, armour, shield ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_ontoggle); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   lua_pushboolean(naevL, on);      /* f, p, po, on */
   if (nlua_pcall( env, 3, 1 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "ontoggle", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
      pilot_outfitLunmem( env, oldmem );
      return 0;
   }

   /* Handle return boolean. */
   ret = lua_toboolean(naevL, -1);
   lua_pop(naevL, 1);
   pilot_outfitLunmem( env, oldmem );
   return ret || pilotoutfit_modified; /* Even if the script says it didn't change, it may have been modified. */
}

struct CooldownData {
   int done;
   int success;
   double timer;
};
static void outfitLCooldown( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   int done, success, oldmem;
   double timer;
   const struct CooldownData *cdat;

   if (po->outfit->lua_cooldown == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;

   cdat = (const struct CooldownData*) data;
   done = cdat->done;
   success = cdat->success;
   timer = cdat->timer;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: cooldown( p, po, done, success/timer ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_cooldown); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   lua_pushboolean(naevL, done); /* f, p, po, done */
   if (done)
      lua_pushboolean(naevL, success); /* f, p, po, done, success */
   else
      lua_pushnumber(naevL, timer); /* f, p, po, done, timer */
   if (nlua_pcall( env, 4, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "cooldown", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Handle cooldown hooks for outfits.
 *
 *    @param pilot Pilot being handled.
 *    @param done Whether or not cooldown is starting or done.
 *    @param success Whether or not it completed successfully.
 *    @param timer How much time is necessary to cooldown. Only used if done is false.
 */
void pilot_outfitLCooldown( Pilot *pilot, int done, int success, double timer )
{
   const struct CooldownData data = { .done = done, .success = success, .timer = timer };
   pilot_outfitLRun( pilot, outfitLCooldown, &data );
}

static void outfitLOnshoot( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   (void) data;
   int oldmem;
   if (po->outfit->lua_onshoot == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: onshoot( p, po ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_onshoot); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   if (nlua_pcall( env, 2, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "onshoot", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Runs the pilot's Lua outfits onshoot script.
 *
 *    @param pilot Pilot to run Lua outfits for.
 */
void pilot_outfitLOnshoot( Pilot *pilot )
{
   pilot_outfitLRun( pilot, outfitLOnshoot, NULL );
}

static void outfitLOnstealth( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   (void) data;
   int oldmem;
   if (po->outfit->lua_onstealth == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: onstealth( p, po, stealthed ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_onstealth); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   lua_pushboolean(naevL, pilot_isFlag(pilot,PILOT_STEALTH) ); /* f, p, po, steathed */
   if (nlua_pcall( env, 3, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "onstealth", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Runs the pilot's Lua outfits onhit script.
 *
 *    @param pilot Pilot to run Lua outfits for.
 *    @return 1 if ship stats were recalculated.
 */
int pilot_outfitLOnstealth( Pilot *pilot )
{
   pilot_outfitLRun( pilot, outfitLOnstealth, NULL );
   return pilotoutfit_modified;
}

static void outfitLOnscan( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   (void) data;
   int oldmem;
   if (po->outfit->lua_onscan == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: onscan( p, po, target ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_onscan); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   lua_pushpilot(naevL, pilot->target); /* f, p, po, t */
   if (nlua_pcall( env, 3, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "onscan", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Runs Lua outfits when pilot scanned their target.
 *
 *    @param pilot Pilot being handled.
 */
void pilot_outfitLOnscan( Pilot *pilot )
{
   pilot_outfitLRun( pilot, outfitLOnscan, NULL );
}

static void outfitLOnscanned( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   const Pilot *scanner;
   int oldmem;
   if (po->outfit->lua_onscanned == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;
   scanner = (const Pilot*) data;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: onscanned( p, po, stealthed ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_onscanned); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   lua_pushpilot(naevL, scanner->id); /* f, p, po, scanner */
   if (nlua_pcall( env, 3, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "onscanned", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Runs Lua outfits when pilot was scanned by scanner.
 *
 *    @param pilot Pilot being handled.
 *    @param scanner Pilot that scanned the pilot.
 */
void pilot_outfitLOnscanned( Pilot *pilot, const Pilot *scanner )
{
   pilot_outfitLRun( pilot, outfitLOnscanned, scanner );
}

static void outfitLOnland( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   (void) data;
   int oldmem;
   if (po->outfit->lua_land == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: land( p, po ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_land); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   if (nlua_pcall( env, 2, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "land", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Runs Lua outfits when pilot lands on a spob.
 *
 *    @param pilot Pilot being handled.
 */
void pilot_outfitLOnland( Pilot *pilot )
{
   pilot_outfitLRun( pilot, outfitLOnland, NULL );
}

static void outfitLOntakeoff( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   (void) data;
   int oldmem;
   if (po->outfit->lua_takeoff == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: takeoff( p, po ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_takeoff); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   if (nlua_pcall( env, 2, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "takeoff", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Runs Lua outfits when pilot takes off from a spob.
 *
 *    @param pilot Pilot being handled.
 */
void pilot_outfitLOntakeoff( Pilot *pilot )
{
   pilot_outfitLRun( pilot, outfitLOntakeoff, NULL );
}

static void outfitLOnjumpin( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   (void) data;
   int oldmem;
   if (po->outfit->lua_jumpin == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: takeoff( p, po ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_jumpin); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   if (nlua_pcall( env, 2, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "jumpin", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Runs Lua outfits when pilot jumps into a system.
 *
 *    @param pilot Pilot being handled.
 */
void pilot_outfitLOnjumpin( Pilot *pilot )
{
   pilot_outfitLRun( pilot, outfitLOnjumpin, NULL );
}

static void outfitLOnboard( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   const Pilot *target;
   int oldmem;
   if (po->outfit->lua_board == LUA_NOREF)
      return;

   nlua_env env = po->outfit->lua_env;
   target = (const Pilot*) data;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: board( p, po, stealthed ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_board); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   lua_pushpilot(naevL, target->id); /* f, p, po, target */
   if (nlua_pcall( env, 3, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "board", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
/**
 * @brief Runs Lua outfits when pilot boards a target.
 *
 *    @param pilot Pilot being handled.
 *    @param target Pilot being boarded.
 */
void pilot_outfitLOnboard( Pilot *pilot, const Pilot *target )
{
   pilot_outfitLRun( pilot, outfitLOnboard, target );
}

static const char *outfitkeytostr( OutfitKey key )
{
   switch (key) {
      case OUTFIT_KEY_ACCEL:
         return "accel";
      case OUTFIT_KEY_LEFT:
         return "left";
      case OUTFIT_KEY_RIGHT:
         return "right";
   }
   return NULL;
}

static void outfitLOnkeydoubletap( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   int oldmem;
   OutfitKey key;
   if (po->outfit->lua_keydoubletap == LUA_NOREF)
      return;
   key = *((const OutfitKey*) data);

   nlua_env env = po->outfit->lua_env;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: takeoff( p, po ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_keydoubletap); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   lua_pushstring(naevL, outfitkeytostr(key) );
   if (nlua_pcall( env, 3, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "keydoubletap", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
void pilot_outfitLOnkeydoubletap( Pilot *pilot, OutfitKey key )
{
   pilot_outfitLRun( pilot, outfitLOnkeydoubletap, &key );
}

static void outfitLOnkeyrelease( const Pilot *pilot, PilotOutfitSlot *po, const void *data )
{
   int oldmem;
   OutfitKey key;
   if (po->outfit->lua_keyrelease == LUA_NOREF)
      return;
   key = *((const OutfitKey*) data);

   nlua_env env = po->outfit->lua_env;

   /* Set the memory. */
   oldmem = pilot_outfitLmem( po, env );

   /* Set up the function: takeoff( p, po ) */
   lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_keyrelease); /* f */
   lua_pushpilot(naevL, pilot->id); /* f, p */
   lua_pushpilotoutfit(naevL, po);  /* f, p, po */
   lua_pushstring(naevL, outfitkeytostr(key) );
   if (nlua_pcall( env, 3, 0 )) {   /* */
      outfitLRunWarning( pilot, po->outfit, "keyrelease", lua_tostring(naevL,-1) );
      lua_pop(naevL, 1);
   }
   pilot_outfitLunmem( env, oldmem );
}
void pilot_outfitLOnkeyrelease( Pilot *pilot, OutfitKey key )
{
   pilot_outfitLRun( pilot, outfitLOnkeyrelease, &key );
}

/**
 * @brief Handle cleanup hooks for outfits.
 *
 *    @param pilot Pilot being handled.
 */
void pilot_outfitLCleanup( Pilot *pilot )
{
   /* TODO we might want to run this on intrinsic outfits too... */
   pilotoutfit_modified = 0;
   for (int i=0; i<array_size(pilot->outfits); i++) {
      int oldmem;
      PilotOutfitSlot *po = pilot->outfits[i];
      if (po->outfit==NULL)
         continue;
      if (po->outfit->lua_cleanup == LUA_NOREF)
         continue;
      /* Pilot could be created and then erased without getting properly
       * initialized. */
      if (po->lua_mem == LUA_NOREF)
         continue;

      nlua_env env = po->outfit->lua_env;

      /* Set the memory. */
      oldmem = pilot_outfitLmem( po, env );

      /* Set up the function: cleanup( p, po ) */
      lua_rawgeti(naevL, LUA_REGISTRYINDEX, po->outfit->lua_cleanup); /* f */
      lua_pushpilot(naevL, pilot->id); /* f, p */
      lua_pushpilotoutfit(naevL, po);  /* f, p, po */
      if (nlua_pcall( env, 2, 0 )) {   /* */
         outfitLRunWarning( pilot, po->outfit, "cleanup", lua_tostring(naevL,-1) );
         lua_pop(naevL, 1);
      }
      pilot_outfitLunmem( env, oldmem );
   }
   /* Pilot gets cleaned up so no need to recalculate stats. */
}
