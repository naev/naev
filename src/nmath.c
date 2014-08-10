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

/**
 * @file nmath.c
 *
 * @brief Some math routines for naev.
 */


#include "nmath.h"
#include "rng.h"
#include "naev.h"

#include <math.h>

#include "log.h"


/**
 * @brief Solves the equation:  a * x^2 + b * x + c = 0
 *
 *    @param results Stores both results.
 *    @param a Quadratic parameter.
 *    @param b Linear parameter.
 *    @param c Offset coefficient.
 *    @return 0 on success, -1 on error.
 */
int nmath_solve2Eq( double results[2], double a, double b, double c )
{
   double root;

   /* Calculate the root. */
   root = b * b - 4 * a * c;
   if (root < 0.)
      return -1;
   root = sqrt(root);

   /* Set the results. */
   results[0] = (-b + root) / (2 * a);
   results[1] = (-b - root) / (2 * a);

   return 0;
}


/**
 * @brief Returns the maximum of 3 values.
 */
double max3( double v1, double v2, double v3 )
{
   double max;

   max = (v1 > v2) ? v1 : v2;
   max = (max > v3) ? max : v3;
   return(max);
}


/**
 * @brief Returns the minimum of 3 values.
 */
double min3( double v1, double v2, double v3 )
{
   double min;

   min = (v1 < v2) ? v1 : v2;
   min = (min < v3) ? min : v3;
   return(min);
}

/**
 *  @brief Randomly sorts an array with the Fisher-Yates shuffle.
 *
 *    @param array Array to be sorted.
 *    @param n Number of elements in the array.
 *    @return Randomly-ordered array.
 */
void** arrayShuffle( void** array, int n)
{
   void* tmp;
   int k;

   while (n > 1) {
      k        = RNG(0, n);
      tmp      = array[--n];
      array[n] = array[k];
      array[k] = tmp;
   }

   return array;
}
