/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot_weapon.c
 *
 * @brief Handles pilot weapon sets.
 */


#include "pilot.h"

#include "naev.h"

#include "nxml.h"

#include "log.h"
#include "array.h"
#include "weapon.h"
#include "escort.h"


/*
 * Prototypes.
 */
static PilotWeaponSet* pilot_weapSet( Pilot* p, int id );
static int pilot_weapSetFire( Pilot *p, PilotWeaponSet *ws, int level );
static int pilot_shootWeapon( Pilot* p, PilotOutfitSlot* w );


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
 */
static int pilot_weapSetFire( Pilot *p, PilotWeaponSet *ws, int level )
{
   int i, ret;

   /* Case no outfits. */
   if (ws->slots == NULL)
      return 0;

   /* Fire. */
   ret = 0;
   for (i=0; i<array_size(ws->slots); i++)
      if ((level == -1) || (ws->slots[i].level == level))
         ret += pilot_shootWeapon( p, ws->slots[i].slot );

   return ret;
}


/**
 * @brief Executes a weapon set.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 */
void pilot_weapSetExec( Pilot* p, int id )
{
   PilotWeaponSet *ws;

   ws = pilot_weapSet(p,id);
   if (ws->fire)
      pilot_weapSetFire( p, ws, -1 );
   else
      p->active_set = id;
}


/**
 * @brief Adds an outfit to a weapon set.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param level Level of the trigger.
 *    @param o Outfit to add.
 */
void pilot_weapSetAdd( Pilot* p, int id, PilotOutfitSlot *o, int level )
{
   PilotWeaponSet *ws;
   PilotWeaponSetOutfit *slot;
   int i;

   ws = pilot_weapSet(p,id);

   /* Create if needed. */
   if (ws->slots == NULL)
      ws->slots = array_create( PilotWeaponSetOutfit );

   /* Check if already there. */
   for (i=0; i<array_size(&ws->slots); i++)
      if (ws->slots[i].slot == o)
         return;

   /* Add it. */
   slot        = &array_grow( &ws->slots );
   slot->level = level;
   slot->slot  = o;
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
   int i;

   /* Make sure it has slots. */
   ws = pilot_weapSet(p,id);
   if (ws->slots == NULL)
      return;

   /* Find the slot. */
   for (i=0; i<array_size(&ws->slots); i++) {
      if (ws->slots[i].slot == o) {
         array_erase( &ws->slots, &ws->slots[i], &ws->slots[i+1] );
         return;
      }
   }
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

   /* Get active set. */
   ws = pilot_weapSet( p, p->active_set );

   /* Fire weapons. */
   return pilot_weapSetFire( p, ws, level );
}


/**
 * @brief Have pilot stop shooting his weapon.
 *
 * Only really deals with beam weapons.
 *
 *    @param p Pilot that was shooting.
 *    @param level Level of the shot.
 */
void pilot_shootStop( Pilot* p, int level )
{
   int i;
   PilotWeaponSet *ws;

   /* Get active set. */
   ws = pilot_weapSet( p, p->active_set );

   /* Case no outfits. */
   if (ws->slots == NULL)
      return;

   /* Stop all beams. */
   for (i=0; i<array_size(ws->slots); i++) {

      /* Must have assosciated outfit. */
      if (ws->slots[i].slot->outfit != NULL)
         continue;

      /* Must match level. */
      if ((level != -1) && (ws->slots[i].level != level))
         continue;

      /* Only handle beams. */
      if (!outfit_isBeam(ws->slots[i].slot->outfit))
         continue;
      
      /* Stop beam. */
      if (ws->slots[i].slot->u.beamid > 0) {
         beam_end( p->id, ws->slots[i].slot->u.beamid );
         ws->slots[i].slot->u.beamid = 0;
      }
   }
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
      for (i=0; i<p->outfit_nweapon; i++) {
         slot = &p->outfit_weapon[i];

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


