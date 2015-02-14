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


#ifndef GUI_OMSG_H
#  define GUI_OMSG_H



#define OMSG_FONT_DEFAULT_SIZE      16


/*
 * Creation and management.
 */
unsigned int omsg_add( const char *msg, double duration, int fontsize );
int omsg_change( unsigned int id, const char *msg, double duration );
int omsg_exists( unsigned int id );
void omsg_rm( unsigned int id );

/*
 * Global stuff.
 */
void omsg_position( double center_x, double center_y, double width );
void omsg_cleanup (void);
void omsg_render( double dt );


#endif /* GUI_OMSG_H */
