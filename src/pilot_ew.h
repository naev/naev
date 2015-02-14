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
double pilot_ewHeat( double T );
double pilot_ewMass( double mass );


#endif /* PILOT_EW_H */
