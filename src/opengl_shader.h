/*
 * See Licensing and Copyright notice in naev.h
 */

#ifndef OPENGL_SHADER_H
# define OPENGL_SHADER_H

GLuint gl_shader_read(GLuint type, const char *filename);
int gl_program_link(GLuint program);
int gl_program_vert_frag(const char *vert, const char *frag);

#endif /* OPENGL_SHADER_H */
