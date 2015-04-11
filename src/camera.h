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

