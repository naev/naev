/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "nxml.h"
#include "opengl.h"
#include "physics.h"

/**
 * @brief Represents a polygon used for collision detection.
 */
typedef struct CollPoly_ {
   float* x; /**< List of X coordinates of the points. */
   float* y; /**< List of Y coordinates of the points. */
   float xmin; /**< Min of x. */
   float xmax; /**< Max of x. */
   float ymin; /**< Min of y. */
   float ymax; /**< Max of y. */
   int npt; /**< Nb of points in the polygon. */
} CollPoly;

/* Loads a polygon data from xml. */
void LoadPolygon( CollPoly* polygon, xmlNodePtr node );

/* Rotates a polygon. */
void RotatePolygon( CollPoly* rpolygon, CollPoly* ipolygon, float theta );

/* Returns 1 if collision is detected */
int CollideSprite( const glTexture* at, const int asx, const int asy, const vec2* ap,
      const glTexture* bt, const int bsx, const int bsy, const vec2* bp,
      vec2* crash);
int CollideLineLine( double s1x, double s1y, double e1x, double e1y,
      double s2x, double s2y, double e2x, double e2y, vec2* crash );
int CollideLineSprite( const vec2* ap, double ad, double al,
      const glTexture* bt, const int bsx, const int bsy, const vec2* bp,
      vec2 crash[2]);
int CollideCirclePolygon( const vec2* ap, double ar,
      const CollPoly* bt, const vec2* bp, vec2 crash[2] );
int CollideLinePolygon( const vec2* ap, double ad, double al,
      const CollPoly* bt, const vec2* bp, vec2 crash[2] );
int CollideSpritePolygon( const CollPoly* at, const vec2* ap,
      const glTexture* bt, const int bsx, const int bsy, const vec2* bp,
      vec2* crash );
int CollidePolygon( const CollPoly* at, const vec2* ap,
      const CollPoly* bt, const vec2* bp, vec2* crash );
int CollideLineCircle( const vec2* p1, const vec2* p2,
      const vec2 *cc, double cr, vec2 crash[2] );
int CollideCircleCircle( const vec2 *p1, double r1,
      const vec2 *p2, double r2, vec2 crash[2] );

/* Intersection area. */
double CollideCircleIntersection( const vec2 *p1, double r1,
      const vec2 *p2, double r2 );
