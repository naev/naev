/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
 * We treat the ship as more or less a constant slab of steel.
 *
 *    @param p Pilot to update heat properties of.
 */
void pilot_heatCalc( Pilot *p )
{
   double mass_kg;
   mass_kg        = 1000. * p->base_mass;
   p->heat_emis   = 0.8; /**< @TODO make it influencable. */
   p->heat_cond   = STEEL_HEAT_CONDUCTIVITY;
   p->heat_C      = STEEL_HEAT_CAPACITY * mass_kg;

   /* We'll approximate area for a sphere.
    *
    * Sphere:
    *  V = 4/3*pi*r^3
    *  A = 4*pi*r^2
    *
    * Ship:
    *  V = mass/density
    *
    * We then equal the ship V and sphere V to obtain r:
    *  r = (3*mass)/(4*pi*density))^(1/3)
    *
    * Substituting r in A we get:
    *  A = 4*pi*((3*mass)/(4*pi*density))^(2/3)
    * */
   p->heat_area = 4.*M_PI*pow( 3./4.*mass_kg/STEEL_DENSITY/M_PI, 2./3. ) * p->stats.heat_dissipation;
}


/**
 * @brief Calculates the thermal mass of an outfit.
 */
double pilot_heatCalcOutfitC( const Outfit *o )
{
   /* Simple thermal mass. */
   return STEEL_HEAT_CAPACITY * 1000. * o->mass;
}


/**
 * @brief Calculates the effective transfer area of an outfit.
 *
 * @note This is currently independent of ship mounting.
 */
double pilot_heatCalcOutfitArea( const Outfit *o )
{
   double mass_kg = 1000. * o->mass;
   /* We consider the effective area of outfits to be half of a sphere. */
   return 2.*M_PI*pow( 3./4.*mass_kg/STEEL_DENSITY/M_PI, 2./3. );
}


/**
 * @brief Calculates the heat parameters for a pilot's slot.
 */
void pilot_heatCalcSlot( PilotOutfitSlot *o )
{
   o->heat_T      = CONST_SPACE_STAR_TEMP; /* Reset temperature. */
   o->heat_start  = CONST_SPACE_STAR_TEMP; /* For cooldown purposes. */
   if (o->outfit == NULL) {
      o->heat_C      = 1.;
      o->heat_area   = 0.;
      return;
   }
   o->heat_C      = pilot_heatCalcOutfitC(    o->outfit );
   o->heat_area   = pilot_heatCalcOutfitArea( o->outfit );
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
   for (i=0; i<p->noutfits; i++)
      p->outfits[i]->heat_T = CONST_SPACE_STAR_TEMP;
}


/**
 * @brief Adds heat to an outfit slot.
 *
 *    @param p Pilot whose slot it is.
 *    @param o The slot in question.
 */
void pilot_heatAddSlot( Pilot *p, PilotOutfitSlot *o )
{
   double hmod;
   /* We consider that only 1% of the energy is lost in the form of heat,
    * this keeps numbers sane. */
   if (o->outfit->type == OUTFIT_TYPE_BOLT)
      hmod = p->stats.fwd_heat;
   else if (o->outfit->type == OUTFIT_TYPE_TURRET_BOLT)
      hmod = p->stats.tur_heat;
   else
      hmod = 1.;
   o->heat_T += hmod * outfit_heat(o->outfit) / o->heat_C;

   /* Enforce a minimum value as a safety measure. */
   o->heat_T = MAX( o->heat_T, CONST_SPACE_STAR_TEMP );
}


/**
 * @brief Adds heat to an outfit slot over a period of time.
 *
 *    @param p Pilot whose slot it is.
 *    @param o The slot in question.
 *    @param dt Delta tick.
 */
void pilot_heatAddSlotTime( Pilot *p, PilotOutfitSlot *o, double dt )
{
   (void) p;
   double hmod;

   /* @todo Handle beam modifiers for ships here. */
   hmod = 1.;
   o->heat_T += (hmod * outfit_heat(o->outfit) / o->heat_C) * dt;

   /* Enforce a minimum value as a safety measure. */
   o->heat_T = MAX( o->heat_T, CONST_SPACE_STAR_TEMP );
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
 *    @return The energy transferred.
 */
double pilot_heatUpdateSlot( Pilot *p, PilotOutfitSlot *o, double dt )
{
   double Q;

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
 *  sigma being the Stefan-Boltzmann constant [5] = 5.67×10−8 W/(m^2 K^4)
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


/**
 * @brief Returns a 0:1 modifier representing efficiency (1. being normal).
 *
 *    @param T  Actual temperature (K)
 *    @param Tb Base temperature for overheating purposes (K)
 *    @param Tc Max temperature for overheating purposes (K)
 */
double pilot_heatEfficiencyMod( double T, double Tb, double Tc )
{
   return CLAMP( 0., 1., 1 - (T - Tb) / Tc );
}

/**
 * @brief Overrides the usual heat model during active cooldown.
 *
 *    @param p  Pilot to update.
 *    @param dt Delta tick.
 */
void pilot_heatUpdateCooldown( Pilot *p )
{
   double t;
   int i;
   PilotOutfitSlot *o;

   t = pow2( 1. - p->ctimer / p->cdelay );
   p->heat_T = p->heat_start - CONST_SPACE_STAR_TEMP - (p->heat_start -
         CONST_SPACE_STAR_TEMP) * t + CONST_SPACE_STAR_TEMP;

   for (i=0; i<p->noutfits; i++) {
      o = p->outfits[i];
      o->heat_T = o->heat_start - CONST_SPACE_STAR_TEMP - (o->heat_start -
            CONST_SPACE_STAR_TEMP) * t + CONST_SPACE_STAR_TEMP;
   }
}

/**
 * @brief Returns a 0:1 modifier representing accuracy (0. being normal).
 */
double pilot_heatAccuracyMod( double T )
{
   return CLAMP( 0., 1., (T-500.)/600. );
}


/**
 * @brief Returns a 0:1 modifier representing fire rate (1. being normal).
 */
double pilot_heatFireRateMod( double T )
{
   return CLAMP( 0., 1., (1100.-T)/300. );
}


/**
 * @brief Returns a 0:2 level of fire, 0:1 is the accuracy point, 1:2 is fire rate point.
 */
double pilot_heatFirePercent( double T )
{
   return 2*pilot_heatAccuracyMod(T);
}

