/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file opengl.c
 *
 * @brief This file handles most of the more generic opengl functions.
 *
 * The main way to work with opengl in naev is to create glTextures and then
 *  use the blit functions to draw them on the screen.  This system will
 *  handle relative and absolute positions.
 *
 * There are two coordinate systems: relative and absolute.
 *
 * Relative:
 *  * Everything is drawn relative to the player, if it doesn't fit on screen
 *    it is clipped.
 *  * Origin (0., 0.) wouldbe ontop of the player.
 *
 * Absolute:
 *  * Everything is drawn in "screen coordinates".
 *  * (0., 0.) is bottom left.
 *  * (SCREEN_W, SCREEN_H) is top right.
 *
 * Note that the game actually uses a third type of coordinates for when using
 *  raw commands.  In this third type, the (0.,0.) is actually in middle of the
 *  screen.  (-SCREEN_W/2.,-SCREEN_H/2.) is bottom left and
 *  (+SCREEN_W/2.,+SCREEN_H/2.) is top right.
 */


#include "opengl.h"

#include "naev.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h> /* va_list for gl_print */

#include <png.h>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_version.h"

#include "log.h"
#include "opengl_ext.h"
#include "ndata.h"
#include "gui.h"


/*
 * Requirements
 */
#define OPENGL_REQ_MULTITEX      2 /**< 2 is minimum OpenGL 1.2 must have */


glInfo gl_screen; /**< Gives data of current opengl settings. */
Vector2d* gl_camera  = NULL; /**< Camera we are using. */
static int gl_activated = 0; /**< Whether or not a window is activated. */

/*
 * graphic list
 */
/**
 * @brief Represents a node in the texture list.
 */
typedef struct glTexList_ {
   struct glTexList_ *next; /**< Next in linked list */
   glTexture *tex; /**< assosciated texture */
   int used; /**< counts how many times texture is being used */
} glTexList;
static glTexList* texture_list = NULL; /**< Texture list. */


/*
 * prototypes
 */
/* misc */
static int SDL_VFlipSurface( SDL_Surface* surface );
static int SDL_IsTrans( SDL_Surface* s, int x, int y );
static uint8_t* SDL_MapTrans( SDL_Surface* s );
/* glTexture */
static GLuint gl_loadSurface( SDL_Surface* surface, int *rw, int *rh );
static glTexture* gl_loadNewImage( const char* path, unsigned int flags );
static void gl_blitTexture( const glTexture* texture, 
      const double x, const double y,
      const double tx, const double ty, const glColour *c );
/* png */
int write_png( const char *file_name, png_bytep *rows,
      int w, int h, int colourtype, int bitdepth );
/* global */
static GLboolean gl_hasExt( char *name );
static GLboolean gl_hasVersion( int major, int minor );


/*
 *
 * M I S C
 *
 */
/**
 * @brief Gets the closest power of two.
 *    @param n Number to get closest power of two to.
 *    @return Closest power of two to the number.
 */
int gl_pot( int n )
{
   int i = 1;
   while (i < n)
      i <<= 1;
   return i;
}


/**
 * @brief Flips the surface vertically.
 *
 *    @param surface Surface to flip.
 *    @return 0 on success.
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


/**
 * @brief Checks to see if a position of the surface is transparent.
 *
 *    @param s Surface to check for transparency.
 *    @param x X position of the pixel to check.
 *    @param y Y position of the pixel to check.
 *    @return 0 if the pixel isn't transparent, 0 if it is.
 */
static int SDL_IsTrans( SDL_Surface* s, int x, int y )
{
   int bpp;
   Uint8 *p;
   Uint32 pixelcolour;
   
   bpp = s->format->BytesPerPixel; 
   /* here p is the address to the pixel we want to retrieve */
   p = (Uint8 *)s->pixels + y*s->pitch + x*bpp; 

   pixelcolour = 0;
   switch(bpp) {        
      case 1: 
         pixelcolour = *p; 
         break; 

      case 2: 
         pixelcolour = *(Uint16 *)p; 
         break; 

      case 3: 
#if HAS_BIGENDIAN
         pixelcolour = p[0] << 16 | p[1] << 8 | p[2]; 
#else /* HAS_BIGENDIAN */
         pixelcolour = p[0] | p[1] << 8 | p[2] << 16; 
#endif /* HAS_BIGENDIAN */
         break; 

      case 4: 
         pixelcolour = *(Uint32 *)p; 
         break; 
   } 

   /* test whether pixels colour == colour of transparent pixels for that surface */
#if SDL_VERSION_ATLEAST(1,3,0)
   return ((pixelcolour & s->format->Amask) == 0);
#else /* SDL_VERSION_ATLEAST(1,3,0) */
   return (pixelcolour == s->format->colorkey);
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
}


/**
 * @brief Maps the surface transparency.
 *
 * Basically generates a map of what pixels are transparent.  Good for pixel
 *  perfect collision routines.
 *
 *    @param s Surface to map it's transparency.
 *    @return 0 on success.
 */
static uint8_t* SDL_MapTrans( SDL_Surface* s )
{
   int i,j;
   int size;
   uint8_t *t;

   /* alloc memory for just enough bits to hold all the data we need */
   size = s->w*s->h/8 + ((s->w*s->h%8)?1:0);;
   t = malloc(size);
   if (t==NULL) {
      WARN("Out of Memory");
      return NULL;
   }
   memset(t, 0, size); /* important, must be set to zero */

   /* Check each pixel individually. */
   for (i=0; i<s->h; i++)
      for (j=0; j<s->w; j++) /* sets each bit to be 1 if not transparent or 0 if is */
         t[(i*s->w+j)/8] |= (SDL_IsTrans(s,j,i)) ? 0 : (1<<((i*s->w+j)%8));

   return t;
}


/**
 * @brief Takes a screenshot.
 *
 *    @param filename Name of the file to save screenshot as.
 */
