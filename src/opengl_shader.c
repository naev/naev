/*
 * See Licensing and Copyright notice in naev.h
 */

#include "naev.h"
#include "opengl.h"
#include "ndata.h"
#include "log.h"
#include "nstring.h"


#define GLSL_VERSION    "#version 140\n\n"


/**
 * @brief Loads a GLSL file with some simple preprocessing like adding #version and handling #include.
 *
 *    @param[in] filename Filename of the shader to load.
 *    @param[out] size Size of the loaded shader.
 *    @param[in] main Whether or not this is the main shader.
 *    @return The loaded shader buffer.
 */
static char* gl_shader_loadfile( const char *filename, size_t *size, int main )
{
   size_t bufsize, ibufsize, fbufsize;
   char *buf, *fbuf, *ibuf, *newbuf;
   char path[PATH_MAX], include[PATH_MAX-sizeof(GLSL_PATH)-1];
   const char *substart, *subs, *subss, *keyword;
   int i, offset, len;

   *size = 0;

   /* Load base file. */
   nsnprintf(path, sizeof(path), GLSL_PATH "%s", filename);
   fbuf = ndata_read(path, &fbufsize);
   if (fbuf == NULL) {
      WARN( _("Shader '%s' not found."), path);
      return NULL;
   }

   if (main) {
      /* Prepend the GLSL version.
      * TODO add #defines as necessary based on the code. */
      bufsize = fbufsize+sizeof(GLSL_VERSION);
      buf = malloc( bufsize );
      snprintf( buf, bufsize, "%s%s", GLSL_VERSION, fbuf );
      free( fbuf );
   }
   else {
      bufsize = fbufsize;
      buf = fbuf;
   }

   /* Preprocess for #include */
   keyword = "#include";
   subs = buf;
   while ((substart = nstrnstr( subs, keyword, bufsize-(subs-buf) ))!=NULL) {
      subs = substart+strlen(keyword)+1;
      i = 0;
      /* Find the argument - we only support " atm. */
      subss = nstrnstr( subs, "\"", bufsize-(subs-buf));
      if (subss == NULL) {
         WARN(_("Invalid #include syntax in '%s%s'!"), GLSL_PATH, filename);
         continue;
      }
      subs = subss+1;
      while (isprint(*subs) && (i<sizeof(include)) && (*subs!='"')) {
         include[i++] = *subs;
         subs++;
      }
      if (*subs != '"') {
         WARN(_("Invalid #include syntax in '%s%s'!"), GLSL_PATH, filename);
         continue;
      }
      include[i] = '\0'; /* Last character should be " or > */

      /* Recursive loading and handling of #includes. */
      ibuf = gl_shader_loadfile( include, &ibufsize, 0 );

      /* Move data over. */
      newbuf = malloc( bufsize+ibufsize );
      offset = 0;
      len    = substart-buf;
      strncpy( &newbuf[offset], buf, len );
      offset += len;
      len    = ibufsize;
      strncpy( &newbuf[offset], ibuf, len );
      offset += len;
      len    = bufsize-(subs-buf-1);
      strncpy( &newbuf[offset], subs+1, bufsize-(subs-buf-1) );
      offset += len;
      newbuf[offset] = '\0';

      /* Reset the pointers. */
      subs = &newbuf[subs-buf];

      /* Swap buffers. */
      free(buf);
      buf = newbuf;
      bufsize = offset;

      /* Clean up. */
      free(ibuf);
   }

   *size = bufsize;
   return buf;
}


/**
 * @brief Open and compile GLSL shader from ndata.
 */
GLuint gl_shader_read(GLuint type, const char *filename) {
   size_t bufsize;
   char *buf, *log;
   GLuint shader;
   GLint length, compile_status, log_length;

   /* Load the main shader. */
   buf = gl_shader_loadfile( filename, &bufsize, 1 );
   length = bufsize;
  
   /* Compile it. */
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
