

#include "opengl.h"

#include <math.h>
#include <stdarg.h> /* va_list for gl_print */
#include <string.h>

#include "SDL.h"
#include "SDL_image.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

#include "all.h"
#include "log.h"


#define	SCREEN_W	gl_screen.w
#define	SCREEN_H gl_screen.h



/* the screen info, gives data of current opengl settings */
gl_info gl_screen;

/* the camera */
Vector2d* gl_camera;

/* default font */
gl_font gl_defFont;


/*
 * prototypes
 */
/* misc */
static int _flipSurface( SDL_Surface* surface );
static int _pot( int n );
/* gl_texture */
static GLuint gl_loadSurface( SDL_Surface* surface, int *rw, int *rh );
/* gl_font */
static void gl_fontMakeDList( FT_Face face, char ch, GLuint list_base, GLuint *tex_base );



/*
 *
 * M I S C
 *
 */
/*
 * gets the closest power of two
 */
static int _pot( int n )
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
static int _flipSurface( SDL_Surface* surface )
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
	potw = _pot(surface->w);
	poth = _pot(surface->h);
	if (rw) *rw = potw;
	if (rh) *rh = poth;

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
gl_texture* gl_loadImage( SDL_Surface* surface )
{
	int rw, rh;

	/* set up the texture defaults */
	gl_texture *texture = MALLOC_ONE(gl_texture);
	texture->w = (double)surface->w;
	texture->h = (double)surface->h;
	texture->sx = 1.;
	texture->sy = 1.;

	texture->texture = gl_loadSurface( surface, &rw, &rh );

	texture->rw = (double)rw;
	texture->rh = (double)rh;
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

	if (_flipSurface(surface)) {
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
	texture->sx = (double)sx;
	texture->sy = (double)sy;
	texture->sw = texture->w/texture->sx;
	texture->sh = texture->h/texture->sy;
	return texture;
}


/*
 * frees the texture
 */
void gl_freeTexture( gl_texture* texture )
{
	glDeleteTextures( 1, &texture->texture );
	free(texture);
}



/*
 *
 * B L I T T I N G
 *
 */
/*
 * blits a sprite at pos
 */
void gl_blitSprite( const gl_texture* sprite, const Vector2d* pos, const int sx, const int sy )
{
	/* don't draw if offscreen */
	if (fabs(pos->x-gl_camera->x) > gl_screen.w/2+sprite->sw/2 ||
			fabs(pos->y-gl_camera->y) > gl_screen.h/2+sprite->sh/2 )
		return;
	
	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
		glTranslated( sprite->sw*(double)(sx)/sprite->rw,
				sprite->sh*(sprite->sy-(double)sy-1)/sprite->rh, 0. );

	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); /* projection translation matrix */
		glTranslated( pos->x - gl_camera->x - sprite->sw/2.,
				pos->y - gl_camera->y - sprite->sh/2., 0.);
		glScaled( (double)gl_screen.w/SCREEN_W, (double)gl_screen.h/SCREEN_H, 0. );

	/* actual blitting */
	glBindTexture( GL_TEXTURE_2D, sprite->texture);
	glBegin(GL_TRIANGLE_STRIP);
		glColor4d( 1., 1., 1., 1. );
		glTexCoord2d( 0., 0.);
			glVertex2d( 0., 0. );
		glTexCoord2d( sprite->sw/sprite->rw, 0.);
			glVertex2d( sprite->sw, 0. );
		glTexCoord2d( 0., sprite->sh/sprite->rh);
			glVertex2d( 0., sprite->sh );
		glTexCoord2d( sprite->sw/sprite->rw, sprite->sh/sprite->rh);
			glVertex2d( sprite->sw, sprite->sh );
	glEnd();

	glPopMatrix(); /* projection translation matrix */

	glMatrixMode(GL_TEXTURE);
	glPopMatrix(); /* sprite translation matrix */

	glDisable(GL_TEXTURE_2D);
}


/*
 * straight out blits a texture at position
 */
void gl_blitStatic( const gl_texture* texture, const Vector2d* pos )
{
	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); /* set up translation matrix */
		glTranslated( pos->x - (double)gl_screen.w/2., pos->y - (double)gl_screen.h/2., 0);
		glScaled( (double)gl_screen.w/SCREEN_W, (double)gl_screen.h/SCREEN_H, 0. );

	/* actual blitting */
	glBindTexture( GL_TEXTURE_2D, texture->texture);
	glBegin( GL_TRIANGLE_STRIP );
		glColor4ub( 1., 1., 1., 1. );
		glTexCoord2d( 0., 0.);
			glVertex2d( 0., 0. );
		glTexCoord2d( texture->w/texture->rw, 0.);
			glVertex2d( texture->w, 0. );
		glTexCoord2d( 0., texture->h/texture->rh);
			glVertex2d( 0., texture->h );
		glTexCoord2d( texture->w/texture->rw, texture->h/texture->rh);
			glVertex2d( texture->w, texture->h );
	glEnd();

	glPopMatrix(); /* pop the translation matrix */

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
 * prints text on screen like printf
 *
 * defaults ft_font to gl_defFont if NULL
 */