void gl_screenshot( const char *filename )
{
   SDL_Surface *screen = SDL_GetVideoSurface();
   unsigned rowbytes = screen->w * 4;
   unsigned char screenbuf[screen->h][rowbytes], *rows[screen->h];
   int i;

   /* Read pixels from buffer -- SLOW. */
   glReadPixels( 0, 0, screen->w, screen->h,
         GL_RGBA, GL_UNSIGNED_BYTE, screenbuf );

   /* Convert data. */
   for (i = 0; i < screen->h; i++)
      rows[i] = screenbuf[screen->h - i - 1];

   /* Save PNG. */
   write_png( filename, rows, screen->w, screen->h,
         PNG_COLOR_TYPE_RGBA, 8);

   /* Check to see if an error occured. */
   gl_checkErr();
}


/**
 * @brief Saves a surface to a file as a png.
 *
 * Ruthlessly stolen from "pygame - Python Game Library"
 *    by Pete Shinners (pete@shinners.org)
 *
 *    @param surface Surface to save.
 *    @param file Path to save surface to.
 *    @return 0 on success.;
 */
int SDL_SavePNG( SDL_Surface *surface, const char *file )
{
   png_bytep* ss_rows;
   int ss_size;
   int ss_w, ss_h;
   SDL_Surface *ss_surface;
   int r, i;
   SDL_Rect rtemp;
#if ! SDL_VERSION_ATLEAST(1,3,0)
   unsigned int surf_flags;
   unsigned int surf_alpha;
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   /* Initialize parameters. */
   ss_rows     = NULL;
   ss_size     = 0;
   ss_surface  = NULL;

   /* Set size. */
   ss_w        = surface->w;
   ss_h        = surface->h;

   /* Handle alpha. */
#if SDL_VERSION_ATLEAST(1,3,0)
   SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

   /* Create base surface. */
   ss_surface = SDL_CreateRGBSurface( 0, ss_w, ss_h, 32, RGBAMASK );
#else /* SDL_VERSION_ATLEAST(1,3,0) */
   surf_flags = surface->flags & (SDL_SRCALPHA | SDL_SRCCOLORKEY);
   surf_alpha = surface->format->alpha;
   if ((surf_flags & SDL_SRCALPHA) == SDL_SRCALPHA) {
      SDL_SetAlpha( surface, 0, SDL_ALPHA_OPAQUE );
      SDL_SetColorKey( surface, 0, surface->format->colorkey );
   }

   /* Create base surface. */
   ss_surface = SDL_CreateRGBSurface( SDL_SRCCOLORKEY, ss_w, ss_h, 32, RGBAMASK );
#endif /* SDL_VERSION_ATLEAST(1,3,0) */
   if (ss_surface == NULL) {
      WARN("Unable to create RGB surface.");
      return -1;
   }
   if (SDL_FillRect( ss_surface, NULL,
            SDL_MapRGBA(surface->format,0,0,0,SDL_ALPHA_TRANSPARENT))) {
      WARN("Unable to fill rect: %s", SDL_GetError());
      return 0;
   }


   /* Blit to new surface. */
   rtemp.x = rtemp.y = 0;
   rtemp.w = surface->w;
   rtemp.h = surface->h;
   SDL_BlitSurface(surface, &rtemp, ss_surface, &rtemp);

   /* Allocate space. */
   ss_size = ss_h;
   ss_rows = malloc(sizeof(png_bytep) * ss_size);
   if (ss_rows == NULL)
      return -1;

   /* Reset flags. */
#if ! SDL_VERSION_ATLEAST(1,3,0)
   /* Set saved alpha */
   if ((surf_flags & SDL_SRCALPHA) == SDL_SRCALPHA)
      SDL_SetAlpha( surface, 0, 0 );
#endif /* ! SDL_VERSION_ATLEAST(1,3,0) */

   /* Copy pixels into data. */
   for (i = 0; i < ss_h; i++)
      ss_rows[i] = ((png_bytep)ss_surface->pixels) + i*ss_surface->pitch;

   /* Save to PNG. */
   r = write_png(file, ss_rows, surface->w, surface->h, PNG_COLOR_TYPE_RGB_ALPHA, 8);

   /* Clean up. */
   free(ss_rows);
   SDL_FreeSurface(ss_surface);

   return r;
}



/*
 *
 * G L _ T E X T U R E
 *
 */
/**
 * @brief Prepares the surface to be loaded as a texture.
 *
 *    @param surface to load that is freed in the process.
 *    @return New surface that is prepared for texture loading.
 */
SDL_Surface* gl_prepareSurface( SDL_Surface* surface )
{
   SDL_Surface* temp;
   int potw, poth;
   SDL_Rect rtemp;
#if ! SDL_VERSION_ATLEAST(1,3,0)
   Uint32 saved_flags;
   Uint8 saved_alpha;
#endif /* ! SDL_VERSION_ATLEAST(1,3,0) */

   /* Make size power of two */
   potw = gl_pot(surface->w);
   poth = gl_pot(surface->h);

   /* we must blit with an SDL_Rect */
   rtemp.x = rtemp.y = 0;
   rtemp.w = surface->w;
   rtemp.h = surface->h;

   /* saves alpha */
#if SDL_VERSION_ATLEAST(1,3,0)
   SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

   /* create the temp POT surface */
   temp = SDL_CreateRGBSurface( 0, potw, poth,
         surface->format->BytesPerPixel*8, RGBAMASK );
#else /* SDL_VERSION_ATLEAST(1,3,0) */
   saved_flags = surface->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
   saved_alpha = surface->format->alpha;
   if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA) {
      SDL_SetAlpha( surface, 0, SDL_ALPHA_OPAQUE );
      SDL_SetColorKey( surface, 0, surface->format->colorkey );
   }

   /* create the temp POT surface */
   temp = SDL_CreateRGBSurface( SDL_SRCCOLORKEY,
         potw, poth, surface->format->BytesPerPixel*8, RGBAMASK );
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   if (temp == NULL) {
      WARN("Unable to create POT surface: %s", SDL_GetError());
      return 0;
   }
   if (SDL_FillRect( temp, NULL,
            SDL_MapRGBA(surface->format,0,0,0,SDL_ALPHA_TRANSPARENT))) {
      WARN("Unable to fill rect: %s", SDL_GetError());
      return 0;
   }

   /* change the surface to the new blitted one */
   SDL_BlitSurface( surface, &rtemp, temp, &rtemp);
   SDL_FreeSurface( surface );
   surface = temp;

