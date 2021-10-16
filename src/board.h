/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "pilot.h"

int player_isBoarded (void);
void player_board (void);
void board_unboard (void);
int pilot_board( Pilot *p );
void pilot_boardComplete( Pilot *p );
void board_exit( unsigned int wdw, const char* str );
