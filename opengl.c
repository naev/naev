

#include "opengl.h"

#include "SDL.h"
#include "SDL_image.h"

#include "all.h"
#include "log.h"


#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#  define RMASK	0xff000000
#  define GMASK	0x00ff0000
#  define BMASK	0x0000ff00
#  define AMASK	0x000000ff
#else
#  define RMASK	0x000000ff
#  define GMASK	0x0000ff00
#  define BMASK	0x00ff0000
#  define AMASK	0xff000000
#endif
#define RGBAMASK	RMASK,GMASK,BMASK,AMASK


int flip_surface( SDL_Surface* surface );


/*
 * flips the surface vertically
 *
 * returns 0 on success
 */
int flip_surface( SDL_Surface* surface )
{
	/* flip the image */
	Uint8 *rowhi, *rowlo, *tmpbuf;
	int y;

	tmpbuf = (Uint8 *)malloc(surface->pitch);
	if ( tmpbuf == NULL ) {
		WARN("Out of memory");
		return -1;
	}

	rowhi = (Uint8 *)surface->pixels;
	rowlo = rowhi + (surface->h * surface->pitch) - surface->pitch;
	for (y = 0; y < surface->h / 2; ++y ) {
		memcpy(tmpbuf, rowhi, surface->pitch);
		memcpy(rowhi, rowlo, surface->pitch);
		memcpy(rowlo, tmpbuf, surface->pitch);
		rowhi += surface->pitch;
		rowlo -= surface->pitch;
	}
	free(tmpbuf);
	/* flipping done */

	return 0;
}


/*
 * loads the image as an opengl texture directly
 */
gl_texture*  gl_newImage( const char* path )
{
	SDL_Surface *temp, *surface;
	Uint32 saved_flags;
	Uint8  saved_alpha;

	temp = IMG_Load( path ); /* loads the surface */
	if (temp == 0) {
		WARN("'%s' could not be opened: %s", path, IMG_GetError());
		return 0;
	}

	surface = SDL_DisplayFormatAlpha( temp ); /* sets the surface to what we use */
	if (surface == 0) {
		WARN( "Error converting image to screen format: %s", SDL_GetError() );
		return 0;
	}

	SDL_FreeSurface( temp ); /* free the temporary surface */

	flip_surface( surface );

	/* set up the texture defaults */
	gl_texture *texture = MALLOC_ONE(gl_texture);
	texture->w = surface->w;
	texture->h = surface->h;
	texture->sx = 1;
	texture->sy = 1;

	/* Make size power of two */
	texture->rw = surface->w;
	if ((texture->rw & (texture->rw - 1)) != 0) {
		texture->rw = 1;
		while (texture->rw < surface->w)
			texture->rw <<= 1;
	}
	texture->rh = surface->h;
	if ((texture->rh & (texture->rh - 1)) != 0) {
		texture->rh = 1;
		while (texture->rh < surface->h)
			texture->rh <<= 1;
	}

	if (surface->w != texture->rw || surface->h != texture->rh ) { /* size isn't original */
		SDL_Rect rtemp;
		rtemp.x = rtemp.y = 0;
		rtemp.w = surface->w;
		rtemp.h = surface->h;

		/* saves alpha */
		saved_flags = surface->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
		saved_alpha = surface->format->alpha;
		if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA )
			SDL_SetAlpha( surface, 0, 0 );

		
		/* create the temp POT surface */
		temp = SDL_CreateRGBSurface( SDL_SRCCOLORKEY,
				texture->rw, texture->rh, surface->format->BytesPerPixel*8, RGBAMASK );
		if (temp == NULL) {
			WARN("Unable to create POT surface: %s", SDL_GetError());
			return 0;
		}
		if (SDL_FillRect( temp, NULL, SDL_MapRGB( surface->format, 0, 255, 255 ))) {
			WARN("Unable to fill rect: %s", SDL_GetError());
			return 0;
		}
		

		SDL_BlitSurface( surface, &rtemp, temp, &rtemp);
		SDL_FreeSurface( surface );

		surface = temp;

		/* set saved alpha */
		if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA )
			SDL_SetAlpha( surface, saved_flags, saved_alpha );
	}

	glGenTextures( 1, &texture->texture ); /* Creates the texture */
	glBindTexture( GL_TEXTURE_2D, texture->texture ); /* Loads the texture */

	/* Filtering, LINEAR is better for scaling, nearest looks nicer */
