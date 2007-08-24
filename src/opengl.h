

#ifndef OPENGL_H
#  define OPENGL_H


#include "SDL.h"
#include "SDL_opengl.h"

#include "physics.h"


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
typedef struct {
	int w, h; /* window dimensions */
	int depth; /* depth in bpp */
	int r, g, b, a; /* framebuffer values in bits */
	int flags; /* stores different propertiers */
} glInfo;
extern glInfo gl_screen; /* local structure set with gl_init and co */


/*
 * Colours
 */
typedef struct {
	double r, g, b, a;
} glColour;
#define COLOUR(x)     glColor4d((x).r,(x).g,(x).b,(x).a)
/*
 * default colors
 */
/* greyscale */
extern glColour cWhite;
#define cGrey	cGrey70
extern glColour cBlack;

extern glColour cGrey90;
extern glColour cGrey80;
extern glColour cGrey70;
extern glColour cGrey60;
extern glColour cGrey50;
extern glColour cGrey40;
extern glColour cGrey30;
extern glColour cGrey20;
extern glColour cGrey10;

extern glColour cGreen;
extern glColour cRed;


/*
 * Spritesheet info
 */
typedef struct {
	double w, h; /* real size of the image (excluding POT buffer) */
	double rw, rh; /* size of POT surface */
	double sx, sy; /* number of sprites on x axis and y axis */
	double sw, sh; /* size of each sprite */
	GLuint texture; /* the opengl texture itself */
	uint8_t* trans; /* maps the transparency */
} glTexture;


/*
 * Font info
 */
typedef struct {
	int h; /* height */
	int* w;
	GLuint *textures;
	GLuint list_base;
} glFont;
extern glFont gl_defFont; /* default font */


/*
 * glFont loading / freeing
 *
 * if font is NULL it uses the internal default font same with gl_print
 */
void gl_fontInit( glFont* font, const char *fname, const unsigned int h );
void gl_freeFont( glFont* font );


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
void gl_blitSprite( const glTexture* sprite, const Vector2d* pos,
		const int sx, const int sy, const glColour *c );
void gl_blitStatic( const glTexture* texture, const Vector2d* pos, const glColour *c );
void gl_bindCamera( const Vector2d* pos );
void gl_print( const glFont *ft_font, const double x, const double y,
		const glColour *c, const char *fmt, ... );
int gl_printMax( const glFont *ft_font, const int max,
		const double x, const double y,
		const glColour *c, const char *fmt, ... );
int gl_printMid( const glFont *ft_font, const int width,
		double x, const double y,
		const glColour* c, const char *fmt, ... );
int gl_printWidth( const glFont *ft_font, const char *fmt, ... );

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
	
