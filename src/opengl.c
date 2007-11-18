/*
 * See Licensing and Copyright notice in naev.h
 */



#include "opengl.h"

#include <math.h>
#include <stdarg.h> /* va_list for gl_print */
#include <string.h>

#include <png.h>

#include "SDL.h"
#include "SDL_image.h"

#include "naev.h"
#include "log.h"
#include "pack.h"


#define	SCREEN_W	gl_screen.w
#define	SCREEN_H gl_screen.h


/* the screen info, gives data of current opengl settings */
glInfo gl_screen;

/* the camera */
Vector2d* gl_camera;

/*
 * used to adjust the pilot's place onscreen to be in the middle even with the GUI
 */
extern double gui_xoff;
extern double gui_yoff;


/*
 * prototypes
 */
/* misc */
static int SDL_VFlipSurface( SDL_Surface* surface );
static int SDL_IsTrans( SDL_Surface* s, int x, int y );
static uint8_t* SDL_MapTrans( SDL_Surface* s );
static int pot( int n );
/* glTexture */
static GLuint gl_loadSurface( SDL_Surface* surface, int *rw, int *rh );
/* png */
int write_png( const char *file_name, png_bytep *rows,
		int w, int h, int colourtype, int bitdepth );


/*
 *
 * M I S C
 *
 */
/*
 * gets the closest power of two
 */
static int pot( int n )
{
	int i = 1;
	while (i < n)
		i <<= 1;
	return i;
}


/*
 * flips the surface vertically
 *
 * returns 0 on success
 */
static int SDL_VFlipSurface( SDL_Surface* surface )
{
	/* flip the image */
	Uint8 *rowhi, *rowlo, *tmpbuf;
	int y;

	tmpbuf = malloc(surface->pitch);
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
 * returns true if position (x,y) of s is transparent
 */
static int SDL_IsTrans( SDL_Surface* s, int x, int y )
{
	int bpp = s->format->BytesPerPixel; 
	// here p is the address to the pixel we want to retrieve 
	Uint8 *p = (Uint8 *)s->pixels + y*s->pitch + x*bpp; 

	Uint32 pixelcolour = 0; 

	switch(bpp) {        
		case 1: 
			pixelcolour = *p; 
			break; 

		case 2: 
			pixelcolour = *(Uint16 *)p; 
			break; 

		case 3: 
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN) 
				pixelcolour = p[0] << 16 | p[1] << 8 | p[2]; 
			else     
				pixelcolour = p[0] | p[1] << 8 | p[2] << 16; 
			break; 

		case 4: 
			pixelcolour = *(Uint32 *)p; 
			break; 
	} 

	// test whether pixels colour == colour of transparent pixels for that surface 
	return (pixelcolour == s->format->colorkey);
}


/*
 * maps the surface transparency
 *
 * returns 0 on success
 */
static uint8_t* SDL_MapTrans( SDL_Surface* s )
{
	/* alloc memory for just enough bits to hold all the data we need */
	int size = s->w*s->h/8 + ((s->w*s->h%8)?1:0);
	uint8_t* t = malloc(size);
	memset(t,0,size); /* important, must be set to zero */

	if (t==NULL) {
		WARN("Out of Memory");
		return NULL;
	}

	int i,j;
	for (i=0; i<s->h; i++)
		for (j=0; j<s->w; j++) /* sets each bit to be 1 if not transparent or 0 if is */
			t[(i*s->w+j)/8] |= (SDL_IsTrans(s,j,i)) ? 0 : (1<<((i*s->w+j)%8));

	return t;
}


/*
 * takes a screenshot
 */
void gl_screenshot( const char *filename )
{
	SDL_Surface *screen = SDL_GetVideoSurface();
	unsigned rowbytes = screen->w * 4;
	unsigned char screenbuf[screen->h][rowbytes], *rows[screen->h];
	int i;

	glReadPixels( 0, 0, screen->w, screen->h,
			GL_RGBA, GL_UNSIGNED_BYTE, screenbuf );

	for (i = 0; i < screen->h; i++) rows[i] = screenbuf[screen->h - i - 1];
	write_png( filename, rows, screen->w, screen->h,
			PNG_COLOR_TYPE_RGBA, 8);
}



/*
 *
 * G L _ T E X T U R E
 *
 */
/*
 * returns the texture ID
 * stores real sizes in rw/rh (from POT padding)
 */
