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
#include "land.h"
#include "equipment.h"

/**
 * @brief Updates the used fleet capacity of the player.
 */
void pfleet_update (void)
{
   const PlayerShip_t *pships = player_getShipStack();
   player.fleet_used = player.p->ship->points;
   for (int i=0; i<array_size(pships); i++) {
      const PlayerShip_t *ps = &pships[i];
      if (ps->deployed)
         player.fleet_used += ps->p->ship->points;
   }

   /* Redistribute the cargo. */
   pfleet_cargoRedistribute();
}

/**
 * @brief Toggles a player ship as deployed.
 *
 *    @param ps Player ship to toggle.
 *    @param deploy Whether or not to set status as deployed.
 *    @return 0 on success
 */
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
         const Escort_t *e = &player.p->escorts[i];
         const Pilot *pe = pilot_get( e->id );
         if (pe == NULL)
            continue;
         if (e->type != ESCORT_TYPE_FLEET)
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
         const PilotCommodity *pc = &p->commodities[i];
         pfleet_cargoAdd( pc->commodity, pc->quantity );
         pilot_cargoRm( p, pc->commodity, pc->quantity );
      }
   }
   ps->deployed = deploy;
   if (!ps->deployed) {
      pilot_stackRemove( ps->p );
      pilot_free( ps->p );
   }
   else
      pfleet_deploy( ps );
   pfleet_update();

   /* Have to update GUI. */
   equipment_updateShips( land_getWid( LAND_WINDOW_EQUIPMENT ), NULL );
   return 0;
}

/**
 * @brief Deploys a player's pilot.
 *
 * Will not deploy duplicates.
 *
 *    @param ps Player ship to deploy.
 *    @return 0 on success
 */
int pfleet_deploy( PlayerShip_t *ps )
{
   double a;
   vec2 v;

   /* Get the position. */
   a = RNGF() * 2. * M_PI;
   vec2_cset( &v, player.p->solid.pos.x + 50.*cos(a),
         player.p->solid.pos.y + 50.*sin(a) );

   /* Add the escort to the fleet. */
   escort_createRef( player.p, ps->p, &v, NULL, a, ESCORT_TYPE_FLEET, 1, -1 );

   /* Initialize. */
   ai_pinit( ps->p, "escort" );
   pilot_reset( ps->p );
   pilot_setFlag( ps->p, PILOT_INVINC_PLAYER );
   pilot_rmFlag( ps->p, PILOT_PLAYER );

   /* AI only knows how to use auto weapon sets. */
   pilot_weaponAuto( ps->p );

   return 0;
}

static void shipCargo( PilotCommodity **pclist, Pilot *p, int remove )
{
   for (int i=array_size(p->commodities)-1; i>=0; i--) {
      const PilotCommodity *pc = &p->commodities[i];
      int q = pc->quantity;

      /* Mission cargo gets added independently. */
      if (pc->id > 0)
         array_push_back( pclist, *pc );
      else {
         /* See if it can be added. */
         int added = 0;
         for (int j=0; j<array_size(*pclist); j++) {
            PilotCommodity *lc = &(*pclist)[j];

            /* Ignore mission cargo. */
            if (lc->id > 0)
               continue;

            /* Cargo must match. */
            if (pc->commodity != lc->commodity)
               continue;

            lc->quantity += q;
            added = 1;
            break;
         }
         if (!added)
            array_push_back( pclist, *pc );
      }

      /* Remove the cargo. TODO use pilot_cargoRm somehow.  */
      if (remove)
         array_erase( &p->commodities, &pc[0], &pc[1] );
   }

   /* Update cargo. */
   if (remove)
      pilot_cargoCalc( p );
}

static int pc_cmp( const void *pa, const void *pb )
{
   const PilotCommodity *pca, *pcb;
   pca = (const PilotCommodity*) pa;
   pcb = (const PilotCommodity*) pb;

   return pcb->commodity->price - pca->commodity->price;
}

/**
 * @brief Redistributes the cargo in the player's fleet.
 */
