/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef PILOT_EW_H
#  define PILOT_EW_H


#include "pilot.h"



/*
 * Sensors and range.
 */
void pilot_updateSensorRange (void);
double pilot_sensorRange( void );
int pilot_inRange( const Pilot *p, double x, double y );
int pilot_inRangePilot( const Pilot *p, const Pilot *target );
int pilot_inRangePlanet( const Pilot *p, int target );
int pilot_inRangeJump( const Pilot *p, int target );

/*
 * Weapon tracking.
 */
double pilot_ewWeaponTrack( const Pilot *p, const Pilot *t, double track );

/*
 * Electronic warfare updating.
 */
void pilot_ewUpdateStatic( Pilot *p );
void pilot_ewUpdateDynamic( Pilot *p );

/*
 * Individual electronic warfare properties.
 */
double pilot_ewMovement( double vmod );
double pilot_ewEvasion( const Pilot *target );
double pilot_ewHeat( double T );
double pilot_ewMass( double mass );


#endif /* PILOT_EW_H */