static GLuint gl_loadSurface( SDL_Surface* surface, int *rw, int *rh )
{
	GLuint texture;
	SDL_Surface* temp;
	Uint32 saved_flags;
	Uint8 saved_alpha;
	int potw, poth;

	/* Make size power of two */
	potw = pot(surface->w);
	poth = pot(surface->h);
	if (rw) *rw = potw;
	if (rh) *rh = poth;

	if ((surface->w != potw) || (surface->h != poth)) { /* size isn't original */
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
				potw, poth, surface->format->BytesPerPixel*8, RGBAMASK );
		if (temp == NULL) {
			WARN("Unable to create POT surface: %s", SDL_GetError());
			return 0;
		}
		if (SDL_FillRect( temp, NULL,
				SDL_MapRGBA(surface->format,0,0,0,SDL_ALPHA_TRANSPARENT))) {
			WARN("Unable to fill rect: %s", SDL_GetError());
			return 0;
		}

		SDL_BlitSurface( surface, &rtemp, temp, &rtemp);
		SDL_FreeSurface( surface );

		surface = temp;

		/* set saved alpha */
		if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA )
			SDL_SetAlpha( surface, 0, 0 );


		/* create the temp POT surface */
		temp = SDL_CreateRGBSurface( SDL_SRCCOLORKEY,
				potw, poth, surface->format->BytesPerPixel*8, RGBAMASK );
		if (temp == NULL) {
			WARN("Unable to create POT surface: %s", SDL_GetError());
			return 0;
		}
		if (SDL_FillRect( temp, NULL, SDL_MapRGBA(surface->format,0,0,0,SDL_ALPHA_TRANSPARENT))) {
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

	glGenTextures( 1, &texture ); /* Creates the texture */
	glBindTexture( GL_TEXTURE_2D, texture ); /* Loads the texture */

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

	return texture;
}

/*
 * loads the SDL_Surface to an opengl texture
 */
glTexture* gl_loadImage( SDL_Surface* surface )
{
	int rw, rh;

	/* set up the texture defaults */
	glTexture *texture = MALLOC_ONE(glTexture);
	texture->w = (double)surface->w;
	texture->h = (double)surface->h;
	texture->sx = 1.;
	texture->sy = 1.;

	texture->texture = gl_loadSurface( surface, &rw, &rh );

	texture->rw = (double)rw;
	texture->rh = (double)rh;
	texture->sw = texture->w;
	texture->sh = texture->h;

	texture->trans = NULL;

	return texture;
}


/*
 * loads the image as an opengl texture directly
 */
glTexture*  gl_newImage( const char* path )
{
	SDL_Surface *temp, *surface;
	glTexture* t;
	uint8_t* trans = NULL;
	uint32_t filesize;
	char *buf = pack_readfile( DATA, (char*)path, &filesize );
	if (buf == NULL) {
		ERR("Loading surface from packfile");
		return NULL;
	}
	SDL_RWops *rw = SDL_RWFromMem(buf, filesize);
	temp = IMG_Load_RW( rw, 1 );
	free(buf);

	if (temp == 0) {
		ERR("'%s' could not be opened: %s", path, IMG_GetError());
		return NULL;
	}

	surface = SDL_DisplayFormatAlpha( temp ); /* sets the surface to what we use */
	if (surface == 0) {
		WARN( "Error converting image to screen format: %s", SDL_GetError() );
		return NULL;
	}

	SDL_FreeSurface(temp); /* free the temporary surface */

	if (SDL_VFlipSurface(surface)) {
		WARN( "Error flipping surface" );
		return NULL;
	}

	/* do after flipping for collision detection */
	SDL_LockSurface(surface);
	trans = SDL_MapTrans(surface);
	SDL_UnlockSurface(surface);

	t = gl_loadImage(surface);
	t->trans = trans;
	return t;
}


/*
 * Loads the texture immediately, but also sets it as a sprite
 */
glTexture* gl_newSprite( const char* path, const int sx, const int sy )
{
	glTexture* texture;
	if ((texture = gl_newImage(path)) == NULL)
		return NULL;
	texture->sx = (double)sx;
	texture->sy = (double)sy;
	texture->sw = texture->w/texture->sx;
	texture->sh = texture->h/texture->sy;
	return texture;
}


/*
 * frees the texture
 */
void gl_freeTexture( glTexture* texture )
{
	glDeleteTextures( 1, &texture->texture );
	if (texture->trans) free(texture->trans);
	free(texture);
}


/*
 * returns true if pixel at pos (x,y) is transparent
 */
int gl_isTrans( const glTexture* t, const int x, const int y )
{
	return !(t->trans[(y*(int)(t->w)+x)/8] & (1<<((y*(int)(t->w)+x)%8)));
}


