/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "pilot.h"

enum {
   PLAYER_BOARD_OK         = 0,
   PLAYER_BOARD_RETRY      = 1,
   PLAYER_BOARD_IMPOSSIBLE = 2,
   PLAYER_BOARD_ERROR      = 3,
};

int  player_isBoarded( void );
int  player_canBoard( int noisy );
int  player_tryBoard( int noisy );
void board_unboard( void );
int  pilot_board( Pilot *p );
void pilot_boardComplete( Pilot *p );