#if ! SDL_VERSION_ATLEAST(1,3,0)
   /* set saved alpha */
   if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA )
      SDL_SetAlpha( surface, 0, 0 );
#endif /* ! SDL_VERSION_ATLEAST(1,3,0) */

   return surface;
}

/**
 * @brief Loads a surface into an opengl texture.
 *
 *    @param surface Surface to load into a texture.
 *    @param[out] rw Real width of the texture.
 *    @param[out] rh Real height of the texture.
 *    @return The opengl texture id.
 */
static GLuint gl_loadSurface( SDL_Surface* surface, int *rw, int *rh )
{
   GLuint texture;

   /* Prepare the surface. */
   surface = gl_prepareSurface( surface );
   if (rw != NULL)
      (*rw) = surface->w;
   if (rh != NULL) 
      (*rh) = surface->h;

   /* opengl texture binding */
   glGenTextures( 1, &texture ); /* Creates the texture */
   glBindTexture( GL_TEXTURE_2D, texture ); /* Loads the texture */

   /* Filtering, LINEAR is better for scaling, nearest looks nicer, LINEAR
    * also seems to create a bit of artifacts around the edges */
   if (gl_screen.scale != 1.) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   }
   else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   }

   /* Always wrap just in case. */
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   /* now lead the texture data up */
   SDL_LockSurface( surface );
   glTexImage2D( GL_TEXTURE_2D, 0, surface->format->BytesPerPixel,
         surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
   SDL_UnlockSurface( surface );

   /* cleanup */
   SDL_FreeSurface( surface );
   gl_checkErr();

   return texture;
}

/**
 * @brief Loads the SDL_Surface to a glTexture.
 *
 *    @param surface Surface to load.
 *    @return The glTexture for surface.
 */
glTexture* gl_loadImage( SDL_Surface* surface )
{
   int rw, rh;

   /* set up the texture defaults */
   glTexture *texture = malloc(sizeof(glTexture));
   memset( texture, 0, sizeof(glTexture) );

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
   texture->name  = NULL;

   return texture;
}


/**
 * @brief Loads an image as a texture.
 *
 * May not necessarily load the image but use one if it's already open.
 *
 *    @param path Image to load.
 *    @param flags Flags to control image parameters.
 *    @return Texture loaded from image.
 */
glTexture* gl_newImage( const char* path, const unsigned int flags )
{
   glTexList *cur, *last;

   /* check to see if it already exists */
   if (texture_list != NULL) {
      for (cur=texture_list; cur!=NULL; cur=cur->next) {
         if (strcmp(path,cur->tex->name)==0) {
            cur->used += 1;
            return cur->tex;
         }
         last = cur;
      }
   }

   /* Create the new node */
   cur = malloc(sizeof(glTexList));
   cur->next = NULL;
   cur->used = 1;

   /* Load the image */
   cur->tex = gl_loadNewImage(path, flags);

   if (texture_list == NULL) /* special condition - creating new list */
      texture_list = cur;
   else
      last->next = cur;

   return cur->tex;
}


/**
 * @brief Only loads the image, does not add to stack unlike gl_newImage.
 *
 *    @param path Image to load.
 *    @param flags Flags to control image parameters.
 *    @return Texture loaded from image.
 */
static glTexture* gl_loadNewImage( const char* path, const unsigned int flags )
{
   SDL_Surface *temp, *surface;
   glTexture* t;
   uint8_t* trans;
   SDL_RWops *rw;

   /* load from packfile */
   rw = ndata_rwops( path );
   if (rw == NULL) {
      ERR("Loading surface from ndata.");
      return NULL;
   }
   temp = IMG_Load_RW( rw, 1 );

   if (temp == NULL) {
      ERR("'%s' could not be opened: %s", path, IMG_GetError());
      return NULL;
   }

   surface = SDL_DisplayFormatAlpha( temp ); /* sets the surface to what we use */
   if (surface == NULL) {
      WARN( "Error converting image to screen format: %s", SDL_GetError() );
      return NULL;
   }

   SDL_FreeSurface(temp); /* free the temporary surface */

   /* we have to flip our surfaces to match the ortho */
   if (SDL_VFlipSurface(surface)) {
      WARN( "Error flipping surface" );
      return NULL;
   }

   /* do after flipping for collision detection */
   if (flags & OPENGL_TEX_MAPTRANS) {
      SDL_LockSurface(surface);
      trans = SDL_MapTrans(surface);
      SDL_UnlockSurface(surface);
   }
   else
      trans = NULL;

   /* set the texture */
   t = gl_loadImage(surface);
   t->trans = trans;
   t->name  = strdup(path);
   return t;
}


/**
 * @brief Loads the texture immediately, but also sets it as a sprite.
 *
 *    @param path Image to load.
 *    @param sx Number of X sprites in image.
 *    @param sy Number of Y sprites in image.
 *    @param flags Flags to control image parameters.
 *    @return Texture loaded.
 */
glTexture* gl_newSprite( const char* path, const int sx, const int sy,
      const unsigned int flags )
{
   glTexture* texture;
   if ((texture = gl_newImage(path, flags)) == NULL)
      return NULL;

   /* will possibly overwrite an existing textur properties
    * so we have to load same texture always the same sprites */
   texture->sx = (double)sx;
   texture->sy = (double)sy;
   texture->sw = texture->w/texture->sx;
   texture->sh = texture->h/texture->sy;
   return texture;
}


/**
 * @brief Frees a texture.
 *
 *    @param texture Texture to free.
 */
