/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot_outfit.c
 *
 * @brief Handles pilot outfits.
 */


#include "pilot.h"

#include "naev.h"

#include "nxml.h"

#include "log.h"
#include "player.h"
#include "space.h"
#include "gui.h"
#include "nstring.h"


/*
 * Prototypes.
 */
static int pilot_hasOutfitLimit( Pilot *p, const char *limit );


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
         x     = t->solid->pos.x - p->solid->pos.x;
         y     = t->solid->pos.y - p->solid->pos.y;
         ang   = ANGLE( x, y );
         *a    = fabs( angle_diff( ang, p->solid->dir ) );
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
      /* Compensate for enemy hide factor. */
      o->u.ammo.lockon_timer -= dt * (o->outfit->u.lau.ew_target/t->ew_hide);

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
   int i;
   PilotOutfitSlot *o;

   for (i=0; i<p->noutfits; i++) {
      o = p->outfits[i];
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
   vect_cset( v, x, y );

   return 0;
}


/**
 * @brief Docks the pilot on its target pilot.
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
   Outfit *o;

   /* Set the outfit. */
   s->outfit   = outfit;

   /* Set some default parameters. */
   s->timer    = 0.;

   /* Some per-case scenarios. */
   if (outfit_isFighterBay(outfit)) {
      s->u.ammo.outfit   = NULL;
      s->u.ammo.quantity = 0;
      s->u.ammo.deployed = 0;
   }
   if (outfit_isTurret(outfit)) /* used to speed up AI */
      pilot->nturrets++;
   else if (outfit_isBolt(outfit))
      pilot->ncannons++;

   if (outfit_isBeam(outfit)) { /* Used to speed up some calculations. */
      s->u.beamid = -1;
      pilot->nbeams++;
   }
   if (outfit_isLauncher(outfit)) {
      s->u.ammo.outfit   = NULL;
      s->u.ammo.quantity = 0;
      s->u.ammo.deployed = 0; /* Just in case. */
   }

   /* Check if active. */
   o = s->outfit;
   s->active = outfit_isActive(o);

   /* Update heat. */
   pilot_heatCalcSlot( s );

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
   else if ((str = pilot_canEquip( pilot, s, outfit)) != NULL) {
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

   /* Decrement counters if necessary. */
   if (s->outfit != NULL) {
      if (outfit_isTurret(s->outfit))
         pilot->nturrets--;
      else if (outfit_isBolt(s->outfit))
         pilot->ncannons--;
      if (outfit_isBeam(s->outfit))
         pilot->nbeams--;
   }

   /* Remove the outfit. */
   ret         = (s->outfit==NULL);
   s->outfit   = NULL;

   /* Remove secondary and such if necessary. */
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

   str = pilot_canEquip( pilot, s, NULL );
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
//TODO: fix comment to conform to Naev's style and represent changes
/**
 * @brief Pilot slot sanity check - makes sure stats are sane.
 *
 *    @param p Pilot to check.
 *    @return 0 if a slot doesn't fit, !0 otherwise.
 */
int pilot_slotsCheckSanity( Pilot *p )
{
   int i;
   for (i=0; i<p->noutfits; i++)
      if ((p->outfits[i]->outfit != NULL) &&
            !outfit_fitsSlot( p->outfits[i]->outfit, &p->outfits[i]->sslot->slot ))
         return 0;
   return 1;
}
//TODO: fix comment to conform to Naev's style and represent changes
/**
 * @brief Pilot required (core) slot filled check - makes sure they are filled.
 *
 *    @param p Pilot to check.
 *    @return 0 if a slot is missing, !0 otherwise.
 */
int pilot_slotsCheckRequired( Pilot *p )
{
   int i;

   for (i=0; i < p->outfit_nstructure; i++)
      if (p->outfit_structure[i].sslot->required && p->outfit_structure[i].outfit == NULL)
         return 0;

   for (i=0; i < p->outfit_nutility; i++)
      if (p->outfit_utility[i].sslot->required && p->outfit_utility[i].outfit == NULL)
         return 0;

   for (i=0; i < p->outfit_nweapon; i++)
      if (p->outfit_weapon[i].sslot->required && p->outfit_weapon[i].outfit == NULL)
         return 0;

   return 1;
}
//TODO: fix comment to conform to Naev's style and represent change
/**
 * @brief Pilot sanity check - makes sure stats are sane.
 *
 *    @param p Pilot to check.
 *    @return The reason why the pilot is not sane (or NULL if sane).
 */
const char* pilot_checkSpaceworthy( Pilot *p )
{
   if (!pilot_slotsCheckSanity(p))
      return "Doesn't fit slot";

   /* CPU. */
   if (p->cpu < 0)
      return "Insufficient CPU";

   /* Movement. */
   if (p->thrust < 0.)
      return "Insufficient Thrust";
   if (p->speed < 0.)
      return "Insufficient Speed";
   if (p->turn < 0.)
      return "Insufficient Turn";

   /* Health. */
   if (p->armour_max < 0.)
      return "Insufficient Armour";
   if (p->armour_regen < 0.)
      return "Insufficient Armour Regeneration";
   if (p->shield_max < 0.)
      return "Insufficient Shield";
   if (p->shield_regen < 0.)
      return "Insufficient Shield Regeneration";
   if (p->energy_max < 0.)
      return "Insufficient Energy";
   if (p->energy_regen < 0.)
      return "Insufficient Energy Regeneration";

   /* Misc. */
   if (p->fuel_max < 0.)
      return "Insufficient Fuel Maximum";
   if (p->fuel_consumption < 0.)
      return "Insufficient Fuel Consumption";
   if (p->cargo_free < 0)
      return "Insufficient Free Cargo Space";
   
   /* Core Slots */
   if (!pilot_slotsCheckRequired(p))
      return "Not All Core Slots are equipped";

   /* All OK. */
   return NULL;
}
/**
 * @brief Pilot sanity report - makes sure stats are sane.
 *
 *    @param p Pilot to check.
 *    @param buf Buffer to fill.
 *    @param bufSize Size of the buffer.
 *    @return Number of issues encountered.
 */
#define SPACEWORTHY_CHECK(cond,msg) \
if (cond){ ret++; \
   if (pos < bufSize) pos += snprintf( &buf[pos], bufSize-pos, (msg) ); }
int pilot_reportSpaceworthy( Pilot *p, char buf[], int bufSize )
{
   int pos = 0;
   int ret = 0;

   /* Core Slots */
   SPACEWORTHY_CHECK( !pilot_slotsCheckRequired(p), "Not All Core Slots are equipped\n" );
   /* CPU. */
   SPACEWORTHY_CHECK( p->cpu < 0, "Insufficient CPU\n" );

   /* Movement. */
   SPACEWORTHY_CHECK( p->thrust < 0, "Insufficient Thrust\n" );
   SPACEWORTHY_CHECK( p->speed < 0,  "Insufficient Speed\n" );
   SPACEWORTHY_CHECK( p->turn < 0,   "Insufficient Turn\n" );

   /* Health. */
   SPACEWORTHY_CHECK( p->armour < 0.,       "Insufficient Armour\n" );
   SPACEWORTHY_CHECK( p->armour_regen < 0., "Insufficient Armour Regeneration\n" );
   SPACEWORTHY_CHECK( p->shield < 0.,       "Insufficient Shield\n" );
   SPACEWORTHY_CHECK( p->shield_regen < 0., "Insufficient Shield Regeneration\n" );
   SPACEWORTHY_CHECK( p->energy_max < 0.,   "Insufficient Energy\n" );
   SPACEWORTHY_CHECK( p->energy_regen < 0., "Insufficient Energy Regeneration\n" );

   /* Misc. */
   SPACEWORTHY_CHECK( p->fuel_max < 0.,         "Insufficient Fuel Maximum\n" );
   SPACEWORTHY_CHECK( p->fuel_consumption < 0., "Insufficient Fuel Consumption\n" );
   SPACEWORTHY_CHECK( p->cargo_free < 0,        "Insufficient Free Cargo Space\n" );
   
   /*buffer is full, lets write that there is more then what's copied */
   if (pos > bufSize-1) {
      buf[bufSize-4]='.';
      buf[bufSize-3]='.';
      buf[bufSize-2]='.';
      /* buf[bufSize-1]='\0'; already done for us */
   }
   else {
      if (pos == 0)
         /*string is empty so no errors encountered */
         nsnprintf( buf, bufSize, "Spaceworthy");
      else
         /*string is not empty, so trunc the last newline */
         buf[pos-1]='\0';
   }

   return ret;
}
#undef SPACEWORTHY_CHECK

/**
 * @brief Checks to see if a pilot has an outfit with a specific outfit type.
 *
 *    @param p Pilot to check.
 *    @param t Outfit type to check.
 *    @return the amount of outfits of this type the pilot has.
 */
static int pilot_hasOutfitLimit( Pilot *p, const char *limit )
{
   int i;
   Outfit *o;
   for (i = 0; i<p->noutfits; i++) {
      o = p->outfits[i]->outfit;
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
const char* pilot_canEquip( Pilot *p, PilotOutfitSlot *s, Outfit *o )
{
   Outfit *o_old;
   const char *err;
   double pa, ps, pe, pf;

   /* Just in case. */
   if ((p==NULL) || (s==NULL))
      return "Nothing selected.";

   if (o!=NULL) {
      /* Check slot type. */
      if (!outfit_fitsSlot( o, &s->sslot->slot ))
         return "Does not fit slot.";
      /* Check outfit limit. */
      if ((o->limit != NULL) && pilot_hasOutfitLimit( p, o->limit ))
         return "Already have an outfit of this type installed";
   }
   else {
      /* Check fighter bay. */
      if ((o==NULL) && (s!=NULL) && (s->u.ammo.deployed > 0))
         return "Recall the fighters first";
   }

   /* Store health. */
   pa = p->armour;
   ps = p->shield;
   pe = p->energy;
   pf = p->fuel;

   /* Swap outfit. */
   o_old       = s->outfit;
   s->outfit   = o;

   /* Check sanity. */
   pilot_calcStats( p );
   /* can now equip outfit even if ship won't be spaceworthy
    * err = pilot_checkSpaceworthy( p );*/
   /* actually, this is also redundant */
   if (!pilot_slotsCheckSanity(p))
      err = "Does not fit in slot";
   else
      err = NULL;

   /* Swap back. */
   s->outfit   = o_old;

   /* Recalc. */
   pilot_calcStats( p );

   /* Recover health. */
   p->armour = pa;
   p->shield = ps;
   p->energy = pe;
   p->fuel   = pf;
   
   return err;
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
   int q, max;
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
   max                 = outfit_amount(s->outfit) - s->u.ammo.deployed;
   q                   = s->u.ammo.quantity; /* Amount have. */
   s->u.ammo.quantity += quantity;
   s->u.ammo.quantity  = MIN( max, s->u.ammo.quantity );
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
   (void) pilot;
   int q;

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
   if (s->u.ammo.outfit == NULL)
      return 0;

   /* Remove ammo. */
   q                   = MIN( quantity, s->u.ammo.quantity );
   s->u.ammo.quantity -= q;
   pilot->mass_outfit -= q * s->u.ammo.outfit->mass;
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
int pilot_countAmmo( Pilot* pilot )
{
   int nammo = 0, i;
   PilotOutfitSlot* po;
   Outfit* outfit;
   for (i=0; i<pilot->noutfits; i++) {
     po = pilot->outfits[i];
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
int pilot_maxAmmo( Pilot* pilot )
{
  int max = 0, i;
  PilotOutfitSlot* po;
  Outfit* outfit;
  for (i=0; i<pilot->noutfits; i++) {
     po = pilot->outfits[i];
     if (po == NULL)
        continue;
     outfit = po->outfit;
     if (outfit == NULL)
        continue;
     if (!outfit_isLauncher(outfit))
        continue;
     max += outfit->u.lau.amount;
  }
  return max;
}


/**
 * @brief Gets all the outfits in nice text form.
 *
 *    @param pilot Pilot to get the outfits from.
 *    @@return A list of all the outfits in a nice form.
 */
char* pilot_getOutfits( const Pilot* pilot )
{
   int i;
   char *buf;
   int p, len;

   len = 1024;

   buf = malloc(len);
   buf[0] = '\0';
   p = 0;
   for (i=1; i<pilot->noutfits; i++) {
      if (pilot->outfits[i]->outfit == NULL)
         continue;
      p += nsnprintf( &buf[p], len-p, (p==0) ? "%s" : ", %s",
            pilot->outfits[i]->outfit->name );
   }

   if (p==0)
      p += nsnprintf( &buf[p], len-p, "None" );

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
   Outfit* o;
   PilotOutfitSlot *slot;
   double ac, sc, ec, fc; /* temporary health coefficients to set */
   ShipStats amount, *s;

   /*
    * set up the basic stuff
    */
   /* mass */
   pilot->solid->mass   = pilot->ship->mass;
   /* cpu */
   pilot->cpu           = 0.;
   /* movement */
   pilot->thrust_base   = pilot->ship->thrust;
   pilot->turn_base     = pilot->ship->turn;
   pilot->speed_base    = pilot->ship->speed;
   /* crew */
   pilot->crew          = pilot->ship->crew;
   /* cargo */
   pilot->cap_cargo     = pilot->ship->cap_cargo;
   /* health */
   ac = (pilot->armour_max > 0.) ? pilot->armour / pilot->armour_max : 0.;
   sc = (pilot->shield_max > 0.) ? pilot->shield / pilot->shield_max : 0.;
   ec = (pilot->energy_max > 0.) ? pilot->energy / pilot->energy_max : 0.;
   fc = (pilot->fuel_max   > 0.) ? pilot->fuel   / pilot->fuel_max   : 0.;
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
   /* Stats. */ 
   s = &pilot->stats;
   memcpy( s, &pilot->ship->stats_array, sizeof(ShipStats) );
   memset( &amount, 0, sizeof(ShipStats) );

   /*
    * Now add outfit changes
    */
   pilot->mass_outfit   = 0.;
   pilot->jamming       = 0;
   for (i=0; i<pilot->noutfits; i++) {
      slot = pilot->outfits[i];
      o    = slot->outfit;

      /* Outfit must exist. */
      if (o==NULL)
         continue;

      /* Modify CPU. */
      pilot->cpu           += outfit_cpu(o);

      /* Add mass. */
      pilot->mass_outfit   += o->mass;

      if (outfit_isAfterburner(o)) { /* Afterburner */
         pilot->afterburner = pilot->outfits[i]; /* Set afterburner */
         continue;
      }

      /* Active outfits must be on to affect stuff. */
      if (slot->active && !(slot->state==PILOT_OUTFIT_ON))
         continue;

      if (outfit_isMod(o)) { /* Modification */
         /* Movement. */
         pilot->thrust_base   += o->u.mod.thrust;
         pilot->turn_base     += o->u.mod.turn;
         pilot->speed_base    += o->u.mod.speed;
         /* Health. */
         pilot->armour_max    += o->u.mod.armour;
         pilot->armour_regen  += o->u.mod.armour_regen;
         pilot->shield_max    += o->u.mod.shield;
         pilot->shield_regen  += o->u.mod.shield_regen;
         pilot->energy_max    += o->u.mod.energy;
         pilot->energy_regen  += o->u.mod.energy_regen;
         pilot->energy_loss   += o->u.mod.energy_loss;
         /* Fuel. */
         pilot->fuel_max      += o->u.mod.fuel;
         /* Misc. */
         pilot->cap_cargo     += o->u.mod.cargo;
         pilot->mass_outfit   += o->u.mod.mass_rel * pilot->ship->mass;
         pilot->crew          += o->u.mod.crew_rel * pilot->ship->crew;
         /*
          * Stats.
          */
         ss_statsModFromList( s, o->u.mod.stats, &amount );
     
      }
      else if (outfit_isAfterburner(o)) { /* Afterburner */
         pilot_setFlag( pilot, PILOT_AFTERBURNER ); /* We use old school flags for this still... */
         pilot->energy_loss += pilot->afterburner->outfit->u.afb.energy; /* energy loss */
      }
      else if (outfit_isJammer(o)) { /* Jammer */
         pilot->jamming        = 1;
         pilot->energy_loss   += o->u.jam.energy;
      }

      /* Add ammo mass. */
      if (outfit_ammo(o) != NULL) {
         if (slot->u.ammo.outfit != NULL)
            pilot->mass_outfit += slot->u.ammo.quantity * slot->u.ammo.outfit->mass;
      }
   }

   if (!pilot_isFlag( pilot, PILOT_AFTERBURNER ))
      pilot->solid->speed_max = pilot->speed;

   /* Set final energy tau. */
   pilot->energy_tau = pilot->energy_max / pilot->energy_regen;

   /* Slot voodoo. */
   s        = &pilot->stats;
   /* Fuel. */
   if (s->fuel_consumption == 0.)
      pilot->fuel_consumption = 100.;
   else
      pilot->fuel_consumption = s->fuel_consumption;
   /*
    * Electronic warfare setting base parameters.
    */
   s->ew_hide            = 1. + (s->ew_hide-1.) * exp( -0.2 * (double)(MAX(amount.ew_hide-1,0)) );
   s->ew_detect          = 1. + (s->ew_detect-1.) * exp( -0.2 * (double)(MAX(amount.ew_detect-1,0)) );
   s->ew_jump_detect     = 1. + (s->ew_jump_detect-1.) * exp( -0.2 * (double)(MAX(amount.ew_jump_detect-1,0)) );
   pilot->ew_base_hide   = s->ew_hide;
   pilot->ew_detect      = s->ew_detect;
   pilot->ew_jump_detect = s->ew_jump_detect;
   /* Fire rate:
    *  amount = p * exp( -0.15 * (n-1) )
    *  1x 15% -> 15%
    *  2x 15% -> 25.82%
    *  3x 15% -> 33.33%
    *  6x 15% -> 42.51%
    */
   if (amount.fwd_firerate > 0) {
      s->fwd_firerate = 1. + (s->fwd_firerate-1.) * exp( -0.15 * (double)(MAX(amount.fwd_firerate-1.,0)) );
   }
   /* Cruiser. */
   if (amount.tur_firerate > 0) {
      s->tur_firerate = 1. + (s->tur_firerate-1.) * exp( -0.15 * (double)(MAX(amount.tur_firerate-1.,0)) );
   }
   /*
    * Electronic warfare setting base parameters.
    * @TODO ew_hide and ew_detect should be squared so XML-sourced values are linear.
    */
   s->ew_hide           = 1. + (s->ew_hide-1.)        * exp( -0.2 * (double)(MAX(amount.ew_hide-1.,0)) );
   s->ew_detect         = 1. + (s->ew_detect-1.)      * exp( -0.2 * (double)(MAX(amount.ew_detect-1.,0)) );
   s->ew_jump_detect    = 1. + (s->ew_jump_detect-1.) * exp( -0.2 * (double)(MAX(amount.ew_jump_detect-1.,0)) );
   pilot->ew_base_hide  = s->ew_hide;
   pilot->ew_detect     = s->ew_detect;
   pilot->ew_jump_detect = pow2(s->ew_jump_detect);

   /*
    * Relative increases.
    */
   /* Movement. */
   pilot->thrust_base  *= s->thrust_mod;
   pilot->turn_base    *= s->turn_mod;
   pilot->speed_base   *= s->speed_mod;
   /* Health. */
   pilot->armour_max   *= s->armour_mod;
   pilot->armour_regen *= s->armour_regen_mod;
   pilot->shield_max   *= s->shield_mod;
   pilot->shield_regen *= s->shield_regen_mod;
   pilot->energy_max   *= s->energy_mod;
   pilot->energy_regen *= s->energy_regen_mod;
   /* cpu */
   pilot->cpu_max       = (pilot->ship->cpu + s->cpu_max)*s->cpu_mod;
   pilot->cpu          += pilot->cpu_max; /* CPU is negative, this just sets it so it's based off of cpu_max. */
   /* Misc. */
   pilot->dmg_absorb    = MAX( 0., pilot->dmg_absorb );
   pilot->crew         *= s->crew_mod;
   pilot->cap_cargo    *= s->cargo_mod;

   /*
    * Flat increases.
    */
   pilot->energy_max   += s->energy_flat;
   pilot->energy       += s->energy_flat;
   pilot->energy_regen += s->energy_regen_flat;

   /* Give the pilot his health proportion back */
   pilot->armour = ac * pilot->armour_max;
   pilot->shield = sc * pilot->shield_max;
   pilot->energy = ec * pilot->energy_max;
   pilot->fuel   = fc * pilot->fuel_max;

   /* Cargo has to be reset. */
   pilot_cargoCalc(pilot);

   /* Calculate mass. */
   pilot->solid->mass = s->mass_mod*pilot->ship->mass + pilot->stats.cargo_inertia*pilot->mass_cargo + pilot->mass_outfit;

   /* Calculate the heat. */
   pilot_heatCalc( pilot );

   /* Modulate by mass. */
   pilot_updateMass( pilot );

   /* Update GUI as necessary. */
   gui_setGeneric( pilot );
}


/**
 * @brief Cures the pilot as if he was landed.
 */
void pilot_healLanded( Pilot *pilot )
{
   pilot->armour = pilot->armour_max;
   pilot->shield = pilot->shield_max;
   pilot->energy = pilot->energy_max;
}


/**
 * @brief Updates the pilot stats after mass change.
 *
 *    @param pilot Pilot to update his mass.
 */
void pilot_updateMass( Pilot *pilot )
{
   double mass, factor;

   /* Set limit. */
   mass = pilot->solid->mass;
   if ((pilot->stats.engine_limit > 0.) && (mass > pilot->stats.engine_limit))
      factor = pilot->stats.engine_limit / mass;
   else
      factor = 1.;

   pilot->thrust  = factor * pilot->thrust_base * mass;
   pilot->turn    = factor * pilot->turn_base;
   pilot->speed   = factor * pilot->speed_base;

   /* Need to recalculate electronic warfare mass change. */
   pilot_ewUpdateStatic( pilot );
}


