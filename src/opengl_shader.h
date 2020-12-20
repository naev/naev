/*
 * See Licensing and Copyright notice in naev.h
 */

#ifndef OPENGL_SHADER_H
# define OPENGL_SHADER_H


#include "opengl.h"

GLuint gl_shader_read(GLuint type, const char *filename);
int gl_program_link(GLuint program);
int gl_program_vert_frag(const char *vert, const char *frag);
void gl_uniformColor(GLint location, const glColour *c);
void gl_uniformAColor(GLint location, const glColour *c, float a);


#endif /* OPENGL_SHADER_H */
