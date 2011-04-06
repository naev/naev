/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot_cargo.c
 *
 * @brief Handles the pilot cargo stuff.
 */


#include "pilot_cargo.h"

#include "naev.h"

#include "log.h"
#include "economy.h"
#include "gui.h"


/* ID Generators. */
static unsigned int mission_cargo_id = 0; /**< ID generator for special mission cargo.
                                               Not guaranteed to be absolutely unique,
                                               only unique for each pilot. */



/**
 * @brief Gets how many of the commodity the player.p has.
 *
 * @param commodityname Commodity to check how many the player.p owns.
 * @return The number of commodities owned matching commodityname.
 */
int pilot_cargoOwned( Pilot* pilot, const char* commodityname )
{
   int i;

   for (i=0; i<pilot->ncommodities; i++)
      if (!pilot->commodities[i].id &&
            strcmp(commodityname, pilot->commodities[i].commodity->name)==0)
         return pilot->commodities[i].quantity;
   return 0;
}


/**
 * @brief Gets the pilot's free cargo space.
 *
 *    @param p Pilot to get the free space of.
 *    @return Free cargo space on pilot.
 */
int pilot_cargoFree( Pilot* p )
{
   return p->cargo_free;
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
   int i;

   /* Nothing to copy, success! */
   if (src->ncommodities == 0)
      return 0;

   /* Check if it fits. */
   if (pilot_cargoUsed(src) > pilot_cargoFree(dest)) {
      WARN("Unable to copy cargo over from pilot '%s' to '%s'", src->name, dest->name );
      return -1;
   }

   /* Allocate new space. */
   i = dest->ncommodities;
   dest->ncommodities += src->ncommodities;
   dest->commodities   = realloc( dest->commodities,
         sizeof(PilotCommodity)*dest->ncommodities);

   /* Copy over. */
   memmove( &dest->commodities[0], &src->commodities[0],
         sizeof(PilotCommodity) * src->ncommodities);

   /* Clean src. */
   if (src->commodities != NULL)
      free(src->commodities);
   src->ncommodities = 0;
   src->commodities  = NULL;

   return 0;
}


/**
 * @brief Adds a cargo raw.
 *
 * Does not check if currently exists.
 *
 *    @param pilot Pilot to add cargo to.
 *    @param cargo Cargo to add.
 *    @param quantity Quantity to add.
 *    @param id Mission ID to add (0 in none).
 */
int pilot_cargoAddRaw( Pilot* pilot, Commodity* cargo,
      int quantity, unsigned int id )
{
   int i, f, q;

   q = quantity;

   /* If not mission cargo check to see if already exists. */
   if (id == 0) {
      for (i=0; i<pilot->ncommodities; i++)
         if (!pilot->commodities[i].id &&
               (pilot->commodities[i].commodity == cargo)) {

            /* Check to see how much to add. */
            f = pilot_cargoFree(pilot);
            if (f < quantity)
               q = f;

            /* Tweak results. */
            pilot->commodities[i].quantity += q;
            pilot->cargo_free              -= q;
            pilot->mass_cargo              += q;
            pilot->solid->mass             += pilot->stats.cargo_inertia * q;
            pilot_updateMass( pilot );
            gui_setGeneric( pilot );
            return q;
         }
   }

   /* Create the memory space. */
   pilot->commodities = realloc( pilot->commodities,
         sizeof(PilotCommodity) * (pilot->ncommodities+1));
   pilot->commodities[ pilot->ncommodities ].commodity = cargo;

   /* See how much to add. */
   f = pilot_cargoFree(pilot);
   if (f < quantity)
      q = f;

   /* Set parameters. */
   pilot->commodities[ pilot->ncommodities ].id       = id;
   pilot->commodities[ pilot->ncommodities ].quantity = q;

   /* Tweak pilot. */
   pilot->cargo_free    -= q;
   pilot->mass_cargo    += q;
   pilot->solid->mass   += pilot->stats.cargo_inertia * q;
   pilot->ncommodities++;
   pilot_updateMass( pilot );
   gui_setGeneric( pilot );

   return q;
}


/**
 * @brief Tries to add quantity of cargo to pilot.
 *
 *    @param pilot Pilot to add cargo to.
 *    @param cargo Cargo to add.
 *    @param quantity Quantity to add.
 *    @return Quantity actually added.
 */
int pilot_cargoAdd( Pilot* pilot, Commodity* cargo, int quantity )
{
   return pilot_cargoAddRaw( pilot, cargo, quantity, 0 );
}


/**
 * @brief Gets how much cargo ship has on board.
 *
 *    @param pilot Pilot to get used cargo space of.
 *    @return The used cargo space by pilot.
 */
