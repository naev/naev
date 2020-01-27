/*
 * See Licensing and Copyright notice in naev.h
 */

#ifndef OPENGL_SHADER_H
# define OPENGL_SHADER_H

GLuint gl_shader_read(GLuint type, const char *filename);
int gl_program_link(GLuint program);

#endif /* OPENGL_SHADER_H */
