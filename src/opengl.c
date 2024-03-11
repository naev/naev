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
 *  * (0., 0.) is bottom left.
 *  * (SCREEN_W, SCREEN_H) is top right.
 *
 * Note that the game actually uses a third type of coordinates for when using
 *  raw commands.  In this third type, the (0.,0.) is actually in middle of the
 *  screen.  (-SCREEN_W/2.,-SCREEN_H/2.) is bottom left and
 *  (+SCREEN_W/2.,+SCREEN_H/2.) is top right.
 */
/** @cond */
#include "physfsrwops.h"
#include "SDL.h"
#include "SDL_error.h"
#include "SDL_image.h"

#include "naev.h"
/** @endcond */

#include "opengl.h"

#include "conf.h"
#include "log.h"
#include "render.h"
#include "gltf.h"

glInfo gl_screen; /**< Gives data of current opengl settings. */
static int gl_activated = 0; /**< Whether or not a window is activated. */

static unsigned int cb_correct_pp = 0; /**< Colourblind post-process shader id for correction. */
static unsigned int cb_simulate_pp = 0; /**< Colourblind post-process shader id for simulation. */

/*
 * Viewport offsets
 */
static int gl_view_x = 0; /* X viewport offset. */
static int gl_view_y = 0; /* Y viewport offset. */
static int gl_view_w = 0; /* Viewport width. */
static int gl_view_h = 0; /* Viewport height. */
mat4 gl_view_matrix = {{{{0}}}};

/*
 * prototypes
 */
/* gl */
static int gl_setupAttributes( int fallback );
static int gl_createWindow( unsigned int flags );
static int gl_getFullscreenMode (void);
static int gl_getGLInfo (void);
static int gl_defState (void);
static int gl_setupScaling (void);

/*
 *
 * M I S C
 *
 */
/**
 * @brief Takes a screenshot.
 *
 *    @param filename PhysicsFS path (e.g., "screenshots/screenshot042.png") of the file to save screenshot as.
 */
void gl_screenshot( const char *filename )
{
   GLubyte *screenbuf;
   SDL_RWops *rw;
   SDL_Surface *surface;
   int w, h;

   /* Allocate data. */
   w           = gl_screen.rw;
   h           = gl_screen.rh;
   screenbuf   = malloc( sizeof(GLubyte) * 3 * w*h );
   surface     = SDL_CreateRGBSurface( 0, w, h, 24, RGBAMASK );

   /* Read pixels from buffer -- SLOW. */
   glPixelStorei(GL_PACK_ALIGNMENT, 1); /* Force them to pack the bytes. */
   glReadPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, screenbuf );

   /* Convert data. */
   for (int i=0; i<h; i++)
      memcpy( (GLubyte*)surface->pixels + i * surface->pitch, &screenbuf[ (h - i - 1) * (3*w) ], 3*w );
   free( screenbuf );

   /* Save PNG. */
   if (!(rw = PHYSFSRWOPS_openWrite( filename )))
      WARN( _("Aborting screenshot") );
   else
      IMG_SavePNG_RW( surface, rw, 1 );

   /* Check to see if an error occurred. */
   gl_checkErr();

   /* Free memory. */
   SDL_FreeSurface( surface );
}

/*
 *
 * G L O B A L
 *
 */
/**
 * @brief Checks to see if opengl version is at least major.minor.
 *
 *    @param major Major version to check.
 *    @param minor Minor version to check.
 *    @return True if major and minor version are met.
 */
GLboolean gl_hasVersion( int major, int minor )
{
   if (GLVersion.major >= major && GLVersion.minor >= minor)
      return GL_TRUE;
   return GL_FALSE;
}

#ifdef DEBUGGING
/**
 * @brief Checks and reports if there's been an error.
 */
void gl_checkHandleError( const char *func, int line )
{
   (void) func;
   (void) line;
#if !DEBUG_GL
   const char* errstr;
   GLenum err = glGetError();

   /* No error. */
   if (err == GL_NO_ERROR)
      return;

   switch (err) {
      case GL_INVALID_ENUM:
         errstr = _("GL invalid enum");
         break;
      case GL_INVALID_VALUE:
         errstr = _("GL invalid value");
         break;
      case GL_INVALID_OPERATION:
         errstr = _("GL invalid operation");
         break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
         errstr = _("GL invalid framebuffer operation");
         break;
      case GL_OUT_OF_MEMORY:
         errstr = _("GL out of memory");
         break;

      default:
         errstr = _("GL unknown error");
         break;
   }
   WARN(_("OpenGL error [%s:%d]: %s"), func, line, errstr);
#endif /* !DEBUG_GL */
}

