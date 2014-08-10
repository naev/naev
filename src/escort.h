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


#ifndef ESCORT_H
#  define ESCORT_H


#include "physics.h"
#include "pilot.h"


/* Creation. */
int escort_addList( Pilot *p, char *ship, EscortType_t type, unsigned int id );
unsigned int escort_create( Pilot *p, char *ship,
      Vector2d *pos, Vector2d *vel, double dir,
      EscortType_t type, int add );

/* Keybind commands. */
int escorts_attack( Pilot *parent );
int escorts_hold( Pilot *parent );
int escorts_return( Pilot *parent );
int escorts_clear( Pilot *parent );


#endif /* ESCORT_H */
