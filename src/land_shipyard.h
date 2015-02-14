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


#ifndef LAND_SHIPYARD_H
#  define LAND_SHIPYARD_H


#include "land.h"


/*
 * Window stuff.
 */
void shipyard_open( unsigned int wid );
void shipyard_update( unsigned int wid, char* str );


/*
 * Helper functions.
 */
int shipyard_canBuy( char *shipname, Planet *planet );
int shipyard_canTrade( char *shipname );


#endif /* LAND_SHIPYARD_H */
