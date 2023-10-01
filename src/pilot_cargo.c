/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file pilot_cargo.c
 *
 * @brief Handles the pilot cargo stuff.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "pilot_cargo.h"

#include "array.h"
#include "economy.h"
#include "gui.h"
#include "log.h"

/* Private common implementation */
static int pilot_cargoAddInternal( Pilot* pilot, const Commodity* cargo,
      int quantity, unsigned int id );

/* ID Generators. */
static unsigned int mission_cargo_id = 0; /**< ID generator for special mission cargo.
                                               Not guaranteed to be absolutely unique,
                                               only unique for each pilot. */

/**
 * @brief Gets how many of the commodity a pilot has.
 *
 *    @param pilot Pilot to check.
 *    @param cargo Commodity to check how many the player.p owns.
 *    @return The number of commodities owned matching cargo.
 */
int pilot_cargoOwned( const Pilot* pilot, const Commodity* cargo )
{
   for (int i=0; i<array_size(pilot->commodities); i++)
      if (!pilot->commodities[i].id &&
            strcmp(cargo->name, pilot->commodities[i].commodity->name)==0)
         return pilot->commodities[i].quantity;
   return 0;
}

/**
 * @brief Gets the pilot's free cargo space.
 *
 *    @param p Pilot to get the free space of.
 *    @return Free cargo space on pilot.
 */
int pilot_cargoFree( const Pilot* p )
{
   return p->cargo_free;
}

/**
 * @brief Moves cargo from one pilot to another without any checks.
 *
 * At the end has dest have exactly the same cargo as src and leaves src with none.
 *
 *    @param dest Destination pilot.
 *    @param src Source pilot.
 *    @return 0 on success.
 */
