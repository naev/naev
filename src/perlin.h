#ifndef _TCOD_PERLIN_H
#define _TCOD_PERLIN_H


#include "opengl.h"


void noise_generateNebulae( const int w, const int h );
glTexture* noise_genCloud( const int w, const int h, double rug );


#endif
