/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef CAMERA_H
#  define CAMERA_H


#include "physics.h"


void cam_setZoom( double zoom );
double cam_getZoom (void);
void cam_bind( Vector2d* pos );
void cam_setStatic( double x, double y );
void cam_getPos( double *x, double *y );
void cam_update( Solid *sld );


#endif /* CAMERA_H */

