/*
 * See Licensing and Copyright notice in naev.h
 */


/**
 * @file pilot_heat.c
 *
 * @brief Handles the pilot heat stuff.
 */


#include "pilot_heat.h"

#include "naev.h"

#include <math.h>

#include "log.h"


/**
 * @brief Calculates the heat parameters for a pilot.
 *
 * We treat the ship as more or less ac onstant slab of steel.
 *
 *    @param p Pilot to update heat properties of.
 */
void pilot_heatCalc( Pilot *p )
{
   double mass_kg;
   mass_kg        = 1000. * p->ship->mass;
   p->heat_emis   = 0.8; /**< @TODO make it influencable. */
   p->heat_cond   = STEEL_HEAT_CONDUCTIVITY;
   p->heat_C      = STEEL_HEAT_CAPACITY * mass_kg;
   p->heat_area   = pow( mass_kg / STEEL_DENSITY, 2./3. );
   p->heat_T      = CONST_SPACE_TEMP; /* Reset temperature. */
}


/**
 * @brief Calculates the heat parameters for a pilot's slot.
 */
void pilot_heatCalcSlot( PilotOutfitSlot *o )
{
   double mass_kg;
   if (o->outfit == NULL)
      return;
   mass_kg        = 1000. * o->outfit->mass;
   o->heat_C      = STEEL_HEAT_CAPACITY * mass_kg;
   o->heat_area   = pow( mass_kg / STEEL_DENSITY, 2./3. );
   o->heat_T      = CONST_SPACE_TEMP; /* Reset temperature. */
}


/**
 * @brief Resets a pilot's heat.
 *
 *    @param p Pilot to reset heat of.
 */
void pilot_heatReset( Pilot *p )
{
   int i;
   
   p->heat_T = CONST_SPACE_STAR_TEMP;
   for (i=0; i<p->noutfits; i++) {
      if (p->outfits[i]->outfit == NULL)
         continue;
      p->heat_T = CONST_SPACE_TEMP;
   }
}


/**
 * @brief Adds heat to an outfit slot.
 *
 *    @param o Outfit to heat.
 *    @param energy Energy recieved by outfit (in W).
 */
void pilot_heatAddSlot( PilotOutfitSlot *o, double energy )
{
   o->heat_T += 1000. * energy / o->heat_C;
}


/**
 * @brief Heats the pilot's slot.
 *
 * We only consider conduction with the ship's chassis.
 *
 * q = -k * dT/dx
 *
 *  q being heat flux W/m^2
 *  k being conductivity W/(m*K)
 *  dT/dx temperature gradient along one dimension K/m 
 *
 * Slots are connected only with the chassis.
 *
 *    @param p Pilot to update.
 *    @param o Outfit slot to update.
 *    @param dt Delta tick.
 *    @return The energy transfered.
 */
double pilot_heatUpdateSlot( Pilot *p, PilotOutfitSlot *o, double dt )
{
   double Q;

   /* Must have an outfit to be valid. */
   if (o->outfit == NULL)
      return 0.;

   /* Calculate energy leaving/entering ship chassis. */
   Q           = -p->heat_cond * (o->heat_T - p->heat_T) * o->heat_area * dt;

   /* Update current temperature. */
   o->heat_T  += Q / o->heat_C;

   /* Return energy moved. */
   return Q;
}


/**
 * @brief Heats the pilot's ship.
 *
 * The ship besides having conduction like in pilot_heatUpdateSlot it also has
 *  radiation. So now the equation we use is:
 *
 *  q = -k * dT/dx + sigma * epsilon * (T^4 - To^4)
 *
 * However the first part is passed as parameter p so we get:
 *
 *  q = p + sigma * epsilon * (T^4 - To^4)
 *
 *  sigma being the stefan-boltzmann constant [5] = 5.67×10−8 W/(m^2 K^4)
 *  epsilon being a parameter between 0 and 1 (1 being black body)
 *  T being body temperature
 *  To being "space temperature"
 *
 *    @param p Pilot to update.
 *    @param Q Heat energy moved from slots.
 *    @param dt Delta tick.
 */
void pilot_heatUpdateShip( Pilot *p, double Q_cond, double dt )
{
   double Q, Q_rad;

   /* Calculate radiation. */
   Q_rad       = CONST_STEFAN_BOLTZMANN * p->heat_area * p->heat_emis *
         (CONST_SPACE_STAR_TEMP_4 - pow(p->heat_T,4.)) * dt;

   /* Total heat movement. */
   Q           = Q_rad - Q_cond;

   /* Update ship temperature. */
   p->heat_T  += Q / p->heat_C;
}


