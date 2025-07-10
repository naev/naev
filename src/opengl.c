/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file opengl.c
 *
 * @brief This file handles most of the more generic opengl functions.
 *
 * The main way to work with opengl in naev is to create glTextures and then
 *  use the blit functions to draw them on the screen.  This system will
 *  handle relative and absolute positions.
 *
 * There are two coordinate systems: relative and absolute.
 *
 * Relative:
 *  * Everything is drawn relative to the player, if it doesn't fit on screen
 *    it is clipped.
 *  * Origin (0., 0.) would be ontop of the player.
 *
 * Absolute:
 *  * Everything is drawn in "screen coordinates".
 *  * (0., 0.) is top left.
 *  * (SCREEN_W, SCREEN_H) is bottom right.
 */
/** @cond */
#include "SDL_PhysFS.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "naev.h"
/** @endcond */

#include "opengl.h"

#include "conf.h"
#include "debug.h" // IWYU pragma: keep
#include "gltf.h"
#include "log.h"
#include "render.h"

glInfo gl_screen = {
   .window = NULL, /* Should be initialized to NULL as is used for cases of
                      SDL_ShowSimpleMessageBox. */
}; /**< Gives data of current opengl settings. */

static unsigned int cb_correct_pp =
   0; /**< Colourblind post-process shader id for correction. */
static unsigned int cb_simulate_pp =
   0; /**< Colourblind post-process shader id for simulation. */

/*
 * Viewport offsets
 */
mat4 gl_view_matrix = { { { { 0 } } } };

/*
 * prototypes
 */
/* gl */
static int gl_getGLInfo( void );

/*
 *
 * M I S C
 *
 */
/**
 * @brief Takes a screenshot.
 *
 *    @param filename PhysicsFS path (e.g., "screenshots/screenshot042.png") of
 * the file to save screenshot as.
 */
void gl_screenshot( const char *filename )
{
   GLubyte      *screenbuf;
   SDL_IOStream *rw;
   SDL_Surface  *surface;
   int           w, h;

   /* Allocate data. */
   w         = gl_screen.rw;
   h         = gl_screen.rh;
   screenbuf = malloc( sizeof( GLubyte ) * 3 * w * h );
   surface   = SDL_CreateSurface(
      w, h, SDL_GetPixelFormatForMasks( 24, RMASK, GMASK, BMASK, AMASK ) );

   /* Read pixels from buffer -- SLOW. */
   glPixelStorei( GL_PACK_ALIGNMENT, 1 ); /* Force them to pack the bytes. */
   glReadPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, screenbuf );

   /* Convert data. */
   for ( int i = 0; i < h; i++ )
      memcpy( (GLubyte *)surface->pixels + i * surface->pitch,
              &screenbuf[( h - i - 1 ) * ( 3 * w )], 3 * w );
   free( screenbuf );

   /* Save PNG. */
   if ( !( rw = SDL_PhysFS_OpenIO( PHYSFS_openWrite( filename ) ) ) )
      WARN( _( "Aborting screenshot" ) );
   else
      IMG_SavePNG_IO( surface, rw, 1 );

   /* Check to see if an error occurred. */
   gl_checkErr();

   /* Free memory. */
   SDL_DestroySurface( surface );
}

void gl_saveFboDepth( GLuint fbo, const char *filename )
{
   GLfloat     *screenbuf;
   SDL_Surface *s;
   int          w, h;

   /* Allocate data. */
   w         = gl_screen.rw; /* TODO get true size. */
   h         = gl_screen.rh;
   screenbuf = malloc( sizeof( GLfloat ) * 1 * w * h );
   s         = SDL_CreateSurface(
      w, h, SDL_GetPixelFormatForMasks( 24, RMASK, GMASK, BMASK, AMASK ) );

   /* Read pixels from buffer -- SLOW. */
   glBindFramebuffer( GL_FRAMEBUFFER, fbo );
   GLint value;
   glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                          &value );
   if ( value != GL_TEXTURE ) {
      WARN( "Trying to save depth of FBO with no depth attachment!" );
      free( screenbuf );
      glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.current_fbo );
      return;
   }
   glPixelStorei( GL_PACK_ALIGNMENT, 1 ); /* Force them to pack the bytes. */
   glReadPixels( 0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, screenbuf );
   glBindFramebuffer( GL_FRAMEBUFFER, gl_screen.current_fbo );

   /* Check to see if an error occurred. */
   gl_checkErr();

   /* Convert data. */
   Uint8 *d = s->pixels;
   for ( int i = 0; i < h; i++ ) {
      for ( int j = 0; j < w; j++ ) {
         float f = screenbuf[( h - i - 1 ) * w + j];
         Uint8 v = f * 255.;
         for ( int k = 0; k < 3; k++ ) {
            d[i * s->pitch + j * SDL_BYTESPERPIXEL( s->format ) + k] = v;
         }
      }
   }
   free( screenbuf );

   /* Save PNG. */
   IMG_SavePNG( s, filename );

   /* Free memory. */
   SDL_DestroySurface( s );
}

