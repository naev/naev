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

/**
 * @file opengl_matrix.c
 *
 * @brief Handles OpenGL matrix stuff.
 */


#include "opengl.h"

#include "naev.h"

#include "log.h"


static int has_glsl = 0; /**< Whether or not using GLSL for matrix stuff. */



/**
 * @brief Initializes the OpenGL matrix subsystem.
 *
 *    @return 0 on success.
 */
int gl_initMatrix (void)
{
   return 0;
}


/**
 * @brief Exits the OpenGL matrix subsystem.
 */
void gl_exitMatrix (void)
{
   has_glsl = 0;
}


/**
 * @brief like glMatrixMode.
 */
void gl_matrixMode( GLenum mode )
{
   if (has_glsl) {
   }
   else {
      glMatrixMode( mode );
   }
}


/**
 * @brief Pushes a new matrix on the stack.
 */
void gl_matrixPush (void)
{
   if (has_glsl) {
   }
   else {
      glPushMatrix();
   }
}


/**
 * @brief Loads the identity matrix.
 */
void gl_matrixIdentity (void)
{
   if (has_glsl) {
   }
   else {
      glLoadIdentity();
   }
}


/**
 * @brief Sets the matrix as orthogonal.
 */
void gl_matrixOrtho( double left, double right,
      double bottom, double top, double nearVal, double farVal )
{
   if (has_glsl) {
   }
   else {
      glOrtho( left, right, bottom, top, nearVal, farVal );
   }
}


/**
 * @brief Translates the matrix.
 *
 *    @param x X to translate by.
 *    @param y Y to translate by.
 */
void gl_matrixTranslate( double x, double y )
{
   if (has_glsl) {
   }
   else {
      glTranslated( x, y, 0. );
   }
}


/**
 * @brief Scales the matrix.
 *
 *    @param x X to scale by.
 *    @param y Y to scale by.
 */
void gl_matrixScale( double x, double y )
{
   if (has_glsl) {
   }
   else {
      glScaled( x, y, 1. );
   }
}


/**
 * @brief Rotates the matrix.
 *
 *    @param a Angle to rotate by.
 */
void gl_matrixRotate( double a )
{
   if (has_glsl) {
   }
   else {
      glRotated( 180./M_PI*a, 0., 0., 1. );
   }
}


/**
 * @brief Destroys the last pushed matrix.
 */
void gl_matrixPop (void)
{
   if (has_glsl) {
   }
   else {
      glPopMatrix();
   }
}