void gl_freeTexture( glTexture* texture )
{
   glTexList *cur, *last;

   /* Shouldn't be NULL (won't segfault though) */
   if (texture == NULL) {
      WARN("Attempting to free NULL texture!");
      return;
   }

   /* see if we can find it in stack */
   last = NULL;
   for (cur=texture_list; cur!=NULL; cur=cur->next) {
      if (cur->tex == texture) { /* found it */
         cur->used--;
         if (cur->used <= 0) { /* not used anymore */
            /* free the texture */
            glDeleteTextures( 1, &texture->texture );
            if (texture->trans != NULL)
               free(texture->trans);
            if (texture->name != NULL)
               free(texture->name);
            free(texture);

            /* free the list node */
            if (last == NULL) { /* case there's no texture before it */
               if (cur->next != NULL)
                  texture_list = cur->next;
               else /* case it's the last texture */
                  texture_list = NULL;
            }
            else
               last->next = cur->next;
            free(cur);
         }
         return; /* we already found it so we can exit */
      }
      last = cur;
   }

   /* Not found */
   if (texture->name != NULL) /* Surfaces will have NULL names */
      WARN("Attempting to free texture '%s' not found in stack!", texture->name);

   /* Free anyways */
   glDeleteTextures( 1, &texture->texture );
   if (texture->trans != NULL) free(texture->trans);
   if (texture->name != NULL) free(texture->name);
   free(texture);

   gl_checkErr();
}


/**
 * @brief Duplicates a texture.
 *
 *    @param texture Texture to duplicate.
 *    @return Duplicate of texture.
 */
glTexture* gl_dupTexture( glTexture *texture )
{
   glTexList *cur, *last;

   /* No segfaults kthxbye. */
   if (texture == NULL)
      return NULL;

   /* check to see if it already exists */
   if (texture_list != NULL) {
      for (cur=texture_list; cur!=NULL; cur=cur->next) {
         if (texture == cur->tex) {
            cur->used += 1;
            return cur->tex;
         }
         last = cur;
      }
   }

   /* Invalid texture. */
   return NULL;
}


/**
 * @brief Checks to see if a pixel is transparent in a texture.
 *
 *    @param t Texture to check for transparency.
 *    @param x X position of the pixel.
 *    @param y Y position of the pixel.
 *    @return 1 if the pixel is transparent or 0 if it isn't.
 */
int gl_isTrans( const glTexture* t, const int x, const int y )
{
   int i;

   /* Get the position in the sheet. */
   i = y*(int)(t->w) + x ;
   /* Now we have to pull out the individual bit. */
   return !(t->trans[ i/8 ] & (1 << (i%8)));
}


/**
 * @brief Sets x and y to be the appropriate sprite for glTexture using dir.
 *
 * Very slow, try to cache if possible like the pilots do instead of using
 *  in O(n^2) or worse functions.
 *
 *    @param[out] x X sprite to use.
 *    @param[out] y Y sprite to use.
 *    @param t Texture to get sprite from.
 *    @param dir Direction to get sprite from.
 */
void gl_getSpriteFromDir( int* x, int* y, const glTexture* t, const double dir )
{
   int s, sx, sy;
   double shard, rdir;

#ifdef DEBUGGING
   if ((dir > 2.*M_PI) || (dir < 0.)) {
      WARN("Angle not between 0 and 2.*M_PI [%f].", dir);
      return;
   }
#endif /* DEBUGGING */

   /* what each image represents in angle */
   shard = 2.*M_PI / (t->sy*t->sx);

   /* real dir is slightly moved downwards */
   rdir = dir + shard/2.;
  
   /* now calculate the sprite we need */
   s = (int)(rdir / shard);
   sx = t->sx;
   sy = t->sy;

   /* makes sure the sprite is "in range" */
   if (s > (sy*sx-1))
      s = s % (sy*sx);

   (*x) = s % sx;
   (*y) = s / sx;
}



/*
 *
 * B L I T T I N G
 *
 */
/**
 * @brief Blits a texture.
 *
 *    @param texture Texture to blit.
 *    @param x X position of the texture on the screen.
 *    @param y Y position of the texture on the screen.
 *    @param tx X position within the texture.
 *    @param ty Y position within the texture.
 *    @param c Colour to use (modifies texture colour).
 */
static void gl_blitTexture( const glTexture* texture,
      const double x, const double y,
      const double tx, const double ty, const glColour *c )
{
   double tw,th;

   /* texture dimensions */
   tw = texture->sw / texture->rw;
   th = texture->sh / texture->rh;

   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, texture->texture);
   glBegin(GL_QUADS);
      /* set colour or default if not set */
      if (c==NULL) glColor4d( 1., 1., 1., 1. );
      else COLOUR(*c);

      glTexCoord2d( tx, ty);
      glVertex2d( x, y );

      glTexCoord2d( tx + tw, ty);
      glVertex2d( x + texture->sw, y );

      glTexCoord2d( tx + tw, ty + th);
      glVertex2d( x + texture->sw, y + texture->sh );

      glTexCoord2d( tx, ty + th);
      glVertex2d( x, y + texture->sh );
   glEnd(); /* GL_QUADS */
   glDisable(GL_TEXTURE_2D);

   /* anything failed? */
   gl_checkErr();
}
/**
 * @brief Blits a sprite, position is relative to the player.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture relative to the player.
 *    @param by Y position of the texture relative to the player.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_blitSprite( const glTexture* sprite, const double bx, const double by,
      const int sx, const int sy, const glColour* c )
{
   double x,y, tx,ty;

   /* calculate position - we'll use relative coords to player */
   x = bx - VX(*gl_camera) - sprite->sw/2. + gui_xoff;
   y = by - VY(*gl_camera) - sprite->sh/2. + gui_yoff;

   /* check if inbounds */
   if ((fabs(x) > SCREEN_W/2 + sprite->sw) ||
         (fabs(y) > SCREEN_H/2 + sprite->sh) )
      return;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->rw;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->rh;

   gl_blitTexture( sprite, x, y, tx, ty, c );
}