/*
 *
 * G L O B A L
 *
 */

#ifdef DEBUGGING
/**
 * @brief Checks and reports if there's been an error.
 */
int gl_checkHandleError( const char *func, int line )
{
   (void)func;
   (void)line;
   const char *errstr;
   GLenum      err = glGetError();

   /* No error. */
   if ( err == GL_NO_ERROR )
      return 0;

   switch ( err ) {
   case GL_INVALID_ENUM:
      errstr = _( "GL invalid enum" );
      break;
   case GL_INVALID_VALUE:
      errstr = _( "GL invalid value" );
      break;
   case GL_INVALID_OPERATION:
      errstr = _( "GL invalid operation" );
      break;
   case GL_INVALID_FRAMEBUFFER_OPERATION:
      errstr = _( "GL invalid framebuffer operation" );
      break;
   case GL_OUT_OF_MEMORY:
      errstr = _( "GL out of memory" );
      break;

   default:
      errstr = _( "GL unknown error" );
      break;
   }
   WARN( _( "OpenGL error [%s:%d]: %s" ), func, line, errstr );
   return 1;
}
#endif /* DEBUGGING */

/**
 * @brief Tries to apply the configured display mode to the window.
 *
 *    @note Caller is responsible for calling gl_resize/naev_resize afterward.
 *    @return 0 on success.
 */
int gl_setupFullscreen( void )
{
   int ok;
   int display_index = SDL_GetDisplayForWindow( gl_screen.window );
   if ( conf.fullscreen && conf.modesetting ) {
      SDL_DisplayMode target, closest;
      /* Try to use desktop resolution if nothing is specifically set. */
      if ( conf.explicit_dim ) {
         target   = *SDL_GetWindowFullscreenMode( gl_screen.window );
         target.w = conf.width;
         target.h = conf.height;
      } else
         target = *SDL_GetDesktopDisplayMode( display_index );

      if ( !SDL_GetClosestFullscreenDisplayMode( display_index, target.w,
                                                 target.h, 0.0, 1, &closest ) )
         return -1;

      SDL_SetWindowFullscreenMode( gl_screen.window, &closest );
   }
   ok = SDL_SetWindowFullscreen( gl_screen.window, conf.fullscreen );
   /* HACK: Force pending resize events to be processed, particularly on
    * Wayland. */
   SDL_PumpEvents();
   SDL_GL_SwapWindow( gl_screen.window );
   SDL_GL_SwapWindow( gl_screen.window );
   return ok;
}

/**
 * @brief Gets some information about the OpenGL window.
 *
 *    @return 0 on success.
 */
