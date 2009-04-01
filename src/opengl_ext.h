/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef OPENGL_EXT_H
#  define OPENGL_EXT_H


#include "SDL_opengl.h"


/* GL_ARB_multitexture */
void (APIENTRY *nglActiveTexture)(GLenum texture);
void (APIENTRY *nglMultiTexCoord2d)(GLenum target,GLdouble s,GLdouble t);


#endif /* OPENGL_EXT_H */
   
