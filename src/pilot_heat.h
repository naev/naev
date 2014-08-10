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


#ifndef PILOT_HEAT_H
#  define PILOT_HEAT_H


#include "pilot.h"


/*
 * Fundamental heat properties.
 */
#define HEAT_WORST_ACCURACY         (38./180.*M_PI) /**< Pretty bad accuracy, a 76 degree arc. */


/*
 * Some random physics constants.
 */
#define CONST_STEFAN_BOLTZMANN      (5.67e-8) /**< Stefan-Botzmann thermal radiation constant. [W/(m^2 K^4)] */
#define CONST_SPACE_TEMP            (3.18) /**< Aproximation of the absolute temperature of space. [K] */
#define CONST_SPACE_TEMP_4          \
(CONST_SPACE_TEMP*CONST_SPACE_TEMP*CONST_SPACE_TEMP*CONST_SPACE_TEMP) /**< CONST_SPACE_TEMP^4 */
#define CONST_SPACE_STAR_TEMP       (250.) /**< Aproximation of the black body temperature near a star. */
#define CONST_SPACE_STAR_TEMP_4     \
(CONST_SPACE_STAR_TEMP*CONST_SPACE_STAR_TEMP*CONST_SPACE_STAR_TEMP*CONST_SPACE_STAR_TEMP) /**< CONST_SPACE_STAR_TEMP^4 */


/*
 * Properties of steel.
 *
 * Yes, there are many different types of steels, these are sort of "average values" for carbon steel.
 */
#define STEEL_HEAT_CONDUCTIVITY     (54.) /**< Thermal conductivity of steel (@ 25C). [W/(m*K)] */
#define STEEL_HEAT_CAPACITY         (0.49) /**< Thermal capacity of steel. [J/(kg*K)] */
#define STEEL_DENSITY               (7.88e3) /**< Density of steel. [kg/m^3] */


/*
 * Outfit core value calculations.
 */
double pilot_heatCalcOutfitC( const Outfit *o );
double pilot_heatCalcOutfitArea( const Outfit *o );

/*
 * Heat initializations.
 */
void pilot_heatCalc( Pilot *p );
void pilot_heatCalcSlot( PilotOutfitSlot *o );

/*
 * Heat management.
 */
void pilot_heatReset( Pilot *p );
void pilot_heatAddSlot( Pilot *p, PilotOutfitSlot *o );
void pilot_heatAddSlotTime( Pilot *p, PilotOutfitSlot *o, double dt );
double pilot_heatUpdateSlot( Pilot *p, PilotOutfitSlot *o, double dt );
void pilot_heatUpdateShip( Pilot *p, double Q_cond, double dt );
void pilot_heatUpdateCooldown( Pilot *p );

/*
 * Modifiers.
 */
double pilot_heatEfficiencyMod( double T, double Tb, double Tc );
double pilot_heatAccuracyMod( double T );
double pilot_heatFireRateMod( double T );
double pilot_heatFirePercent( double T );


#endif /* PILOT_HEAT_H */
