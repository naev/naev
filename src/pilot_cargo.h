/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "pilot.h"

/*
 * Normal Cargo.
 */
int pilot_cargoUsed(
   const Pilot *pilot ); /* gets how much cargo it has onboard */
int  pilot_cargoUsedMission( const Pilot *pilot ); /* For mission cargo. */
int  pilot_cargoFree( const Pilot *p );            /* cargo space */
int  pilot_cargoOwned( const Pilot *pilot, const Commodity *cargo );
int  pilot_cargoOwnedMission( const Pilot *pilot, int id );
int  pilot_cargoAdd( Pilot *pilot, const Commodity *cargo, int quantity,
                     unsigned int id );
int  pilot_cargoRm( Pilot *pilot, const Commodity *cargo, int quantity );
int  pilot_cargoJet( Pilot *p, const Commodity *cargo, int quantity,
                     int simulate );
int  pilot_cargoMove( Pilot *dest, Pilot *src );
int  pilot_cargoMoveRaw( Pilot *dest, Pilot *src );
void pilot_cargoCalc( Pilot *pilot );

/*
 * Normal cargo below the abstractions.
 */
int pilot_cargoRmRaw( Pilot *pilot, const Commodity *cargo, int quantity,
                      int cleanup );
int pilot_cargoRmAll( Pilot *pilot, int cleanup );
int pilot_cargoAddRaw( Pilot *pilot, const Commodity *cargo, int quantity,
                       unsigned int id );

/*
 * Mission cargo - not to be confused with normal cargo.
 */
unsigned int pilot_addMissionCargo( Pilot *pilot, const Commodity *cargo,
                                    int quantity );
int pilot_rmMissionCargo( Pilot *pilot, unsigned int cargo_id, int jettison );
