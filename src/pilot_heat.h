/*
 * See Licensing and Copyright notice in naev.h
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
double pilot_heatUpdateSlot( Pilot *p, PilotOutfitSlot *o, double dt );
void pilot_heatUpdateShip( Pilot *p, double Q_cond, double dt );
void pilot_heatUpdateCooldown( Pilot *p );

/*
 * Modifiers.
 */
double pilot_heatAccuracyMod( double T );
double pilot_heatFireRateMod( double T );
double pilot_heatFirePercent( double T );


#endif /* PILOT_HEAT_H */
