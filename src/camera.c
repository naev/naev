/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file camera.c
 *
 * @brief Handles the camera.
 */


#include "camera.h"

#include "naev.h"

#include "log.h"


static Vector2d* camera_pos = NULL; /**< Camera we are using. */
static double camera_Z    = 1.; /**< Current in-game zoom. */
static double camera_X    = 0.; /**< X position of camera. */
static double camera_Y    = 0.; /**< Y position of camera. */


/**
 * @brief Sets the camera zoom.
 *
 * This is the zoom used in game coordinates.
 *
 *    @param zoom Zoom to set to.
 */
void cam_setZoom( double zoom )
{
   camera_Z = zoom;
}


/**
 * @brief Gets the camera zoom.
 *
 *    @return The camera's zoom.
 */
double cam_getZoom (void)
{
   return camera_Z;
}


/**
 * @brief Makes the camera static and set on a position.
 *
 *    @param x X position to set camera to.
 *    @param y Y position to set camera to.
 */
void cam_setStatic( double x, double y )
{
   camera_X = x;
   camera_Y = y;
   camera_pos  = NULL;
}


/**
 * @brief Gets the camera position.
 *
 *    @param[out] x X position to get.
 *    @param[out] y Y position to get.
 */
void cam_getPos( double *x, double *y )
{
   *x = camera_X;
   *y = camera_Y;
}


/**
 * @brief Binds the camera to a solid.
 */
void cam_update( Solid *sld )
{
   camera_X = sld->pos.x;
   camera_Y = sld->pos.y;
}

