/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nmath.c
 *
 * @brief Some math routines for naev.
 */

#include "nmath.h"

#include <math.h>

#include "naev.h"
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

