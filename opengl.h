

#ifndef OPENGL_H
#  define OPENGL_H


#include "SDL_opengl.h"

#include "physics.h"


#define WINDOW_CAPTION	"game"


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
 * Spritesheet info
 */
typedef struct {
	FP w,h; /* real size of the image (excluding POT buffer) */
	FP rw,rh; /* size of POT surface */
	FP sx, sy; /* number of sprites on x axis and y axis */
	FP sw, sh; /* size of each sprite */
	GLuint texture; /* the opengl texture itself */
} gl_texture;


/*
 * gl_texture loading / freeing
 */
gl_texture* gl_newImage( const char* path );
gl_texture* gl_newSprite( const char* path, const int sx, const int sy );
void gl_free( gl_texture* texture );

/*
 * opengl drawing
 */
void gl_blitSprite( gl_texture* sprite, Vector2d* pos, const int sx, const int sy );
void gl_blit( gl_texture* texture, Vector2d* pos );
void gl_bindCamera( Vector2d* pos );

/*
 * initialization / cleanup
 */
int gl_init(void);
void gl_exit(void);


#endif /* OPENGL_H */
	
