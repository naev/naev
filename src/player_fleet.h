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
int             pfleet_cargoMissionFree( void );
int             pfleet_cargoOwned( CommodityRef com );
int             pfleet_cargoAdd( CommodityRef com, int q );
unsigned int    pfleet_cargoMissionAdd( CommodityRef com, int q );
int             pfleet_cargoRm( CommodityRef com, int q, int jet );
PilotCommodity *pfleet_cargoList( void );
PFleetCargo    *pfleet_cargoListShips( CommodityRef com );
