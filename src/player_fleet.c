/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file player_fleet.c
 *
 * @brief Contains all the player fleet related stuff.
 */
/** @cond */
#include "naev.h"
/** @endcond */

#include "player_fleet.h"

#include "array.h"
#include "dialogue.h"
#include "escort.h"

/**
 * @brief Updates the used fleet capacity of the player.
 */
void pfleet_update (void)
{
   const PlayerShip_t *player_ships = player_getShipStack();
   player.fleet_used = player.p->ship->points;
   for (int i=0; i<array_size(player_ships); i++) {
      const PlayerShip_t *ps = &player_ships[i];
      if (ps->deployed)
         player.fleet_used += ps->p->ship->points;
   }

   /* Redistribute the cargo. */
   pfleet_cargoRedistribute();
}

int pfleet_deploy( PlayerShip_t *ps, int deploy )
{
   /* When undeploying we want to make sure cargo fits. */
   if (ps->deployed && !deploy) {
      Pilot *p = pilot_get( ps->id );
      if (p != NULL) {
         int idx;
         int q = pilot_cargoUsed( p ); /* Amount we have to allocate. */
         int f = pfleet_cargoFree() - pilot_cargoFree( p ); /* Real free amount. */
         if (f < q) {
            char buf_amount[ECON_MASS_STRLEN], buf_free[ECON_MASS_STRLEN], buf_needed[ECON_MASS_STRLEN];
            tonnes2str( buf_amount, q );
            tonnes2str( buf_free, -f );
            tonnes2str( buf_needed, q-f );
            if (!dialogue_YesNo( _("Not Enough Cargo Space"), _("Your ship '%s' has %s of cargo but there is only %s of free space in the rest of the fleet. Get rid of %s of cargo to shrink your fleet?"), p->name, buf_amount, buf_free, buf_needed ))
               return -1;
         }
         /* Try to make room for the commodities. */
         idx = -1;
         for (int i=0; i<array_size(player.p->escorts); i++) {
            Escort_t *e = &player.p->escorts[i];
            Pilot *pe = pilot_get( e->id );
            if (pe == NULL)
               continue;
            if (strcmp( pe->name, p->name )==0) {
               idx = i;
               break;
            }
         }
         if (idx < 0)
            WARN(_("Player deployed ship '%s' not found in escort list!"), p->name);
         else
            escort_rmListIndex( player.p, idx );

         /* Try to add the cargo. */
         for (int i=0; i<array_size(p->commodities); i++) {
            PilotCommodity *pc = &p->commodities[i];
            pfleet_cargoAdd( pc->commodity, pc->quantity );
         }
      }
   }
   ps->deployed = deploy;
   if (!ps->deployed) {
      Pilot *p = pilot_get( ps->id );
      if (p != NULL)
         pilot_delete( p );
   }
   else
      player_addEscorts();
   pfleet_update();
   return 0;
}

static void shipCargo( PilotCommodity **pclist, Pilot *p )
{
   for (int i=0; i<array_size(p->commodities); i++) {
      const PilotCommodity *pc = &p->commodities[i];
      int added = 0;
      for (int j=array_size(*pclist); j >= 0; j--) {
         PilotCommodity *lc = &(*pclist)[j];

         /* Ignore mission cargo. */
         if (lc->id > 0)
            continue;

         if (pc->commodity != lc->commodity)
            continue;

         lc->quantity += pc->quantity;
         added = 1;
      }
      if (!added)
         array_push_back( pclist, *pc );

      /* Remove the cargo. */
      array_erase( &p->commodities, &pc[0], &pc[1] );
   }
}

/**
 * @brief Redistributes the cargo in the player's fleet.
 */
void pfleet_cargoRedistribute (void)
{
   PilotCommodity *pclist = array_create( PilotCommodity );

   /* First build up a list of all the potential cargo. */
   shipCargo( &pclist, player.p );
   for (int i=0; i<array_size(player.p->escorts); i++) {
      Escort_t *e = &player.p->escorts[i];
      Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      shipCargo( &pclist, pe );
   }

   /* TODO sort based on something? */

   /* Re-add the cargo. */
   for (int i=0; i<array_size(pclist); i++) {
      PilotCommodity *pc = &pclist[i];
      int q = pfleet_cargoAdd( pc->commodity, pc->quantity );
#ifdef DEBUGGING
      if (q != pc->quantity)
         WARN(_("Failure to add cargo '%s' to player fleeet. Only %d of %d added."), pc->commodity->name, q, pc->quantity );
#endif /* DEBUGGING */
   }

   array_free( pclist );
}

/**
 * @brief Gets the total cargo space used by the player's fleet.
 *
 *    @return Total amount of used cargo.
 */
int pfleet_cargoUsed (void)
{
   int cargo_used = pilot_cargoUsed( player.p );
   if (player.fleet_capacity <= 0)
      return cargo_used;
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      const Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      cargo_used += pilot_cargoUsed( pe );
   }
   return cargo_used;
}

/**
 * @brief Gets the total amount of free cargo space in the player's fleet.
 *
 *    @return Total amount of free cargo space.
 */
int pfleet_cargoFree (void)
{
   int cargo_free = pilot_cargoFree( player.p );
   if (player.fleet_capacity <= 0)
      return cargo_free;
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      const Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      cargo_free += pilot_cargoFree( pe );
   }
   return cargo_free;
}

/**
 * @brief Gets the total amount of a commodity type owned by the player's fleet.
 *
 *    @param com Commodity to add.
 *    @return Total amount of a cargo owned.
 */
int pfleet_cargoOwned( const Commodity *com )
{
   int amount = pilot_cargoOwned( player.p, com );
   if (player.fleet_capacity <= 0)
      return amount;
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      const Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      amount += pilot_cargoOwned( pe, com );
   }
   return amount;
}

/**
 * @brief Adds some cargo to the player's fleet.
 *
 *    @param com Commodity to add.
 *    @param q Quantity to add.
 *    @return Total amount of cargo added (less than q if it doesn't fit).
 */
int pfleet_cargoAdd( const Commodity *com, int q )
{
   int added = pilot_cargoAdd( player.p, com, q, 0 );
   if (player.fleet_capacity <= 0)
      return added;
   for (int i=0; i<array_size(player.p->escorts); i++) {
      Escort_t *e = &player.p->escorts[i];
      Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      added += pilot_cargoAdd( pe, com, q-added, 0 );
      if (q-added <= 0)
         break;
   }
   return added;
}

/**
 * @brief Removes some cargo from the player's fleet.
 *
 *    @param com Commodity to remove.
 *    @param q Quantity to remove.
 *    @return Total amount of cargo removed (can be less than q).
 */
int pfleet_cargoRm( const Commodity *com, int q )
{
   int removed;
   if (player.fleet_capacity <= 0)
      return pilot_cargoRm( player.p, com, q );
   removed = 0;
   for (int i=0; i<array_size(player.p->escorts); i++) {
      Escort_t *e = &player.p->escorts[i];
      Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      removed += pilot_cargoRm( pe, com, q-removed );
      if (q-removed <= 0)
         break;
   }
   if (q-removed > 0)
      removed += pilot_cargoRm( player.p, com, q );
   return removed;
}
