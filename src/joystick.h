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


#ifndef JOYSTICK_H
#  define JOYSTICK_H


/*
 * gets the joystick index number based on its name
 */
int joystick_get( const char* namjoystick );

/*
 * sets the game to use the joystick of index indjoystick
 */
int joystick_use( int indjoystick );

/*
 * init/exit functions
 */
int joystick_init (void);
void joystick_exit (void);


#endif /* JOYSTICK_H */
