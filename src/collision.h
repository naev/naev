/*
 * See Licensing and Copyright notice in naev.h
 */




#ifndef COLLISION_H
#  define COLLISION_H


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

/* Returns 1 if collision is detected */
int CollideSprite( const glTexture* at, const int asx, const int asy, const Vector2d* ap,
      const glTexture* bt, const int bsx, const int bsy, const Vector2d* bp,
      Vector2d* crash);
int CollideLineLine( double s1x, double s1y, double e1x, double e1y,
      double s2x, double s2y, double e2x, double e2y, Vector2d* crash );
int CollideLineSprite( const Vector2d* ap, double ad, double al,
      const glTexture* bt, const int bsx, const int bsy, const Vector2d* bp,
      Vector2d crash[2]);
int CollideLinePolygon( const Vector2d* ap, double ad, double al,
      const CollPoly* bt, const Vector2d* bp, Vector2d crash[2] );
int CollideSpritePolygon( const CollPoly* at, const Vector2d* ap,
      const glTexture* bt, const int bsx, const int bsy, const Vector2d* bp,
      Vector2d* crash );
int CollidePolygon( const CollPoly* at, const Vector2d* ap,
      const CollPoly* bt, const Vector2d* bp, Vector2d* crash );

#endif /* COLLISION_H */
