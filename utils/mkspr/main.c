

#include <stdlib.h>

#include <png.h>

#include "SDL.h"
#include "SDL_image.h"


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



/* logging macros */
#define LOG(str, args...)	\
		(fprintf(stdout,str"\n", ## args))
#define WARN(str, args...)	\
		(fprintf(stderr,"Warning: "str"\n", ## args))
#define ERR(str, args...)	\
		{fprintf(stderr,"ERROR %s:%d: "str"\n", \
		__FILE__, __LINE__, ## args); return EXIT_FAILURE;}



#define WS	6
#define HS	6


/*
 * prototypes
 */
static int SavePNG( SDL_Surface *surface, const char *file);
static int write_png( const char *file_name, png_bytep *rows, int w, int h,
      int colortype, int bitdepth );


/*
 * main
 */
int main(int argc, char* argv[])
{
	int i, ws, hs;
	char file[8];
	SDL_Surface *final, *temp, **load;
	SDL_Rect r;

	/* init variables */
	r.w = r.h = 0;

	if (argc == 2) {
		ws = hs = atoi(argv[1]);
	} else if (argc == 3) {
		ws = atoi(argv[1]);
		hs = atoi(argv[2]);
	} else {
		ws = WS;
		hs = HS;
	}
	load = NULL;
	temp = NULL;
	final = NULL;
	
	/* Init SDL */
	if (SDL_Init(SDL_INIT_VIDEO)) ERR("Initializing SDL: %s", SDL_GetError());
	/* Create the window */
	/* temp = SDL_SetVideoMode( 320, 240, 0, SDL_NOFRAME ); */
	/* if (temp == NULL) ERR("Initializing Video Surface: %s", SDL_GetError()); */

	/* open ram for the images */
	load = malloc(sizeof(SDL_Surface*)*(ws*hs));
	if (load == NULL) ERR("Out of RAM");

	/* load all the images to ram */
	for (i=0; i<(ws*hs); i++) {
		/* names will be 000.png, 001.png, ..., 035.png, ... etc... */
		sprintf( file, "%d%d%d.png", (unsigned char)i/100, (unsigned char)(i%100)/10, (unsigned char)i%10 );

		/* load the image properly */
		temp = IMG_Load( file );
		if (temp == NULL) ERR("Problem loading file '%s': %s", file, IMG_GetError());
		SDL_SetSurfaceBlendMode(temp, SDL_BLENDMODE_NONE);
		load[i] = temp;

		/* check to see if size has changed */
		if (r.w==0 && r.h==0) {
			r.w = load[i]->w;
			r.h = load[i]->h;
		}
		else if ((r.w != load[i]->w) || (r.h != load[i]->h))
			ERR("File '%s' isn't of the same dimensions as the files before!", file);

		/* create the surface if it hasn't been created yet */
		if (!final) {
			final = SDL_CreateRGBSurface( 0, ws*r.w, hs*r.h,
					load[i]->format->BitsPerPixel, RGBAMASK );
			if (!final)
            ERR("Problem creating RGB Surface: %s", SDL_GetError());
		}

		/* new position */
		r.y = r.h * (i/ws);
		r.x = r.w * (i%ws);
		if (SDL_BlitSurface( load[i], NULL, final, &r ))
			ERR("Problem blitting surface '%s' to final surface: %s", file, SDL_GetError());

		SDL_FreeSurface(load[i]);
	}

	/* draw the result and clean up */
	SavePNG( final, "sprite.png" );
	SDL_FreeSurface(final);
	free(load);

	SDL_Quit();

	return EXIT_SUCCESS;
}






/* -------------------------------------------------------------------------- *\
   Ruthlessly stolen from "pygame - Python Game Library"
     by Pete Shinners (pete@shinners.org)
\* -------------------------------------------------------------------------- */
static int SavePNG( SDL_Surface *surface, const char *file)
{
	static unsigned char** ss_rows;
	static int ss_size;
	static int ss_w, ss_h;
	SDL_Surface *ss_surface;
	SDL_Rect ss_rect;
	int r, i;
	int alpha = 0;
	int pixel_bits = 32;

	ss_rows = 0;
	ss_size = 0;
	ss_surface = 0;

	ss_w = surface->w;
	ss_h = surface->h;

	if (surface->format->Amask) {
		alpha = 1;
		pixel_bits = 32;
	} else {
		pixel_bits = 24;
	}

	ss_surface = SDL_CreateRGBSurface( 0, ss_w, ss_h,
			pixel_bits, RGBAMASK );

	if( ss_surface == 0 ) {
		return -1;
	}

	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

	ss_rect.x = 0;
	ss_rect.y = 0;
	ss_rect.w = ss_w;
	ss_rect.h = ss_h;
	SDL_BlitSurface(surface, &ss_rect, ss_surface, 0);

	if ( ss_size == 0 ) {
		ss_size = ss_h;
		ss_rows = (unsigned char**)malloc(sizeof(unsigned char*) * ss_size);
		if( ss_rows == 0 ) {
			return -1;
		}
	}
	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

	for (i = 0; i < ss_h; i++) {
		ss_rows[i] = ((unsigned char*)ss_surface->pixels) + i * ss_surface->pitch;
	}

	if (alpha) {
		r = write_png(file, ss_rows, surface->w, surface->h, PNG_COLOR_TYPE_RGB_ALPHA, 8);
	} else {
		r = write_png(file, ss_rows, surface->w, surface->h, PNG_COLOR_TYPE_RGB, 8);
	}

	free(ss_rows);
	SDL_FreeSurface(ss_surface);
	ss_surface = NULL;

	return r;
}

static void warning( const char *msg )
{
	WARN( "Write_png: could not %s", msg );
}

static int write_png( const char *file_name, png_bytep *rows, int w, int h,
		int colortype, int bitdepth )
{

	png_structp png_ptr;
	png_infop info_ptr;
	FILE *fp = NULL;

	if (!(fp = fopen(file_name, "wb"))) {
      warning( "open for writing" );
      return -1;
   }

	if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
      warning( "create png write struct" );
      return -1;
   }

	if (!(info_ptr = png_create_info_struct(png_ptr)) || setjmp(png_jmpbuf(png_ptr))) {
      warning( "create png info struct" );
      return -1;
   }

	/*doing = "init IO";*/
	png_init_io(png_ptr, fp);

	/*doing = "write header";*/
	png_set_IHDR(png_ptr, info_ptr, w, h, bitdepth, colortype, 
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, 
			PNG_FILTER_TYPE_BASE);

	/*doing = "write info";*/
	png_write_info(png_ptr, info_ptr);

	/*doing = "write image";*/
	png_write_image(png_ptr, rows);

	/*doing = "write end";*/
	png_write_end(png_ptr, NULL);

	if (0 != fclose(fp)) {
      warning( "closing file" );
      return -1;
   }

	return 0;
}


