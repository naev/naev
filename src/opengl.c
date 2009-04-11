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
 *  * Origin (0., 0.) wouldbe ontop of the player.
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

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h> /* va_list for gl_print */

#include <png.h>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_version.h"

#include "log.h"
#include "opengl_ext.h"
#include "ndata.h"
#include "gui.h"


/*
 * Requirements
 */
#define OPENGL_REQ_MULTITEX      2 /**< 2 is minimum OpenGL 1.2 must have */


glInfo gl_screen; /**< Gives data of current opengl settings. */
static int gl_activated = 0; /**< Whether or not a window is activated. */

/*
 * graphic list
 */
/**
 * @brief Represents a node in the texture list.
 */
typedef struct glTexList_ {
   struct glTexList_ *next; /**< Next in linked list */
   glTexture *tex; /**< assosciated texture */
   int used; /**< counts how many times texture is being used */
} glTexList;
static glTexList* texture_list = NULL; /**< Texture list. */


/*
 * prototypes
 */
/* gl */
static int gl_setupAttributes (void);
static int gl_setupFullscreen( unsigned int *flags, const SDL_VideoInfo *vidinfo );
static int gl_createWindow( unsigned int flags );
static int gl_getGLInfo (void);
static int gl_defState (void);
static int gl_setupScaling (void);
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
   SDL_Surface *screen = SDL_GetVideoSurface();
   unsigned rowbytes = screen->w * 4;
   unsigned char screenbuf[screen->h][rowbytes], *rows[screen->h];
   int i;

   /* Read pixels from buffer -- SLOW. */
   glReadPixels( 0, 0, screen->w, screen->h,
         GL_RGBA, GL_UNSIGNED_BYTE, screenbuf );

   /* Convert data. */
   for (i = 0; i < screen->h; i++)
      rows[i] = screenbuf[screen->h - i - 1];

   /* Save PNG. */
   write_png( filename, rows, screen->w, screen->h,
         PNG_COLOR_TYPE_RGBA, 8);

   /* Check to see if an error occured. */
   gl_checkErr();
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
#if ! SDL_VERSION_ATLEAST(1,3,0)
   unsigned int surf_flags;
   unsigned int surf_alpha;
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   /* Initialize parameters. */
   ss_rows     = NULL;
   ss_size     = 0;
   ss_surface  = NULL;

   /* Set size. */
   ss_w        = surface->w;
   ss_h        = surface->h;

   /* Handle alpha. */
#if SDL_VERSION_ATLEAST(1,3,0)
   SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

   /* Create base surface. */
   ss_surface = SDL_CreateRGBSurface( 0, ss_w, ss_h, 32, RGBAMASK );
