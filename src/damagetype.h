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


#ifndef _DTYPE_H
#  define _DTYPE_H


#include "outfit.h"

/*
 * stack manipulation
 */
int dtype_get( char* name );
char* dtype_damageTypeToStr( int type );

/*
 * dtype effect loading and freeing
 */
int dtype_load (void);
void dtype_free (void);

/*
 * misc
 */
void dtype_calcDamage( double *dshield, double *darmour, double absorb,
      double *knockback, const Damage *dmg, ShipStats *s );


#endif /* _DTYPE_H */