void gl_print( const gl_font *ft_font, const Vector2d *pos, const char *fmt, ...)
{
	/*float h = ft_font->h / .63;*/ /* slightly increase fontsize */
	char text[256]; /* holds the string */
	va_list ap;

	if (ft_font == NULL) ft_font = &gl_defFont;

	if (fmt == NULL) return;
	else { /* convert the symbols to text */
		va_start(ap, fmt);
		vsprintf(text, fmt, ap);
		va_end(ap);
	}

	glEnable(GL_TEXTURE_2D);

	glListBase(ft_font->list_base);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); /* translation matrix */
		glTranslated( pos->x - (double)gl_screen.w/2., pos->y - (double)gl_screen.h/2., 0);

	glColor4d( 1., 1., 1., 1. );
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, &text);

	glPopMatrix(); /* translation matrx */

	glDisable(GL_TEXTURE_2D);
}


/*
 *
 * G L _ F O N T
 *
 */
/*
 * basically taken from NeHe lesson 43
 * http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=43
 */
static void gl_fontMakeDList( FT_Face face, char ch, GLuint list_base, GLuint *tex_base )
{
	FT_Glyph glyph;
	FT_Bitmap bitmap;
	GLubyte* expanded_data;
	int w,h;
	int i,j;

	if (FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT ))
		WARN("FT_Load_Glyph failed");

	if (FT_Get_Glyph( face->glyph, &glyph ))
		WARN("FT_Ge_Glyph failed");

	/* converting our glyph to a bitmap */
	FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

	bitmap = bitmap_glyph->bitmap; /* to simplify */

	/* need the POT wrapping for opengl */
	w = _pot(bitmap.width);
	h = _pot(bitmap.rows);

	/* memory for textured data
	 * bitmap is using two channels, one for luminosity and one for alpha */
	expanded_data = (GLubyte*) malloc(sizeof(GLubyte)*2* w*h);
	for (j=0; j < h; j++) {
		for (i=0; i < w; i++ ) {
			expanded_data[2*(i+j*w)]= expanded_data[2*(i+j*w)+1] = 
				(i>=bitmap.width || j>=bitmap.rows) ?
				0 : bitmap.buffer[i + bitmap.width*j];
		}
	}

	/* creating the opengl texture */
	glBindTexture( GL_TEXTURE_2D, tex_base[(int)ch]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );

	free(expanded_data); /* no use for this anymore */

	/* creating of the display list */
	glNewList(list_base+ch,GL_COMPILE);

	/* corrects a spacing flaw between letters and
	 * downwards correction for letters like g or y */
	glPushMatrix();
		glTranslated(bitmap_glyph->left,bitmap_glyph->top-bitmap.rows,0);


	/* take into account opengl POT wrapping */
	double x = (double)bitmap.width/(double)w;
	double y = (double)bitmap.rows/(double)h;

	/* draw the texture mapped QUAD */
	glBindTexture(GL_TEXTURE_2D,tex_base[(int)ch]);
	glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2d(0,0);
			glVertex2d(0,bitmap.rows);
		glTexCoord2d(x,0);
			glVertex2d(bitmap.width,bitmap.rows);
		glTexCoord2d(0,y);
			glVertex2d(0,0);
		glTexCoord2d(x,y);
			glVertex2d(bitmap.width,0);
	glEnd();

	glPopMatrix();
	glTranslated(face->glyph->advance.x >> 6 ,0,0);

	/* end of display list */
	glEndList();
}
void gl_fontInit( gl_font* font, const char *fname, unsigned int h )
{
	if (font == NULL) font = &gl_defFont;

	font->textures = malloc(sizeof(GLuint)*128);
	font->h = h;

	/* create a FreeType font library */
	FT_Library library;
	if (FT_Init_FreeType(&library)) 
		WARN("FT_Init_FreeType failed");

	/* object which freetype uses to store font info */
	FT_Face face;
	if (FT_New_Face( library, fname, 0, &face ))
		WARN("FT_New_Face failed loading library from %s", fname );

	/* FreeType is cool and measures using 1/64 of a pixel, therefore expand */
	FT_Set_Char_Size( face, h << 6, h << 6, 96, 96);

	/* have OpenGL allocate space for the textures / display list */
	font->list_base = glGenLists(128);
	glGenTextures( 128, font->textures );


	/* create each of the font display lists */
	unsigned char i;
	for (i=0; i<128; i++)
		gl_fontMakeDList( face, i, font->list_base, font->textures );

	/* we can now free the face and library */
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}
void gl_freeFont( gl_font* font )
{
	if (font == NULL) font = &gl_defFont;
	glDeleteLists(font->list_base,128);
	glDeleteTextures(128,font->textures);
	free(font->textures);
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
	int depth, i, supported = 0;
	SDL_Rect** modes;
	int flags = SDL_OPENGL;
	flags |= SDL_FULLSCREEN * gl_screen.fullscreen;

	/* Initializes Video */
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		WARN("Unable to initialize SDL: %s", SDL_GetError());
		return -1;
	}

	/* get available fullscreen modes */
	if (gl_screen.fullscreen) {
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
	glClearColor( 0., 0., 0., 1. );
	glDisable(GL_DEPTH_TEST); /* set for doing 2d */
//	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING); /* no lighting, it's done when rendered */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho( -SCREEN_W/2, /* left edge */
			SCREEN_W/2, /* right edge */
			-SCREEN_H/2, /* bottom edge */
			SCREEN_H/2, /* top edge */
			-1., /* near */
			1. ); /* far */
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); /* alpha */
	glEnable( GL_BLEND );

/*	glPointSize(1.);*/ /* default is 1. */

	glClear( GL_COLOR_BUFFER_BIT );


	return 0;
}


/*
 * Cleans up SDL/OpenGL, the works
 */
void gl_exit()
{
	SDL_Quit();
}
