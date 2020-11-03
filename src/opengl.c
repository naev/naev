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


#include "opengl.h"

#include "SDL_error.h"
#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include "nstring.h"
#include <stdarg.h> /* va_list for gl_print */

#include <zlib.h> /* Z_DEFAULT_COMPRESSION */
#include <png.h>

#include "SDL.h"
#include "SDL_version.h"

#include "log.h"
#include "ndata.h"
#include "gui.h"
#include "conf.h"


/*
 * Requirements
 */
#define OPENGL_WINDOW_MIN_HEIGHT  720 /**< Minimum window height. */
#define OPENGL_WINDOW_MIN_WIDTH   960 /**< Minimum window width. */
#define OPENGL_REQ_MULTITEX         2 /**< 2 is minimum OpenGL 1.2 must have */


glInfo gl_screen; /**< Gives data of current opengl settings. */
static int gl_activated = 0; /**< Whether or not a window is activated. */


/*
 * Viewport offsets
 */
static int gl_view_x = 0; /* X viewport offset. */
static int gl_view_y = 0; /* Y viewport offset. */
static int gl_view_w = 0; /* Viewport width. */
static int gl_view_h = 0; /* Viewport height. */
gl_Matrix4 gl_view_matrix = {0};


/* Whether Intel is the OpenGL vendor. */
static int intel_vendor = 0;


/*
 * prototypes
 */
/* gl */
static int gl_setupAttributes (void);
static int gl_setupFullscreen( unsigned int *flags );
static int gl_createWindow( unsigned int flags );
static int gl_getGLInfo (void);
static int gl_defState (void);
static int gl_setupScaling (void);
static int gl_hint (void);
/* png */
static int write_png( const char *file_name, png_bytep *rows,
      int w, int h, int colourtype, int bitdepth );


/*
 *
 * M I S C
 *
 */
/**
 * @brief Takes a screenshot.
 *
 *    @param filename Name of the file to save screenshot as.
 */
void gl_screenshot( const char *filename )
{
   GLubyte *screenbuf;
   png_bytep *rows;
   int i, w, h;

   /* Allocate data. */
   w           = gl_screen.rw;
   h           = gl_screen.rh;
   screenbuf   = malloc( sizeof(GLubyte) * 3 * w*h );
   rows        = malloc( sizeof(png_bytep) * h );

   /* Read pixels from buffer -- SLOW. */
   glPixelStorei(GL_PACK_ALIGNMENT, 1); /* Force them to pack the bytes. */
   glReadPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, screenbuf );

   /* Convert data. */
   for (i = 0; i < h; i++)
      rows[i] = &screenbuf[ (h - i - 1) * (3*w) ];

   /* Save PNG. */
   write_png( filename, rows, w, h, PNG_COLOR_TYPE_RGB, 8);

   /* Check to see if an error occurred. */
   gl_checkErr();

   /* Free memory. */
   free( screenbuf );
   free( rows );
}


/**
 * @brief Saves a surface to a file as a png.
 *
 * Ruthlessly stolen from "pygame - Python Game Library"
 *    by Pete Shinners (pete@shinners.org)
 *
 *    @param surface Surface to save.
 *    @param file Path to save surface to.
 *    @return 0 on success.;
 */
int SDL_SavePNG( SDL_Surface *surface, const char *file )
{
   png_bytep* ss_rows;
   int ss_size;
   int ss_w, ss_h;
   SDL_Surface *ss_surface;
   int r, i;
   SDL_Rect rtemp;

   /* Initialize parameters. */
   ss_surface  = NULL;

   /* Set size. */
   ss_w        = surface->w;
   ss_h        = surface->h;

   /* Handle alpha. */
   SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

   /* Create base surface. */
   ss_surface = SDL_CreateRGBSurface( 0, ss_w, ss_h, 32, RGBAMASK );
   if (ss_surface == NULL) {
      WARN(_("Unable to create RGB surface."));
      return -1;
   }
   if (SDL_FillRect( ss_surface, NULL,
            SDL_MapRGBA(surface->format,0,0,0,SDL_ALPHA_TRANSPARENT))) {
      WARN(_("Unable to fill rect: %s"), SDL_GetError());
      return 0;
   }


   /* Blit to new surface. */
   rtemp.x = rtemp.y = 0;
   rtemp.w = surface->w;
   rtemp.h = surface->h;
   SDL_BlitSurface(surface, &rtemp, ss_surface, &rtemp);

   /* Allocate space. */
   ss_size = ss_h;
   ss_rows = malloc(sizeof(png_bytep) * ss_size);
   if (ss_rows == NULL)
      return -1;

   /* Copy pixels into data. */
   for (i = 0; i < ss_h; i++)
      ss_rows[i] = ((png_bytep)ss_surface->pixels) + i*ss_surface->pitch;

   /* Save to PNG. */
   r = write_png(file, ss_rows, surface->w, surface->h, PNG_COLOR_TYPE_RGB_ALPHA, 8);

   /* Clean up. */
   free(ss_rows);
   SDL_FreeSurface(ss_surface);

   return r;
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


/**
 * @brief Returns whether the OpenGL vendor is Intel.
 *
 * This is a bit ugly, but it seems that Intel integrated graphics tend to lie
 * about their capabilities with regards to smooth points and lines.
 *
 *    @return 1 if Intel is the vendor, 0 otherwise.
 */
int gl_vendorIsIntel (void)
{
   return intel_vendor;
}


#ifdef DEBUGGING
/**
 * @brief Checks and reports if there's been an error.
 */
void gl_checkHandleError( const char *func, int line )
{
   GLenum err;
   const char* errstr;

   err = glGetError();

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
}
#endif /* DEBUGGING */


/**
 * @brief Tries to set up the OpenGL attributes for the OpenGL context.
 *
 *    @return 0 on success.
 */
static int gl_setupAttributes (void)
{
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); /* Ideally want double buffering. */
   if (conf.fsaa > 1) {
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, conf.fsaa);
   }

   return 0;
}