#else /* SDL_VERSION_ATLEAST(1,3,0) */
   surf_flags = surface->flags & (SDL_SRCALPHA | SDL_SRCCOLORKEY);
   surf_alpha = surface->format->alpha;
   if ((surf_flags & SDL_SRCALPHA) == SDL_SRCALPHA) {
      SDL_SetAlpha( surface, 0, SDL_ALPHA_OPAQUE );
      SDL_SetColorKey( surface, 0, surface->format->colorkey );
   }

   /* Create base surface. */
   ss_surface = SDL_CreateRGBSurface( SDL_SRCCOLORKEY, ss_w, ss_h, 32, RGBAMASK );
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
   if (ss_surface == NULL) {
      WARN("Unable to create RGB surface.");
      return -1;
   }
   if (SDL_FillRect( ss_surface, NULL,
            SDL_MapRGBA(surface->format,0,0,0,SDL_ALPHA_TRANSPARENT))) {
      WARN("Unable to fill rect: %s", SDL_GetError());
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

   /* Reset flags. */
#if ! SDL_VERSION_ATLEAST(1,3,0)
   /* Set saved alpha */
   if ((surf_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
      SDL_SetAlpha( surface, 0, 0 );
#endif /* ! SDL_VERSION_ATLEAST(1,3,0) */

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
static double gl_contextVersion = -1.;
GLboolean gl_hasVersion( int major, int minor )
{
   const char *p;
   double f, c;

   if (gl_contextVersion < 0.) {
      p = (const char*) glGetString(GL_VERSION);

      /* Get version and compare version. */
      gl_contextVersion = atof(p);
   }

   c  = (double) major;
   c += 0.1 * (double) minor;

   if (f <= gl_contextVersion)
      return GL_TRUE;
   return GL_FALSE;
}


/**
 * @brief Checks for on opengl extension.
 *
 *    @param name Extension to check for.
 *    @return GL_TRUE if found, GL_FALSE if isn't.
 */
GLboolean gl_hasExt( char *name )
{
   /*
    * Search for name in the extensions string.  Use of strstr()
    * is not sufficient because extension names can be prefixes of
    * other extension names.  Could use strtok() but the constant
    * string returned by glGetString can be in read-only memory.
    */
   const char *p, *end;
   size_t len, n;

   p = (const char*) glGetString(GL_EXTENSIONS);
   len = strlen(name);
   end = p + strlen(p);

   while (p < end) {
      n = strcspn(p, " ");
      if ((len == n) && (strncmp(name,p,n)==0))
         return GL_TRUE;

      p += (n + 1);
   }
   return GL_FALSE;
}


#ifndef gl_checkErr /**< i agree it's a bit hackish :) */
/**
 * @brief Checks and reports if there's been an error.
 */
void gl_checkErr (void)
{
   GLenum err;
   char* errstr;

   err = glGetError();

   if (err == GL_NO_ERROR) return; /* no error */

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
      case GL_STACK_OVERFLOW:
         errstr = "GL stack overflow";
         break;
      case GL_STACK_UNDERFLOW:
         errstr = "GL stack underflow";
         break;
      case GL_OUT_OF_MEMORY:
         errstr = "GL out of memory";
         break;
      case GL_TABLE_TOO_LARGE:
         errstr = "GL table too large";
         break;

      default:
         errstr = "GL unknown error";
         break;
   }
   WARN("OpenGL error: %s",errstr);
}
#endif /* DEBUG */


/**
 * @brief Tries to set up the OpenGL attributes for the OpenGL context.
 *
 *    @return 0 on success.
 */
static int gl_setupAttributes (void)
{
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); /* Ideally want double buffering. */
   if (gl_has(OPENGL_FSAA)) {
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, gl_screen.fsaa);
   }
   if (gl_has(OPENGL_VSYNC))
#if SDL_VERSION_ATLEAST(1,3,0)
      SDL_GL_SetSwapInterval(1);
#else /* SDL_VERSION_ATLEAST(1,3,0) */
      SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   return 0;
}


/**
 * @brief Tries to set up fullscreen environment.
 *
 *    @param flags Flags to modify.
 *    @param vidinfo Video information.
 *    @return 0 on success.
 */