/**
 * @brief Blits a sprite, position is in absolute screen coordinates.
 *
 *    @param sprite Sprite to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param sx X position of the sprite to use.
 *    @param sy Y position of the sprite to use.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_blitStaticSprite( const glTexture* sprite, const double bx, const double by,
      const int sx, const int sy, const glColour* c )
{
   double x,y, tx,ty;

   x = bx - (double)SCREEN_W/2.;
   y = by - (double)SCREEN_H/2.;

   /* texture coords */
   tx = sprite->sw*(double)(sx)/sprite->rw;
   ty = sprite->sh*(sprite->sy-(double)sy-1)/sprite->rh;

   /* actual blitting */
   gl_blitTexture( sprite, x, y, tx, ty, c );
}


/**
 * @brief Blits a texture scaling it.
 *
 *    @param texture Texture to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param bw Width to scale to.
 *    @param bh Height to scale to.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_blitScale( const glTexture* texture,
      const double bx, const double by,     
      const double bw, const double bh, const glColour* c )
{
   double x,y;
   double tw,th;
   double tx,ty;

   /* here we use absolute coords */
   x = bx - (double)SCREEN_W/2.;
   y = by - (double)SCREEN_H/2.;

   /* texture dimensions */
   tw = texture->sw / texture->rw;
   th = texture->sh / texture->rh;
   tx = ty = 0.;

   glEnable(GL_TEXTURE_2D);
   glBindTexture( GL_TEXTURE_2D, texture->texture);
   glBegin(GL_QUADS);
      /* set colour or default if not set */
      if (c==NULL) glColor4d( 1., 1., 1., 1. );
      else COLOUR(*c);

      glTexCoord2d( tx, ty);
      glVertex2d( x, y );

      glTexCoord2d( tx + tw, ty);
      glVertex2d( x + bw, y );

      glTexCoord2d( tx + tw, ty + th);
      glVertex2d( x + bw, y + bh );

      glTexCoord2d( tx, ty + th);
      glVertex2d( x, y + bh );
   glEnd(); /* GL_QUADS */
   glDisable(GL_TEXTURE_2D);

   /* anything failed? */
   gl_checkErr();
}

/**
 * @brief Blits a texture to a position
 *
 *    @param texture Texture to blit.
 *    @param bx X position of the texture in screen coordinates.
 *    @param by Y position of the texture in screen coordinates.
 *    @param c Colour to use (modifies texture colour).
 */
void gl_blitStatic( const glTexture* texture, 
      const double bx, const double by, const glColour* c )
{
   double x,y;

   /* here we use absolute coords */
   x = bx - (double)SCREEN_W/2.;
   y = by - (double)SCREEN_H/2.;

   /* actual blitting */
   gl_blitTexture( texture, x, y, 0, 0, c );
}


/**
 * @brief Binds the camera to a vector.
 *
 * All stuff displayed with relative functions will be affected by the camera's
 *  position.  Does not affect stuff in screen coordinates.
 *
 *    @param pos Vector to use as camera.
 */
void gl_bindCamera( Vector2d* pos )
{
   gl_camera = pos;
}


/**
 * @brief Draws a circle.
 *
 *    @param cx X position of the center in screen coordinates..
 *    @param cy Y position of the center in screen coordinates.
 *    @param r Radius of the circle.
 */
void gl_drawCircle( const double cx, const double cy, const double r )
{
   double x,y,p;

   x = 0;
   y = r;
   p = (5. - (r*4.)) / 4.;

   glBegin(GL_POINTS);
      glVertex2d( cx,   cy+y );
      glVertex2d( cx,   cy-y );
      glVertex2d( cx+y, cy   );
      glVertex2d( cx-y, cy   );

      while (x<y) {
         x++;
         if (p < 0) p += 2*(double)(x)+1;
         else p += 2*(double)(x-(--y))+1;

         if (x==0) {
            glVertex2d( cx,   cy+y );
            glVertex2d( cx,   cy-y );
            glVertex2d( cx+y, cy   );
            glVertex2d( cx-y, cy   );
         }
         else
            if (x==y) {
               glVertex2d( cx+x, cy+y );
               glVertex2d( cx-x, cy+y );
               glVertex2d( cx+x, cy-y );
               glVertex2d( cx-x, cy-y );
            }
            else
               if (x<y) {
                  glVertex2d( cx+x, cy+y );
                  glVertex2d( cx-x, cy+y );
                  glVertex2d( cx+x, cy-y );
                  glVertex2d( cx-x, cy-y );
                  glVertex2d( cx+y, cy+x );
                  glVertex2d( cx-y, cy+x );
                  glVertex2d( cx+y, cy-x );
                  glVertex2d( cx-y, cy-x );
               }
      }
   glEnd(); /* GL_POINTS */

   gl_checkErr();
}


/**
 * @brief Only displays the pixel if it's in the screen.
 */
#define PIXEL(x,y)   \
if ((x>rx) && (y>ry) && (x<rxw) && (y<ryh))  \
   glVertex2d(x,y)
/**
 * @brief Draws a circle in a rectangle.
 *
 *    @param cx X position of the center in screen coordinates..
 *    @param cy Y position of the center in screen coordinates.
 *    @param r Radius of the circle.
 *    @param rx X position of the rectangle limiting the circle in screen coords.
 *    @param ry Y position of the rectangle limiting the circle in screen coords.
 *    @param rw Width of the limiting rectangle.
 *    @param rh Height of the limiting rectangle.
 */
