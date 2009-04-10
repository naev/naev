/*
 * See Licensing and Copyright notice in naev.h
 */


#ifndef OPENGL_H
#  define OPENGL_H

#include <stdint.h>

#include "SDL.h"
#include "SDL_opengl.h"

#include "physics.h"
#include "colour.h"

#include "ncompat.h"


/* Recommended for compatibility and such */
#if HAS_BIGENDIAN
#  define RMASK   0xff000000 /**< Red bit mask. */
#  define GMASK   0x00ff0000 /**< Green bit mask. */
#  define BMASK   0x0000ff00 /**< Blue bit mask. */
#  define AMASK   0x000000ff /**< Alpha bit mask. */
#else
#  define RMASK   0x000000ff /**< Red bit mask. */
#  define GMASK   0x0000ff00 /**< Green bit mask. */
#  define BMASK   0x00ff0000 /**< Blue bit mask. */
#  define AMASK   0xff000000 /**< Alpha bit mask. */
#endif
#define RGBAMASK  RMASK,GMASK,BMASK,AMASK


/*
 * Contains info about the opengl screen
 */
#define OPENGL_FULLSCREEN  (1<<0) /**< Fullscreen. */
#define OPENGL_DOUBLEBUF   (1<<1) /**< Doublebuffer. */
#define OPENGL_AA_POINT    (1<<2) /**< Antialiasing points. */
#define OPENGL_AA_LINE     (1<<3) /**< Antialiasing lines. */
#define OPENGL_AA_POLYGON  (1<<4) /**< Antialiasing polygons. */
#define OPENGL_VSYNC       (1<<5) /**< Sync to monitor vertical refresh rate. */
#define OPENGL_FRAG_SHADER (1<<6) /**< Fragment shaders. */
#define OPENGL_VERT_SHADER (1<<7) /**< Vertex shaders. */
#define OPENGL_DIM_DEF     (1<<8) /**< Dimensions specifically defined. */
#define OPENGL_FSAA        (1<<9) /**< Full Screen Anti Aliasing. */
#define gl_has(f)    (gl_screen.flags & (f)) /**< Check for the flag */
/**
 * @brief Stores data about the current opengl environment.
 */
typedef struct glInfo_ {
   int w; /**< Window viewport width. */
   int h; /**< Window viewport height. */
   int nw; /**< Scaled window width. */
   int nh; /**< Scaled window height. */
   int rw; /**< Real window width. */
   int rh; /**< Real window height. */
   double scale; /**< Scale factor. */
   double wscale; /**< Width scale factor. */
   double hscale; /**< Height scale factor. */
   double mxscale; /**< Mouse X scale factor. */
   double myscale; /**< Mouse y scale factor. */
   int depth; /**< Depth in bpp */
   int r; /**< How many red bits we have. */
   int g; /**< How many green bits we have. */
   int b; /**< How many blue bits we have. */
   int a; /**< How many alpha bits we have. */
   unsigned int flags; /**< Stores different propertiers */
   int tex_max; /**< Maximum texture size */
   int multitex_max; /**< Maximum multitexture levels */
   int fsaa; /**< Full Scene Anti Aliasing level. */
} glInfo;
extern glInfo gl_screen; /* local structure set with gl_init and co */

#define  SCREEN_W gl_screen.w /**< Screen width. */
#define  SCREEN_H gl_screen.h /**< Screen height. */


/*
 * used with colour.h
 */
#define COLOUR(x)    glColor4d((x).r,(x).g,(x).b,(x).a) /**< Change colour. */
#define ACOLOUR(x,a) glColor4d((x).r,(x).g,(x).b,a) /**< Change colour and override alpha. */


/*
 * Texture flags.
 */
#define OPENGL_TEX_MAPTRANS   (1<<0)  /**< Create a transparency map. */

/**
 * @brief Abstraction for rendering spriteshets.
 *
 * The basic unit all the graphic rendering works with.
 */
typedef struct glTexture_ {
   char *name; /**< name of the graphic */

   /* dimensions */
   double w; /**< Real width of the image. */
   double h; /**< Real heiht of the image. */
   double rw; /**< Padded POT width of the image. */
   double rh; /**< Padded POT height of the image. */

   /* sprites */
   double sx; /**< Number of sprites on the x axis. */
   double sy; /**< Number of sprites on the y axis. */
   double sw; /**< Width of a sprite. */
   double sh; /**< Height of a sprite. */

   /* data */
   GLuint texture; /**< the opengl texture itself */
   uint8_t* trans; /**< maps the transparency */

   /* properties */
   uint8_t flags; /**< flags used for texture properties */
} glTexture;



/*
 * glTexture loading / freeing
 */
SDL_Surface* gl_prepareSurface( SDL_Surface* surface ); /* Only preps it */
glTexture* gl_loadImage( SDL_Surface* surface ); /* Frees the surface. */
glTexture* gl_newImage( const char* path, const unsigned int flags );
glTexture* gl_newSprite( const char* path, const int sx, const int sy,
      const unsigned int flags );
void gl_freeTexture( glTexture* texture );
glTexture* gl_dupTexture( glTexture *texture );

/*
 * opengl drawing
 */
/* blits a sprite, relative pos */
void gl_blitSprite( const glTexture* sprite,
      const double bx, const double by,
      const int sx, const int sy, const glColour *c );
/* blits a sprite, absolute pos */
void gl_blitStaticSprite( const glTexture* sprite,
      const double bx, const double by,
      const int sx, const int sy, const glColour* c );
/* blits a texture scaled, absolute pos */
void gl_blitScale( const glTexture* texture,
      const double bx, const double by,
      const double bw, const double bh, const glColour* c );
/* blits the entire image, absolute pos */
void gl_blitStatic( const glTexture* texture,
      const double bx, const double by, const glColour *c );
/* binds the camera to a vector */
void gl_bindCamera( Vector2d* pos );
/* circle drawing */
void gl_drawCircle( const double x, const double y, const double r );
void gl_drawCircleInRect( const double x, const double y, const double r,
      const double rx, const double ry, const double rw, const double rh );


/*
 * initialization / cleanup
 */
int gl_init (void);
void gl_exit (void);

/*
 * misc
 */
double gl_setScale( double scalefactor );
void gl_defViewport (void);
int gl_pot( int n );
int gl_isTrans( const glTexture* t, const int x, const int y );
void gl_getSpriteFromDir( int* x, int* y, const glTexture* t, const double dir );
void gl_screenshot( const char *filename );
int SDL_SavePNG( SDL_Surface *surface, const char *file );
#if DEBUG == 1
void gl_checkErr (void);
#else /* DEBUG */
#define gl_checkErr() /**< Hack to ignore errors when debugging. */
#endif /* DEBUG */


#endif /* OPENGL_H */
   
