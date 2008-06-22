/*
 * See Licensing and Copyright notice in naev.h
 */




#ifndef COLLISION_H
#  define COLLISION_H


#include "opengl.h"
#include "physics.h"


/* Returns 1 if collision is detected */
int CollideSprite( const glTexture* at, const int asx, const int asy, const Vector2d* ap,
      const glTexture* bt, const int bsx, const int bsy, const Vector2d* bp );
/* Gets direction to face the target */
/*double CollidePath( const Vector2d* p, Vector2d* v,
      const Vector2d* tp, Vector2d* tv, const double limit );*/


#endif /* COLLISION_H */
