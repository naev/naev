/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef CAMERA_H
#  define CAMERA_H


#define CAMERA_DEFSPEED    2500 /**< Default camera speed. */


/*
 * Get values.
 */
double cam_getZoom (void);
double cam_getZoomTarget (void);
void cam_getPos( double *x, double *y );
int cam_getTarget( void );


/*
 * Set targets.
 */
void cam_setZoom( double zoom );
void cam_setZoomTarget( double zoom );
void cam_setTargetPilot( unsigned int follow, int soft_over );
void cam_setTargetPos( double x, double y, int soft_over );


/*
 * Update.
 */
void cam_update( double dt );


#endif /* CAMERA_H */

