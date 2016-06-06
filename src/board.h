/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef BOARD_H
#  define BOARD_H


#include "pilot.h"


int player_isBoarded (void);
void player_board (void);
void board_unboard (void);
int pilot_board( Pilot *p );
void pilot_boardComplete( Pilot *p );
void board_exit( unsigned int wdw, char* str );


#endif
