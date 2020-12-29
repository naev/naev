/*
 * See Licensing and Copyright notice in naev.h
 */

#ifndef OPENGL_SHADER_H
# define OPENGL_SHADER_H


#include "opengl.h"

int gl_program_vert_frag( const char *vert, const char *frag );
int gl_program_vert_frag_string( const char *vert, size_t vert_size, const char *frag, size_t frag_size );
void gl_uniformColor( GLint location, const glColour *c );
void gl_uniformAColor( GLint location, const glColour *c, float a );


#endif /* OPENGL_SHADER_H */
