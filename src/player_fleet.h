/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "player.h"

void player_fleetUpdate (void);
int player_fleetDeploy( PlayerShip_t *ps, int deploy );
int player_fleetCargoUsed (void);
int player_fleetCargoFree (void);
int player_fleetCargoOwned( const Commodity *com );
int player_fleetCargoAdd( const Commodity *com, int q );
int player_fleetCargoRm( const Commodity *com, int q );
