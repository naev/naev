/*
 * See Licensing and Copyright notice in naev.h
 */
/** @cond */
#include <ctype.h>

#include "naev.h"
/** @endcond */

#include "conf.h"
#include "log.h"
#include "ndata.h"
#include "nstring.h"
#include "opengl.h"

#define GLSL_VERSION    "#version 420\n\n" /**< Version to use for all shaders. */
#define GLSL_SUBROUTINE "#define HAS_GL_ARB_shader_subroutine 1\n" /**< Has subroutines. */

/*
 * Prototypes.
 */
static char* gl_shader_preprocess( size_t *size, const char *fbuf, size_t fbufsize, const char *prepend, const char *filename );
static char* gl_shader_loadfile( const char *filename, size_t *size, const char *prepend );
static GLuint gl_shader_compile( GLuint type, const char *buf,
      GLint length, const char *filename);
static int gl_program_link( GLuint program );
static GLuint gl_program_make( GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader );
static int gl_log_says_anything( const char* log );

/**
 * @brief Loads a GLSL file with some simple preprocessing like adding #version and handling #include.
 *
 *    @param[in] filename Filename of the shader to load.
 *    @param[out] size Size of the loaded shader.
 *    @param[in] prepend String that should be prepended.
 *    @return The loaded shader buffer.
 */
static char* gl_shader_loadfile( const char *filename, size_t *size, const char *prepend )
{
   size_t fbufsize;
   char *buf, *fbuf;
   char path[PATH_MAX];

   /* Load base file. */
   *size = 0;
   snprintf(path, sizeof(path), GLSL_PATH "%s", filename);
   fbuf = ndata_read(path, &fbufsize);
   if (fbuf == NULL) {
      WARN( _("Shader '%s' not found."), path);
      return NULL;
   }
   buf = gl_shader_preprocess( size, fbuf, fbufsize, prepend, filename );
   free( fbuf );
   return buf;
}

/**
 * @brief Preprocesses a GLSL string with some simple preprocessing like adding #version and handling #include.
 *
 *    @param[out] size Size of the loaded shader.
 *    @param[in] fbuf Buffer to preprocess.
 *    @param[in] fbufsize Size of the buffer.
 *    @param[in] prepend String that should be prepended.
 *    @param[in] filename Name of the file being loaded.
 *    @return The loaded shader buffer.
 */
