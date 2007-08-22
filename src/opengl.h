

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
typedef struct {
	int w, h; /* window dimensions */
	int depth; /* depth in bpp */
	int fullscreen; /* 1 = fullscreen, 0 = not fullscreen */
	int r, g, b, a; /* framebuffer values in bits */
	int doublebuf; /* doublebuffer? */
} gl_info;
extern gl_info gl_screen; /* local structure set with gl_init and co */


/*
 * Colours
 */
typedef struct {
	double r, g, b, a;
} gl_colour;
#define COLOUR(x)     glColor4d((x).r,(x).g,(x).b,(x).a)
/* default colors */
extern gl_colour cLightGrey;
extern gl_colour cGrey;
extern gl_colour cDarkGrey;
extern gl_colour cGreen;
extern gl_colour cRed;


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
} gl_texture;


/*
 * Font info
 */
typedef struct {
	int h; /* height */
	int* w;
	GLuint *textures;
	GLuint list_base;
} gl_font;
extern gl_font gl_defFont; /* default font */


/*
 * gl_font loading / freeing
 *
 * if font is NULL it uses the internal default font same with gl_print
 */
void gl_fontInit( gl_font* font, const char *fname, const unsigned int h );
void gl_freeFont( gl_font* font );


/*
 * gl_texture loading / freeing
 */
gl_texture* gl_loadImage( SDL_Surface* surface ); /* frees the surface */
gl_texture* gl_newImage( const char* path );
gl_texture* gl_newSprite( const char* path, const int sx, const int sy );
void gl_freeTexture( gl_texture* texture );

/*
 * opengl drawing
 */
void gl_blitSprite( const gl_texture* sprite, const Vector2d* pos,
		const int sx, const int sy, const gl_colour *c );
void gl_blitStatic( const gl_texture* texture, const Vector2d* pos, const gl_colour *c );
void gl_bindCamera( const Vector2d* pos );
void gl_print( const gl_font *ft_font, const Vector2d *pos,
		const gl_colour *c, const char *fmt, ... );
int gl_printWidth( const gl_font *ft_font, const char *fmt, ... );

/*
 * initialization / cleanup
 */
int gl_init (void);
void gl_exit (void);

/*
 * misc
 */
int gl_isTrans( const gl_texture* t, const int x, const int y );
void gl_getSpriteFromDir( int* x, int* y, const gl_texture* t, const double dir );
void gl_screenshot( const char *filename );


#endif /* OPENGL_H */
	
