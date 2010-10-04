/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef MAP_OVERLAY_H
#  define MAP_OVERLAY_H

#include "SDL.h"


int ovr_isOpen (void);
int ovr_input( SDL_Event *event );
void ovr_key( int type );
void ovr_render( double dt );
void ovr_refresh (void);


#endif /* MAP_OVERLAY_H */

