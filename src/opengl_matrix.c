/*
 * See Licensing and Copyright notice in naev.h
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
 * @brief Pushes a new matrix on the stack.
 */
void gl_matrixPush (void)
{
   if (has_glsl) {
   }
   else {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
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
