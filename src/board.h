/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "pilot.h"

int player_isBoarded (void);
int player_tryBoard (void);
void board_unboard (void);
int pilot_board( Pilot *p );
void pilot_boardComplete( Pilot *p );