int pilot_cargoUsed( Pilot* pilot )
{
   int i, q;

   q = 0;
   for (i=0; i<pilot->ncommodities; i++)
      q += pilot->commodities[i].quantity;

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
   pilot->cargo_free  = pilot->ship->cap_cargo - pilot->mass_cargo;
   pilot->solid->mass = pilot->ship->mass + pilot->stats.cargo_inertia * pilot->mass_cargo + pilot->mass_outfit;
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
unsigned int pilot_addMissionCargo( Pilot* pilot, Commodity* cargo, int quantity )
{
   int i;
   unsigned int id, max_id;
   int q;
   q = quantity;

   /* Get ID. */
   id = ++mission_cargo_id;

   /* Check for collisions with pilot and set ID generator to the max. */
   max_id = 0;
   for (i=0; i<pilot->ncommodities; i++)
      if (pilot->commodities[i].id > max_id)
         max_id = pilot->commodities[i].id;
   if (max_id >= id) {
      mission_cargo_id = max_id;
      id = ++mission_cargo_id;
   }

   /* Add the cargo. */
   pilot_cargoAddRaw( pilot, cargo, quantity, id );

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
   for (i=0; i<pilot->ncommodities; i++)
      if (pilot->commodities[i].id == cargo_id)
         break;
   if (i>=pilot->ncommodities)
      return 1; /* pilot doesn't have it */

   if (jettison)
      commodity_Jettison( pilot->id, pilot->commodities[i].commodity,
            pilot->commodities[i].quantity );

   /* remove cargo */
   pilot->cargo_free    += pilot->commodities[i].quantity;
   pilot->mass_cargo    -= pilot->commodities[i].quantity;
   pilot->solid->mass   -= pilot->stats.cargo_inertia * pilot->commodities[i].quantity;
   pilot->ncommodities--;
   if (pilot->ncommodities <= 0) {
      if (pilot->commodities != NULL) {
         free( pilot->commodities );
         pilot->commodities   = NULL;
      }
      pilot->ncommodities  = 0;
   }
   else {
      memmove( &pilot->commodities[i], &pilot->commodities[i+1],
            sizeof(PilotCommodity) * (pilot->ncommodities-i) );
      pilot->commodities = realloc( pilot->commodities,
            sizeof(PilotCommodity) * pilot->ncommodities );
   }

   /* Update mass. */
   pilot_updateMass( pilot );
   gui_setGeneric( pilot );

   return 0;
}


/**
 * @brief Tries to get rid of quantity cargo from pilot.  Can remove mission cargo.
 *
 *    @param pilot Pilot to get rid of cargo.
 *    @param cargo Cargo to get rid of.
 *    @param quantity Amount of cargo to get rid of.
 *    @param cleanup Whether we're cleaning up or not (removes mission cargo).
 *    @return Amount of cargo gotten rid of.
 */
int pilot_cargoRmRaw( Pilot* pilot, Commodity* cargo, int quantity, int cleanup )
{
   int i;
   int q;

   /* check if pilot has it */
   q = quantity;
   for (i=0; i<pilot->ncommodities; i++) {
      if (pilot->commodities[i].commodity != cargo)
         continue;

      /* Must not be mission cargo unless cleaning up. */
      if (!cleanup && (pilot->commodities[i].id != 0))
         continue;

      if (quantity >= pilot->commodities[i].quantity) {
         q = pilot->commodities[i].quantity;

         /* remove cargo */
         pilot->ncommodities--;
         if (pilot->ncommodities <= 0) {
            if (pilot->commodities != NULL) {
               free( pilot->commodities );
               pilot->commodities   = NULL;
            }
            pilot->ncommodities  = 0;
         }
         else {
            memmove( &pilot->commodities[i], &pilot->commodities[i+1],
                  sizeof(PilotCommodity) * (pilot->ncommodities-i) );
            pilot->commodities = realloc( pilot->commodities,
                  sizeof(PilotCommodity) * pilot->ncommodities );
         }
      }
      else
         pilot->commodities[i].quantity -= q;
      pilot->cargo_free    += q;
      pilot->mass_cargo    -= q;
      pilot->solid->mass   -= pilot->stats.cargo_inertia * q;
      pilot_updateMass( pilot );
      gui_setGeneric( pilot );
      return q;
   }
   return 0; /* pilot didn't have it */
}

/**
 * @brief Tries to get rid of quantity cargo from pilot.
 *
 *    @param pilot Pilot to get rid of cargo.
 *    @param cargo Cargo to get rid of.
 *    @param quantity Amount of cargo to get rid of.
 *    @return Amount of cargo gotten rid of.
 */
int pilot_cargoRm( Pilot* pilot, Commodity* cargo, int quantity )
{
   return pilot_cargoRmRaw( pilot, cargo, quantity, 0 );
}


