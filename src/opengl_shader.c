/*
 * See Licensing and Copyright notice in naev.h
 */

#include "opengl.h"
#include "ndata.h"
#include "log.h"

GLuint gl_shader_read(GLuint type, const char *filename) {
   size_t bufsize;
   char *buf;
   char *path;
   GLuint shader, length;
   GLuint compile_status, log_length;
   char *log;

   path = malloc(strlen(GLSL_PATH) + strlen(filename) + 1);
   strcpy(path, GLSL_PATH);
   strcat(path, filename);

   buf = ndata_read(path, &bufsize);
   if (buf == NULL) {
      free(path);
      return 0;
   }
   length = bufsize;
   
   shader = glCreateShader(type);
   glShaderSource(shader, 1, (const char**)&buf, &length);
   glCompileShader(shader);

   // Check for compile error
   glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
   if (compile_status == GL_FALSE) {
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
      log = malloc(log_length + 1);
      glGetShaderInfoLog(shader, log_length, &log_length, log);
      ERR("%s\n", log);
      free(log);
      shader = 0;
   }
   
   free(path);
   free(buf);

   return shader;
}

int gl_program_link(GLuint program) {
   GLuint link_status, log_length;
   char *log;

   glLinkProgram(program);

   /* Check for linking error */
   glGetProgramiv(program, GL_LINK_STATUS, &link_status);
   if (link_status == GL_FALSE) {
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
      log = malloc(log_length + 1);
      glGetProgramInfoLog(program, log_length, &log_length, log);
      ERR("%s\n", log);
      free(log);
      return -1;
   }

   return 0;
}