/*
 * sets x and y to be the appropriate sprite for glTexture using dir
 *
 * gprof claims to be second slowest thing in the game
 */
void gl_getSpriteFromDir( int* x, int* y, const glTexture* t, const double dir )
{
	int s = (int)(dir / (2.0*M_PI / (t->sy*t->sx)));

	/* makes sure the sprite is "in range" */
	if (s > (int)(t->sy*t->sx)-1) s = s % (int)(t->sy*t->sx);

	*x = s % (int)t->sx;
	*y = s / (int)t->sy;
}



/*
 *
 * B L I T T I N G
 *
 */
/*
 * blits a sprite at pos (blits relative to player)
 */
void gl_blitSprite( const glTexture* sprite, const double bx, const double by,
		const int sx, const int sy, const glColour* c )
{
	/* don't draw if offscreen */
	if (fabs(bx-VX(*gl_camera)+gui_xoff) > gl_screen.w/2+sprite->sw/2 ||
			fabs(by-VY(*gl_camera)+gui_yoff) > gl_screen.h/2+sprite->sh/2 )
		return;

	double x,y, tx,ty;
	
	glEnable(GL_TEXTURE_2D);

	x = bx - VX(*gl_camera) - sprite->sw/2. + gui_xoff;
	y = by - VY(*gl_camera) - sprite->sh/2. + gui_yoff;

	tx = sprite->sw*(double)(sx)/sprite->rw;
	ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->rh;
	
	/* actual blitting */
	glBindTexture( GL_TEXTURE_2D, sprite->texture);
	glBegin(GL_QUADS);

		if (c==NULL) glColor4d( 1., 1., 1., 1. );
		else COLOUR(*c);

		glTexCoord2d( tx, ty);
			glVertex2d( x, y );

		glTexCoord2d( tx + sprite->sw/sprite->rw, ty);
			glVertex2d( x + sprite->sw, y );

		glTexCoord2d( tx + sprite->sw/sprite->rw, ty + sprite->sh/sprite->rh);
			glVertex2d( x + sprite->sw, y + sprite->sh );

		glTexCoord2d( tx, ty + sprite->sh/sprite->rh);
			glVertex2d( x, y + sprite->sh );

	glEnd(); /* GL_QUADS */

	glDisable(GL_TEXTURE_2D);
}


/*
 * straight out blits a texture at position
 */
void gl_blitStatic( const glTexture* texture, 
		const double bx, const double by, const glColour* c )
{
	double x,y;
	glEnable(GL_TEXTURE_2D);

	x = bx - (double)gl_screen.w/2.;
	y = by - (double)gl_screen.h/2.;

	/* actual blitting */
	glBindTexture( GL_TEXTURE_2D, texture->texture);
	glBegin(GL_QUADS);

		if (c==NULL) glColor4d( 1., 1., 1., 1. );
		else COLOUR(*c);

		glTexCoord2d( 0., 0.);
			glVertex2d( x, y );

		glTexCoord2d( texture->sw/texture->rw, 0.);
			glVertex2d( x + texture->sw, y );

		glTexCoord2d( texture->sw/texture->rw, texture->sh/texture->rh);
			glVertex2d( x + texture->sw, y + texture->sh );

		glTexCoord2d( 0., texture->sh/texture->rh);
			glVertex2d( x, y +texture->sh );

	glEnd(); /* GL_QUADS */

	glDisable(GL_TEXTURE_2D);
}


/*
 * Binds the camero to a vector
 */
void gl_bindCamera( const Vector2d* pos )
{
	gl_camera = (Vector2d*)pos;
}



/*
 *
 * G L O B A L
 *
 */
/*
 * Initializes SDL/OpenGL and the works
 */
