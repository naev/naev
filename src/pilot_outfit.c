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

   /* Check if active. */
   o = s->outfit;
   if (outfit_isForward(o) || outfit_isTurret(o) || outfit_isLauncher(o) || outfit_isFighterBay(o))
      s->active = 1;
   else
      s->active = 0;

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

   /* Check slot type. */
   if ((s != NULL) && !outfit_fitsSlot( o, &s->slot ))
      return "Does not fit slot.";

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
   Outfit* o;
   PilotOutfitSlot *slot;
   double ac, sc, ec, fc; /* temporary health coeficients to set */
   double arel, srel, erel; /* relative health bonuses. */
   ShipStats *s, *os;
   int nfirerate_turret, nfirerate_forward;
   int njammers;
   int ew_ndetect, ew_nhide;

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
   /* Jamming */
   pilot->jam_range     = 0.;
   pilot->jam_chance    = 0.;
   /* Stats. */
   memcpy( s, &pilot->ship->stats, sizeof(ShipStats) );

   /* cargo has to be reset */
   pilot_cargoCalc(pilot);

   /*
    * now add outfit changes
    */
   nfirerate_forward = nfirerate_turret = 0;
   pilot->mass_outfit   = 0.;
   njammers             = 0;
   ew_ndetect           = 0;
   ew_nhide             = 0;
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
         pilot->crew          += o->u.mod.crew_rel * pilot->ship->crew * q;
         /*
          * Stats.
          */
         os = &o->u.mod.stats;
         /* Freighter. */
         s->jump_delay        += os->jump_delay * q;
         s->jump_range        += os->jump_range * q;
         s->cargo_inertia     += os->cargo_inertia * q;
         /* Scout. */
         if (os->ew_hide != 0.) {
            s->ew_hide           += os->ew_hide * q;
            ew_nhide++;
         }
         if (os->ew_detect != 0.) {
            s->ew_detect         += os->ew_detect * q;
            ew_ndetect++;
         }
         s->jam_range         += os->jam_range * q;
         /* Military. */
         s->heat_dissipation  += os->heat_dissipation * q;
         /* Bomber. */
         s->launch_rate       += os->launch_rate * q;
         s->launch_range      += os->launch_range * q;
         s->jam_counter       += os->jam_counter * q;
         s->ammo_capacity     += os->ammo_capacity * q;
         /* Fighter. */
         s->heat_forward      += os->heat_forward * q;
         s->damage_forward    += os->damage_forward * q;
         s->energy_forward    += os->energy_forward * q;
         if (os->firerate_forward != 0.) {
            s->firerate_forward  += os->firerate_forward * q;
            nfirerate_forward    += q;
         }
         /* Cruiser. */
         s->heat_turret       += os->heat_turret * q;
         s->damage_turret     += os->damage_turret * q;
         s->energy_turret     += os->energy_turret * q;
         if (os->firerate_turret != 0.) {
            s->firerate_turret   += os->firerate_turret * q;
            if (os->firerate_turret > 0.) /* Only modulate bonuses. */
               nfirerate_turret     += q;
         }
         /* Misc. */
         s->nebula_damage += os->nebula_damage * q;
      }
      else if (outfit_isAfterburner(o)) /* Afterburner */
         pilot->afterburner = pilot->outfits[i]; /* Set afterburner */
      else if (outfit_isJammer(o)) { /* Jammer */
         pilot->jam_range        += o->u.jam.range * q;
         pilot->jam_chance       += o->u.jam.chance * q;
         pilot->energy_regen     -= o->u.jam.energy * q;
         njammers                += q;;
      }

      /* Add ammo mass. */
      if (outfit_ammo(o) != NULL) {
         if (slot->u.ammo.outfit != NULL)
            pilot->mass_outfit += slot->u.ammo.quantity * slot->u.ammo.outfit->mass;
      }
   }

   /* Set final energy tau. */
   pilot->energy_tau = pilot->energy_max / pilot->energy_regen;

   /*
    * Electronic warfare setting base parameters.
    */
   s->ew_hide           = 1. + s->ew_hide/100. * exp( -0.2 * (double)(MAX(ew_nhide-1,0)) );
   s->ew_detect         = 1. + s->ew_detect/100. * exp( -0.2 * (double)(MAX(ew_ndetect-1,0)) );
   pilot->ew_base_hide  = s->ew_hide;
   pilot->ew_detect     = s->ew_detect;

   /*
    * Normalize stats.
    */
   /* Freighter. */
   s->jump_range        = s->jump_range/100. + 1.;
   s->jump_delay        = s->jump_delay/100. + 1.;
   s->cargo_inertia     = s->cargo_inertia/100. + 1.;
   /* Scout. */
   s->jam_range         = s->jam_range/100. + 1.;
   /* Military. */
   s->heat_dissipation  = s->heat_dissipation/100. + 1.;
   /* Bomber. */
   s->launch_rate       = s->launch_rate/100. + 1.;
   s->launch_range      = s->launch_range/100. + 1.;
   s->jam_counter       = s->jam_counter/100. + 1.;
   s->ammo_capacity     = s->ammo_capacity/100. + 1.;
   /* Fighter. */
   s->heat_forward      = s->heat_forward/100. + 1.;
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
      s->firerate_forward *= exp( -0.15 * (double)(MAX(nfirerate_forward-1,0)) );
   s->firerate_forward += 1.;
   /* Cruiser. */
   s->heat_turret       = s->heat_turret/100. + 1.;
   s->damage_turret     = s->damage_turret/100. + 1.;
   s->energy_turret     = s->energy_turret/100. + 1.;
   s->firerate_turret   = s->firerate_turret/100.;
   if (nfirerate_turret > 0)
      s->firerate_turret  *= exp( -0.15 * (double)(MAX(nfirerate_turret-1,0)) );
   s->firerate_turret  += 1.;
   /* Misc. */
   s->nebula_damage     = s->nebula_damage/100. + 1.;

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
      pilot->jam_range  *= s->jam_range;
      pilot->jam_chance *= exp( -0.2 * (double)(MAX(njammers-1,0)) );
   }

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
   pilot->solid->mass = pilot->ship->mass + s->cargo_inertia*pilot->mass_cargo + pilot->mass_outfit;

   /* Calculate the heat. */
   pilot_heatCalc( pilot );

   /* Modulate by mass. */
   pilot_updateMass( pilot );

   /* Update GUI as necessary. */
   gui_setGeneric( pilot );
}


/**
 * @brief Updates the pilot stats after mass change.
 */
void pilot_updateMass( Pilot *pilot )
{
   pilot->turn = pilot->turn_base * pilot->ship->mass / pilot->solid->mass;

   /* Need to recalculate electronic warfare mass change. */
   pilot_ewUpdateStatic( pilot );
}


