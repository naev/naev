/*
 * See Licensing and Copyright notice in naev.h
 */

#include "naev.h"
#include "opengl.h"
#include "ndata.h"
#include "log.h"
#include "nstring.h"


#define GLSL_VERSION    "#version 140\n"

/**
 * @brief Open and compile GLSL shader from ndata.
 */
GLuint gl_shader_read(GLuint type, const char *filename) {
   size_t bufsize, fbufsize;
   char *buf, *fbuf;
   char path[PATH_MAX];
   GLuint shader;
   GLint length, compile_status, log_length;
   char *log;

   /* Load the file. */
   nsnprintf(path, sizeof(path), GLSL_PATH "%s", filename);
   fbuf = ndata_read(path, &fbufsize);
   if (fbuf == NULL) {
      WARN( _("Shader '%s' not found."), path);
      return 0;
   }

   /* Prepend the GLSL version. */
   bufsize = fbufsize+sizeof(GLSL_VERSION);
   buf = malloc( bufsize );
   snprintf( buf, bufsize, "%s%s", GLSL_VERSION, fbuf );
   free( fbuf);

   length = bufsize;
   
   shader = glCreateShader(type);
   glShaderSource(shader, 1, (const char**)&buf, &length);
   glCompileShader(shader);

   /* Check for compile error */
   glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
   if (compile_status == GL_FALSE) {
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
      log = malloc(log_length + 1);
      glGetShaderInfoLog(shader, log_length, &log_length, log);
      WARN("%s %s\n", filename, log);
      free(log);
      shader = 0;
   }
   
   free(buf);

   return shader;
}

/**
 * @brief Link a GLSL program and check for link error.
 */
int gl_program_link(GLuint program) {
   GLint link_status, log_length;
   char *log;

   glLinkProgram(program);

   /* Check for linking error */
   glGetProgramiv(program, GL_LINK_STATUS, &link_status);
   if (link_status == GL_FALSE) {
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
      log = malloc(log_length + 1);
      glGetProgramInfoLog(program, log_length, &log_length, log);
      WARN("%s\n", log);
      free(log);
      return -1;
   }

   return 0;
}

int gl_program_vert_frag(const char *vert, const char *frag) {
   GLuint vertex_shader, fragment_shader, program;

   vertex_shader = gl_shader_read(GL_VERTEX_SHADER, vert);
   fragment_shader = gl_shader_read(GL_FRAGMENT_SHADER, frag);

   program = 0;
   if (vertex_shader != 0 && fragment_shader != 0) {
      program = glCreateProgram();
      glAttachShader(program, vertex_shader);
      glAttachShader(program, fragment_shader);
      if (gl_program_link(program) == -1) {
         program = 0;
      }
   }

   glDeleteShader(vertex_shader);
   glDeleteShader(fragment_shader);

   gl_checkErr();

   return program;
}

void gl_uniformColor(GLint location, const glColour *c) {
   glUniform4f(location, c->r, c->g, c->b, c->a);
}

void gl_uniformAColor(GLint location, const glColour *c, float a) {
   glUniform4f(location, c->r, c->g, c->b, a);
}