static int gl_setupFullscreen( unsigned int *flags, const SDL_VideoInfo *vidinfo )
{
   int i, j, off, toff, supported;
   SDL_Rect** modes;

   /* Unsupported by default. */
   supported = 0;

   /* Try to use desktop resolution if nothing is specifically set. */
#if SDL_VERSION_ATLEAST(1,2,10)
   if (!gl_has(OPENGL_DIM_DEF)) {
      gl_screen.w = vidinfo->current_w;
      gl_screen.h = vidinfo->current_h;
   }
#endif /* SDL_VERSION_ATLEAST(1,2,10) */

   /* Get available modes and see what we can use. */
   modes = SDL_ListModes( NULL, SDL_OPENGL | SDL_FULLSCREEN );
   if (modes == NULL) { /* rare case, but could happen */
      WARN("No fullscreen modes available");
      if ((*flags) & SDL_FULLSCREEN) {
         WARN("Disabling fullscreen mode");
         (*flags) &= ~SDL_FULLSCREEN;
      }
   }
   else if (modes == (SDL_Rect **)-1)
      DEBUG("All fullscreen modes available");
   else {
      DEBUG("Available fullscreen modes:");
      for (i=0; modes[i]; i++) {
         DEBUG("  %d x %d", modes[i]->w, modes[i]->h);
         if (((*flags) & SDL_FULLSCREEN) && (modes[i]->w == SCREEN_W) &&
               (modes[i]->h == SCREEN_H))
            supported = 1; /* mode we asked for is supported */
      }
   }
   /* makes sure fullscreen mode is supported */
   if (((*flags) & SDL_FULLSCREEN) && !supported) {

      /* try to get closest aproximation to mode asked for */
      off = -1;
      j = 0;
      for (i=0; modes[i]; i++) {
         toff = ABS(SCREEN_W-modes[i]->w) + ABS(SCREEN_H-modes[i]->h);
         if ((off == -1) || (toff < off)) {
            j = i;
            off = toff;
         }
      }
      WARN("Fullscreen mode %dx%d is not supported by your setup\n"
            "   switching to %dx%d",
            SCREEN_W, SCREEN_H,
            modes[j]->w, modes[j]->h );
      gl_screen.w = modes[j]->w;
      gl_screen.h = modes[j]->h;
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
   int depth;

   /* Test the setup - aim for 32. */
   gl_screen.depth = 32;
   depth = SDL_VideoModeOK( SCREEN_W, SCREEN_H, gl_screen.depth, flags);
   if (depth == 0)
      WARN("Video Mode %dx%d @ %d bpp not supported"
           "   going to try to create it anyways...",
            SCREEN_W, SCREEN_H, gl_screen.depth );
   if (depth != gl_screen.depth)
      DEBUG("Depth %d bpp unavailable, will use %d bpp", gl_screen.depth, depth);
   gl_screen.depth = depth;

   /* Actually creating the screen. */
   if (SDL_SetVideoMode( SCREEN_W, SCREEN_H, gl_screen.depth, flags)==NULL) {
      /* Try again possibly disabling FSAA. */
      if (gl_has(OPENGL_FSAA)) {
         LOG("Unable to create OpenGL window: Trying without FSAA.");
         gl_screen.flags &= ~OPENGL_FSAA;
         SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
         SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
      }
      if (SDL_SetVideoMode( SCREEN_W, SCREEN_H, gl_screen.depth, flags)==NULL) {
         ERR("Unable to create OpenGL window: %s", SDL_GetError());
         return -1;
      }
   }
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
   int doublebuf, fsaa;

   SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &gl_screen.r );
   SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &gl_screen.g );
   SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &gl_screen.b );
   SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &gl_screen.a );
   SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &doublebuf );
   SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &fsaa );
   if (doublebuf)
      gl_screen.flags |= OPENGL_DOUBLEBUF;
   /* Calculate real depth. */
   gl_screen.depth = gl_screen.r + gl_screen.g + gl_screen.b + gl_screen.a;

   /* Texture information */
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_screen.tex_max);
   glGetIntegerv(GL_MAX_TEXTURE_UNITS, &gl_screen.multitex_max);

   /* Debug happiness */
   DEBUG("OpenGL Window Created: %dx%d@%dbpp %s", SCREEN_W, SCREEN_H, gl_screen.depth,
         gl_has(OPENGL_FULLSCREEN)?"fullscreen":"window");
   DEBUG("r: %d, g: %d, b: %d, a: %d, db: %s, fsaa: %d, tex: %d",
         gl_screen.r, gl_screen.g, gl_screen.b, gl_screen.a,
         gl_has(OPENGL_DOUBLEBUF) ? "yes" : "no",
         fsaa, gl_screen.tex_max);
   DEBUG("Renderer: %s", glGetString(GL_RENDERER));
   DEBUG("Version: %s", glGetString(GL_VERSION));

   /* Now check for things that can be bad. */
   if (gl_screen.multitex_max < OPENGL_REQ_MULTITEX)
      WARN("Missing texture units (%d required, %d found)",
            OPENGL_REQ_MULTITEX, gl_screen.multitex_max );
   if (gl_has(OPENGL_FSAA) && (fsaa != gl_screen.fsaa))
      WARN("Unable to get requested FSAA level (%d requested, got %d)",
            gl_screen.fsaa, fsaa );

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
   glDisable( GL_LIGHTING ); /* no lighting, it's done when rendered */
   glEnable(  GL_BLEND ); /* alpha blending ftw */

   /* Set the blending/shading model to use. */
   glShadeModel( GL_FLAT ); /* default shade model, functions should keep this when done */
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); /* good blend model */

   return 0;
}


/**
 * @brief Checks ot see if window needs to handle scaling.
 *
 *    @return 0 on success.
 */
