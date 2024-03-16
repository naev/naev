/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "pilot.h"
#include "player.h"

typedef struct PFleetCargo_ {
   Pilot *p;
   int    q;
} PFleetCargo;

void            pfleet_update( void );
void            pfleet_cargoRedistribute( void );
int             pfleet_toggleDeploy( PlayerShip_t *ps, int deploy );
int             pfleet_deploy( PlayerShip_t *ps );
int             pfleet_cargoUsed( void );
int             pfleet_cargoFree( void );
int             pfleet_cargoOwned( const Commodity *com );
int             pfleet_cargoAdd( const Commodity *com, int q );
int             pfleet_cargoRm( const Commodity *com, int q, int jet );
PilotCommodity *pfleet_cargoList( void );
PFleetCargo    *pfleet_cargoListShips( const Commodity *com );