#if DEBUG_GL
/**
 * @brief Checks and reports if there's been an error.
 */
static void GLAPIENTRY gl_debugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* p )
{
   static int errors_seen = 0;
   (void) source;
   (void) id;
   (void) length;
   (void) p;
   const char *typestr;

   if (++errors_seen == 10)
      WARN( _("Too many OpenGL diagnostics reported! Suppressing further reports.") );
   if (errors_seen >= 10)
      return;

   switch (type) {
      case GL_DEBUG_TYPE_ERROR:
         typestr = " GL_DEBUG_TYPE_ERROR";
         break;
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
         typestr = " GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
         break;
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
         typestr = " GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
         break;
      case GL_DEBUG_TYPE_PORTABILITY:
         typestr = " GL_DEBUG_TYPE_PORTABILITY";
         break;
      case GL_DEBUG_TYPE_PERFORMANCE:
         typestr = " GL_DEBUG_TYPE_PERFORMANCE";
         break;
      case GL_DEBUG_TYPE_OTHER:
         typestr = " GL_DEBUG_TYPE_OTHER";
         break;
      case GL_DEBUG_TYPE_MARKER: /* fallthrough */
      case GL_DEBUG_TYPE_PUSH_GROUP: /* fallthrough */
      case GL_DEBUG_TYPE_POP_GROUP: /* fallthrough */
         return;
      default:
         typestr = "";
   }
   WARN( _("[type = 0x%x%s], severity = 0x%x, message = %s backtrace:"), type, typestr, severity, message );
   debug_logBacktrace();
}
#endif /* DEBUG_GL */
#endif /* DEBUGGING */

/**
 * @brief Tries to set up the OpenGL attributes for the OpenGL context.
 *
 *    @return 0 on success.
 */
static int gl_setupAttributes( int fallback )
{
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, fallback ? 3 : 4);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, fallback ? 2 : 6);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); /* Ideally want double buffering. */
   if (conf.fsaa > 1) {
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, conf.fsaa);
   }
   SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
#if DEBUG_GL
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif /* DEBUG_GL */

   return 0;
}

/**
 * @brief Tries to apply the configured display mode to the window.
 *
 *    @note Caller is responsible for calling gl_resize/naev_resize afterward.
 *    @return 0 on success.
 */
int gl_setupFullscreen (void)
{
   int ok;
   int display_index = SDL_GetWindowDisplayIndex( gl_screen.window );
   if (conf.fullscreen && conf.modesetting) {
      SDL_DisplayMode target, closest;
      /* Try to use desktop resolution if nothing is specifically set. */
      if (conf.explicit_dim) {
         SDL_GetWindowDisplayMode( gl_screen.window, &target );
         target.w = conf.width;
         target.h = conf.height;
      }
      else
         SDL_GetDesktopDisplayMode( display_index, &target );

      if (SDL_GetClosestDisplayMode( display_index, &target, &closest ) == NULL)
         SDL_GetDisplayMode( display_index, 0, &closest ); /* fall back to the best one */

      SDL_SetWindowDisplayMode( gl_screen.window, &closest );
   }
   ok = SDL_SetWindowFullscreen( gl_screen.window, gl_getFullscreenMode() );
   /* HACK: Force pending resize events to be processed, particularly on Wayland. */
   SDL_PumpEvents();
   SDL_GL_SwapWindow(gl_screen.window);
   SDL_GL_SwapWindow(gl_screen.window);
   return ok;
}

/**
 * @brief Returns the fullscreen configuration as SDL2 flags.
 *
 * @return Appropriate combination of SDL_WINDOW_FULLSCREEN* flags.
 */
static int gl_getFullscreenMode (void)
{
   if (conf.fullscreen)
      return conf.modesetting ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP;
   return 0;
}

