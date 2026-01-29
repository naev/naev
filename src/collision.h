/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nxml.h" // IWYU pragma: keep
#include "vec2.h"

/**
 * @brief Represents a polygon used for collision detection.
 */
typedef struct CollPolyView CollPolyView;
typedef struct CollPoly     CollPoly;

/* Loads a polygon data from xml. */
CollPoly *poly_load_xml( const char *name );
CollPoly *poly_load_2d( const char *name, int sx, int sy );
void      poly_free( CollPoly *polygon );
void      poly_free_view( CollPolyView *view );

/* Rotates a polygon. */
CollPolyView *poly_rotate( const CollPoly *polygon, float theta );

/* Gets a polygon view for an angle. */
const CollPolyView *poly_view( const CollPoly *poly, double dir );
const vec2         *poly_points( const CollPolyView *view, int *n );

/* Returns 1 if collision is detected */
int collide_line_line( double s1x, double s1y, double e1x, double e1y,
                       double s2x, double s2y, double e2x, double e2y,
                       vec2 *crash );
int collide_circle_polygon( const vec2 *ap, double ar, const CollPolyView *bt,
                            const vec2 *bp, vec2 crash[2] );
int collide_line_polygon( const vec2 *ap, double ad, double al,
                          const CollPolyView *bt, const vec2 *bp,
                          vec2 crash[2] );
int collide_polygon_polygon( const CollPolyView *at, const vec2 *ap,
                             const CollPolyView *bt, const vec2 *bp,
                             vec2 *crash );
int collide_line_circle( const vec2 *p1, const vec2 *p2, const vec2 *cc,
                         double cr, vec2 crash[2] );
int collide_circle_circle( const vec2 *p1, double r1, const vec2 *p2, double r2,
                           vec2 crash[2] );
