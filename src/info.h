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


#ifndef INFO_H
#  define INFO_H


#define INFO_MAIN       0 /**< Main info window. */
#define INFO_SHIP       1 /**< Ship info window. */
#define INFO_WEAPONS    2 /**< Weapons info window. */
#define INFO_CARGO      3 /**< Cargo info window. */
#define INFO_MISSIONS   4 /**< Missions info window. */
#define INFO_STANDINGS  5 /**< Standings info window. */


/*
 * Menu opening routines.
 */
void menu_info( int window );
void info_update (void);


#endif /* INFO_H */
