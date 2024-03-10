/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file nmath.c
 *
 * @brief Some math routines for naev.
 */
/** @cond */
#include <math.h>

#include "naev.h"
/** @endcond */

#include "nmath.h"

#include "array.h"
#include "log.h"
#include "rng.h"

/**
 * @brief Solves the equation:  a * x^2 + b * x + c = 0
 *
 *    @param results Stores both results.
 *    @param a Quadratic parameter.
 *    @param b Linear parameter.
 *    @param c Offset coefficient.
 *    @return 0 on success, -1 on error (or complex).
 */
int nmath_solve2Eq( double results[2], double a, double b, double c )
{
   /* Calculate the root. */
   double root = b * b - 4 * a * c;
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
   double max = (v1 > v2) ? v1 : v2;
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
 *  @brief Randomly sorts an array (array.h) of pointers in place with the Fisher-Yates shuffle.
 */
void arrayShuffle( void** array )
{
   for (int n=array_size( array ); n > 1; ) {
      int k       = RNG(0, n);
      void *tmp   = array[--n];
      array[n]    = array[k];
      array[k]    = tmp;
   }
}

/**
 * @brief Checks whether two rectangles overlap at any point.
 *
 *    @param x X coordinate of first rectangle
 *    @param y Y coordinate of first rectangle
 *    @param w Width of first rectangle
 *    @param h Height of first rectangle
 *    @param x2 X coordinate of second rectangle
 *    @param y2 Y coordinate of second rectangle
 *    @param w2 Width of second rectangle
 *    @param h2 Height of second rectangle
 *    @return 1 if the rectangles overlap, 0 otherwise.
 */
int rectOverlap( double x, double y, double w, double h,
      double x2, double y2, double w2, double h2 )
{
   /* Too far left, down, right, or up, respectively. */
   if ((x+w < x2) || (y+h < y2) || (x > x2+w2) || (y > y2+h2))
      return 0;

   return 1;
}

/**
 * @brief Simple symmetric sine-based easing.
 */
double ease_SineInOut( double x )
{
   return 0.5 * (1. - cos(x * M_PI));
}

/**
 * @brief Simple symmetric quadratic easing.
 */
double ease_QuadraticInOut( double x )
{
   if (x < 0.5)
      return 2.*x*x;
   return -2.*x*x + 4*x - 1.;
}

/**
 * @brief Simple symmetric cubic easing.
 */
double ease_CubicInOut( double x )
{
   if (x < 0.5)
      return 4.*x*x*x;
   double y = 2.*x - 2.;
   return 0.5*y*y*y+1.;
}