/**
 * @brief Creates the OpenGL window.
 *
 *    @return 0 on success.
 */
static int gl_createWindow( unsigned int flags )
{
   flags |= SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
   if (!conf.notresizable)
      flags |= SDL_WINDOW_RESIZABLE;
   if (conf.borderless)
      flags |= SDL_WINDOW_BORDERLESS;

   /* Create the window. */
   gl_screen.window = SDL_CreateWindow( APPNAME,
         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
         conf.width, conf.height, flags );
   if (gl_screen.window == NULL)
      ERR(_("Unable to create window! %s"), SDL_GetError());

   /* Set focus loss behaviour. */
   SDL_SetHint( SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS,
         conf.minimize ? "1" : "0" );

   /* Create the OpenGL context, note we don't need an actual renderer. */
   for (int fallback=0; fallback <= 1; fallback++) {
      gl_setupAttributes( fallback );
      gl_screen.context = SDL_GL_CreateContext( gl_screen.window );
      if (gl_screen.context != NULL)
         break;
   }
   if (!gl_screen.context)
      ERR(_("Unable to create OpenGL context! %s"), SDL_GetError());

   /* Save and store version. */
   SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &gl_screen.major );
   SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &gl_screen.minor );
   if (gl_screen.major*100+gl_screen.minor*10 > 320)
      gl_screen.glsl = 100*gl_screen.major+10*gl_screen.minor;
   else
      gl_screen.glsl = 150;

   /* Set Vsync. */
   if (conf.vsync) {
      int ret = SDL_GL_SetSwapInterval( 1 );
      if (ret == 0)
         gl_screen.flags |= OPENGL_VSYNC;
   } else {
      SDL_GL_SetSwapInterval( 0 );
   }

   /* Finish getting attributes. */
   gl_screen.current_fbo = 0; /* No FBO set. */
   for (int i=0; i<OPENGL_NUM_FBOS; i++) {
      gl_screen.fbo[i]     = GL_INVALID_VALUE;
      gl_screen.fbo_tex[i] = GL_INVALID_VALUE;
   }
   SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &gl_screen.depth );
   gl_activated = 1; /* Opengl is now activated. */

   return 0;
}

/**
 * @brief Gets some information about the OpenGL window.
 *
 *    @return 0 on success.
 */
static int gl_getGLInfo (void)
{
   int doublebuf;
   SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &gl_screen.r );
   SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &gl_screen.g );
   SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &gl_screen.b );
   SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &gl_screen.a );
   SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &doublebuf );
   SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &gl_screen.fsaa );
   if (doublebuf)
      gl_screen.flags |= OPENGL_DOUBLEBUF;
   if (GLAD_GL_ARB_shader_subroutine && glGetSubroutineIndex && glGetSubroutineUniformLocation && glUniformSubroutinesuiv)
      gl_screen.flags |= OPENGL_SUBROUTINES;
   /* Calculate real depth. */
   gl_screen.depth = gl_screen.r + gl_screen.g + gl_screen.b + gl_screen.a;

   /* Texture information */
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_screen.tex_max);
   glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gl_screen.multitex_max);

   /* Debug happiness */
   DEBUG(_("OpenGL Drawable Created: %dx%d@%dbpp"), gl_screen.rw, gl_screen.rh, gl_screen.depth);
   DEBUG(_("r: %d, g: %d, b: %d, a: %d, db: %s, fsaa: %d, tex: %d"),
         gl_screen.r, gl_screen.g, gl_screen.b, gl_screen.a,
         gl_has(OPENGL_DOUBLEBUF) ? _("yes") : _("no"),
         gl_screen.fsaa, gl_screen.tex_max);
   DEBUG(_("vsync: %s"), gl_has(OPENGL_VSYNC) ? _("yes") : _("no"));
   DEBUG(_("Renderer: %s"), glGetString(GL_RENDERER));
   DEBUG(_("Version: %s"), glGetString(GL_VERSION));

   /* Now check for things that can be bad. */
   if ((conf.fsaa > 1) && (gl_screen.fsaa != conf.fsaa))
      WARN(_("Unable to get requested FSAA level (%d requested, got %d)"),
            conf.fsaa, gl_screen.fsaa );

   return 0;
}

