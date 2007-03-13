

#ifndef OPENGL_H
#  define OPENGL_H


#include "SDL_opengl.h"

#include "physics.h"


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


/*
 * Spritesheet info
 */
typedef struct {
	int w,h; /* real size of the image (excluding POT buffer) */
	int rw,rh; /* size of POT surface */
	int sx, sy; /* number of sprites on x axis and y axis */
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
void gl_blit( gl_texture* texture, Vector2d* pos );

/*
 * initialization / cleanup
 */
int gl_init(gl_info* info);
void gl_exit(void);


#endif /* OPENGL_H */
	