/**
 * @brief Tries to set up fullscreen environment.
 *
 *    @param flags Flags to modify.
 *    @return 0 on success.
 */
static int gl_setupFullscreen( unsigned int *flags )
{
   int i, j, off, toff, supported;

   /* Unsupported by default. */
   supported = 0;

   /* Try to use desktop resolution if nothing is specifically set. */
   if ((gl_screen.desktop_w > 0) && (gl_screen.desktop_h > 0) && !conf.explicit_dim) {
      gl_screen.w = gl_screen.desktop_w;
      gl_screen.h = gl_screen.desktop_h;
   }

   (void) flags;
   SDL_DisplayMode mode;
   int n = SDL_GetNumDisplayModes( 0 );

   /* Try to get closest approximation to mode asked for */
   off = -1;
   j   = -1;
   for (i=0; i<n; i++) {
      SDL_GetDisplayMode( 0, i, &mode  );

      /* Found supported mode. */
      if ((mode.w == SCREEN_W) && (mode.h == SCREEN_H)) {
         supported = 1;
         break;
      }

      /* Get Manhattan distance. */
      toff = ABS(SCREEN_W-mode.w) + ABS(SCREEN_H-mode.h);
      if ((off == -1) || (toff < off)) {
         j   = i;
         off = toff;
      }
   }

   /* Failed to find. */
   if (!supported) {
      if (j<0) {
         ERR(_("Fullscreen mode %dx%d is not supported by your setup, however no other modes are supported, bailing!"),
               SCREEN_W, SCREEN_H);
      }

      SDL_GetDisplayMode( 0, j, &mode );
      WARN(_("Fullscreen mode %dx%d is not supported by your setup\n"
            "   switching to %dx%d"),
            SCREEN_W, SCREEN_H,
            mode.w, mode.h );
      gl_screen.w = mode.w;
      gl_screen.h = mode.h;
   }

   return 0;
}


/**
 * @brief Creates the OpenGL window.
 *
 *    @return 0 on success.
 */