/**
 * @brief Sets the opengl state to it's default parameters.
 *
 *    @return 0 on success.
 */
static int gl_defState (void)
{
   glDisable( GL_DEPTH_TEST ); /* set for doing 2d */
   glEnable(  GL_BLEND ); /* alpha blending ftw */
   glEnable(  GL_LINE_SMOOTH ); /* We use SDF shaders for most shapes, but star trails & map routes are thin & anti-aliased. */
#if DEBUG_GL
   glEnable(  GL_DEBUG_OUTPUT ); /* Log errors immediately.. */
   glDebugMessageCallback( gl_debugCallback, 0 );
   glDebugMessageControl( GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, 0, NULL, GL_FALSE );
   glDebugMessageControl( GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 0, NULL, GL_FALSE );
#endif /* DEBUG_GL */

   /* Set the blending/shading model to use. */
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); /* good blend model */

   return 0;
}

/**
 * @brief Sets up dimensions in gl_screen, including scaling as needed.
 *
 *    @return 0 on success.
 */
static int gl_setupScaling (void)
{
   /* Get the basic dimensions from SDL2. */
   SDL_GetWindowSize(gl_screen.window, &gl_screen.w, &gl_screen.h);
   SDL_GL_GetDrawableSize(gl_screen.window, &gl_screen.rw, &gl_screen.rh);
   /* Calculate scale factor, if OS has native HiDPI scaling. */
   gl_screen.dwscale = (double)gl_screen.w / (double)gl_screen.rw;
   gl_screen.dhscale = (double)gl_screen.h / (double)gl_screen.rh;

   /* Combine scale factor from OS with the one in Naev's config */
   gl_screen.scale = fmax(gl_screen.dwscale, gl_screen.dhscale) / conf.scalefactor;
   glLineWidth(1. / gl_screen.scale);
   glPointSize(1. / gl_screen.scale + 2.0);

   /* New window is real window scaled. */
   gl_screen.nw = (double)gl_screen.rw * gl_screen.scale;
   gl_screen.nh = (double)gl_screen.rh * gl_screen.scale;
   /* Small windows get handled here. */
   if ((gl_screen.nw < RESOLUTION_W_MIN)
         || (gl_screen.nh < RESOLUTION_H_MIN)) {
      double scalew, scaleh;
      if (gl_screen.scale != 1.)
         DEBUG(_("Screen size too small, upscaling..."));
      scalew = RESOLUTION_W_MIN / (double)gl_screen.nw;
      scaleh = RESOLUTION_H_MIN / (double)gl_screen.nh;
      gl_screen.scale *= MAX( scalew, scaleh );
      /* Rescale. */
      gl_screen.nw = (double)gl_screen.rw * gl_screen.scale;
      gl_screen.nh = (double)gl_screen.rh * gl_screen.scale;
   }
   /* Viewport matches new window size. */
   gl_screen.w  = gl_screen.nw;
   gl_screen.h  = gl_screen.nh;
   /* Set scale factors. */
   gl_screen.wscale  = (double)gl_screen.nw / (double)gl_screen.w;
   gl_screen.hscale  = (double)gl_screen.nh / (double)gl_screen.h;
   gl_screen.mxscale = (double)gl_screen.w / (double)gl_screen.rw;
   gl_screen.myscale = (double)gl_screen.h / (double)gl_screen.rh;

   return 0;
}

/**
 * @brief Initializes SDL/OpenGL and the works.
 *    @return 0 on success.
 */