void gl_drawCircleInRect( const double cx, const double cy, const double r,
      const double rx, const double ry, const double rw, const double rh )
{
   double rxw,ryh, x,y,p;

   rxw = rx+rw;
   ryh = ry+rh;

   /* is offscreen? */
   if ((cx+r < rx) || (cy+r < ry) || (cx-r > rxw) || (cy-r > ryh))
      return;
   /* can be drawn normally? */
   else if ((cx-r > rx) && (cy-r > ry) && (cx+r < rxw) && (cy+r < ryh)) {
      gl_drawCircle( cx, cy, r );
      return;
   }

   x = 0;
   y = r;    
   p = (5. - (r*4.)) / 4.;

   glBegin(GL_POINTS);
      PIXEL( cx,   cy+y );
      PIXEL( cx,   cy-y );
      PIXEL( cx+y, cy   );
      PIXEL( cx-y, cy   );

      while (x<y) {
         x++;
         if (p < 0) p += 2*(double)(x)+1;
         else p += 2*(double)(x-(--y))+1;

         if (x==0) {
            PIXEL( cx,   cy+y );
            PIXEL( cx,   cy-y );
            PIXEL( cx+y, cy   );
            PIXEL( cx-y, cy   );
         }         
         else      
            if (x==y) {
               PIXEL( cx+x, cy+y );
               PIXEL( cx-x, cy+y );
               PIXEL( cx+x, cy-y );
               PIXEL( cx-x, cy-y );
            }        
            else     
               if (x<y) {
                  PIXEL( cx+x, cy+y );
                  PIXEL( cx-x, cy+y );
                  PIXEL( cx+x, cy-y );
                  PIXEL( cx-x, cy-y );
                  PIXEL( cx+y, cy+x );
                  PIXEL( cx-y, cy+x );
                  PIXEL( cx+y, cy-x );
                  PIXEL( cx-y, cy-x );
               }
      }
   glEnd(); /* GL_POINTS */

   gl_checkErr();
}
#undef PIXEL


/*
 *
 * G L O B A L
 *
 */


/**
 * @brief Checks to see if opengl version is at least major.minor.
 *
 *    @param major Major version to check.
 *    @param minor Minor version to check.
 */
static GLboolean gl_hasVersion( int major, int minor )
{
   const char *p;
   double f, c;

   p = (const char*) glGetString(GL_VERSION);

   /* Get version and compare version. */
   f = atof(p);
   c  = (double) major;
   c += 0.1 * (double) minor;

   if (f <= c)
      return GL_TRUE;
   return GL_FALSE;
}


/**
 * @brief Checks for on opengl extension.
 *
 *    @param name Extension to check for.
 *    @return GL_TRUE if found, GL_FALSE if isn't.
 */
static GLboolean gl_hasExt( char *name )
{
   /*
    * Search for name in the extensions string.  Use of strstr()
    * is not sufficient because extension names can be prefixes of
    * other extension names.  Could use strtok() but the constant
    * string returned by glGetString can be in read-only memory.
    */
   const char *p, *end;
   size_t len, n;

   p = (const char*) glGetString(GL_EXTENSIONS);
   len = strlen(name);
   end = p + strlen(p);

   while (p < end) {
      n = strcspn(p, " ");
      if ((len == n) && (strncmp(name,p,n)==0))
         return GL_TRUE;

      p += (n + 1);
   }
   return GL_FALSE;
}


#ifndef gl_checkErr /**< i agree it's a bit hackish :) */
/**
 * @brief Checks and reports if there's been an error.
 */
void gl_checkErr (void)
{
   GLenum err;
   char* errstr;

   err = glGetError();

   if (err == GL_NO_ERROR) return; /* no error */

   switch (err) {
      case GL_INVALID_ENUM:
         errstr = "GL invalid enum";
         break;
      case GL_INVALID_VALUE:
         errstr = "GL invalid value";
         break;
      case GL_INVALID_OPERATION:
         errstr = "GL invalid operation";
         break;
      case GL_STACK_OVERFLOW:
         errstr = "GL stack overflow";
         break;
      case GL_STACK_UNDERFLOW:
         errstr = "GL stack underflow";
         break;
      case GL_OUT_OF_MEMORY:
         errstr = "GL out of memory";
         break;
      case GL_TABLE_TOO_LARGE:
         errstr = "GL table too large";
         break;

      default:
         errstr = "GL unknown error";
         break;
   }
   WARN("OpenGL error: %s",errstr);
}
#endif /* DEBUG */


/**
 * @brief Initializes opengl extensions.
 *
 *    @return 0 on success.
 */
static int gl_initExtensions (void)
{
   /* Clear values. */
   nglActiveTexture = NULL;
   nglMultiTexCoord2d = NULL;
   nglGenBuffers = NULL;
   nglBindBuffer = NULL;
   nglBufferData = NULL;
   nglBufferSubData = NULL;
   nglDeleteBuffers = NULL;

   /* Multitexture. */
   if (gl_hasExt("GL_ARB_multitexture")) {
      nglActiveTexture = SDL_GL_GetProcAddress("glActiveTexture");
      nglMultiTexCoord2d = SDL_GL_GetProcAddress("glMultiTexCoord2d");
   }
   else
      WARN("GL_ARB_multitexture not found!");

   /* Vertex Buffers. */
   if (gl_hasVersion( 1, 5)) {
      nglGenBuffers = SDL_GL_GetProcAddress("glGenBuffers");
      nglBindBuffer = SDL_GL_GetProcAddress("glBindBuffer");
      nglBufferData = SDL_GL_GetProcAddress("glBufferData");
      nglBufferSubData = SDL_GL_GetProcAddress("glBufferSubData");
      nglDeleteBuffers = SDL_GL_GetProcAddress("glDeleteBuffers");
   }
   else if (gl_hasExt("GL_ARB_vertex_buffer_object")) {
      nglGenBuffers = SDL_GL_GetProcAddress("glGenBuffersARB");
      nglBindBuffer = SDL_GL_GetProcAddress("glBindBufferARB");
      nglBufferData = SDL_GL_GetProcAddress("glBufferDataARB");
      nglBufferSubData = SDL_GL_GetProcAddress("glBufferSubDataARB");
      nglDeleteBuffers = SDL_GL_GetProcAddress("glDeleteBuffersARB");
   }
   else
      WARN("GL_ARB_vertex_buffer_object not found!");

   return 0;
}


/**
 * @brief Initializes SDL/OpenGL and the works.
 *    @return 0 on success.
 */
int gl_init (void)
{
   int doublebuf, depth, i, j, off, toff, supported, fsaa;
   SDL_Rect** modes;
   int flags;
   const SDL_VideoInfo *vidinfo;

   /* Defaults. */
   supported = 0;
   flags  = SDL_OPENGL;
   flags |= SDL_FULLSCREEN * (gl_has(OPENGL_FULLSCREEN) ? 1 : 0);

   /* Initializes Video */
   if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
      WARN("Unable to initialize SDL Video: %s", SDL_GetError());
      return -1;
   }

   /* Get the video information. */
   vidinfo = SDL_GetVideoInfo();

   /* Set opengl flags. */
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); /* Ideally want double buffering. */
   if (gl_has(OPENGL_FSAA)) {
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, gl_screen.fsaa);
   }
   if (gl_has(OPENGL_VSYNC))