static char* gl_shader_preprocess( size_t *size, const char *fbuf, size_t fbufsize, const char *prepend, const char *filename )
{
   size_t i, bufsize, ibufsize;
   char *buf, *ibuf, *newbuf;
   char include[PATH_MAX-sizeof(GLSL_PATH)-1];
   const char *substart, *subs, *subss, *keyword;
   int offset, len;

   /* Prepend useful information if available. */
   if (prepend != NULL) {
      bufsize = SDL_asprintf( &buf, "%s%s", prepend, fbuf ) + 1 /* the null byte */;
   }
   else {
      bufsize = fbufsize;
      buf = strdup(fbuf);
   }

   /* Preprocess for #include.
    * GLSL Compilers support most preprocessor things like #define and #ifdef,
    * however, #include is not supported. For this purpose, we do a very simple
    * preprocessing to handle #includes. */
   /* TODO Actually handle this by processing line-by-line so that #include
    * can't be anywhere in the code. Extra whitespace should also be handled. */
   keyword = "#include";
   subs = buf;
   while ((substart = strnstr( subs, keyword, bufsize-(subs-buf) ))!=NULL) {
      subs = substart+strlen(keyword)+1;
      if ((substart!=buf) && (substart[-1]!='\n') && (substart[-1]!='\r'))
         continue;
      i = 0;
      /* Find the argument - we only support " atm. */
      subss = strnstr( subs, "\"", bufsize-(subs-buf));
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
      ibuf = gl_shader_loadfile( include, &ibufsize, NULL );

      /* Move data over. */
      newbuf = malloc( bufsize+ibufsize );
      offset = 0;
      len    = substart-buf;
      strncpy( &newbuf[offset], buf, len );
      offset += len;
      len    = ibufsize;
      strncpy( &newbuf[offset], ibuf, len );
      offset += len;
      subs   = subs+1;
      len    = bufsize-(subs-buf);
      strncpy( &newbuf[offset], subs, bufsize-(subs-buf) );
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
   GLuint shader;
   GLint compile_status, log_length;

   /* Compile it. */
   shader = glCreateShader(type);
   glShaderSource(shader, 1, (const char**)&buf, &length);
   glCompileShader(shader);

   /* Check for compile error */
   glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
   glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
   if (log_length > 0) {
      char *log = malloc(log_length + 1);
      glGetShaderInfoLog(shader, log_length, &log_length, log);
      if (gl_log_says_anything( log )) {
         print_with_line_numbers( buf );
         WARN("compile_status==%d: %s: [[\n%s\n]]", compile_status, filename, log);
      }
      free(log);
      if (compile_status == GL_FALSE)
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

   glLinkProgram(program);

   /* Check for linking error */
   glGetProgramiv(program, GL_LINK_STATUS, &link_status);
   glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
   if (log_length > 0) {
      char *log = malloc(log_length + 1);
      glGetProgramInfoLog(program, log_length, &log_length, log);
      if (gl_log_says_anything( log ))
         WARN("link_status==%d: [[\n%s\n]]", link_status, log);
      free(log);
      if (link_status == GL_FALSE)
         return -1;
   }

   return 0;
}

GLuint gl_program_vert_frag_geom( const char *vert, const char *frag, const char *geom )
{
   return gl_program_backend( vert, frag, geom, NULL );
}

GLuint gl_program_vert_frag( const char *vert, const char *frag )
{
   return gl_program_backend( vert, frag, NULL, NULL );
}

/**
 * @brief Loads a vertex and fragment shader from files.
 *
 *    @param[in] vertfile Vertex shader filename.
 *    @param[in] fragfile Fragment shader filename.
 *    @param[in,opt] geomfile Optional geometry shader name.
 *    @return The shader compiled program or 0 on failure.
 */
GLuint gl_program_backend( const char *vertfile, const char *fragfile, const char *geomfile, const char *prependtext )
{
   char *vert_str, *frag_str, prepend[STRMAX];
   size_t vert_size, frag_size;
   GLuint vertex_shader, fragment_shader, geometry_shader, program;

   strncpy( prepend, GLSL_VERSION, sizeof(prepend)-1 );
   if (gl_has( OPENGL_SUBROUTINES ))
      strncat( prepend, GLSL_SUBROUTINE, sizeof(prepend)-strlen(prepend)-1 );
   if (prependtext != NULL)
      strncat( prepend, prependtext, sizeof(prepend)-strlen(prepend)-1 );

   vert_str = gl_shader_loadfile( vertfile, &vert_size, prepend );
   frag_str = gl_shader_loadfile( fragfile, &frag_size, prepend );

   vertex_shader     = gl_shader_compile( GL_VERTEX_SHADER, vert_str, vert_size, vertfile );
   fragment_shader   = gl_shader_compile( GL_FRAGMENT_SHADER, frag_str, frag_size, fragfile );

   free( vert_str );
   free( frag_str );

   if (geomfile != NULL) {
      size_t geom_size;
      char *geom_str = gl_shader_loadfile( geomfile, &geom_size, prepend );
      geometry_shader = gl_shader_compile( GL_GEOMETRY_SHADER, geom_str, geom_size, geomfile );
      free( geom_str );
   }
   else
      geometry_shader = 0;

   program = gl_program_make( vertex_shader, fragment_shader, geometry_shader );
   if (program==0)
      WARN(_("Failed to link vertex shader '%s' and fragment shader '%s'!"), vertfile, fragfile);

   return program;
}

/**
 * @brief Loads a vertex and fragment shader from strings.
 *
 *    @param[in] vert Vertex shader string.
 *    @param[in] vert_size Size of the vertex shader string.
 *    @param[in] frag Fragment shader string.
 *    @param[in] frag_size Size of the fragment shader string.
 *    @return The shader compiled program or 0 on failure.
 */
GLuint gl_program_vert_frag_string( const char *vert, size_t vert_size, const char *frag, size_t frag_size )
{
   GLuint vertex_shader, fragment_shader;
   char *vbuf, *fbuf;
   size_t vlen, flen;

   vbuf = gl_shader_preprocess( &vlen, vert, vert_size, NULL, NULL );
   fbuf = gl_shader_preprocess( &flen, frag, frag_size, NULL, NULL );

   /* Compile the shaders. */
   vertex_shader     = gl_shader_compile( GL_VERTEX_SHADER, vbuf, vlen, NULL );
   fragment_shader   = gl_shader_compile( GL_FRAGMENT_SHADER, fbuf, flen, NULL );

   /* Clean up. */
   free( vbuf );
   free( fbuf );

   /* Link. */
   return gl_program_make( vertex_shader, fragment_shader, 0 );
}

/**
 * @brief Makes a shader program from a vertex and fragment shader.
 *
 *    @param vertex_shader Vertex shader to make program from.
 *    @param fragment_shader Fragment shader to make program from.
 *    @param[opt] geometry_shader Optional geometry shader to amke program from.
 *    @return New shader program or 0 on failure.
 */
static GLuint gl_program_make( GLuint vertex_shader, GLuint fragment_shader, GLuint geometry_shader )
{
   GLuint program = 0;
   if (vertex_shader != 0 && fragment_shader != 0) {
      program = glCreateProgram();
      glAttachShader(program, vertex_shader);
      glAttachShader(program, fragment_shader);
      if (geometry_shader != 0)
         glAttachShader(program, geometry_shader);

      if (gl_program_link(program) == -1) {
         /* Spec specifies 0 as failure value for glCreateProgram() */
         program = 0;
      }

      glDeleteShader(vertex_shader);
      glDeleteShader(fragment_shader);
      if (geometry_shader != 0)
         glDeleteShader(geometry_shader);
   }

   gl_checkErr();

   return program;
}

void gl_uniformColour(GLint location, const glColour *c)
{
   glUniform4f(location, c->r, c->g, c->b, c->a);
}

void gl_uniformAColour(GLint location, const glColour *c, GLfloat a)
{
   glUniform4f(location, c->r, c->g, c->b, a);
}

void gl_uniformMat4( GLint location, const mat4 *m )
{
   glUniformMatrix4fv(location, 1, GL_FALSE, m->ptr );
}

/**
 * @brief Return true iff the input string has content besides whitespace and non-diagnostic messages we'e seen drivers emit.
 *        We log warnings in great detail, so filtering false positives is critical.
 */
static int gl_log_says_anything( const char* log )
{
   const char *junk[] = {
      "No errors.",     /* Renderer: Intel(R) HD Graphics 3000; Version: 3.1.0 - Build 9.17.10.4229 */
   };
   while (*log) {
      int progress = 0;
      if (isspace(*log)) {
         log += 1;
         progress = 1;
      }
      for (size_t i = 0; i*sizeof(junk[0]) < sizeof(junk); i++)
         if (!strncmp(log, junk[i], strlen(junk[i]))) {
            log += strlen(junk[i]);
            progress = 1;
         }
      if (!progress)
         return 1;
   }
   return 0;
}