void pfleet_cargoRedistribute (void)
{
   PilotCommodity *pclist = array_create( PilotCommodity );

   /* First build up a list of all the potential cargo. */
   shipCargo( &pclist, player.p, 1 );
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      if (e->type != ESCORT_TYPE_FLEET)
         continue;
      shipCargo( &pclist, pe, 1 );
   }

   /* Sort based on base price. */
   qsort( pclist, array_size(pclist), sizeof(PilotCommodity), pc_cmp );

   /* Re-add the cargo. */
   for (int i=0; i<array_size(pclist); i++) {
      int q;
      const PilotCommodity *pc = &pclist[i];

      if (pc->id > 0)
         q = pilot_cargoAdd( player.p, pc->commodity, pc->quantity, pc->id );
      else
         q = pfleet_cargoAdd( pc->commodity, pc->quantity );
#ifdef DEBUGGING
      if (q != pc->quantity)
         WARN(_("Failure to add cargo '%s' to player fleet. Only %d of %d added."), pc->commodity->name, q, pc->quantity );
#endif /* DEBUGGING */
      (void) q;
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
   if (player.p == NULL)
      return 0;
   int cargo_used = pilot_cargoUsed( player.p );
   if (player.fleet_capacity <= 0)
      return cargo_used;
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      const Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      if (e->type != ESCORT_TYPE_FLEET)
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
   if (player.p == NULL)
      return 0;
   int cargo_free = pilot_cargoFree( player.p );
   if (player.fleet_capacity <= 0)
      return cargo_free;
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      const Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      if (e->type != ESCORT_TYPE_FLEET)
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
   if (player.p == NULL)
      return 0;
   int amount = pilot_cargoOwned( player.p, com );
   if (player.fleet_capacity <= 0)
      return amount;
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      const Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      if (e->type != ESCORT_TYPE_FLEET)
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
   if (player.p == NULL)
      return 0;
   int added = pilot_cargoAdd( player.p, com, q, 0 );
   if ((player.fleet_capacity <= 0) || (q-added <= 0))
      return added;
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      if (e->type != ESCORT_TYPE_FLEET)
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
 *    @param jet Whether or not to jet into space.
 *    @return Total amount of cargo removed (can be less than q).
 */
int pfleet_cargoRm( const Commodity *com, int q, int jet )
{
   int removed;
   if (player.p == NULL)
      return 0;
   if (player.fleet_capacity <= 0)
      return pilot_cargoRm( player.p, com, q );
   removed = 0;
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      if (e->type != ESCORT_TYPE_FLEET)
         continue;

      if (jet)
         removed += pilot_cargoJet( pe, com, q-removed, 0 );
      else
         removed += pilot_cargoRm( pe, com, q-removed );

      if (q-removed <= 0)
         break;
   }
   if (q-removed > 0) {
      if (jet)
         removed += pilot_cargoJet( player.p, com, q, 0 );
      else
         removed += pilot_cargoRm( player.p, com, q );
   }
   return removed;
}

/**
 * @brief Gets a list of all the cargo in the fleet.
 *
 *    @return List of all the cargo in the fleet (array.h). Individual elements do not have to be freed, but the list does.
 */
PilotCommodity* pfleet_cargoList (void)
{
   PilotCommodity *pclist = array_create( PilotCommodity );
   shipCargo( &pclist, player.p, 0 );
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      if (e->type != ESCORT_TYPE_FLEET)
         continue;
      shipCargo( &pclist, pe, 0 );
   }
   return pclist;
}

/**
 * @brief Gets the list of ships that are carry a certain commodity in the player fleet and the amount they are carrying.
 *
 *    @param com Commodity to see which ships have.
 *    @return An array of ships and the amount they have (array.h). Must be freed with array_free.
 */
PFleetCargo* pfleet_cargoListShips( const Commodity *com )
{
   PFleetCargo *plist = array_create( PFleetCargo );
   int q = pilot_cargoOwned( player.p, com );
   if (q > 0) {
      PFleetCargo fc = { .p=player.p, .q=q };
      array_push_back( &plist, fc );
   }
   for (int i=0; i<array_size(player.p->escorts); i++) {
      const Escort_t *e = &player.p->escorts[i];
      Pilot *pe = pilot_get( e->id );
      if (pe == NULL)
         continue;
      if (e->type != ESCORT_TYPE_FLEET)
         continue;
      q = pilot_cargoOwned( pe, com );
      if (q > 0) {
         PFleetCargo fc = { .p=pe, .q=q };
         array_push_back( &plist, fc );
      }
   }
   return plist;
}
