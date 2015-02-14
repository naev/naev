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
