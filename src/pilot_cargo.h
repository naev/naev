/*
 * See Licensing and Copyright notice in naev.h
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
int pilot_cargoAdd( Pilot* pilot, Commodity* cargo,
      int quantity, unsigned int id );
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
