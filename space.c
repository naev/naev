

#include "space.h"

#include <malloc.h>

#include "all.h"
#include "log.h"
#include "physics.h"
#include "opengl.h"
#include "rng.h"
#include "pilot.h"


#define	STAR_LAYERS	1
static gl_texture* starBG[STAR_LAYERS];
static Vector2d starPos[STAR_LAYERS];


#define STAR_BUF	100	/* area to leave around screen */
typedef struct {
	Vector2d pos;
	Uint8 brightness;
} Star;
Star *stars;
int nstars;


/*
 * prototypes
 */
static gl_texture* starBG_create( const int density );
static void put_pixel( SDL_Surface* surface, const int x, const int y,
		const Uint8 R, const Uint8 G, const Uint8 B, const Uint8 A );


/*
 * modifies the pixel at x,y of surface to be of color R G B
 */
static void put_pixel( SDL_Surface* surface, const int x, const int y,
		const Uint8 R, const Uint8 G, const Uint8 B, const Uint8 A )
{
	Uint32 pixel = SDL_MapRGBA(surface->format, R, G, B, A);
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
		case 1:
			*p = pixel;
			break;

		case 2:
			*(Uint16 *)p = pixel;
			break;

		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				p[0] = (pixel >> 16) & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = pixel & 0xff;
			} else {
				p[0] = pixel & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = (pixel >> 16) & 0xff;
			}
			break;

		case 4:
			*(Uint32 *)p = pixel;
			break;
	}

}


/*
 * creates a background of stars to use
 * background consists of four tiles together
 */
static gl_texture* starBG_create( const int density )
{
	SDL_Surface *surface;
	int w, h;
	int i, d;

	w = (int)((float)gl_screen.w*1.5);
	if ((w & (w - 1)) != 0) {
		w = 1;
		while (w < (int)((float)gl_screen.w*1.5))
			w <<= 1;
	};
	h = (int)((float)gl_screen.h*1.5);
	if ((h & (h - 1)) != 0) {
		h = 1;
		while (h < (int)((float)gl_screen.h*1.5))
			h <<= 1;
	};


	surface = SDL_CreateRGBSurface( SDL_SRCCOLORKEY,
			w, h, SDL_GetVideoSurface()->format->BytesPerPixel*8, RGBAMASK );

	if (surface == NULL) {
		WARN("Unable to create RGB surface");
		return NULL;
	}

	SDL_LockSurface(surface);

	d = (int)((double)(density)*(double)(gl_screen.w)*(double)(gl_screen.h)/1000./1000.);
	for (i=0; i < d; i++)
		put_pixel(surface,RNG(0,w-1),RNG(0,h-1),255,255,255,RNG(50,255));

	SDL_UnlockSurface(surface);

	return gl_loadImage(surface);
}


void space_init (void)
{
	int i;
	nstars = 2000;
	stars = malloc(sizeof(Star)*nstars);
	for (i=0; i < nstars; i++) {
		stars[i].brightness = RNG( 50, 255 );
		stars[i].pos.x = RNG( STAR_BUF, gl_screen.w + STAR_BUF );
		stars[i].pos.y = RNG( -STAR_BUF, gl_screen.h + STAR_BUF );
	}
#if 0
	for (i=0; i < STAR_LAYERS; i++) {
		starBG[i] = starBG_create(1000);
		starPos[i].x = 0.;
		starPos[i].y = 0.;
	}
#endif
}


void space_render( double dt )
{
	int i;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
//		glTranslated( -(double)gl_screen.w/2., -(double)gl_screen.h/2., 0.);
	glBegin( GL_POINTS );
	for (i=0; i < nstars; i++) {
		glColor4ub( 255, 255, 255, stars[i].brightness );
		glVertex2d( stars[i].pos.x, stars[i].pos.y );
	}
	glEnd();
	glPopMatrix();
#if 0
	int i;
	Vector2d temp;
	double f;

	for (i=0; i < STAR_LAYERS; i++) {
		/* movement */
		starPos[i].x -= player->solid->vel.x/(double)(2*i+10)*dt;
		starPos[i].y -= player->solid->vel.y/(double)(2*i+10)*dt;

		/* displaces X if reaches edge */
		if (starPos[i].x > 0)
			starPos[i].x -= starBG[i]->w;
		else if (starPos[i].x < -starBG[i]->w)
			starPos[i].x += starBG[i]->w;

		/* displaces Y if reaches edge */
		if (starPos[i].y > 0)
			starPos[i].y -= starBG[i]->h;
		else if (starPos[i].y < -starBG[i]->h)
			starPos[i].y += starBG[i]->h;

		/* primary blit */
		gl_blitStatic( starBG[i], &starPos[i] );

		temp.x = starPos[i].x;
		temp.y = starPos[i].y;

		/* more blits if part of the screen is blank */
		if (starPos[i].x < starBG[i]->w/4.)
			temp.x += starBG[i]->w;
		else if (starPos[i].x < starBG[i]->w*3./4.)
			temp.x -= starBG[i]->w;

		if (starPos[i].y < starBG[i]->h/4.)
			temp.y += starBG[i]->h;
		else if (starPos[i].y < starBG[i]->h*3./4.)
			temp.y -= starBG[i]->h;
		
		if (temp.x != starPos[i].x && temp.y != starPos[i].y) {
			gl_blitStatic( starBG[i], &temp );
			f = temp.x;
			temp.x = starPos[i].x;
			gl_blitStatic( starBG[i], &temp );
			temp.x = f;
			temp.y = starPos[i].y;
			gl_blitStatic( starBG[i], &temp );
		}
		else if (temp.x != starPos[i].x || temp.y != starPos[i].y)
			gl_blitStatic( starBG[i], &temp );
	}
#endif
}


void space_exit (void)
{
	free(stars);
#if 0
	int i;
	for (i=0; i < STAR_LAYERS; i++)
		gl_freeTexture(starBG[i]);
#endif
}