static int gl_getGLInfo( void )
{
   int doublebuf;
   SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &doublebuf );
   if ( doublebuf )
      gl_screen.flags |= OPENGL_DOUBLEBUF;
   if ( GLAD_GL_ARB_shader_subroutine && glGetSubroutineIndex &&
        glGetSubroutineUniformLocation && glUniformSubroutinesuiv )
      gl_screen.flags |= OPENGL_SUBROUTINES;

   /* Debug happiness */
   DEBUG( _( "OpenGL Drawable Created: %dx%d@%dbpp" ), gl_screen.rw,
          gl_screen.rh, gl_screen.depth );
   DEBUG( _( "r: %d, g: %d, b: %d, a: %d, db: %s, fsaa: %d, tex: %d" ),
          gl_screen.r, gl_screen.g, gl_screen.b, gl_screen.a,
          gl_has( OPENGL_DOUBLEBUF ) ? _( "yes" ) : _( "no" ), gl_screen.fsaa,
          gl_screen.tex_max );
   DEBUG( _( "VSync: %s" ), gl_has( OPENGL_VSYNC ) ? _( "yes" ) : _( "no" ) );
   DEBUG( _( "Renderer: %s" ), glGetString( GL_RENDERER ) );
   DEBUG( _( "Vendor: %s" ), glGetString( GL_VENDOR ) );
   DEBUG( _( "Version: %s" ), glGetString( GL_VERSION ) );

   /* Now check for things that can be bad. */
   if ( ( conf.fsaa > 1 ) && ( gl_screen.fsaa != conf.fsaa ) )
      WARN( _( "Unable to get requested FSAA level (%d requested, got %d)" ),
            conf.fsaa, gl_screen.fsaa );

   return 0;
}

/**
 * @brief Initializes SDL/OpenGL and the works.
 *    @return 0 on success.
 */
int gl_init( void )
{
   /* Set Vsync. */
   int interval;
   if ( SDL_GL_GetSwapInterval( &interval ) )
      gl_screen.flags |= OPENGL_VSYNC;

   /* Finish getting attributes. */
   gl_screen.current_fbo = 0; /* No FBO set. */
   for ( int i = 0; i < OPENGL_NUM_FBOS; i++ ) {
      gl_screen.fbo[i]           = GL_INVALID_VALUE;
      gl_screen.fbo_tex[i]       = GL_INVALID_VALUE;
      gl_screen.fbo_depth_tex[i] = GL_INVALID_VALUE;
   }

   /* Load extensions. */
   if ( !gladLoadGLLoader( (void *)SDL_GL_GetProcAddress ) ) {
      char buf[STRMAX];
      snprintf( buf, sizeof( buf ), _( "Unable to load OpenGL using GLAD!" ) );
      SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR,
                                _( "Naev Critical Error" ), buf,
                                gl_screen.window );
      ERR( "%s", buf );
   }

   /* We are interested in 3.3 because it drops all the deprecated stuff and
    * gives us:
    * 1. instancing
    */
   if ( !GLAD_GL_VERSION_3_3 )
      WARN( "Naev requires OpenGL %d.%d, but got OpenGL %d.%d!", 3, 3,
            GLVersion.major, GLVersion.minor );

   /* Handles resetting the viewport and scaling, rw/rh are set in createWindow.
    */
   gl_resize();

   /* Initialize subsystems.*/
   gl_initTextures();
   gl_initVBO();
   gl_initRender();

   /* Get info about the OpenGL window */
   gl_getGLInfo();

   shaders_load();

   /* Set colourblind shader if necessary. */
   gl_colourblind();

   /* Load the gltf framework. */
   gltf_init();

   /* Cosmetic new line. */
   DEBUG_BLANK();

   return 0;
}

/**
 * @brief Handles a window resize and resets gl_screen parameters.
 */
void gl_resize_c( void )
{
   /* Set up framebuffer. */
   for ( int i = 0; i < OPENGL_NUM_FBOS; i++ ) {
      if ( gl_screen.fbo[i] != GL_INVALID_VALUE ) {
         glDeleteFramebuffers( 1, &gl_screen.fbo[i] );
         glDeleteTextures( 1, &gl_screen.fbo_tex[i] );
         glDeleteTextures( 1, &gl_screen.fbo_depth_tex[i] );
      }

      gl_fboCreate( &gl_screen.fbo[i], &gl_screen.fbo_tex[i], gl_screen.rw,
                    gl_screen.rh );
      gl_fboAddDepth( gl_screen.fbo[i], &gl_screen.fbo_depth_tex[i],
                      gl_screen.rw, gl_screen.rh );

      /* Names for debugging. */
      if ( gl_supportsDebug() ) {
         char buf[STRMAX_SHORT];
         snprintf( buf, sizeof( buf ), "Screen Framebuffer %d", i );
         glObjectLabel( GL_FRAMEBUFFER, gl_screen.fbo[i], strlen( buf ), buf );
         snprintf( buf, sizeof( buf ), "Screen Texture %d", i );
         glObjectLabel( GL_TEXTURE, gl_screen.fbo_tex[i], strlen( buf ), buf );
         snprintf( buf, sizeof( buf ), "Screen Depth %d", i );
         glObjectLabel( GL_TEXTURE, gl_screen.fbo_depth_tex[i], strlen( buf ),
                        buf );
      }
   }

   gl_checkErr();
}