int gl_init (void)
{
   unsigned int flags;
   GLuint VaoId;

   /* Defaults. */
   memset( &gl_screen, 0, sizeof(gl_screen) );
   SDL_SetHint( "SDL_WINDOWS_DPI_SCALING", "1" );
   flags = SDL_WINDOW_OPENGL | gl_getFullscreenMode();

   /* Initializes Video */
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
      WARN(_("Unable to initialize SDL Video: %s"), SDL_GetError());
      return -1;
   }

   /* Create the window. */
   gl_createWindow( flags );

   /* Apply the configured fullscreen display mode, if any. */
   gl_setupFullscreen();

   /* Load extensions. */
   if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
      ERR("Unable to load OpenGL using GLAD");

   /* We are interested in 3.1 because it drops all the deprecated stuff. */
   if ( !GLAD_GL_VERSION_3_2 )
      WARN( "Naev requires OpenGL %d.%d, but got OpenGL %d.%d!", 3, 2, GLVersion.major, GLVersion.minor );

   /* Some OpenGL options. */
   glClearColor( 0., 0., 0., 1. );

   /* Set default opengl state. */
   gl_defState();

   /* Handles resetting the viewport and scaling, rw/rh are set in createWindow. */
   gl_resize();

   /* Finishing touches. */
   glClear( GL_COLOR_BUFFER_BIT ); /* must clear the buffer first */
   gl_checkErr();

   /* Initialize subsystems.*/
   gl_initTextures();
   gl_initVBO();
   gl_initRender();

   /* Get info about the OpenGL window */
   gl_getGLInfo();

   /* Modern OpenGL requires at least one VAO */
   glGenVertexArrays(1, &VaoId);
   glBindVertexArray(VaoId);

   shaders_load();

   /* Set colourblind shader if necessary. */
   gl_colourblind();

   /* Set colourspace. */
   glEnable( GL_FRAMEBUFFER_SRGB );

   /* Load the gltf framework. */
   gltf_init();

   /* Cosmetic new line. */
   DEBUG_BLANK();

   return 0;
}

/**
 * @brief Handles a window resize and resets gl_screen parameters.
 */
void gl_resize (void)
{
   gl_setupScaling();
   glViewport( 0, 0, gl_screen.rw, gl_screen.rh );
   gl_setDefViewport( 0, 0, gl_screen.nw, gl_screen.nh );
   gl_defViewport();

   /* Set up framebuffer. */
   for (int i=0; i<OPENGL_NUM_FBOS; i++) {
      if (gl_screen.fbo[i] != GL_INVALID_VALUE) {
         glDeleteFramebuffers( 1, &gl_screen.fbo[i] );
         glDeleteTextures( 1, &gl_screen.fbo_tex[i] );
         glDeleteTextures( 1, &gl_screen.fbo_depth_tex[i] );
      }
      gl_fboCreate( &gl_screen.fbo[i], &gl_screen.fbo_tex[i], gl_screen.rw, gl_screen.rh );
      gl_fboAddDepth( gl_screen.fbo[i], &gl_screen.fbo_depth_tex[i], gl_screen.rw, gl_screen.rh );
   }

   gl_checkErr();
}

/**
 * @brief Sets the opengl viewport.
 */
void gl_viewport( int x, int y, int w, int h )
{
   mat4 proj = mat4_ortho( 0., /* Left edge. */
            gl_screen.nw, /* Right edge. */
            0., /* Bottom edge. */
            gl_screen.nh, /* Top edge. */
            -1., /* near */
            1. ); /* far */

   /* Take into account possible translation. */
   gl_screen.x = x;
   gl_screen.y = y;
   mat4_translate_xy( &proj, x, y );

   /* Set screen size. */
   gl_screen.w = w;
   gl_screen.h = h;

   /* Take into account possible scaling. */
   if (gl_screen.scale != 1.)
      mat4_scale( &proj, gl_screen.wscale, gl_screen.hscale, 1. );

   gl_view_matrix = proj;
}

/**
 * @brief Sets the default viewport.
 */
void gl_setDefViewport( int x, int y, int w, int h )
{
   gl_view_x  = x;
   gl_view_y  = y;
   gl_view_w  = w;
   gl_view_h  = h;
}

/**
 * @brief Resets viewport to default
 */
void gl_defViewport (void)
{
   gl_viewport( gl_view_x, gl_view_y, gl_view_w, gl_view_h );
}

/**
 * @brief Translates the window position to screen position.
 */
void gl_windowToScreenPos( int *sx, int *sy, int wx, int wy )
{
   wx /= gl_screen.dwscale;
   wy /= gl_screen.dhscale;

   *sx = gl_screen.mxscale * (double)wx - (double)gl_screen.x;
   *sy = gl_screen.myscale * (double)(gl_screen.rh - wy) - (double)gl_screen.y;
}

/**
 * @brief Translates the screen position to windos position.
 */
