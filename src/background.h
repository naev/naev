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


#ifndef BACKGROUND_H
#  define BACKGROUND_H


#include "opengl.h"
#include "colour.h"


/* Render. */
void background_render( double dt );

/* Add images. */
unsigned int background_addImage( glTexture *image, double x, double y,
      double move, double scale, const glColour *col, int foreground );

/* Stars. */
void background_initStars( int n );
void background_renderStars( const double dt );
void background_moveStars( double x, double y );

/* Init. */
int background_init (void);
int background_load( const char *name );


/* Clean up. */
void background_clear (void);
void background_free (void);


#endif /* BACKGROUND_H */


