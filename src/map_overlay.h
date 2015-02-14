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


#ifndef MAP_OVERLAY_H
#  define MAP_OVERLAY_H

#include "SDL.h"

/* Map overlay. */
int ovr_isOpen (void);
int ovr_input( SDL_Event *event );
void ovr_setOpen( int open );
void ovr_key( int type );
void ovr_render( double dt );
void ovr_refresh (void);

/* Markers. */
void ovr_mrkFree (void);
void ovr_mrkClear (void);
unsigned int ovr_mrkAddPoint( const char *text, double x, double y );
void ovr_mrkRm( unsigned int id );


#endif /* MAP_OVERLAY_H */

