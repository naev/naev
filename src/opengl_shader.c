/*
 * See Licensing and Copyright notice in naev.h
 */

/** @cond */
#include "naev.h"
/** @endcond */

#include "log.h"
#include "ndata.h"
#include "nstring.h"
#include "opengl.h"


#define GLSL_VERSION    "#version 140\n\n" /**< Version to use for all shaders. */


/*
 * Prototypes.
 */
static char* gl_shader_loadfile( const char *filename, size_t *size, int main, const char *prepend );
static GLuint gl_shader_compile( GLuint type, const char *buf,
      GLint length, const char *filename);
static int gl_program_link( GLuint program );
static int gl_program_make( GLuint vertex_shader, GLuint fragment_shader );


/**
 * @brief Loads a GLSL file with some simple preprocessing like adding #version and handling #include.
 *
 *    @param[in] filename Filename of the shader to load.
 *    @param[out] size Size of the loaded shader.
 *    @param[in] main Whether or not this is the main shader.
 *    @param[in] prepend String that should be prepended.
 *    @return The loaded shader buffer.
 */
static char* gl_shader_loadfile( const char *filename, size_t *size, int main, const char *prepend )
{
   size_t i, bufsize, ibufsize, fbufsize;
   char *buf, *fbuf, *ibuf, *newbuf;
   char path[PATH_MAX], include[PATH_MAX-sizeof(GLSL_PATH)-1];
   const char *substart, *subs, *subss, *keyword;
   int offset, len;

   *size = 0;

   /* Load base file. */
   nsnprintf(path, sizeof(path), GLSL_PATH "%s", filename);
   fbuf = ndata_read(path, &fbufsize);
   if (fbuf == NULL) {
      WARN( _("Shader '%s' not found."), path);
      return NULL;
   }

   /* Prepend useful information if available. */
   if (main && (prepend != NULL)) {
      bufsize = fbufsize+strlen(prepend)+1;
      buf = malloc( bufsize );
      snprintf( buf, bufsize, "%s%s", prepend, fbuf );
      buf[bufsize-1] = '\0';
      free( fbuf );
   }
   else {
      bufsize = fbufsize;
      buf = fbuf;
   }

   /* Preprocess for #include.
    * GLSL Compilers support most preprocessor things like #define and #ifdef,
    * however, #include is not supported. For this purpose, we do a very simple
    * preprocessing to handle #includes. */
   /* TODO Actually handle this by processing line-by-line so that #include
    * can't be anywhere in the code. Extra whitespace should also be handled. */
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
      ibuf = gl_shader_loadfile( include, &ibufsize, 0, NULL );

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
static GLuint gl_shader_compile( GLuint type, const char *buf,
      GLint length, const char *filename)
{
   char *log;
   GLuint shader;
   GLint compile_status, log_length;

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
   gl_checkErr();
   return shader;
}


/**
 * @brief Link a GLSL program and check for link error.
 *
 *    @param[in] program Program to link.
 *    @return 0 on success.
 */
static int gl_program_link( GLuint program )
{
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


/**
 * @brief Loads a vertex and fragment shader from files.
 *
 *    @param[in] vertfile Vertex shader filename.
 *    @param[in] fragfile Fragment shader filename.
 *    @return The shader compiled program.
 */
int gl_program_vert_frag( const char *vertfile, const char *fragfile )
{
   char *vert_str, *frag_str;
   size_t vert_size, frag_size;
   GLuint vertex_shader, fragment_shader;

   vert_str = gl_shader_loadfile( vertfile, &vert_size, 1, GLSL_VERSION );
   frag_str = gl_shader_loadfile( fragfile, &frag_size, 1, GLSL_VERSION );

   vertex_shader = gl_shader_compile( GL_VERTEX_SHADER, vert_str, vert_size, vertfile );
   fragment_shader = gl_shader_compile( GL_FRAGMENT_SHADER, frag_str, frag_size, fragfile );

   free( vert_str );
   free( frag_str );

   return gl_program_make( vertex_shader, fragment_shader );
}


/**
 * @brief Loads a vertex and fragment shader from strings.
 *
 *    @param[in] vert Vertex shader string.
 *    @param[in] vert_size Size of the vertex shader string.
 *    @param[in] frag Fragment shader string.
 *    @param[in] frag_size Size of the fragment shader string.
 *    @return The shader compiled program.
 */
int gl_program_vert_frag_string( const char *vert, size_t vert_size, const char *frag, size_t frag_size )
{
   GLuint vertex_shader, fragment_shader;
   vertex_shader = gl_shader_compile( GL_VERTEX_SHADER, vert, vert_size, NULL );
   fragment_shader = gl_shader_compile( GL_FRAGMENT_SHADER, frag, frag_size, NULL );
   return gl_program_make( vertex_shader, fragment_shader );
}


/**
 * @brief Makes a shader program from a vertex and fragment shader.
 *
 *    @param vertex_shader Vertex shader to make program from.
 *    @param fragment_shader Fragment shader to make program from.
 *    @return New shader program.
 */
static int gl_program_make( GLuint vertex_shader, GLuint fragment_shader )
{
   GLuint program = 0;
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
