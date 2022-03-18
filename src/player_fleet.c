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

   /* TODO update cargo too! */
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
