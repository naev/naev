/*
 * See Licensing and Copyright notice in naev.h
 */



#ifndef OPENGL_H
#  define OPENGL_H


#include "SDL.h"
#include "SDL_opengl.h"

#include "physics.h"
#include "colour.h"


/* Recommended for compatibility and such */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#  define RMASK   0xff000000
#  define GMASK   0x00ff0000
#  define BMASK   0x0000ff00
#  define AMASK   0x000000ff
#else
#  define RMASK   0x000000ff
#  define GMASK   0x0000ff00
#  define BMASK   0x00ff0000
#  define AMASK   0xff000000
#endif
#define RGBAMASK  RMASK,GMASK,BMASK,AMASK


/*
 * Contains info about the opengl screen
 */
#define OPENGL_FULLSCREEN	(1<<0)
#define OPENGL_DOUBLEBUF	(1<<1)
#define OPENGL_AA_POINT		(1<<2)
#define OPENGL_AA_LINE		(1<<3)
#define OPENGL_AA_POLYGON	(1<<4)
#define gl_has(f)		(gl_screen.flags & (f)) /* check for the flag */
typedef struct glInfo_ {
	int w, h; /* window dimensions */
	int depth; /* depth in bpp */
	int r, g, b, a; /* framebuffer values in bits */
	int flags; /* stores different propertiers */
} glInfo;
extern glInfo gl_screen; /* local structure set with gl_init and co */


/*
 * used with colour.h
 */
#define COLOUR(x)     glColor4d((x).r,(x).g,(x).b,(x).a)


/*
 * Spritesheet info
 */
typedef struct glTexture_ {
	double w, h; /* real size of the image (excluding POT buffer) */
	double rw, rh; /* size of POT surface */
	double sx, sy; /* number of sprites on x axis and y axis */
	double sw, sh; /* size of each sprite */
	GLuint texture; /* the opengl texture itself */
	uint8_t* trans; /* maps the transparency */
} glTexture;



/*
 * glTexture loading / freeing
 */
glTexture* gl_loadImage( SDL_Surface* surface ); /* frees the surface */
glTexture* gl_newImage( const char* path );
glTexture* gl_newSprite( const char* path, const int sx, const int sy );
void gl_freeTexture( glTexture* texture );

/*
 * opengl drawing
 */
/* blits a sprite */
void gl_blitSprite( const glTexture* sprite,
		const double bx, const double by,
		const int sx, const int sy, const glColour *c );
/* blits the entire image */
void gl_blitStatic( const glTexture* texture,
		const double bx, const double by, const glColour *c );
/* binds the camera to a vector */
void gl_bindCamera( const Vector2d* pos );
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
int gl_isTrans( const glTexture* t, const int x, const int y );
void gl_getSpriteFromDir( int* x, int* y, const glTexture* t, const double dir );
void gl_screenshot( const char *filename );


#endif /* OPENGL_H */
	
