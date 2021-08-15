/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef MAP_OVERLAY_H
#  define MAP_OVERLAY_H

/** @cond */
#include "SDL.h"
/** @endcond */

/* Map overlay. */
int ovr_isOpen (void);
int ovr_input( SDL_Event *event );
void ovr_setOpen( int open );
void ovr_key( int type );
void ovr_render( double dt );
void ovr_refresh (void);

/* Markers. */
void ovr_mrkFree (void);
void ovr_mrkClear (void);
unsigned int ovr_mrkAddPoint( const char *text, double x, double y );
void ovr_mrkRm( unsigned int id );


#endif /* MAP_OVERLAY_H */