/**
 * @brief Translates the window position to screen position.
 */
void gl_windowToScreenPos( float *sx, float *sy, float wx, float wy )
{
   wx /= gl_screen.dwscale;
   wy /= gl_screen.dhscale;

   *sx = gl_screen.mxscale * (double)wx;
   *sy = gl_screen.myscale * (double)( gl_screen.rh - wy );
}

/**
 * @brief Translates the screen position to windos position.
 */
void gl_screenToWindowPos( float *wx, float *wy, float sx, float sy )
{
   *wx = sx / gl_screen.mxscale;
   *wy = (double)gl_screen.rh - sy / gl_screen.myscale;

   *wx *= gl_screen.dwscale;
   *wy *= gl_screen.dhscale;
}

/**
 * @brief Gets the associated min/mag filter from a string.
 *
 *    @param s String to get filter from.
 *    @return Filter.
 */
GLenum gl_stringToFilter( const char *s )
{
   if ( SDL_strcasecmp( s, "linear" ) == 0 )
      return GL_LINEAR;
   else if ( SDL_strcasecmp( s, "nearest" ) == 0 )
      return GL_NEAREST;
   WARN( _( "Unknown %s '%s'!" ), "OpenGL Filter", s );
   return 0;
}

/**
 * @brief Gets the associated min/mag filter from a string.
 *
 *    @param s String to get filter from.
 *    @return Filter.
 */
GLenum gl_stringToClamp( const char *s )
{
   if ( SDL_strcasecmp( s, "clamp" ) == 0 )
      return GL_CLAMP_TO_EDGE;
   else if ( SDL_strcasecmp( s, "repeat" ) == 0 )
      return GL_REPEAT;
   else if ( SDL_strcasecmp( s, "mirroredrepeat" ) == 0 )
      return GL_MIRRORED_REPEAT;
   WARN( _( "Unknown %s '%s'!" ), "OpenGL Clamp", s );
   return 0;
}

/**
 * @brief Gets a blend function from a string.
 *
 *    @param s String to get blend function from.
 *    @return The blend function corresponding to the string.
 */
GLenum gl_stringToBlendFunc( const char *s )
{
   if ( SDL_strcasecmp( s, "add" ) == 0 )
      return GL_FUNC_ADD;
   else if ( SDL_strcasecmp( s, "subrtract" ) == 0 )
      return GL_FUNC_SUBTRACT;
   else if ( SDL_strcasecmp( s, "reverse_subtract" ) == 0 )
      return GL_FUNC_REVERSE_SUBTRACT;
   else if ( SDL_strcasecmp( s, "min" ) == 0 )
      return GL_MIN;
   else if ( SDL_strcasecmp( s, "max" ) == 0 )
      return GL_MAX;
   WARN( _( "Unknown %s '%s'!" ), "OpenGL BlendFunc", s );
   return 0;
}

/**
 * @brief Gets a blend factor from a string.
 *
 *    @param s String to get blend factor from.
 *    @return The blend factor corresponding to the string.
 */
GLenum gl_stringToBlendFactor( const char *s )
{
   if ( SDL_strcasecmp( s, "zero" ) == 0 )
      return GL_ZERO;
   else if ( SDL_strcasecmp( s, "one" ) == 0 )
      return GL_ONE;
   else if ( SDL_strcasecmp( s, "src_color" ) == 0 )
      return GL_SRC_COLOR;
   else if ( SDL_strcasecmp( s, "one_minus_src_color" ) == 0 )
      return GL_ONE_MINUS_SRC_COLOR;
   else if ( SDL_strcasecmp( s, "src_alpha" ) == 0 )
      return GL_SRC_ALPHA;
   else if ( SDL_strcasecmp( s, "one_minus_src_alpha" ) == 0 )
      return GL_ONE_MINUS_SRC_ALPHA;
   else if ( SDL_strcasecmp( s, "dst_color" ) == 0 )
      return GL_DST_COLOR;
   else if ( SDL_strcasecmp( s, "one_minus_dst_color" ) == 0 )
      return GL_ONE_MINUS_DST_COLOR;
   else if ( SDL_strcasecmp( s, "dst_alpha" ) == 0 )
      return GL_DST_ALPHA;
   else if ( SDL_strcasecmp( s, "one_minus_dst_alpha" ) == 0 )
      return GL_ONE_MINUS_DST_ALPHA;
   else if ( SDL_strcasecmp( s, "src_alpha_saturate" ) == 0 )
      return GL_SRC_ALPHA_SATURATE;
   WARN( _( "Unknown %s '%s'!" ), "OpenGL BlendFactor", s );
   return 0;
}

