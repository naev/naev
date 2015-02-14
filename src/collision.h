/*
 * Copyright 2006-2012 Edgar Simo Serra
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef COLLISION_H
#  define COLLISION_H


#include "opengl.h"
#include "physics.h"


/* Returns 1 if collision is detected */
int CollideSprite( const glTexture* at, const int asx, const int asy, const Vector2d* ap,
      const glTexture* bt, const int bsx, const int bsy, const Vector2d* bp,
      Vector2d* crash);
int CollideLineLine( double s1x, double s1y, double e1x, double e1y,
      double s2x, double s2y, double e2x, double e2y, Vector2d* crash );
int CollideLineSprite( const Vector2d* ap, double ad, double al,
      const glTexture* bt, const int bsx, const int bsy, const Vector2d* bp,
      Vector2d crash[2]);


#endif /* COLLISION_H */
