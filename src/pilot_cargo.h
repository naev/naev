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


#ifndef PILOT_CARGO_H
#  define PILOT_CARGO_H


#include "pilot.h"


/*
 * Normal Cargo.
 */
int pilot_cargoUsed( Pilot* pilot ); /* gets how much cargo it has onboard */
int pilot_cargoFree( Pilot* p ); /* cargo space */
int pilot_cargoOwned( Pilot* pilot, const char* commodityname );
int pilot_cargoAdd( Pilot* pilot, Commodity* cargo, int quantity );
int pilot_cargoRm( Pilot* pilot, Commodity* cargo, int quantity );
int pilot_cargoMove( Pilot* dest, Pilot* src );
void pilot_cargoCalc( Pilot* pilot );

/*
 * Normal cargo below the abstractions.
 */
int pilot_cargoRmRaw( Pilot* pilot, Commodity* cargo, int quantity, int cleanup );
int pilot_cargoAddRaw( Pilot* pilot, Commodity* cargo,
      int quantity, unsigned int id );


/*
 * Mission cargo - not to be confused with normal cargo.
 */
unsigned int pilot_addMissionCargo( Pilot* pilot, Commodity* cargo, int quantity );
int pilot_rmMissionCargo( Pilot* pilot, unsigned int cargo_id, int jettison );


#endif /* PILOT_CARGO_H */
