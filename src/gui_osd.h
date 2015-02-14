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


#ifndef OSD_H
#  define OSD_H


/*
 * OSD usage.
 */
unsigned int osd_create( const char *title,
      int nitems, const char **items, int priority );
int osd_destroy( unsigned int osd );
int osd_active( unsigned int osd, int msg );
int osd_getActive( unsigned int osd );
char *osd_getTitle( unsigned int osd );
char **osd_getItems( unsigned int osd, int *nitems );


/*
 * Subsystem usage.
 */
int osd_setup( int x, int y, int w, int h );
void osd_exit (void);
void osd_render (void);


#endif /* OSD_H */