int pilot_cargoMoveRaw( Pilot *dest, Pilot *src )
{
   /* Copy over. */
   for (int i=array_size(src->commodities)-1; i>=0; i--) {
      const PilotCommodity *pc = &src->commodities[i];
      pilot_cargoAddRaw( dest, pc->commodity, pc->quantity, pc->id );
   }
   pilot_cargoRmAll( src, 1 );
   /* Clean src. */
   array_free(src->commodities);
   src->commodities  = NULL;
   return 0;
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
int pilot_cargoMove( Pilot* dest, Pilot* src )
{
   /* Check if it fits. */
   if (pilot_cargoUsed(src) > pilot_cargoFree(dest)) {
      WARN(_("Unable to copy cargo over from pilot '%s' to '%s'. Leaving cargo as is."), src->name, dest->name );
      return -1;
   }

   return pilot_cargoMoveRaw( dest, src );
}

/**
 * @brief Adds cargo to the pilot's "commodities" array only.
 *
 *    @param pilot Pilot to add cargo to.
 *    @param cargo Cargo to add.
 *    @param quantity Quantity to add.
 *    @param id Mission ID to add (0 is none).
 */
static int pilot_cargoAddInternal( Pilot* pilot, const Commodity* cargo,
      int quantity, unsigned int id )
{
   PilotCommodity *pc;
   int q = quantity;

   /* If not mission cargo check to see if already exists. */
   if (id == 0) {
      for (int i=0; i<array_size(pilot->commodities); i++) {
         pc = &pilot->commodities[i];
         if (!pc->id && (pc->commodity == cargo)) {
            pc->quantity += q;
            return q;
         }
      }
   }

   /* Create the memory space. */
   if (pilot->commodities == NULL)
      pilot->commodities = array_create( PilotCommodity );
   pc = &array_grow( &pilot->commodities );
   pc->commodity = cargo;
   pc->id       = id;
   pc->quantity = q;

   return q;
}

/**
 * @brief Adds cargo without checking the pilot's free space.
 *
 * Does not check if currently exists.
 *
 *    @param pilot Pilot to add cargo to.
 *    @param cargo Cargo to add.
 *    @param quantity Quantity to add.
 *    @param id Mission ID to add (0 is none).
 */
int pilot_cargoAddRaw( Pilot* pilot, const Commodity* cargo,
      int quantity, unsigned int id )
{
   int q = pilot_cargoAddInternal( pilot, cargo, quantity, id );
   pilot->cargo_free    -= q;
   pilot->mass_cargo    += q;
   pilot->solid.mass   += pilot->stats.cargo_inertia * q;
   pilot_updateMass( pilot );
   gui_setGeneric( pilot );

   return q;
}

/**
 * @brief Tries to add quantity of cargo to pilot.
 *
 * Note that the commodity will not be added as 0 quantity unless 0 is explicitly specified, or it is mission cargo.
 *
 *    @param pilot Pilot to add cargo to.
 *    @param cargo Cargo to add.
 *    @param quantity Quantity to add.
 *    @param id Mission ID to add (0 is none).
 *    @return Quantity actually added.
 */
int pilot_cargoAdd( Pilot* pilot, const Commodity* cargo,
      int quantity, unsigned int id )
{
   /* Check to see how much to add. */
   int freespace = pilot_cargoFree(pilot);
   if (freespace < quantity) {
      quantity = freespace;
      if ((quantity==0) && (id==0))
         return 0;
   }

   return pilot_cargoAddRaw( pilot, cargo, quantity, id );
}

/**
 * @brief Gets how much cargo ship has on board.
 *
 *    @param p Pilot to get used cargo space of.
 *    @return The used cargo space by pilot.
 */
int pilot_cargoUsed( const Pilot* p )
{
   int q = 0;
   for (int i=0; i<array_size(p->commodities); i++)
      q += p->commodities[i].quantity;
   return q;
}

/**
 * @brief Gets how much mission cargo ship has on board.
 *
 *    @param p Pilot to get used cargo space of.
 *    @return The used cargo space by pilot.
 */
int pilot_cargoUsedMission( const Pilot* p )
{
   int q = 0;
   for (int i=0; i<array_size(p->commodities); i++) {
      PilotCommodity *pc = &p->commodities[i];
      if (pc->id > 0)
         q += pc->quantity;
   }

   return q;
}

/**
 * @brief Calculates how much cargo ship has left and such.
 *
 *    @param pilot Pilot to calculate free cargo space of.
 */
void pilot_cargoCalc( Pilot* pilot )
{
   pilot->mass_cargo  = pilot_cargoUsed( pilot );
   pilot->cargo_free  = pilot->cap_cargo - pilot->mass_cargo;
   pilot_updateMass( pilot );
}

/**
 * @brief Adds special mission cargo, can't sell it and such.
 *
 *    @param pilot Pilot to add it to.
 *    @param cargo Commodity to add.
 *    @param quantity Quantity to add.
 *    @return The Mission Cargo ID of created cargo.
 */
unsigned int pilot_addMissionCargo( Pilot* pilot, const Commodity* cargo, int quantity )
{
   unsigned int id, max_id;

   /* Get ID. */
   id = ++mission_cargo_id;

   /* Check for collisions with pilot and set ID generator to the max. */
   max_id = 0;
   for (int i=0; i<array_size(pilot->commodities); i++)
      max_id = MAX( max_id, pilot->commodities[i].id );
   if (max_id >= id)
      id = mission_cargo_id = max_id+1;

   /* Add the cargo. */
   pilot_cargoAdd( pilot, cargo, quantity, id );

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
   for (i=0; i<array_size(pilot->commodities); i++)
      if (pilot->commodities[i].id == cargo_id)
         break;
   if (i >= array_size(pilot->commodities))
      return 1; /* pilot doesn't have it */

   if (jettison)
      pilot_cargoJet( pilot, pilot->commodities[i].commodity,
            pilot->commodities[i].quantity, 1 );

   /* remove cargo */
   pilot->cargo_free    += pilot->commodities[i].quantity;
   pilot->mass_cargo    -= pilot->commodities[i].quantity;
   pilot->solid.mass   -= pilot->stats.cargo_inertia * pilot->commodities[i].quantity;
   array_erase( &pilot->commodities, &pilot->commodities[i], &pilot->commodities[i+1] );
   if (array_size(pilot->commodities) <= 0) {
      array_free( pilot->commodities );
      pilot->commodities   = NULL;
   }

   /* Update mass. */
   pilot_updateMass( pilot );
   gui_setGeneric( pilot );

   return 0;
}

/**
 * @brief Tries to get rid of quantity cargo from pilot. Can remove mission cargo.
 *
 *    @param pilot Pilot to get rid of cargo.
 *    @param cargo Cargo to get rid of.
 *    @param quantity Amount of cargo to get rid of.
 *    @param cleanup Whether we're cleaning up or not (removes mission cargo).
 *    @return Amount of cargo gotten rid of.
 */
int pilot_cargoRmRaw( Pilot* pilot, const Commodity* cargo, int quantity, int cleanup )
{
   /* Check if pilot has it */
   int q = quantity;
   for (int i=0; i<array_size(pilot->commodities); i++) {
      if (pilot->commodities[i].commodity != cargo)
         continue;

      /* Must not be mission cargo unless cleaning up. */
      if (!cleanup && (pilot->commodities[i].id != 0))
         continue;

      if (quantity >= pilot->commodities[i].quantity) {
         q = pilot->commodities[i].quantity;

         /* remove cargo */
         array_erase( &pilot->commodities, &pilot->commodities[i], &pilot->commodities[i+1] );
         if (array_size(pilot->commodities) <= 0) {
            array_free( pilot->commodities );
            pilot->commodities = NULL;
         }
      }
      else
         pilot->commodities[i].quantity -= q;
      pilot->cargo_free    += q;
      pilot->mass_cargo    -= q;
      pilot->solid.mass   -= pilot->stats.cargo_inertia * q;
      pilot_updateMass( pilot );
      /* This can call Lua code and be called during takeoff (pilot cleanup), causing
       * the Lua code to be run with a half-assed pilot state crashing the game. */
      if (!cleanup)
         gui_setGeneric( pilot );
      return q;
   }
   return 0; /* pilot didn't have it */
}

/**
 * @brief Gets rid of all cargo from pilot.  Can remove mission cargo.
 *
 *    @param pilot Pilot to get rid of cargo.
 *    @param cleanup Whether we're cleaning up or not (removes mission cargo).
 *    @return Amount of cargo gotten rid of.
 */
int pilot_cargoRmAll( Pilot* pilot, int cleanup )
{
   /* No commodities. */
   if (pilot->commodities == NULL)
      return 0;

   /* Check if pilot has it */
   int q = 0;
   for (int i=array_size(pilot->commodities)-1; i>=0; i--) {
      /* Must not be mission cargo unless cleaning up. */
      if (!cleanup && (pilot->commodities[i].id != 0))
         continue;

      q += pilot->commodities[i].quantity;
      array_erase( &pilot->commodities, &pilot->commodities[i], &pilot->commodities[i+1] );
   }

   if (array_size(pilot->commodities) <= 0) {
      array_free( pilot->commodities );
      pilot->commodities = NULL;
   }

   pilot->cargo_free    += q;
   pilot->mass_cargo    -= q;
   pilot->solid.mass   -= pilot->stats.cargo_inertia * q;
   pilot_updateMass( pilot );

   /* If we're updating this ship's status, communicate the update to the GUI.
    * Caution: it could make sense to communicate a deletion, but particularly in the middle of pilots_clean() that's unsafe. */
   if (!cleanup)
      gui_setGeneric( pilot );
   return q;
}

/**
 * @brief Tries to get rid of quantity cargo from pilot.
 *
 *    @param pilot Pilot to get rid of cargo.
 *    @param cargo Cargo to get rid of.
 *    @param quantity Amount of cargo to get rid of.
 *    @return Amount of cargo gotten rid of.
 */
int pilot_cargoRm( Pilot* pilot, const Commodity* cargo, int quantity )
{
   return pilot_cargoRmRaw( pilot, cargo, quantity, 0 );
}

/**
 * @brief Tries to get rid of quantity cargo from pilot, jetting it into space.
 *
 *    @param p Pilot to get rid of cargo.
 *    @param cargo Cargo to get rid of.
 *    @param quantity Amount of cargo to get rid of.
 *    @param simulate Doesn't actually remove cargo, just simulates the removal.
 *    @return Amount of cargo gotten rid of.
 */
int pilot_cargoJet( Pilot *p, const Commodity *cargo, int quantity, int simulate )
{
   int n;
   double px,py, bvx, bvy;

   if (!simulate)
      quantity = pilot_cargoRmRaw( p, cargo, quantity, 0 );

   n   = MAX( 1, RNG(quantity/10, quantity/5) );
   px  = p->solid.pos.x;
   py  = p->solid.pos.y;
   bvx = p->solid.vel.x;
   bvy = p->solid.vel.y;
   for (int i=0; i<n; i++) {
      int effect = spfx_get("cargo");

      /* Radial distribution gives much nicer results */
      double r  = RNGF()*25. - 12.5;
      double a  = 2. * M_PI * RNGF();
      double vx = bvx + r*cos(a);
      double vy = bvy + r*sin(a);

      /* Add the cargo effect */
      spfx_add( effect, px, py, vx, vy, SPFX_LAYER_BACK );
   }

   return quantity;
}
