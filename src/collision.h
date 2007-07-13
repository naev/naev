


#ifndef COLLISION_H
#  define COLLISION_H


#include "opengl.h"
#include "physics.h"


int CollideSprite( const gl_texture* at, const int asx, const int asy, const Vector2d* ap,
      const gl_texture* bt, const int bsx, const int bsy, const Vector2d* bp );


#endif /* COLLISION_H */
