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


#ifndef MAP_H
#  define MAP_H


#include "space.h"


/* init/exit */
int map_init (void);
void map_exit (void);

/* open the map window */
void map_open (void);
void map_close (void);
int map_isOpen (void);

/* misc */
StarSystem* map_getDestination( int *jumps );
void map_setZoom( double zoom );
void map_select( StarSystem *sys, char shifted );
void map_cleanup (void);
void map_clear (void);
void map_jump (void);

/* manipulate universe stuff */
StarSystem** map_getJumpPath( int* njumps, const char* sysstart,
     const char* sysend, int ignore_known, int show_hidden,
     StarSystem** old_data );
int map_map( const Outfit *map );
int map_isMapped( const Outfit* map );

/* Local map stuff. */
int localmap_map( const Outfit *lmap );
int localmap_isMapped( const Outfit *lmap );

/* shows a map at x, y (relative to wid) with size w,h  */
void map_show( int wid, int x, int y, int w, int h, double zoom );
int map_center( const char *sys );

/* Internal rendering sort of stuff. */
void map_renderParams( double bx, double by, double xpos, double ypos,
      double w, double h, double zoom, double *x, double *y, double *r );
void map_renderFactionDisks( double x, double y, int editor);
void map_renderJumps( double x, double y, int editor);
void map_renderSystems( double bx, double by, double x, double y,
      double w, double h, double r, int editor );
void map_renderNames( double x, double y, int editor );


#endif /* MAP_H */

