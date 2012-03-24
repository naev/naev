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
      pilot_setFlag(pilot, PILOT_HASTURRET);

   if (outfit_isBeam(outfit)) { /* Used to speed up some calculations. */
      s->u.beamid = -1;
      pilot_setFlag(pilot, PILOT_HASBEAMS);
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
 *    @return The reason why the pilot is not sane (or NULL if sane).
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
 *    @param o Outfit to check.
 *    @param add Whether or not to consider it's being added or removed.
 *    @return NULL if can swap, or error message if can't.
 */
const char* pilot_canEquip( Pilot *p, PilotOutfitSlot *s, Outfit *o, int add )
{
   /* Just in case. */
   if ((p==NULL) || (o==NULL))
      return "Nothing selected.";

   /* Check slot type. */
   if ((s != NULL) && !outfit_fitsSlot( o, &s->slot ))
      return "Does not fit slot.";

   /* Adding outfit. */
   if (add) {
      if ((outfit_cpu(o) > 0) && (p->cpu < outfit_cpu(o)))
         return "Insufficient CPU";

      /* Can't add more than one outfit of the same type if the outfit type is limited. */
      if ((o->limit != NULL) && pilot_hasOutfitLimit( p, o->limit ))
         return "Already have an outfit of this type installed";

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
         if ((s!=NULL) && (s->u.ammo.deployed > 0))
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

   buf = malloc(sizeof(char)*len);
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
   double arel, srel, erel; /* relative health bonuses. */
   ShipStats amount, *s;

   /* @TODO remove old school PILOT_AFTERBURN flags. */
   pilot_rmFlag( pilot, PILOT_AFTERBURNER );

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
   /* crew */
   pilot->crew          = pilot->ship->crew;
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

   /* cargo has to be reset */
   pilot_cargoCalc(pilot);

   /* Slot voodoo. */

   /*
    * now add outfit changes
    */
   pilot->mass_outfit   = 0.;
   pilot->jamming       = 0;
   arel                 = 0.;
   srel                 = 0.;
   erel                 = 0.;
   for (i=0; i<pilot->noutfits; i++) {
      slot = pilot->outfits[i];
      o    = slot->outfit;

      /* Outfit must exist. */
      if (o==NULL)
         continue;

      /* Subtract CPU. */
      pilot->cpu           -= outfit_cpu(o);
      if (outfit_cpu(o) < 0.)
         pilot->cpu_max    -= outfit_cpu(o);

      /* Add mass. */
      pilot->mass_outfit   += o->mass;

      /* Add ammo mass. */
      if (outfit_ammo(o) != NULL)
         if (slot->u.ammo.outfit != NULL)
            pilot->mass_outfit += slot->u.ammo.quantity * slot->u.ammo.outfit->mass;

      /* Set afterburner. */
      if (outfit_isAfterburner(o))
         pilot->afterburner = pilot->outfits[i];

      /* Active outfits must be on to affect stuff. */
      if (slot->active && !(slot->state==PILOT_OUTFIT_ON))
         continue;

      if (outfit_isMod(o)) { /* Modification */
         /* movement */
         pilot->thrust        += o->u.mod.thrust * pilot->ship->mass;
         pilot->thrust        += o->u.mod.thrust_rel * pilot->ship->thrust;
         pilot->turn_base     += o->u.mod.turn;
         pilot->turn_base     += o->u.mod.turn_rel * pilot->ship->turn;
         pilot->speed         += o->u.mod.speed;
         pilot->speed         += o->u.mod.speed_rel * pilot->ship->speed;
         /* health */
         pilot->armour_max    += o->u.mod.armour;
         pilot->armour_regen  += o->u.mod.armour_regen;
         arel                 += o->u.mod.armour_rel;
         pilot->shield_max    += o->u.mod.shield;
         pilot->shield_regen  += o->u.mod.shield_regen;
         srel                 += o->u.mod.shield_rel;
         pilot->energy_max    += o->u.mod.energy;
         pilot->energy_regen  += o->u.mod.energy_regen;
         erel                 += o->u.mod.energy_rel;
         /* fuel */
         pilot->fuel_max      += o->u.mod.fuel;
         /* misc */
         pilot->cargo_free    += o->u.mod.cargo;
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
         pilot->solid->speed_max = pilot->speed +
               pilot->speed * pilot->afterburner->outfit->u.afb.speed *
               MIN( 1., pilot->afterburner->outfit->u.afb.mass_limit/pilot->solid->mass);
      }
      else if (outfit_isJammer(o)) { /* Jammer */
         pilot->jamming        = 1;
         pilot->energy_loss   += o->u.jam.energy;
      }
   }

   if (!pilot_isFlag( pilot, PILOT_AFTERBURNER ))
      pilot->solid->speed_max = pilot->speed;

   /* Set final energy tau. */
   pilot->energy_tau = pilot->energy_max / pilot->energy_regen;

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

   /* Increase health by relative bonuses. */
   pilot->armour_max += arel * pilot->ship->armour;
   pilot->armour_max *= s->armour_mod;
   pilot->shield_max += srel * pilot->ship->shield;
   pilot->shield_max *= s->shield_mod;
   pilot->energy_max += erel * pilot->ship->energy;
   /* pilot->energy_max *= s->energy_mod; */

   /* Give the pilot his health proportion back */
   pilot->armour = ac * pilot->armour_max;
   pilot->shield = sc * pilot->shield_max;
   pilot->energy = ec * pilot->energy_max;
   pilot->fuel   = fc * pilot->fuel_max;

   /* Calculate the heat. */
   pilot_heatCalc( pilot );

   /* Modulate by mass. */
   pilot_updateMass( pilot );

   /* Update GUI as necessary. */
   gui_setGeneric( pilot );
}


/**
 * @brief Updates the pilot stats after mass change.
 *
 *    @param pilot Pilot to update his mass.
 */
void pilot_updateMass( Pilot *pilot )
{
   /* Calculate mass. */
   pilot->solid->mass = pilot->ship->mass + pilot->stats.cargo_inertia*pilot->mass_cargo + pilot->mass_outfit;

   pilot->turn = pilot->turn_base * pilot->ship->mass / pilot->solid->mass;
   /* Need to recalculate electronic warfare mass change. */
   pilot_ewUpdateStatic( pilot );
}


