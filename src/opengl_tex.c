/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file opengl_tex.c
 *
 * @brief This file handles the opengl texture wrapper routines.
 */


#include "opengl.h"

#include "naev.h"

#include "SDL_image.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "ndata.h"
#include "gui.h"


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


/**
 * @brief Initializes the opengl texture subsystem.
 *
 *    @return 0 on success.
 */
int gl_initTextures (void)
{
   return 0;
}


/**
 * @brief Cleans up the opengl texture subsystem.
 */
void gl_exitTextures (void)
{
}

