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


#ifndef DEV_UNIEDIT_H
#  define DEV_UNIEDIT_H

#define HIDE_DEFAULT_JUMP        1.25 /**< Default hide value for new jumps. */
#define RADIUS_DEFAULT           10000 /**< Default radius for new systems. */
#define STARS_DENSITY_DEFAULT    400 /**< Default stars density for new systems. */


void uniedit_open( unsigned int wid_unused, char *unused );
void uniedit_selectText (void);
char *uniedit_nameFilter( char *name );
void uniedit_autosave( unsigned int wid_unused, char *unused );
void uniedit_updateAutosave (void);


#endif /* DEV_UNIEDIT_H */