/*	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	SDL_LockSurface( surface );
	glTexImage2D( GL_TEXTURE_2D, 0, surface->format->BytesPerPixel,
			 surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
	SDL_UnlockSurface( surface );

	SDL_FreeSurface( surface );

	return texture;
}


/*
 * Loads the texture immediately, but also sets it as a sprite
 */
gl_texture* gl_newSprite( const char* path, const int sx, const int sy )
{
	gl_texture* texture;
	if ((texture = gl_newImage(path)) == NULL)
		return NULL;
	texture->sx = sx;
	texture->sy = sy;
	return texture;
}


/*
 * frees the texture
 */
void gl_free( gl_texture* texture )
{
	glDeleteTextures( 1, &texture->texture );
	free(texture);
}


void gl_blit( gl_texture* texture, Vector2d* pos )
{
	glPushMatrix(); /* set up translation matrix */
		glTranslated( pos->x, pos->y, 0);

	glBindTexture( GL_TEXTURE_2D, texture->texture);
	glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 0.0f, 0.0f);
			glVertex2f( 0.0f, 0.0f );
		glTexCoord2f( (FP)texture->w/(FP)texture->rw, 0.0f);
			glVertex2f( (FP)texture->w, 0.0f );
		glTexCoord2f( 0.0f, (FP)texture->h/(FP)texture->rh);
			glVertex2f( 0.0f, (FP)texture->h );
		glTexCoord2f( (FP)texture->w/(FP)texture->rw, (FP)texture->h/(FP)texture->rh);
			glVertex2f( (FP)texture->w, (FP)texture->h );
	glEnd();

	glPopMatrix(); /* pop the translation matrix */
}


/*
 * Initializes SDL/OpenGL and the works
 */
int gl_init( gl_info* info )
{
	int depth;
	int flags = SDL_OPENGL;

	/* Initializes Video */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		WARN("Unable to initialize SDL: %s", SDL_GetError());
		return -1;
	}

	/* we don't want none of that ugly cursor */
	SDL_ShowCursor(SDL_DISABLE);

	flags |= SDL_FULLSCREEN * info->fullscreen;
	depth = SDL_VideoModeOK( info->w, info->h, info->depth, flags); /* test set up */
	if (depth != info->depth)
		WARN("Depth %d bpp unavailable, will use %d bpp", info->depth, depth);

	info->depth = depth;

	/* actually creating the screen */
	if (SDL_SetVideoMode( info->w, info->h, info->depth, flags) == NULL) {
		WARN("Unable to create OpenGL window: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}
	DEBUG("OpenGL Window Created: %dx%d@%dbpp %s", info->w, info->h, info->depth, 
			info->fullscreen?"fullscreen":"window");


	/* Get info about the OpenGL window */
	SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &info->r );
	SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &info->g );
	SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &info->b );
	SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &info->a );
	SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &info->doublebuf );

	DEBUG("r: %d, g: %d, b: %d, a: %d, doublebuffer: %s",
			info->r, info->g, info->b, info->a, info->doublebuf?"yes":"no");
	DEBUG("Renderer: %s", glGetString(GL_RENDERER));

	/* some OpenGL options */
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glDisable( GL_DEPTH_TEST ); /* set for doing 2d */
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glEnable( GL_TEXTURE_2D );
	glOrtho( 0.0f, /* left edge */
			info->w, /* right edge */
			0.0f, /* bottom edge */
			info->h, /* top edge */
			-1.0f, /* near */
			1.0f ); /* far */

	glClear( GL_COLOR_BUFFER_BIT );


	gl_texture *llama;
	if ((llama = gl_newImage("llama.png")) == NULL) {
		WARN("Unable to load image");
		return -1;
	}
	SDL_GL_SwapBuffers();

	Vector2d pos = { .x=100.0, .y=100.0 };
	gl_blit( llama, &pos ); 
	SDL_GL_SwapBuffers();
	gl_free(llama);


	SDL_WM_SetCaption( "GAME", NULL );

	return 0;

}


/*
 * Cleans up SDL/OpenGL, the works
 */
void gl_exit()
{
	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();
}