static int gl_createWindow( unsigned int flags )
{
   int ret;

   /* Create the window. */
   gl_screen.window = SDL_CreateWindow( APPNAME,
         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
         SCREEN_W, SCREEN_H, flags | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
   if (gl_screen.window == NULL)
      ERR(_("Unable to create window! %s"), SDL_GetError());

   /* Set focus loss behaviour. */
   SDL_SetHint( SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS,
         conf.minimize ? "1" : "0" );

   /* Create the OpenGL context, note we don't need an actual renderer. */
   gl_screen.context = SDL_GL_CreateContext( gl_screen.window );
   if (!gl_screen.context)
      ERR(_("Unable to create OpenGL context! %s"), SDL_GetError());

   /* Set Vsync. */
   if (conf.vsync) {
      ret = SDL_GL_SetSwapInterval( 1 );
      if (ret == 0)
         gl_screen.flags |= OPENGL_VSYNC;
   } else {
      SDL_GL_SetSwapInterval( 0 );
   }

   /* Finish getting attributes. */
   SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &gl_screen.depth );
   gl_screen.rw = SCREEN_W;
   gl_screen.rh = SCREEN_H;
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
   char *vendor;

   SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &gl_screen.r );
   SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &gl_screen.g );
   SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &gl_screen.b );
   SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &gl_screen.a );
   SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &doublebuf );
   SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &gl_screen.fsaa );
   if (doublebuf)
      gl_screen.flags |= OPENGL_DOUBLEBUF;
   /* Calculate real depth. */
   gl_screen.depth = gl_screen.r + gl_screen.g + gl_screen.b + gl_screen.a;

   /* Texture information */
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_screen.tex_max);
   glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gl_screen.multitex_max);

   /* Ugly, but Intel hardware seems to be uniquely problematic. */
   vendor = (char*)glGetString(GL_VENDOR);
   intel_vendor = !!(nstrcasestr(vendor, "Intel") != NULL);

   /* Debug happiness */
   DEBUG(_("OpenGL Window Created: %dx%d@%dbpp %s"), SCREEN_W, SCREEN_H, gl_screen.depth,
         gl_has(OPENGL_FULLSCREEN)?_("fullscreen"):_("window"));
   DEBUG(_("r: %d, g: %d, b: %d, a: %d, db: %s, fsaa: %d, tex: %d"),
         gl_screen.r, gl_screen.g, gl_screen.b, gl_screen.a,
         gl_has(OPENGL_DOUBLEBUF) ? _("yes") : _("no"),
         gl_screen.fsaa, gl_screen.tex_max);
   DEBUG(_("vsync: %s, mm: %s, compress: %s, npot: %s"),
         gl_has(OPENGL_VSYNC) ? _("yes") : _("no"),
         gl_texHasMipmaps() ? _("yes") : _("no"),
         gl_texHasCompress() ? _("yes") : _("no"),
         gl_needPOT() ? _("no") : _("yes") );
   DEBUG(_("Renderer: %s"), glGetString(GL_RENDERER));
   DEBUG(_("Version: %s"), glGetString(GL_VERSION));

   /* Now check for things that can be bad. */
   if (gl_screen.multitex_max < OPENGL_REQ_MULTITEX)
      WARN(_("Missing texture units (%d required, %d found)"),
            OPENGL_REQ_MULTITEX, gl_screen.multitex_max );
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
/* glEnable(  GL_TEXTURE_2D ); never enable globally, breaks non-texture blits */
   glEnable(  GL_BLEND ); /* alpha blending ftw */

   /* Set the blending/shading model to use. */
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); /* good blend model */

   return 0;
}


/**
 * @brief Checks to see if window needs to handle scaling.
 *
 *    @return 0 on success.
 */