void gl_screenToWindowPos( int *wx, int *wy, int sx, int sy )
{
   *wx = (sx + (double)gl_screen.x) / gl_screen.mxscale;
   *wy = (double)gl_screen.rh - (sy + (double)gl_screen.y) / gl_screen.myscale;

   *wx *= gl_screen.dwscale;
   *wy *= gl_screen.dhscale;
}

/**
 * @brief Gets the associated min/mag filter from a string.
 *
 *    @param s String to get filter from.
 *    @return Filter.
 */
GLint gl_stringToFilter( const char *s )
{
   if (strcmp(s,"linear")==0)
      return GL_LINEAR;
   else if (strcmp(s,"nearest")==0)
      return GL_NEAREST;
   return 0;
}

/**
 * @brief Gets the associated min/mag filter from a string.
 *
 *    @param s String to get filter from.
 *    @return Filter.
 */
GLint gl_stringToClamp( const char *s )
{
   if (strcmp(s,"clamp")==0)
      return GL_CLAMP_TO_EDGE;
   else if (strcmp(s,"repeat")==0)
      return GL_REPEAT;
   else if (strcmp(s,"mirroredrepeat")==0)
      return GL_MIRRORED_REPEAT;
   return 0;
}

/**
 * @brief Enables or disables the colourblind shader.
 */
void gl_colourblind (void)
{
   /* Load up shader uniforms. */
   glUseProgram( shaders.colourblind_sim.program );
   glUniform1i( shaders.colourblind_sim.type, conf.colourblind_type );
   glUniform1f( shaders.colourblind_sim.intensity, conf.colourblind_sim );
   glUseProgram( shaders.colourblind_correct.program );
   glUniform1i( shaders.colourblind_correct.type, conf.colourblind_type );
   glUniform1f( shaders.colourblind_correct.intensity, conf.colourblind_correct );
   glUseProgram( 0 );

   /* See if we have to correct. */
   if (conf.colourblind_sim > 0.) {
      LuaShader_t shader;
      if (cb_simulate_pp != 0)
         return;
      memset( &shader, 0, sizeof(LuaShader_t) );
      shader.program    = shaders.colourblind_sim.program;
      shader.VertexPosition = shaders.colourblind_sim.VertexPosition;
      shader.ClipSpaceFromLocal = shaders.colourblind_sim.ClipSpaceFromLocal;
      shader.MainTex    = shaders.colourblind_sim.MainTex;
      cb_simulate_pp = render_postprocessAdd( &shader, PP_LAYER_CORE, 99, PP_SHADER_PERMANENT );
   } else {
      if (cb_simulate_pp != 0)
         render_postprocessRm( cb_simulate_pp );
      cb_simulate_pp = 0;
   }

   /* See if we have to correct. */
   if (conf.colourblind_correct > 0.) {
      LuaShader_t shader;
      if (cb_correct_pp != 0)
         return;
      memset( &shader, 0, sizeof(LuaShader_t) );
      shader.program    = shaders.colourblind_correct.program;
      shader.VertexPosition = shaders.colourblind_correct.VertexPosition;
      shader.ClipSpaceFromLocal = shaders.colourblind_correct.ClipSpaceFromLocal;
      shader.MainTex    = shaders.colourblind_correct.MainTex;
      cb_correct_pp = render_postprocessAdd( &shader, PP_LAYER_CORE, 100, PP_SHADER_PERMANENT );
   } else {
      if (cb_correct_pp != 0)
         render_postprocessRm( cb_correct_pp );
      cb_correct_pp = 0;
   }
}

/**
 * @brief Cleans up OpenGL, the works.
 */
void gl_exit (void)
{
   for (int i=0; i<OPENGL_NUM_FBOS; i++) {
      if (gl_screen.fbo[i] != GL_INVALID_VALUE) {
         glDeleteFramebuffers( 1, &gl_screen.fbo[i] );
         glDeleteTextures( 1, &gl_screen.fbo_tex[i] );
         glDeleteTextures( 1, &gl_screen.fbo_depth_tex[i] );
         gl_screen.fbo[i] = GL_INVALID_VALUE;
         gl_screen.fbo_tex[i] = GL_INVALID_VALUE;
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
   SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