/**
 * @brief Enables or disables the colourblind shader.
 */
void gl_colourblind( void )
{
   /* Load up shader uniforms. */
   glUseProgram( shaders.colourblind_sim.program );
   glUniform1i( shaders.colourblind_sim.type, conf.colourblind_type );
   glUniform1f( shaders.colourblind_sim.intensity, conf.colourblind_sim );
   glUseProgram( shaders.colourblind_correct.program );
   glUniform1i( shaders.colourblind_correct.type, conf.colourblind_type );
   glUniform1f( shaders.colourblind_correct.intensity,
                conf.colourblind_correct );
   glUseProgram( 0 );

   /* See if we have to correct. */
   if ( conf.colourblind_sim > 0. ) {
      LuaShader_t shader;
      if ( cb_simulate_pp != 0 )
         return;
      memset( &shader, 0, sizeof( LuaShader_t ) );
      shader.program            = shaders.colourblind_sim.program;
      shader.VertexPosition     = shaders.colourblind_sim.VertexPosition;
      shader.ClipSpaceFromLocal = shaders.colourblind_sim.ClipSpaceFromLocal;
      shader.MainTex            = shaders.colourblind_sim.MainTex;
      cb_simulate_pp = render_postprocessAdd( &shader, PP_LAYER_CORE, 99,
                                              PP_SHADER_PERMANENT );
   } else {
      if ( cb_simulate_pp != 0 )
         render_postprocessRm( cb_simulate_pp );
      cb_simulate_pp = 0;
   }

   /* See if we have to correct. */
   if ( conf.colourblind_correct > 0. ) {
      LuaShader_t shader;
      if ( cb_correct_pp != 0 )
         return;
      memset( &shader, 0, sizeof( LuaShader_t ) );
      shader.program        = shaders.colourblind_correct.program;
      shader.VertexPosition = shaders.colourblind_correct.VertexPosition;
      shader.ClipSpaceFromLocal =
         shaders.colourblind_correct.ClipSpaceFromLocal;
      shader.MainTex = shaders.colourblind_correct.MainTex;
      cb_correct_pp  = render_postprocessAdd( &shader, PP_LAYER_CORE, 100,
                                              PP_SHADER_PERMANENT );
   } else {
      if ( cb_correct_pp != 0 )
         render_postprocessRm( cb_correct_pp );
      cb_correct_pp = 0;
   }
}

/**
 * @brief Cleans up OpenGL, the works.
 */
void gl_exit( void )
{
   for ( int i = 0; i < OPENGL_NUM_FBOS; i++ ) {
      if ( gl_screen.fbo[i] != GL_INVALID_VALUE ) {
         glDeleteFramebuffers( 1, &gl_screen.fbo[i] );
         glDeleteTextures( 1, &gl_screen.fbo_tex[i] );
         glDeleteTextures( 1, &gl_screen.fbo_depth_tex[i] );
         gl_screen.fbo[i]           = GL_INVALID_VALUE;
         gl_screen.fbo_tex[i]       = GL_INVALID_VALUE;
         gl_screen.fbo_depth_tex[i] = GL_INVALID_VALUE;
      }
   }

   /* Exit the OpenGL subsystems. */
   gltf_exit();
   gl_exitRender();
   gl_exitVBO();
   gl_exitTextures();

   shaders_unload();

   /* Shut down the subsystem */
   SDL_QuitSubSystem( SDL_INIT_VIDEO );
}