static int gl_setupScaling (void)
{
   double scalew, scaleh;
   /* New window is real window scaled. */
   gl_screen.nw = (double)gl_screen.rw * gl_screen.scale;
   gl_screen.nh = (double)gl_screen.rh * gl_screen.scale;
   /* Small windows get handled here. */
   if ((gl_screen.nw < OPENGL_WINDOW_MIN_WIDTH) ||
         (gl_screen.nh < OPENGL_WINDOW_MIN_HEIGHT)) {
      if (gl_screen.scale != 1.)
         DEBUG(_("Screen size too small, upscaling..."));
      scalew = OPENGL_WINDOW_MIN_WIDTH / (double)gl_screen.nw;
      scaleh = OPENGL_WINDOW_MIN_HEIGHT / (double)gl_screen.nh;
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
 * Sets up the opengl hints.
 */
static int gl_hint (void)
{
   GLenum mod;

   /* Choose what quality to do it at. */
   mod = GL_NICEST;

   /* Do some hinting. */
   glHint(GL_TEXTURE_COMPRESSION_HINT, mod);

   return 0;
}


/**
 * @brief Initializes SDL/OpenGL and the works.
 *    @return 0 on success.
 */
int gl_init (void)
{
   unsigned int flags;
   int dw, dh;
   GLuint VaoId;

   /* Defaults. */
   /* desktop_w and desktop_h get set in naev.c when initializing. */
   dw = gl_screen.desktop_w;
   dh = gl_screen.desktop_h;
   memset( &gl_screen, 0, sizeof(gl_screen) );
   flags  = SDL_WINDOW_OPENGL;
   gl_screen.desktop_w = dw;
   gl_screen.desktop_h = dh;

   /* Load configuration. */

   gl_screen.w = conf.width;
   gl_screen.h = conf.height;
   if (conf.fullscreen) {
      gl_screen.flags |= OPENGL_FULLSCREEN;
      if (conf.modesetting)
         flags |= SDL_WINDOW_FULLSCREEN;
      else
         flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
   }

   /* Initializes Video */
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
      WARN(_("Unable to initialize SDL Video: %s"), SDL_GetError());
      return -1;
   }

   /* Set opengl flags. */
   gl_setupAttributes();

   /* See if should set up fullscreen. */
   if (conf.fullscreen)
      gl_setupFullscreen( &flags );

   /* Create the window. */
   gl_createWindow( flags );

   /* Load extensions. */
   if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
      ERR("Unable to load OpenGL using GLAD");

   if ( !GLAD_GL_VERSION_3_1 )
      WARN( "Naev requires OpenGL 3.1, but got OpenGL %d.%d!", GLVersion.major, GLVersion.minor );

   /* Some OpenGL options. */
   glClearColor( 0., 0., 0., 1. );

   /* Set default opengl state. */
   gl_defState();

   /* Handles resetting the viewport and scaling, rw/rh are set in createWindow. */
   gl_resize( gl_screen.rw, gl_screen.rh );

   /* Finishing touches. */
   glClear( GL_COLOR_BUFFER_BIT ); /* must clear the buffer first */
   gl_checkErr();

   /* Start hinting. */
   gl_hint();

   /* Initialize subsystems.*/
   gl_initMatrix();
   gl_initTextures();
   gl_initVBO();
   gl_initRender();

   /* Get info about the OpenGL window */
   gl_getGLInfo();

   /* Modern OpenGL requires at least one VAO */
   glGenVertexArrays(1, &VaoId);
   glBindVertexArray(VaoId);

   shaders_load();

   /* Cosmetic new line. */
   DEBUG("");

   return 0;
}

/**
 * @brief Handles a window resize and resets gl_screen parametes.
 *
 *    @param w New width.
 *    @param h New height.
 */
void gl_resize( int w, int h )
{
   glViewport( 0, 0, w, h );

   gl_screen.rw = w;
   gl_screen.rh = h;

   /* Reset scaling. */
   gl_screen.scale = 1./conf.scalefactor;

   gl_setupScaling();
   gl_setDefViewport( 0, 0, gl_screen.nw, gl_screen.nh );
   gl_defViewport();

   gl_checkErr();
}

/**
 * @brief Sets the opengl viewport.
 */
void gl_viewport( int x, int y, int w, int h )
{
   gl_Matrix4 proj;

   proj = gl_Matrix4_Ortho( 0., /* Left edge. */
            gl_screen.nw, /* Right edge. */
            0., /* Bottom edge. */
            gl_screen.nh, /* Top edge. */
            -1., /* near */
            1. ); /* far */

   /* Take into account possible translation. */
   gl_screen.x = x;
   gl_screen.y = y;
   proj = gl_Matrix4_Translate(proj, x, y, 0);

   /* Set screen size. */
   gl_screen.w = w;
   gl_screen.h = h;

   /* Take into account possible scaling. */
   if (gl_screen.scale != 1.)
      proj = gl_Matrix4_Scale(proj, gl_screen.wscale, gl_screen.hscale, 1);

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
}


/**
 * @brief Cleans up OpenGL, the works.
 */
void gl_exit (void)
{
   /* Exit the OpenGL subsystems. */
   gl_exitRender();
   gl_exitVBO();
   gl_exitTextures();
   gl_exitMatrix();

   shaders_unload();

   /* Shut down the subsystem */
   SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


/**
 * @brief Saves a png.
 *
 *    @param file_name Name of the file to save the png as.
 *    @param rows Rows containing the data.
 *    @param w Width of the png.
 *    @param h Height of the png.
 *    @param colourtype Colour type of the png.
 *    @param bitdepth Bit depth of the png.
 *    @return 0 on success.
 */
static int write_png( const char *file_name, png_bytep *rows,
      int w, int h, int colourtype, int bitdepth )
{
   png_structp png_ptr;
   png_infop info_ptr;
   FILE *fp;

   /* Open file for writing. */
   if (!(fp = fopen(file_name, "wb"))) {
      WARN(_("Unable to open '%s' for writing."), file_name);
      return -1;
   }

   /* Create working structs. */
   if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
      WARN(_("Unable to create png write struct."));
      goto ERR_FAIL;
   }
   if (!(info_ptr = png_create_info_struct(png_ptr))) {
      WARN(_("Unable to create PNG info struct."));
      goto ERR_FAIL;
   }

   /* Set image details. */
   png_init_io(png_ptr, fp);
   png_set_compression_level(png_ptr, Z_DEFAULT_COMPRESSION);
   png_set_IHDR(png_ptr, info_ptr, w, h, bitdepth, colourtype,
         PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
         PNG_FILTER_TYPE_DEFAULT);

   /* Write image. */
   png_write_info(png_ptr, info_ptr);
   png_write_image(png_ptr, rows);
   png_write_end(png_ptr, NULL);

   /* Clean up. */
   fclose(fp);
   png_destroy_write_struct( &png_ptr, &info_ptr );

   return 0;

ERR_FAIL:
   fclose(fp);
   png_destroy_write_struct( &png_ptr, &info_ptr );
   return -1;
}


