#ifndef _TCOD_PERLIN_H
#define _TCOD_PERLIN_H


#include "opengl.h"


float* noise_genNebulaeMap( const int w, const int h, const int n, float rug );
SDL_Surface* noise_surfaceFromNebulaeMap( float* map, const int w, const int h );
glTexture* noise_genCloud( const int w, const int h, double rug );


#endif
