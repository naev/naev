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


#ifndef PLAYER_GUI_H
#  define PLAYER_GUI_H


/* Clean up. */
void player_guiCleanup (void);

/* Manipulation. */
int player_guiAdd( char* name );
void player_guiRm( char* name );
int player_guiCheck( char* name );

/* High level. */
char** player_guiList( int *n );


#endif /* PLAYER_GUI_H */

