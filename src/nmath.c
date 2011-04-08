/*
 * See Licensing and Copyright notice in naev.h
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
 *    @param c Offset coeficient.
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
char** arrayShuffle( char** array, int n)
{
   char* tmp;
   int k;

   while (n > 1) {
      k = RNG(0, n);
      tmp = array[--n];
      array[n] = array[k];
      array[k] = tmp;
   }
   return array;
}
