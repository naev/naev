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


#ifndef NMATH_H
#  define NMATH_H


int nmath_solve2Eq( double results[2], double a, double b, double c );
double max3( double v1, double v2, double v3 );
double min3( double v1, double v2, double v3 );
void** arrayShuffle( void** array, int n);


#endif /* NMATH_H */

