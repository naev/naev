/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

int    nmath_solve2Eq( double results[2], double a, double b, double c );
double max3( double v1, double v2, double v3 );
double min3( double v1, double v2, double v3 );
void   arrayShuffle( void **array );
int rectOverlap( double x, double y, double w, double h, double x2, double y2,
                 double w2, double h2 );

/* Easing. */
double ease_SineInOut( double x );
double ease_QuadraticInOut( double x );
double ease_CubicInOut( double x );