#if SDL_VERSION_ATLEAST(1,3,0)
      SDL_GL_SetSwapInterval(1);
#else /* SDL_VERSION_ATLEAST(1,3,0) */
      SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
#endif /* SDL_VERSION_ATLEAST(1,3,0) */

   if (gl_has(OPENGL_FULLSCREEN)) {
      /* Try to use desktop resolution if nothing is specifically set. */
#if SDL_VERSION_ATLEAST(1,2,10)
      if (!gl_has(OPENGL_DIM_DEF)) {
         gl_screen.w = vidinfo->current_w;
         gl_screen.h = vidinfo->current_h;
      }
#endif /* SDL_VERSION_ATLEAST(1,2,10) */

      /* Get available modes and see what we can use. */
      modes = SDL_ListModes( NULL, SDL_OPENGL | SDL_FULLSCREEN );
      if (modes == NULL) { /* rare case, but could happen */
         WARN("No fullscreen modes available");
         if (flags & SDL_FULLSCREEN) {
            WARN("Disabling fullscreen mode");
            flags &= ~SDL_FULLSCREEN;
         }
      }
      else if (modes == (SDL_Rect **)-1)
         DEBUG("All fullscreen modes available");
      else {
         DEBUG("Available fullscreen modes:");
         for (i=0; modes[i]; i++) {
            DEBUG("  %d x %d", modes[i]->w, modes[i]->h);
            if ((flags & SDL_FULLSCREEN) && (modes[i]->w == SCREEN_W) &&
                  (modes[i]->h == SCREEN_H))
               supported = 1; /* mode we asked for is supported */
         }
      }
      /* makes sure fullscreen mode is supported */
      if ((flags & SDL_FULLSCREEN) && !supported) {

         /* try to get closest aproximation to mode asked for */
         off = -1;
         j = 0;
         for (i=0; modes[i]; i++) {
            toff = ABS(SCREEN_W-modes[i]->w) + ABS(SCREEN_H-modes[i]->h);
            if ((off == -1) || (toff < off)) {
               j = i;
               off = toff;
            }
         }
         WARN("Fullscreen mode %dx%d is not supported by your setup\n"
               "   switching to %dx%d",
               SCREEN_W, SCREEN_H,
               modes[j]->w, modes[j]->h );
         gl_screen.w = modes[j]->w;
         gl_screen.h = modes[j]->h;
      }
   }

   /* Check to see if trying to create above screen resolution without player
    * asking for such a large size. */
#if SDL_VERSION_ATLEAST(1,2,10)
   if (!gl_has(OPENGL_DIM_DEF)) {
      gl_screen.w = MIN(gl_screen.w, vidinfo->current_w);
      gl_screen.h = MIN(gl_screen.h, vidinfo->current_h);
   }
#endif /* SDL_VERSION_ATLEAST(1,2,10) */
   
   /* Test the setup - aim for 32. */
   gl_screen.depth = 32;
   depth = SDL_VideoModeOK( SCREEN_W, SCREEN_H, gl_screen.depth, flags);
   if (depth == 0)
      WARN("Video Mode %dx%d @ %d bpp not supported"
           "   going to try to create it anyways...",
            SCREEN_W, SCREEN_H, gl_screen.depth );
   if (depth != gl_screen.depth)
      DEBUG("Depth %d bpp unavailable, will use %d bpp", gl_screen.depth, depth);
   gl_screen.depth = depth;

   /* Actually creating the screen. */
   if (SDL_SetVideoMode( SCREEN_W, SCREEN_H, gl_screen.depth, flags)==NULL) {
      /* Try again possibly disabling FSAA. */
      if (gl_has(OPENGL_FSAA)) {
         LOG("Unable to create OpenGL window: Trying without FSAA.");
         gl_screen.flags &= ~OPENGL_FSAA;
         SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
         SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
      }
      if (SDL_SetVideoMode( SCREEN_W, SCREEN_H, gl_screen.depth, flags)==NULL) {
         ERR("Unable to create OpenGL window: %s", SDL_GetError());
         return -1;
      }
   }
   gl_screen.rw = SCREEN_W;
   gl_screen.rh = SCREEN_H;
   gl_activated = 1; /* Opengl is now activated. */

   /* Get info about the OpenGL window */
   SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &gl_screen.r );
   SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &gl_screen.g );
   SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &gl_screen.b );
   SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &gl_screen.a );
   SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &doublebuf );
   SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &fsaa );
   if (doublebuf)
      gl_screen.flags |= OPENGL_DOUBLEBUF;
   /* Calculate real depth. */
   gl_screen.depth = gl_screen.r + gl_screen.g + gl_screen.b + gl_screen.a;

   /* Get info about some extensions */
   if (gl_hasExt("GL_ARB_vertex_program")==GL_TRUE)
      gl_screen.flags |= OPENGL_VERT_SHADER;
   if (gl_hasExt("GL_ARB_fragment_program")==GL_TRUE)
      gl_screen.flags |= OPENGL_FRAG_SHADER;

   /* Texture information */
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_screen.tex_max);
   glGetIntegerv(GL_MAX_TEXTURE_UNITS, &gl_screen.multitex_max);

   /* Debug happiness */
   DEBUG("OpenGL Window Created: %dx%d@%dbpp %s", SCREEN_W, SCREEN_H, gl_screen.depth,
         gl_has(OPENGL_FULLSCREEN)?"fullscreen":"window");
   DEBUG("r: %d, g: %d, b: %d, a: %d, db: %s, fsaa: %d, tex: %d",
         gl_screen.r, gl_screen.g, gl_screen.b, gl_screen.a,
         gl_has(OPENGL_DOUBLEBUF) ? "yes" : "no",
         fsaa, gl_screen.tex_max);
   DEBUG("Renderer: %s", glGetString(GL_RENDERER));
   DEBUG("Version: %s", glGetString(GL_VERSION));
   /* Now check for things that can be bad. */
   if (gl_screen.multitex_max < OPENGL_REQ_MULTITEX)
      WARN("Missing texture units (%d required, %d found)",
            OPENGL_REQ_MULTITEX, gl_screen.multitex_max );
   if (gl_has(OPENGL_FSAA) && (fsaa != gl_screen.fsaa))
      WARN("Unable to get requested FSAA level (%d requested, got %d)",
            gl_screen.fsaa, fsaa );
   /* Initialize extensions. */
   gl_initExtensions();
   DEBUG("");

   /* Some OpenGL options. */
   glClearColor( 0., 0., 0., 1. );

   /* Set default opengl state. */
   glDisable( GL_DEPTH_TEST ); /* set for doing 2d */
