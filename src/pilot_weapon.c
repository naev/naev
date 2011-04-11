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
static void pilot_weapSetUpdateOutfits( Pilot* p, PilotWeaponSet *ws );
static PilotWeaponSet* pilot_weapSet( Pilot* p, int id );
static int pilot_weapSetFire( Pilot *p, PilotWeaponSet *ws, int level );
static int pilot_shootWeaponSetOutfit( Pilot* p, PilotWeaponSet *ws, Outfit *o, int level );
static int pilot_shootWeapon( Pilot* p, PilotOutfitSlot* w );
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
 */
static int pilot_weapSetFire( Pilot *p, PilotWeaponSet *ws, int level )
{
   int i, j, ret, s;
   Pilot *pt;
   double dist2;

   /* Case no outfits. */
   if (ws->slots == NULL)
      return 0;

   /* If inrange is set we only fire at targets in range. */
   if (ws->inrange) {
      if (p->target == p->id)
         return 0;
      pt = pilot_get( p->target );
      if (pt == NULL)
         return 0;
      dist2 = vect_dist2( &p->solid->pos, &pt->solid->pos );
   }

   /* Fire. */
   ret = 0;
   for (i=0; i<array_size(ws->slots); i++) {

      /* Ignore NULL outfits. */
      if (ws->slots[i].slot->outfit == NULL)
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
         if (ws->slots[j].slot->outfit == ws->slots[i].slot->outfit) {
            s = 1;
            break;
         }
      }
      if (s!=0)
         continue;

      /* Only "inrange" outfits. */
      if (ws->inrange && (dist2 > ws->slots[i].range2))
         continue;

      /* Shoot the weapon of the weaponset. */
      ret += pilot_shootWeaponSetOutfit( p, ws, ws->slots[i].slot->outfit, level );
   }

   return ret;
}


/**
 * @brief Handles a weapon set press.
 */
void pilot_weapSetPress( Pilot* p, int id, int type )
{
   PilotWeaponSet *ws;

   ws = pilot_weapSet(p,id);

   /* Handle fire groups. */
   if (ws->fire) {
      if (type > 0)
         ws->active = 1;
      else if (type < 0)
         ws->active = 0;
   }
   else if (type > 0)
      pilot_weapSetExec( p, id );
}


/**
 * @brief Updates the pilot's weapon sets.
 */