int gl_init()
{
	int doublebuf, depth, i, j, off, toff, supported = 0;
	SDL_Rect** modes;
	int flags = SDL_OPENGL;
	flags |= SDL_FULLSCREEN * (gl_has(OPENGL_FULLSCREEN) ? 1 : 0);

	/* Initializes Video */
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		WARN("Unable to initialize SDL: %s", SDL_GetError());
		return -1;
	}

	/* get available fullscreen modes */
	if (gl_has(OPENGL_FULLSCREEN)) {
		modes = SDL_ListModes( NULL, SDL_OPENGL | SDL_FULLSCREEN );
		if (modes == NULL) { /* rare case, but could happen */
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
			for (i=0; modes[i]; i++) {
				DEBUG("  %d x %d", modes[i]->w, modes[i]->h);
				if ((flags & SDL_FULLSCREEN) && (modes[i]->w == gl_screen.w) &&
						(modes[i]->h == gl_screen.h))
					supported = 1; /* mode we asked for is supported */
			}
		}

		/* makes sure fullscreen mode is supported */
		if ((flags & SDL_FULLSCREEN) && !supported) {

			/* try to get closest aproximation to mode asked for */
			off = -1;
			j = 0;
			for (i=0; modes[i]; i++) {
				toff = ABS(gl_screen.w-modes[i]->w) + ABS(gl_screen.h-modes[i]->h);
				if ((off == -1) || (toff < off)) {
					j = i;
					off = toff;
				}
			}

			WARN("Fullscreen mode %dx%d is not supported by your setup\n"
					"   switching to %dx%d",
					gl_screen.w, gl_screen.h,
					modes[j]->w, modes[j]->h );
			gl_screen.w = modes[j]->w;
			gl_screen.h = modes[j]->h;
		}

		/* free the modes */
		for (i=0;modes[i];++i)
			free(modes[i]);
		free(modes);
	}

	
	/* test the setup */
	depth = SDL_VideoModeOK( gl_screen.w, gl_screen.h, gl_screen.depth, flags);
	if (depth != gl_screen.depth)
		WARN("Depth %d bpp unavailable, will use %d bpp", gl_screen.depth, depth);

	gl_screen.depth = depth;


	/* actually creating the screen */
	if (SDL_SetVideoMode( gl_screen.w, gl_screen.h, gl_screen.depth, flags) == NULL) {
		ERR("Unable to create OpenGL window: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	/* Get info about the OpenGL window */
	SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &gl_screen.r );
	SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &gl_screen.g );
	SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &gl_screen.b );
	SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &gl_screen.a );
	SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &doublebuf );
	if (doublebuf) gl_screen.flags |= OPENGL_DOUBLEBUF;
	gl_screen.depth = gl_screen.r + gl_screen.g + gl_screen.b + gl_screen.a;

	/* Debug happiness */
	DEBUG("OpenGL Window Created: %dx%d@%dbpp %s", gl_screen.w, gl_screen.h, gl_screen.depth,
			(gl_has(OPENGL_FULLSCREEN))?"fullscreen":"window");
	DEBUG("r: %d, g: %d, b: %d, a: %d, doublebuffer: %s",
			gl_screen.r, gl_screen.g, gl_screen.b, gl_screen.a,
			(gl_has(OPENGL_DOUBLEBUF)) ? "yes" : "no");
	DEBUG("Renderer: %s", glGetString(GL_RENDERER));

	/* some OpenGL options */
	glClearColor( 0., 0., 0., 1. );
	glDisable( GL_DEPTH_TEST ); /* set for doing 2d */
/*	glEnable(  GL_TEXTURE_2D ); never enable globally, breaks non-texture blits */
	glDisable( GL_LIGHTING ); /* no lighting, it's done when rendered */
	glEnable(  GL_BLEND );
	glShadeModel( GL_FLAT ); /* default shade model, functions should keep this when done */
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); /* alpha */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho( -SCREEN_W/2, /* left edge */
			SCREEN_W/2, /* right edge */
			-SCREEN_H/2, /* bottom edge */
			SCREEN_H/2, /* top edge */
			-1., /* near */
			1. ); /* far */

	glClear( GL_COLOR_BUFFER_BIT );


	return 0;
}


/*
 * Cleans up SDL/OpenGL, the works
 */
void gl_exit()
{
}


/*
 * saves a png
 */
int write_png( const char *file_name, png_bytep *rows,
		int w, int h, int colourtype, int bitdepth )
{

	png_structp png_ptr;
	png_infop info_ptr;
	FILE *fp = NULL;
	char *doing = "open for writing";

	if (!(fp = fopen(file_name, "wb"))) goto fail;

	doing = "create png write struct";
	if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) goto fail;

	doing = "create png info struct";
	if (!(info_ptr = png_create_info_struct(png_ptr))) goto fail;
	if (setjmp(png_jmpbuf(png_ptr))) goto fail;

	doing = "init IO";
	png_init_io(png_ptr, fp);

	doing = "write header";
	png_set_IHDR(png_ptr, info_ptr, w, h, bitdepth, colourtype, 
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
			PNG_FILTER_TYPE_BASE);

	doing = "write info";
	png_write_info(png_ptr, info_ptr);

	doing = "write image";
	png_write_image(png_ptr, rows);

	doing = "write end";
	png_write_end(png_ptr, NULL);

	doing = "closing file";
	if(0 != fclose(fp)) goto fail;

	return 0;

fail:
	WARN( "Write_png: could not %s", doing );
	return -1;
}   
