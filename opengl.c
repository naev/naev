

#include "opengl.h"

#include "SDL.h"
#include "SDL_image.h"

#include "all.h"
#include "log.h"


#define	SCREEN_W	gl_screen.w
#define	SCREEN_H gl_screen.h



/* the screen info, gives data of current opengl settings */
gl_info gl_screen;

/* the camera */
Vector2d* gl_camera;


static int flip_surface( SDL_Surface* surface );


/*
 * flips the surface vertically
 *
 * returns 0 on success
 */
static int flip_surface( SDL_Surface* surface )
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
 * loads the SDL_Surface to an opengl texture
 */
gl_texture* gl_loadImage( SDL_Surface* surface )
{
	SDL_Surface* temp;
	Uint32 saved_flags;
	Uint8  saved_alpha;
	int potw, poth;

	/* set up the texture defaults */
	gl_texture *texture = MALLOC_ONE(gl_texture);
	texture->w = (FP)surface->w;
	texture->h = (FP)surface->h;
	texture->sx = 1.;
	texture->sy = 1.;

	/* Make size power of two */
	potw = surface->w;
	if ((potw & (potw - 1)) != 0) {
		potw = 1;
		while (potw < surface->w)
			potw <<= 1;
	} texture->rw = potw;
	poth = surface->h;
	if ((poth & (poth - 1)) != 0) {
		poth = 1;
		while (poth < surface->h)
			poth <<= 1;
	} texture->rh = poth;

	if (surface->w != potw || surface->h != poth ) { /* size isn't original */
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
			return NULL;
		}
		if (SDL_FillRect( temp, NULL, SDL_MapRGBA(surface->format,0,0,0,SDL_ALPHA_TRANSPARENT))) {
			WARN("Unable to fill rect: %s", SDL_GetError());
			return NULL;
		}

		SDL_BlitSurface( surface, &rtemp, temp, &rtemp);
		SDL_FreeSurface( surface );

		surface = temp;

		/* set saved alpha */
		if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA )
			SDL_SetAlpha( surface, 0, 0 );


		/* create the temp POT surface */
		temp = SDL_CreateRGBSurface( SDL_SRCCOLORKEY,
				texture->rw, texture->rh, surface->format->BytesPerPixel*8, RGBAMASK );
		if (temp == NULL) {
			WARN("Unable to create POT surface: %s", SDL_GetError());
			return NULL;
		}
		if (SDL_FillRect( temp, NULL, SDL_MapRGBA(surface->format,0,0,0,SDL_ALPHA_TRANSPARENT))) {
			WARN("Unable to fill rect: %s", SDL_GetError());
			return NULL;
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

	/* Filtering, LINEAR is better for scaling, nearest looks nicer, LINEAR
	 * also seems to create a bit of artifacts around the edges */
	/* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	SDL_LockSurface( surface );
	glTexImage2D( GL_TEXTURE_2D, 0, surface->format->BytesPerPixel,
			surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
	SDL_UnlockSurface( surface );

	SDL_FreeSurface( surface );

	texture->sx = 1.;
	texture->sy = 1.;
	texture->sw = texture->w;
	texture->sh = texture->h;

	return texture;

}


/*
 * loads the image as an opengl texture directly
 */
gl_texture*  gl_newImage( const char* path )
{
	SDL_Surface *temp, *surface;

	temp = IMG_Load( path ); /* loads the surface */
	if (temp == 0) {
		WARN("'%s' could not be opened: %s", path, IMG_GetError());
		return NULL;
	}

	surface = SDL_DisplayFormatAlpha( temp ); /* sets the surface to what we use */
	if (surface == 0) {
		WARN( "Error converting image to screen format: %s", SDL_GetError() );
		return NULL;
	}

	SDL_FreeSurface(temp); /* free the temporary surface */

	if (flip_surface(surface)) {
		WARN( "Error flipping surface" );
		return NULL;
	}

	return gl_loadImage(surface);
}


/*
 * Loads the texture immediately, but also sets it as a sprite
 */
gl_texture* gl_newSprite( const char* path, const int sx, const int sy )
{
	gl_texture* texture;
	if ((texture = gl_newImage(path)) == NULL)
		return NULL;
	texture->sx = (FP)sx;
	texture->sy = (FP)sy;
	texture->sw = texture->w/texture->sx;
	texture->sh = texture->h/texture->sy;
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


/*
 * blits a sprite at pos
 */
void gl_blitSprite( gl_texture* sprite, Vector2d* pos, const int sx, const int sy )
{
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
		glTranslatef( sprite->sw*(FP)(sx)/sprite->rw,
				sprite->sh*(sprite->sy-(FP)sy-1)/sprite->rh, 0. );

	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); /* projection translation matrix */
		glTranslatef( pos->x - gl_camera->x - sprite->sw/2.,
				pos->y - gl_camera->y - sprite->sh/2., 0.);
		glScalef( (FP)gl_screen.w/SCREEN_W, (FP)gl_screen.h/SCREEN_H, 0. );

	/* actual blitting */
	glBindTexture( GL_TEXTURE_2D, sprite->texture);
	glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 0., 0.);
			glVertex2f( 0., 0. );
		glTexCoord2f( sprite->sw/sprite->rw, 0.);
			glVertex2f( sprite->sw, 0. );
		glTexCoord2f( 0., sprite->sh/sprite->rh);
			glVertex2f( 0., sprite->sh );
		glTexCoord2f( sprite->sw/sprite->rw, sprite->sh/sprite->rh);
			glVertex2f( sprite->sw, sprite->sh );
	glEnd();

	glPopMatrix(); /* projection translation matrix */

	glMatrixMode(GL_TEXTURE);
	glPopMatrix(); /* sprite translation matrix */
}


