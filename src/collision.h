/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nxml.h" // IWYU pragma: keep
#include "opengl_tex.h"
#include "vec2.h"

/**
 * @brief Represents a polygon used for collision detection.
 */
typedef struct CollPolyView CollPolyView;
typedef struct CollPoly     CollPoly;

/* Loads a polygon data from xml. */
CollPoly *poly_load( xmlNodePtr node, const char *name );
void      poly_free( CollPoly *polygon );
void      poly_freeView( CollPolyView *view );

/* Rotates a polygon. */
CollPolyView *poly_rotate( const CollPoly *polygon, float theta );

/* Gets a polygon view for an angle. */
const CollPolyView *poly_view( const CollPoly *poly, double dir );

/* Returns 1 if collision is detected */
int CollideSprite( const glTexture *at, const int asx, const int asy,
                   const vec2 *ap, const glTexture *bt, const int bsx,
                   const int bsy, const vec2 *bp, vec2 *crash );
int CollideLineLine( double s1x, double s1y, double e1x, double e1y, double s2x,
                     double s2y, double e2x, double e2y, vec2 *crash );
int CollideLineSprite( const vec2 *ap, double ad, double al,
                       const glTexture *bt, const int bsx, const int bsy,
                       const vec2 *bp, vec2 crash[2] );
int CollideCirclePolygon( const vec2 *ap, double ar, const CollPolyView *bt,
                          const vec2 *bp, vec2 crash[2] );
int CollideCircleSprite( const vec2 *ap, double ar, const glTexture *bt,
                         const int bsx, const int bsy, const vec2 *bp,
                         vec2 *crash );
int CollideLinePolygon( const vec2 *ap, double ad, double al,
                        const CollPolyView *bt, const vec2 *bp, vec2 crash[2] );
int CollideSpritePolygon( const CollPolyView *apoly, const vec2 *ap,
                          const glTexture *bt, const int bsx, const int bsy,
                          const vec2 *bp, vec2 *crash );
int CollidePolygon( const CollPolyView *at, const vec2 *ap,
                    const CollPolyView *bt, const vec2 *bp, vec2 *crash );
int CollideLineCircle( const vec2 *p1, const vec2 *p2, const vec2 *cc,
                       double cr, vec2 crash[2] );
int CollideCircleCircle( const vec2 *p1, double r1, const vec2 *p2, double r2,
                         vec2 crash[2] );

/* Intersection area. */
double CollideCircleIntersection( const vec2 *p1, double r1, const vec2 *p2,
                                  double r2 );
