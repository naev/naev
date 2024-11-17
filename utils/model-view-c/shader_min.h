/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include "glad.h"

GLuint gl_program_backend( const char *vertfile, const char *fragfile, const char *geom, const char *prependtext );
GLuint gl_program_vert_frag_string( const char *vert, size_t vert_size, const char *frag, size_t frag_size );

int gl_fboCreate( GLuint *fbo, GLuint *tex, GLsizei width, GLsizei height );
