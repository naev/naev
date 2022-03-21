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

int pfleet_toggleDeploy( PlayerShip_t *ps, int deploy )
{
   /* When undeploying we want to make sure cargo fits. */
   if (ps->deployed && !deploy) {
      int idx;
      Pilot *p = ps->p;
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
   ps->deployed = deploy;
   if (!ps->deployed)
      pilot_delete( ps->p );
   else
      player_addEscorts();
   pfleet_update();
   return 0;
}

int pfleet_deploy( PlayerShip_t *ps )
{
   double a;
   vec2 v;

   /* Get the position. */
   a = RNGF() * 2. * M_PI;
   vec2_cset( &v, player.p->solid->pos.x + 50.*cos(a),
         player.p->solid->pos.y + 50.*sin(a) );

   /* Add the escort to the fleet. */
   escort_createRef( player.p, ps->p, &v, NULL, a, ESCORT_TYPE_FLEET, 1, -1 );

   /* Initialize. */
   ai_pinit( ps->p, "player" );
   pilot_reset( ps->p );
   pilot_rmFlag( ps->p, PILOT_PLAYER );

   return 0;
}

static void shipCargo( PilotCommodity **pclist, Pilot *p )
{
   for (int i=array_size(p->commodities)-1; i>=0; i--) {
      const PilotCommodity *pc = &p->commodities[i];
      int q = pc->quantity;
      int added;

      /* Ignore mission cargo. */
      if (pc->id > 0)
         continue;

      /* See if it can be added. */
      added = 0;
      for (int j=0; j<array_size(*pclist); j++) {
         PilotCommodity *lc = &(*pclist)[j];

         if (pc->commodity != lc->commodity)
            continue;

         lc->quantity += q;
         added = 1;
         break;
      }
      if (!added)
         array_push_back( pclist, *pc );

      /* Remove the cargo. TODO use pilot_cargoRm somehow.  */
      array_erase( &p->commodities, &pc[0], &pc[1] );
      p->cargo_free  += q;
      p->mass_cargo  -= q;
      p->solid->mass -= p->stats.cargo_inertia * q;
   }
   pilot_updateMass( p );
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
      if (q != pc->quantity) {
         WARN(_("Failure to add cargo '%s' to player fleeet. Only %d of %d added."), pc->commodity->name, q, pc->quantity );
      }
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