/*
 * straight out blits a texture at position
 */
void gl_blitStatic( gl_texture* texture, Vector2d* pos )
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); /* set up translation matrix */
		glTranslatef( pos->x - (FP)gl_screen.w/2., pos->y - (FP)gl_screen.h/2., 0);
		glScalef( (FP)gl_screen.w/SCREEN_W, (FP)gl_screen.h/SCREEN_H, 0. );

	/* actual blitting */
	glBindTexture( GL_TEXTURE_2D, texture->texture);
	glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 0., 0.);
			glVertex2f( 0., 0. );
		glTexCoord2f( texture->w/texture->rw, 0.);
			glVertex2f( texture->w, 0. );
		glTexCoord2f( 0., texture->h/texture->rh);
			glVertex2f( 0., texture->h );
		glTexCoord2f( texture->w/texture->rw, texture->h/texture->rh);
			glVertex2f( texture->w, texture->h );
	glEnd();

	glPopMatrix(); /* pop the translation matrix */
}


/*
 * Binds the camero to a vector
 */
void gl_bindCamera( Vector2d* pos )
{
	gl_camera = pos;
}


/*
 * Initializes SDL/OpenGL and the works
 */
int gl_init()
{
	int depth, i, supported = 0;
	SDL_Rect** modes;
	int flags = SDL_OPENGL;
	flags |= SDL_FULLSCREEN * gl_screen.fullscreen;

	/* Initializes Video */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		WARN("Unable to initialize SDL: %s", SDL_GetError());
		return -1;
	}

	/* get available fullscreen modes */
	modes = SDL_ListModes( NULL, SDL_OPENGL | SDL_FULLSCREEN );
	if (modes == NULL) {
		WARN("No fullscreen modes available");
		if (flags & SDL_FULLSCREEN) {
			WARN("Disabling fullscreen mode");
			flags ^= SDL_FULLSCREEN;
		}
	}
	else if (modes == (SDL_Rect **)-1)
		DEBUG("All fullscreen modes available");
	else {
		DEBUG("Available fullscreen modes:");
		for (i=0;modes[i];++i) {
			DEBUG("  %d x %d", modes[i]->w, modes[i]->h);
			if (flags & SDL_FULLSCREEN && modes[i]->w == gl_screen.w && modes[i]->h == gl_screen.h)
				supported = 1;
		}
	}
	/* makes sure fullscreen mode is supported */
	if (flags & SDL_FULLSCREEN && !supported) {
		WARN("Fullscreen mode %d x %d is not supported by your setup, switching to another mode",
				gl_screen.w, gl_screen.h);
		gl_screen.w = modes[0]->w;
		gl_screen.h = modes[0]->h;
	}

	
	/* test the setup */
	depth = SDL_VideoModeOK( gl_screen.w, gl_screen.h, gl_screen.depth, flags);
	if (depth != gl_screen.depth)
		WARN("Depth %d bpp unavailable, will use %d bpp", gl_screen.depth, depth);

	gl_screen.depth = depth;


	/* actually creating the screen */
	if (SDL_SetVideoMode( gl_screen.w, gl_screen.h, gl_screen.depth, flags) == NULL) {
		WARN("Unable to create OpenGL window: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	/* Get info about the OpenGL window */
	SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &gl_screen.r );
	SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &gl_screen.g );
	SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &gl_screen.b );
	SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &gl_screen.a );
	SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &gl_screen.doublebuf );
	gl_screen.depth = gl_screen.r + gl_screen.g + gl_screen.b + gl_screen.a;

	/* Debug happiness */
	DEBUG("OpenGL Window Created: %dx%d@%dbpp %s", gl_screen.w, gl_screen.h, gl_screen.depth,
			gl_screen.fullscreen?"fullscreen":"window");
	DEBUG("r: %d, g: %d, b: %d, a: %d, doublebuffer: %s",
			gl_screen.r, gl_screen.g, gl_screen.b, gl_screen.a, gl_screen.doublebuf?"yes":"no");
	DEBUG("Renderer: %s", glGetString(GL_RENDERER));

	/* some OpenGL options */
	glClearColor( 0., 0., 0., 0. );
	glDisable( GL_DEPTH_TEST ); /* set for doing 2d */
	glEnable( GL_TEXTURE_2D );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( -SCREEN_W/2, /* left edge */
			SCREEN_W/2,//gl_screen.w/2, /* right edge */
			-SCREEN_H/2,//-gl_screen.h/2, /* bottom edge */
			SCREEN_H/2,//gl_screen.h/2, /* top edge */
			-1., /* near */
			1. ); /* far */
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); /* alpha */
	glEnable( GL_BLEND );

	glClear( GL_COLOR_BUFFER_BIT );


	SDL_WM_SetCaption( WINDOW_CAPTION, NULL );

	return 0;
}


/*
 * Cleans up SDL/OpenGL, the works
 */
void gl_exit()
{
	SDL_Quit();
}
