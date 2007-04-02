

#include "space.h"

#include "log.h"
#include "physics.h"
#include "opengl.h"
#include "rng.h"
#include "pilot.h"


#define	STAR_LAYERS	3
static gl_texture* starBG[STAR_LAYERS];
static Vector2d starPos[STAR_LAYERS];


/*
 * prototypes
 */
static gl_texture* starBG_create( const int density );
static void put_pixel( SDL_Surface* surface, const int x, const int y,
		const Uint8 R, const Uint8 G, const Uint8 B );


/*
 * modifies the pixel at x,y of surface to be of color R G B
 */
static void put_pixel( SDL_Surface* surface, const int x, const int y,
		const Uint8 R, const Uint8 G, const Uint8 B )
{
	Uint32  color = SDL_MapRGB(surface->format, R, G, B);

	Uint8 *bufp8;
	Uint16 *bufp16;
	Uint32 *bufp32;

	switch ( surface->format->BytesPerPixel ) {
		case 1: // 8 bpp
			bufp8 = (Uint8 *)surface->pixels + y*surface->pitch + x;
			*bufp8 = color;
			break;
		case 2: // 15 or 16 bpp
			bufp16 = (Uint16 *)surface->pixels + 
				y*surface->pitch/2 + x;
			*bufp16 = color;
			break;
		case 3: // 24 bpp, SLOW
			bufp8 = (Uint8 *)surface->pixels + y*surface->pitch + x;
			*(bufp8+surface->format->Rshift/8) = R;
			*(bufp8+surface->format->Gshift/8) = G;
			*(bufp8+surface->format->Bshift/8) = B;
			break;
		case 4: // 32 bpp
			bufp32 = (Uint32 *)surface->pixels +  
				y*surface->pitch/4 + x;
			*bufp32 = color;
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
	int i, b, d, x, y;

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

	d = (int)((FP)(density)*(FP)(gl_screen.w)*(FP)(gl_screen.h)/1000./1000.);
	for (i=0; i < d; i++) {
		b = RNG(50,255);
		x = RNG(0,w-1);
		y = RNG(0,h-1);
		put_pixel(surface, x, y,b,b,b);
	}

	return gl_loadImage(surface);
}


void space_init (void)
{
	int i;
	for (i=0; i < STAR_LAYERS; i++) {
		starBG[i] = starBG_create(1000);
		starPos[i].x = 0.;
		starPos[i].y = 0.;
	}
}


void space_render( FP dt )
{
	int i;
	Vector2d temp = { .x = starPos[0].x };

	for (i=0; i < STAR_LAYERS; i++) {
		/* movement */
		starPos[i].x -= player->solid->vel.x/(FP)(2*i+4)*dt;
		starPos[i].y -= player->solid->vel.y/(FP)(2*i+4)*dt;

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

		/*
		 * TODO OPTIMIZATION
		 */
		temp.x -= starBG[i]->w;
		gl_blitStatic( starBG[i], &temp );
		temp.y += starBG[i]->h;
		gl_blitStatic( starBG[i], &temp );
		temp.x += starBG[i]->w;
		gl_blitStatic( starBG[i], &temp );
		temp.x += starBG[i]->w;
		gl_blitStatic( starBG[i], &temp );
		temp.y -= starBG[i]->h;
		gl_blitStatic( starBG[i], &temp );
		temp.y -= starBG[i]->h;
		gl_blitStatic( starBG[i], &temp );
		temp.x -= starBG[i]->w;
		gl_blitStatic( starBG[i], &temp );
		temp.x -= starBG[i]->w;
		gl_blitStatic( starBG[i], &temp );

		/*if (starPos[i].x < starBG[i]->w/4.)
			temp.x = starPos[i].x + starBG[i]->w;
		else if (starPos[i].x < starBG[i]->w*3./4.)
			temp.x = starPos[i].x - starBG[i]->w;

		if (starPos[i].y < starBG[i]->h/4.)
			temp.y = starPos[i].y + starBG[i]->h;
		else if (starPos[i].y < starBG[i]->h*3./4.)
			temp.y = starPos[i].y - starBG[i]->h;

		gl_blitStatic( starBG[i], &temp );*/
	}
}


void space_exit (void)
{
	int i;
	for (i=0; i < STAR_LAYERS; i++)
		gl_free(starBG[i]);
}