/* glEnable(  GL_TEXTURE_2D ); never enable globally, breaks non-texture blits */
   glDisable( GL_LIGHTING ); /* no lighting, it's done when rendered */
   glEnable(  GL_BLEND ); /* alpha blending ftw */

   /* Set the blending/shading model to use. */
   glShadeModel( GL_FLAT ); /* default shade model, functions should keep this when done */
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); /* good blend model */

   /* New window is real window scaled. */
   gl_screen.nw = (double)gl_screen.rw * gl_screen.scale;
   gl_screen.nh = (double)gl_screen.rh * gl_screen.scale;
   /* Viewport matches new window size. */
   gl_screen.w  = gl_screen.nw;
   gl_screen.h  = gl_screen.nh;
   /* Small windows get handled here. */
   if ((SCREEN_W < 600) && (SCREEN_W <= SCREEN_H)) {
      if (gl_screen.scale != 1.)
         DEBUG("Screen size too small, upscaling...");
      gl_screen.scale = (double)gl_screen.w / 600.;
      /* Must keep the proportion the same for the screen. */
      gl_screen.h  = (gl_screen.h * 600) / SCREEN_W;
      gl_screen.nh = (gl_screen.rh * SCREEN_W) / 600;
      gl_screen.w  = 600;
   }
   else if ((SCREEN_H < 600) && (SCREEN_W >= SCREEN_H)) {
      if (gl_screen.scale != 1.)
         DEBUG("Screen size too small, upscaling...");
      gl_screen.scale = (double)gl_screen.h / 600.;
      /* Must keep the proportion the same for the screen. */
      gl_screen.w  = (gl_screen.w * 600) / SCREEN_H;
      gl_screen.nw = (gl_screen.rw * SCREEN_H) / 600;
      gl_screen.h  = 600;
   }
   /* Set scale factors. */
   gl_screen.wscale  = (double)gl_screen.nw / (double)gl_screen.w;
   gl_screen.hscale  = (double)gl_screen.nh / (double)gl_screen.h;
   gl_screen.mxscale = (double)gl_screen.w / (double)gl_screen.rw;
   gl_screen.myscale = (double)gl_screen.h / (double)gl_screen.rh;
   /* Handle setting the default viewport. */
   gl_defViewport();

   /* Finishing touches. */
   glClear( GL_COLOR_BUFFER_BIT ); /* must clear the buffer first */
   gl_checkErr();

   return 0;
}


/**
 * @brief Sets the scale factor.
 *
 *    @param scalefactor Factor to scale by.
 */
double gl_setScale( double scalefactor )
{
   gl_screen.scale = 1./scalefactor;

   /* Return actual used scalefactor. */
   return gl_screen.scale;
}


/**
 * @brief Resets viewport to default
 */
void gl_defViewport (void)
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho( -(double)gl_screen.nw/2, /* left edge */
         (double)gl_screen.nw/2, /* right edge */
         -(double)gl_screen.nh/2, /* bottom edge */
         (double)gl_screen.nh/2, /* top edge */
         -1., /* near */
         1. ); /* far */
   /* Take into account posible scaling. */
   if (gl_screen.scale != 1.)
      glScaled( gl_screen.wscale, gl_screen.hscale, 1. );
}


/**
 * @brief Cleans up OpenGL, the works.
 */
void gl_exit (void)
{
   glTexList *tex;

   /* Make sure there's no texture leak */
   if (texture_list != NULL) {
      DEBUG("Texture leak detected!");
      for (tex=texture_list; tex!=NULL; tex=tex->next)
         DEBUG("   '%s' opened %d times", tex->tex->name, tex->used );
   }

   /* Shut down the subsystem */
   SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


/**
 * @brief Saves a png.
 *
 *    @param file_name Name of the file to save the png as.
 *    @param rows Rows containing the data.
 *    @param w Width of the png.
 *    @param h Height of the png.
 *    @param colourtype Colour type of the png.
 *    @param bitdepth Bit depth of the png.
 *    @return 0 on success.
 */
int write_png( const char *file_name, png_bytep *rows,
      int w, int h, int colourtype, int bitdepth )
{
   png_structp png_ptr;
   png_infop info_ptr;
   FILE *fp;

   /* Open file for writing. */
   if (!(fp = fopen(file_name, "wb"))) {
      WARN("Unable to open '%s' for writing.", file_name);
      return -1;
   }

   /* Create working structs. */
   if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
      WARN("Unable to create png write struct.");
      return -1;
   }
   if (!(info_ptr = png_create_info_struct(png_ptr))) {
      WARN("Unable to create PNG info struct.");
      return -1;
   }

   /* Set image details. */
   png_init_io(png_ptr, fp);
   png_set_compression_level(png_ptr, Z_DEFAULT_COMPRESSION);
   png_set_IHDR(png_ptr, info_ptr, w, h, bitdepth, colourtype, 
         PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
         PNG_FILTER_TYPE_DEFAULT);

   /* Write image. */
   png_write_info(png_ptr, info_ptr);
   png_write_image(png_ptr, rows);
   png_write_end(png_ptr, NULL);

   /* Clean up. */
   fclose(fp);

   return 0;

}

