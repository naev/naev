/*
 * See Licensing and Copyright notice in naev.h
 */
#include "glad.h"

#include "SDL.h"
#include "physfs.h"

#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "nstr.h"

//#define GLSL_PATH "../../dat/glsl/"
//#define GLSL_PATH "./"
#define GLSL_PATH ""

#define GLSL_VERSION    "#version 140\n\n" /**< Version to use for all shaders. */
#define GLSL_SUBROUTINE "#define HAS_GL_ARB_shader_subroutine 1\n" /**< Has subroutines. */

/*
 * Prototypes.
 */
static void print_with_line_numbers( const char *str );
static char* gl_shader_preprocess( size_t *size, const char *fbuf, size_t fbufsize, const char *prepend, const char *filename );
static char* gl_shader_loadfile( const char *filename, size_t *size, const char *prepend );
static GLuint gl_shader_compile( GLuint type, const char *buf,
      GLint length, const char *filename);
static int gl_program_link( GLuint program );
static GLuint gl_program_make( GLuint vertex_shader, GLuint fragment_shader );

static void* ndata_read( const char* path, size_t *filesize )
{
   char *buf;
   PHYSFS_file *file;
   PHYSFS_sint64 len, n;
   PHYSFS_Stat path_stat;

   if (!PHYSFS_stat( path, &path_stat )) {
      WARN( _( "Error occurred while opening '%s': %s" ), path,
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      *filesize = 0;
      return NULL;
   }
   if (path_stat.filetype != PHYSFS_FILETYPE_REGULAR) {
      WARN( _( "Error occurred while opening '%s': It is not a regular file" ), path );
      *filesize = 0;
      return NULL;
   }

   /* Open file. */
   file = PHYSFS_openRead( path );
   if ( file == NULL ) {
      WARN( _( "Error occurred while opening '%s': %s" ), path,
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      *filesize = 0;
      return NULL;
   }

   /* Get file size. TODO: Don't assume this is always possible? */
   len = PHYSFS_fileLength( file );
   if ( len == -1 ) {
      WARN( _( "Error occurred while seeking '%s': %s" ), path,
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
      PHYSFS_close( file );
      *filesize = 0;
      return NULL;
   }

   /* Allocate buffer. */
   buf = malloc( len+1 );
   if (buf == NULL) {
      WARN(_("Out of Memory"));
      PHYSFS_close( file );
      *filesize = 0;
      return NULL;
   }
   buf[len] = '\0';

   /* Read the file. */
   n = 0;
   while (n < len) {
      size_t pos = PHYSFS_readBytes( file, &buf[ n ], len - n );
      if (pos == 0) {
         WARN( _( "Error occurred while reading '%s': %s" ), path,
            _(PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) ) );
         PHYSFS_close( file );
         *filesize = 0;
         free(buf);
         return NULL;
      }
      n += pos;
   }

   /* Close the file. */
   PHYSFS_close(file);

   *filesize = len;
   return buf;
}

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
      bufsize = asprintf( &buf, "%s%s", prepend, fbuf ) + 1 /* the null byte */;
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

static void print_with_line_numbers( const char *str )
{
   int counter = 0;
   for (int i=0; str[i] != '\0'; i++) {
      if ((i==0) || (str[i]=='\n'))
         fprintf( stdout, "\n%03d: ", ++counter );
      if (str[i]!='\n')
         fprintf( stdout, "%c", str[i] );
   }
   fprintf( stdout, "\n" );
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
   //if (GL_COMPILE_STATUS == GL_FALSE) {
   if (log_length > 0) {
      char *log = malloc(log_length + 1);
      glGetShaderInfoLog(shader, log_length, &log_length, log);
      print_with_line_numbers( buf );
      WARN("%s\n%s\n", filename, log);
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

   glLinkProgram(program);

   /* Check for linking error */
   glGetProgramiv(program, GL_LINK_STATUS, &link_status);
   glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
   //if (link_status == GL_FALSE) {
   if (log_length > 0) {
      char *log;
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
 *    @return The shader compiled program or 0 on failure.
 */
GLuint gl_program_backend( const char *vertfile, const char *fragfile, const char *geom, const char *prependtext )
{
   (void) geom;
   char *vert_str, *frag_str, prepend[STRMAX];
   size_t vert_size, frag_size;
   GLuint vertex_shader, fragment_shader, program;

   strncpy( prepend, GLSL_VERSION, sizeof(prepend)-1 );
   if (prependtext != NULL)
      strncat( prepend, prependtext, sizeof(prepend)-strlen(prepend)-1 );
   //if (gl_has( OPENGL_SUBROUTINES ))
   //   strncat( prepend, GLSL_SUBROUTINE, sizeof(prepend)-strlen(prepend)-1 );

   vert_str = gl_shader_loadfile( vertfile, &vert_size, prepend );
   frag_str = gl_shader_loadfile( fragfile, &frag_size, prepend );

   vertex_shader     = gl_shader_compile( GL_VERTEX_SHADER, vert_str, vert_size, vertfile );
   fragment_shader   = gl_shader_compile( GL_FRAGMENT_SHADER, frag_str, frag_size, fragfile );

   free( vert_str );
   free( frag_str );

   program = gl_program_make( vertex_shader, fragment_shader );
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
   return gl_program_make( vertex_shader, fragment_shader );
}

/**
 * @brief Makes a shader program from a vertex and fragment shader.
 *
 *    @param vertex_shader Vertex shader to make program from.
 *    @param fragment_shader Fragment shader to make program from.
 *    @return New shader program or 0 on failure.
 */
static GLuint gl_program_make( GLuint vertex_shader, GLuint fragment_shader )
{
   GLuint program = 0;
   if (vertex_shader != 0 && fragment_shader != 0) {
      program = glCreateProgram();
      glAttachShader(program, vertex_shader);
      glAttachShader(program, fragment_shader);
      if (gl_program_link(program) == -1) {
         /* Spec specifies 0 as failure value for glCreateProgram() */
         program = 0;
      }
   }

   glDeleteShader(vertex_shader);
   glDeleteShader(fragment_shader);

   gl_checkErr();

   return program;
}

void gl_checkHandleError( const char *func, int line )
{
   const char* errstr;
   GLenum err = glGetError();

   /* No error. */
   if (err == GL_NO_ERROR)
      return;

   switch (err) {
      case GL_INVALID_ENUM:
         errstr = "GL invalid enum";
         break;
      case GL_INVALID_VALUE:
         errstr = "GL invalid value";
         break;
      case GL_INVALID_OPERATION:
         errstr = "GL invalid operation";
         break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
         errstr = "GL invalid framebuffer operation";
         break;
      case GL_OUT_OF_MEMORY:
         errstr = "GL out of memory";
         break;

      default:
         errstr = "GL unknown error";
         break;
   }
   WARN("OpenGL error [%s:%d]: %s", func, line, errstr);
}