static int gl_setupScaling (void)
{
   /* New window is real window scaled. */
   gl_screen.nw = (double)gl_screen.rw * gl_screen.scale;
   gl_screen.nh = (double)gl_screen.rh * gl_screen.scale;
   /* Viewport matches new window size. */
   gl_screen.w  = gl_screen.nw;
   gl_screen.h  = gl_screen.nh;
   /* Small windows get handled here. */
   if ((SCREEN_W < 600) && (SCREEN_W <= SCREEN_H)) {
      if (gl_screen.scale != 1.)
         DEBUG("Screen size too small, upscaling...");
      gl_screen.scale = (double)gl_screen.w / 600.;
      /* Must keep the proportion the same for the screen. */
      gl_screen.h  = (gl_screen.h * 600) / SCREEN_W;
      gl_screen.nh = (gl_screen.rh * SCREEN_W) / 600;
      gl_screen.w  = 600;
   }
   else if ((SCREEN_H < 600) && (SCREEN_W >= SCREEN_H)) {
      if (gl_screen.scale != 1.)
         DEBUG("Screen size too small, upscaling...");
      gl_screen.scale = (double)gl_screen.h / 600.;
      /* Must keep the proportion the same for the screen. */
      gl_screen.w  = (gl_screen.w * 600) / SCREEN_H;
      gl_screen.nw = (gl_screen.rw * SCREEN_H) / 600;
      gl_screen.h  = 600;
   }
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
   const SDL_VideoInfo *vidinfo;

   /* Defaults. */
   flags  = SDL_OPENGL;
   flags |= SDL_FULLSCREEN * (gl_has(OPENGL_FULLSCREEN) ? 1 : 0);

   /* Initializes Video */
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
      WARN("Unable to initialize SDL Video: %s", SDL_GetError());
      return -1;
   }

   /* Get the video information. */
   vidinfo = SDL_GetVideoInfo();

   /* Set opengl flags. */
   gl_setupAttributes();

   /* See if should set up fullscreen. */
   if (gl_has(OPENGL_FULLSCREEN))
      gl_setupFullscreen( &flags, vidinfo );

   /* Check to see if trying to create above screen resolution without player
    * asking for such a large size. */
#if SDL_VERSION_ATLEAST(1,2,10)
   if (!gl_has(OPENGL_DIM_DEF)) {
      gl_screen.w = MIN(gl_screen.w, vidinfo->current_w);
      gl_screen.h = MIN(gl_screen.h, vidinfo->current_h);
   }
#endif /* SDL_VERSION_ATLEAST(1,2,10) */

   /* Create the window. */
   gl_createWindow( flags );

   /* Get info about the OpenGL window */
   gl_getGLInfo();

   /* Some OpenGL options. */
   glClearColor( 0., 0., 0., 1. );

   /* Set default opengl state. */
   gl_defState();

   /* Set up possible scaling. */
   gl_setupScaling();

   /* Handle setting the default viewport. */
   gl_defViewport();

   /* Finishing touches. */
   glClear( GL_COLOR_BUFFER_BIT ); /* must clear the buffer first */
   gl_checkErr();

   /* Initialize subsystems.*/
   gl_initExtensions();
   gl_initTextures();
   gl_initRender();

   /* Cosmetic new line. */
   DEBUG("");

   return 0;
}


/**
 * @brief Sets the scale factor.
 *
 *    @param scalefactor Factor to scale by.
 */
double gl_setScale( double scalefactor )
{
   gl_screen.scale = 1./scalefactor;

   /* Return actual used scalefactor. */
   return gl_screen.scale;
}


/**
 * @brief Resets viewport to default
 */
void gl_defViewport (void)
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho( -(double)gl_screen.nw/2, /* left edge */
         (double)gl_screen.nw/2, /* right edge */
         -(double)gl_screen.nh/2, /* bottom edge */
         (double)gl_screen.nh/2, /* top edge */
         -1., /* near */
         1. ); /* far */
   /* Take into account posible scaling. */
   if (gl_screen.scale != 1.)
      glScaled( gl_screen.wscale, gl_screen.hscale, 1. );
}


/**
 * @brief Cleans up OpenGL, the works.
 */
void gl_exit (void)
{
   glTexList *tex;

   /* Exit the OpenGL subsystems. */
   gl_exitRender();
   gl_exitTextures();
   gl_exitExtensions();

   /* Make sure there's no texture leak */
   if (texture_list != NULL) {
      DEBUG("Texture leak detected!");
      for (tex=texture_list; tex!=NULL; tex=tex->next)
         DEBUG("   '%s' opened %d times", tex->tex->name, tex->used );
   }

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
      WARN("Unable to open '%s' for writing.", file_name);
      return -1;
   }

   /* Create working structs. */
   if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
      WARN("Unable to create png write struct.");
      return -1;
   }
   if (!(info_ptr = png_create_info_struct(png_ptr))) {
      WARN("Unable to create PNG info struct.");
      return -1;
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

   return 0;

}