void pilot_weapSetUpdate( Pilot* p )
{
   int i;

   for (i=0; i<PILOT_WEAPON_SETS; i++)
      if (p->weapon_sets[i].active)
         pilot_weapSetExec( p, i );
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
   if (ws->fire) {
      pilot_weapSetFire( p, ws, -1 );
   }
   else {
      if (id != p->active_set)
         pilot_weapSetUpdateOutfits( p, ws );
      p->active_set = id;
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
 * @brief Checks the current weapon set mode.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set to check.
 *    @return The fire mode of the weapon set.
 */
int pilot_weapSetModeCheck( Pilot* p, int id )
{
   PilotWeaponSet *ws;
   ws = pilot_weapSet(p,id);
   return ws->fire;
}


/**
 * @brief Changes the weapon sets mode.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param fire Whether or not to enable fire mode.
 */
void pilot_weapSetMode( Pilot* p, int id, int fire )
{
   PilotWeaponSet *ws;
   ws = pilot_weapSet(p,id);
   ws->fire = fire;
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
 * @brief CHanges the weapon set inrange property.
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
 * @brief Sets the weapon set name.
 *
 *    @param p Pilot to manipulate.
 *    @param id ID of the weapon set.
 *    @param name Name to set for the weapon set.
 */
void pilot_weapSetNameSet( Pilot* p, int id, const char *name )
{
   PilotWeaponSet *ws;

   ws = pilot_weapSet(p,id);
   if (ws->name != NULL) {
      if (strcmp(ws->name,name)==0)
         return;
      free( ws->name );
   }
   ws->name = strdup(name);
}


/**
 * @brief Gets the name of a weapon set.
 */
const char *pilot_weapSetName( Pilot* p, int id )
{
   PilotWeaponSet *ws;
   ws = pilot_weapSet(p,id);
   return ws->name;
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
   int i;

   ws = pilot_weapSet(p,id);

   /* Make sure outfit is valid. */
   oo = o->outfit;
   if (oo == NULL)
      return;

   /* Make sure outfit type is weapon (or usable). */
   if (!(outfit_isBeam(oo) || outfit_isBolt(oo) ||
            outfit_isLauncher(oo) || outfit_isFighterBay(oo)))
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
   slot->range2 = pow2(outfit_range(oo));

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
   int i;

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
  
   if (ws->name != NULL)
      free( ws->name );
   ws->name = NULL;

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
      if (ws->slots[i].slot->outfit == NULL)
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
 * @brief Calculates and shoots the appropriate weapons in a weapon set matching an outfit.
 */
static int pilot_shootWeaponSetOutfit( Pilot* p, PilotWeaponSet *ws, Outfit *o, int level )
{
   int i, ret;
   int is_launcher;
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
            ret += pilot_shootWeapon( p, ws->slots[i].slot );
      return ret;
   }

   /* Stores if it is a launcher. */
   is_launcher = outfit_isLauncher(o);

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
      if (is_launcher && ((w->u.ammo.outfit == NULL) || (w->u.ammo.quantity <= 0)))
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
   ret += pilot_shootWeapon( p, ws->slots[minh].slot );

   return ret;
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
   int is_launcher;
   double rate_mod, energy_mod;
   double energy;

   /* Make sure weapon has outfit. */
   if (w->outfit == NULL)
      return 0;

   /* check to see if weapon is ready */
   if (w->timer > 0.)
      return 0;

   /* See if is launcher. */
   is_launcher = outfit_isLauncher(w->outfit);

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

      energy      = outfit_energy(w->u.ammo.outfit)*energy_mod;
      p->energy  -= energy;
      pilot_heatAddSlot( p, w );
      weapon_add( w->u.ammo.outfit, w->heat_T, p->solid->dir,
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
         *rate_mod   = 2. - p->stats.firerate_forward; /* Invert. */
         *energy_mod = p->stats.energy_forward;
         break;
      case OUTFIT_TYPE_TURRET_BOLT:
         *rate_mod   = 2. - p->stats.firerate_turret; /* Invert. */
         *energy_mod = p->stats.energy_turret;
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
 *    @param p Pilot to clear his weapons.
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
   pilot_weapSetMode( p, 0, 0 );
   pilot_weapSetMode( p, 1, 0 );
   pilot_weapSetMode( p, 2, 0 );
   pilot_weapSetMode( p, 3, 0 );
   pilot_weapSetMode( p, 4, 1 );
   pilot_weapSetMode( p, 5, 1 );
   pilot_weapSetMode( p, 6, 0 );
   pilot_weapSetMode( p, 7, 0 );
   pilot_weapSetMode( p, 8, 0 );
   pilot_weapSetMode( p, 9, 0 );

   /* All should be inrange. */
   for (i=0; i<PILOT_WEAPSET_MAX_LEVELS; i++)
      pilot_weapSetInrange( p, i, 1 );

   /* Set names. */
   pilot_weapSetNameSet( p, 0, "All" );
   pilot_weapSetNameSet( p, 1, "Forward" );
   pilot_weapSetNameSet( p, 2, "Turret" );
   pilot_weapSetNameSet( p, 3, "Fwd/Tur" );
   pilot_weapSetNameSet( p, 4, "Seekers" );
   pilot_weapSetNameSet( p, 5, "Fighter Bays" );
   pilot_weapSetNameSet( p, 6, "Weaponset 7" );
   pilot_weapSetNameSet( p, 7, "Weaponset 8" );
   pilot_weapSetNameSet( p, 8, "Weaponset 9" );
   pilot_weapSetNameSet( p, 9, "Weaponset 0" );

   /* Iterate through all the outfits. */
   for (i=0; i<p->outfit_nweapon; i++) {
      slot = &p->outfit_weapon[i];
      o    = slot->outfit;

      /* Must have outfit. */
      if (o == NULL) {
         slot->level = -1; /* Clear level. */
         continue;
      }

      /* Bolts and beams. */
      if (outfit_isBolt(o) || outfit_isBeam(o) ||
            (outfit_isLauncher(o) && !outfit_isSeeker(o->u.lau.ammo))) {
         id    = outfit_isTurret(o) ? 2 : 1;
         level = (outfit_ammo(o) != NULL) ? 1 : 0;
      }
      /* Seekers. */
      else if (outfit_isLauncher(o) && outfit_isSeeker(o->u.lau.ammo)) {
         id    = 4;
         level = 1;
      }
      /* Fighter bays. */
      else if (outfit_isFighterBay(o)) {
         id    = 5;
         level = 0;
      }
      /* Ignore rest. */
      else {
         slot->level = -1;
         continue;
      }
   
      /* Add to it's base group. */
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
   if (!p->weapon_sets[ p->active_set ].fire)
      return;

   /* Find first fire gorup. */
   for (i=0; i<PILOT_WEAPON_SETS; i++)
      if (!p->weapon_sets[i].fire)
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
   PilotWeaponSet *ws;

   for (j=0; j<PILOT_WEAPON_SETS; j++) {
      ws = &p->weapon_sets[j];
      if (ws->slots == NULL)
         continue;

      for (i=0; i<array_size(ws->slots); i++) {
         if (ws->slots[i].slot->outfit != NULL)
            continue;

         array_erase( &ws->slots, &ws->slots[i], &ws->slots[i+1] );
         i--;
      }
   }

   /* Update range. */
   pilot_weapSetUpdateRange( ws );
}

