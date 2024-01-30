/*
 * See Licensing and Copyright notice in naev.h
 */
/**
 * @file opengl_tex.c
 *
 * @brief This file handles the opengl texture wrapper routines.
 */
/** @cond */
#include <stdio.h>
#include <stdlib.h>
#include "physfsrwops.h"
#include "SDL_image.h"

#include "naev.h"
/** @endcond */

#include "distance_field.h"
#include "array.h"
#include "conf.h"
#include "gui.h"
#include "log.h"
#include "md5.h"
#include "nfile.h"
#include "nstring.h"
#include "opengl.h"

/*
 * graphic list
 */
/**
 * @brief Represents a node in the texture list.
 */
typedef struct glTexList_ {
   glTexture *tex; /**< associated texture */
   const char *path; /**< Path pointer, stored in tex. */
   int used; /**< counts how many times texture is being used */
   /* TODO We currently treat images with different number of sprites as
    * different images, i.e., they get reloaded and use more memory. However,
    * it should be possible to do something fancier and share the texture to
    * avoid this increase of memory (without sharing other parameters). */
   int sx; /**< X sprites */
   int sy; /**< Y sprites */
} glTexList;
static glTexList* texture_list = NULL; /**< Texture list. */
static SDL_threadID tex_mainthread;
static SDL_mutex* tex_lock = NULL;

/*
 * prototypes
 */
/* misc */
static uint8_t SDL_GetAlpha( SDL_Surface* s, int x, int y );
static int SDL_IsTrans( SDL_Surface* s, int x, int y );
static uint8_t* SDL_MapAlpha( SDL_Surface* s, int w, int h, int tight );
static size_t gl_transSize( const int w, const int h );
/* glTexture */
static GLuint gl_texParameters( unsigned int flags );
static GLuint gl_loadSurface( SDL_Surface* surface, unsigned int flags, int freesur, double *vmax );
static glTexture* gl_loadNewImage( const char* path, unsigned int flags );
static glTexture* gl_loadNewImageRWops( const char *path, SDL_RWops *rw, unsigned int flags );
/* List. */
static glTexture* gl_texExists( const char* path, int sx, int sy );
static int gl_texAdd( glTexture *tex, int sx, int sy );
static int tex_cmp( const void *p1, const void *p2 );

static void tex_ctxSet (void)
{
   if (SDL_ThreadID() != tex_mainthread)
      SDL_GL_MakeCurrent( gl_screen.window, gl_screen.context );
}

static void tex_ctxUnset (void)
{
   if (SDL_ThreadID() != tex_mainthread)
      SDL_GL_MakeCurrent( gl_screen.window, NULL );
}

void gl_contextSet (void)
{
   SDL_mutexP( tex_lock );
   tex_ctxSet();
}

void gl_contextUnset (void)
{
   tex_ctxUnset();
   SDL_mutexV( tex_lock );
}

static int tex_cmp( const void *p1, const void *p2 )
{
   const glTexList *t1 = (const glTexList*) p1;
   const glTexList *t2 = (const glTexList*) p2;
   int ret = strcmp( t1->path, t2->path );
   if (ret != 0)
      return ret;
   ret = t1->sx - t2->sx;
   if (ret != 0)
      return ret;
   return t1->sy - t2->sy;
}

/**
 * @brief Gets the alpha value of a pixel.
 *
 *    @param s Surface to get value from.
 *    @param x X position of the pixel to check.
 *    @param y Y position of the pixel to check.
 *    @return Alpha value of the pixel.
 */
static uint8_t SDL_GetAlpha( SDL_Surface* s, int x, int y )
{
   size_t bytes_per_pixel = s->format->BytesPerPixel;
   void *p;
   Uint32 pixel;
   Uint8 r, g, b, a;

   SDL_LockSurface(s);
   p = (Uint8 *)s->pixels + y * s->pitch + x * bytes_per_pixel;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
   SDL_memcpy(((Uint8 *) &pixel) + (sizeof(pixel) - bytes_per_pixel), p, bytes_per_pixel);
#else /* SDL_BYTEORDER == SDL_BIG_ENDIAN */
   SDL_memcpy(&pixel, p, bytes_per_pixel);
#endif /* SDL_BYTEORDER == SDL_BIG_ENDIAN */
   SDL_GetRGBA(pixel, s->format, &r, &g, &b, &a);
   SDL_UnlockSurface(s);
   return a;
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
   uint8_t a = SDL_GetAlpha( s, x, y );
   /* Test whether pixels colour == colour of transparent pixels for that surface */

   return a > 127;
}

/**
 * @brief Maps the surface transparency.
 *
 * Basically generates a map of what pixels are transparent.  Good for pixel
 *  perfect collision routines.
 *
 *    @param s Surface to map it's transparency.
 *    @param w Width to map.
 *    @param h Height to map.
 *    @param tight Whether or not to store transparency per bit or
 *    @return 0 on success.
 */
static uint8_t* SDL_MapAlpha( SDL_Surface* s, int w, int h, int tight )
{
   uint8_t *t;

   /* Get limit.s */
   if (w < 0)
      w = s->w;
   if (h < 0)
      h = s->h;

   if (tight) {
      /* alloc memory for just enough bits to hold all the data we need */
      size_t size = gl_transSize(w, h);
      t = malloc(size);
      if (t==NULL) {
         WARN(_("Out of Memory"));
         return NULL;
      }
      memset(t, 0, size); /* important, must be set to zero */

      /* Check each pixel individually. */
      for (int i=0; i<h; i++)
         for (int j=0; j<w; j++) /* sets each bit to be 1 if not transparent or 0 if is */
            t[(i*w+j)/8] |= (SDL_IsTrans(s,j,i)) ? 0 : (1<<((i*w+j)%8));
   }
   else {
      t = malloc( w*h );
      /* Check each pixel individually. */
      for (int i=0; i<h; i++)
         for (int j=0; j<w; j++) /* sets each bit to be 1 if not transparent or 0 if is */
            t[i*w+j] = SDL_GetAlpha(s,j,i); /* Flipped with tight version, this is not good :/ */
   }

   return t;
}

/*
 * @brief Gets the size needed for a transparency map.
 *
 *    @param w Width of the image.
 *    @param h Height of the image.
 *    @return The size in bytes.
 */
static size_t gl_transSize( const int w, const int h )
{
   /* One bit per pixel, plus remainder. */
   return w*h/8 + ((w*h%8)?1:0);
}

/**
 * @brief Sets default texture parameters.
 */
static GLuint gl_texParameters( unsigned int flags )
{
   GLuint texture;

   /* opengl texture binding */
   glGenTextures( 1, &texture ); /* Creates the texture */
   glBindTexture( GL_TEXTURE_2D, texture ); /* Loads the texture */

   /* Filtering, LINEAR is better for scaling, nearest looks nicer, LINEAR
    * also seems to create a bit of artifacts around the edges */
   if ((gl_screen.scale != 1.) || (flags & OPENGL_TEX_MIPMAPS)) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      if (flags & OPENGL_TEX_MIPMAPS)
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      else
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   }
   else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   }

   /* Always wrap just in case. */
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   /* Check errors. */
   gl_checkErr();

   return texture;
}

/**
 * @brief Creates a framebuffer and its associated texture.
 *
 *    @param[out] fbo Framebuffer object id.
 *    @param[out] tex Texture id.
 *    @param width Width to use.
 *    @param height Height to use.
 *    @return 0 on success.
 */
int gl_fboCreate( GLuint *fbo, GLuint *tex, GLsizei width, GLsizei height )
{
   GLenum status;

   SDL_mutexP( tex_lock );
   //tex_ctxSet();

   /* Create the render buffer. */
   glGenTextures(1, tex);
   glBindTexture(GL_TEXTURE_2D, *tex);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
   glBindTexture(GL_TEXTURE_2D, 0);

   /* Create the frame buffer. */
   glGenFramebuffers( 1, fbo );
   glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

   /* Attach the colour buffer. */
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tex, 0);

   /* Check status. */
   status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (status != GL_FRAMEBUFFER_COMPLETE)
      WARN(_("Error setting up framebuffer!"));

   /* Restore state. */
   glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);

   //tex_ctxUnset();
   SDL_mutexV( tex_lock );

   gl_checkErr();

   return (status==GL_FRAMEBUFFER_COMPLETE);
}

/**
 * @brief Adds a depth attachment to an FBO.
 */
int gl_fboAddDepth( GLuint fbo, GLuint *tex, GLsizei width, GLsizei height )
{
   GLenum status;

   SDL_mutexP( tex_lock );
   //tex_ctxSet();

   /* Create the render buffer. */
   glGenTextures(1, tex);
   glBindTexture(GL_TEXTURE_2D, *tex);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
   glBindTexture(GL_TEXTURE_2D, 0);

   /* Attach the depth. */
   glBindFramebuffer(GL_FRAMEBUFFER, fbo);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *tex, 0);

   /* Check status. */
   status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (status != GL_FRAMEBUFFER_COMPLETE)
      WARN(_("Error attaching depth to framebuffer!"));

   /* Restore state. */
   glBindFramebuffer(GL_FRAMEBUFFER, gl_screen.current_fbo);

   //tex_ctxUnset();
   SDL_mutexV( tex_lock );

   gl_checkErr();

   return (status==GL_FRAMEBUFFER_COMPLETE);
}

glTexture* gl_loadImageData( float *data, int w, int h, int sx, int sy, const char* name )
{
   SDL_mutexP( tex_lock );
   tex_ctxSet();

   /* Set up the texture defaults */
   glTexture *texture = calloc( 1, sizeof(glTexture) );

   texture->w     = (double) w;
   texture->h     = (double) h;
   texture->sx    = (double) sx;
   texture->sy    = (double) sy;

   /* Set up texture. */
   texture->texture = gl_texParameters( 0 );

   /* Copy over. */
   glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, w, h, 0, GL_RGBA, GL_FLOAT, data );
   glBindTexture( GL_TEXTURE_2D, 0 );

   /* Check errors. */
   gl_checkErr();

   /* Set up values. */
   texture->sw    = texture->w / texture->sx;
   texture->sh    = texture->h / texture->sy;
   texture->srw   = texture->sw / texture->w;
   texture->srh   = texture->sh / texture->h;

   /* Add to list. */
   if (name != NULL) {
      texture->name = strdup(name);
      gl_texAdd( texture, sx, sy );
   }

   tex_ctxUnset();
   SDL_mutexV( tex_lock );

   return texture;
}

/**
 * @brief Loads a surface into an opengl texture.
 *
 *    @param surface Surface to load into a texture.
 *    @param flags Flags to use.
 *    @param freesur Whether or not to free the surface.
 *    @param[out] vmax The maximum value in the case of an SDF texture.
 *    @return The opengl texture id.
 */
static GLuint gl_loadSurface( SDL_Surface* surface, unsigned int flags, int freesur, double *vmax )
{
   const SDL_PixelFormatEnum fmt = SDL_PIXELFORMAT_ABGR8888;
   GLuint texture;
   SDL_Surface *rgba;

   SDL_mutexP( tex_lock );
   tex_ctxSet();

   /* Get texture. */
   texture = gl_texParameters( flags );

   /* Now load the texture data up
    * It doesn't work with indexed ones, so I guess converting is best bet. */
   if (surface->format->format != fmt)
      rgba = SDL_ConvertSurfaceFormat( surface, fmt, 0 );
   else
      rgba = surface;

   SDL_LockSurface(rgba);
   if (flags & OPENGL_TEX_SDF) {
      float border[] = { 0., 0., 0., 0. };
      uint8_t *trans = SDL_MapAlpha( rgba, rgba->w, rgba->h, 0 );
      GLfloat *dataf = make_distance_mapbf( trans, rgba->w, rgba->h, vmax );
      free( trans );
      glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, rgba->w, rgba->h, 0, GL_RED, GL_FLOAT, dataf );
      glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
      free( dataf );
   }
   else {
      *vmax = 1.;
      glPixelStorei( GL_UNPACK_ALIGNMENT, MIN( rgba->pitch&-rgba->pitch, 8 ) );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB_ALPHA,
            rgba->w, rgba->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels );
      glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
   }
   SDL_UnlockSurface(rgba);
   if (rgba != surface)
      SDL_FreeSurface(rgba);

   /* Create mipmaps. */
   if (flags & OPENGL_TEX_MIPMAPS) {
      /* Do fancy stuff. */
      if (GLAD_GL_ARB_texture_filter_anisotropic) {
         GLfloat param;
         glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &param);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, param);
      }

      /* Now generate the mipmaps. */
      glGenerateMipmap(GL_TEXTURE_2D);
   }

   /* Unbind the texture. */
   glBindTexture(GL_TEXTURE_2D, 0);

   /* cleanup */
   if (freesur)
      SDL_FreeSurface( surface );
   gl_checkErr();

   tex_ctxUnset();
   SDL_mutexV( tex_lock );

   return texture;
}

/**
 * @brief Wrapper for gl_loadImagePad that includes transparency mapping.
 *
 *    @param name Name to load with.
 *    @param surface Surface to load.
 *    @param rw RWops containing data to hash.
 *    @param flags Flags to use.
 *    @param w Non-padded width.
 *    @param h Non-padded height.
 *    @param sx X sprites.
 *    @param sy Y sprites.
 *    @param freesur Whether or not to free the surface.
 *    @return The glTexture for surface.
 */
glTexture* gl_loadImagePadTrans( const char *name, SDL_Surface* surface, SDL_RWops *rw,
      unsigned int flags, int w, int h, int sx, int sy, int freesur )
{
   glTexture *texture = NULL;
   size_t filesize, cachesize;
   uint8_t *trans;
   char *cachefile;
   char digest[33];

   SDL_mutexP( tex_lock );

   if ((name != NULL) && !(flags & OPENGL_TEX_SKIPCACHE)) {
      texture = gl_texExists( name, sx, sy );
      if ((texture != NULL) && (texture->trans != NULL)) {
         if (freesur)
            SDL_FreeSurface( surface );
         SDL_mutexV( tex_lock );
         return texture;
      }
   }

   if (flags & OPENGL_TEX_MAPTRANS)
      flags ^= OPENGL_TEX_MAPTRANS;

   /* Appropriate size for the transparency map, see SDL_MapAlpha */
   cachesize = gl_transSize(w, h);

   cachefile = NULL;
   trans     = NULL;

   if (rw != NULL) {
      size_t pngsize;
      md5_state_t md5;
      char *data;
      md5_byte_t *md5val = malloc(16);
      md5_init(&md5);

      pngsize = SDL_RWseek( rw, 0, SEEK_END );
      SDL_RWseek( rw, 0, SEEK_SET );

      data = malloc(pngsize);
      if (data == NULL)
         WARN(_("Out of Memory"));
      else {
         SDL_RWread( rw, data, pngsize, 1 );
         md5_append( &md5, (md5_byte_t*)data, pngsize );
         free(data);
      }
      md5_finish( &md5, md5val );

      for (int i=0; i<16; i++)
         snprintf( &digest[i * 2], 3, "%02x", md5val[i] );
      free(md5val);

      SDL_asprintf( &cachefile, "%scollisions/%s",
         nfile_cachePath(), digest );

      /* Attempt to find a cached transparency map. */
      if (nfile_fileExists(cachefile)) {
         trans = (uint8_t*)nfile_readFile( &filesize, cachefile );

         /* Consider cached data invalid if the length doesn't match. */
         if (trans != NULL && cachesize != (unsigned int)filesize) {
            free(trans);
            trans = NULL;
         }
         /* Cached data matches, no need to overwrite. */
         else {
            free(cachefile);
            cachefile = NULL;
         }
      }
   }
   else {
      /* We could hash raw pixel data here, but that's slower than just
       * generating the map from scratch. */
      WARN(_("Texture '%s' has no RWops"), name);
   }

   if (trans == NULL) {
      SDL_LockSurface(surface);
      trans = SDL_MapAlpha( surface, w, h, 1 );
      SDL_UnlockSurface(surface);

      if (cachefile != NULL) {
         /* Cache newly-generated transparency map. */
         char dirpath[PATH_MAX];
         snprintf( dirpath, sizeof(dirpath), "%s/%s", nfile_cachePath(), "collisions/" );
         nfile_dirMakeExist( dirpath );
         nfile_writeFile( (char*)trans, cachesize, cachefile );
         free(cachefile);
      }
   }

   if (texture == NULL)
      texture = gl_loadImagePad( name, surface, flags, w, h, sx, sy, freesur );
   else if (freesur)
      SDL_FreeSurface( surface );
   texture->trans = trans;
   SDL_mutexV( tex_lock );
   return texture;
}

/**
 * @brief Loads the already padded SDL_Surface to a glTexture.
 *
 *    @param name Name to load with.
 *    @param surface Surface to load.
 *    @param flags Flags to use.
 *    @param w Non-padded width.
 *    @param h Non-padded height.
 *    @param sx X sprites.
 *    @param sy Y sprites.
 *    @param freesur Whether or not to free the surface.
 *    @return The glTexture for surface.
 */
glTexture* gl_loadImagePad( const char *name, SDL_Surface* surface,
      unsigned int flags, int w, int h, int sx, int sy, int freesur )
{
   glTexture *texture;
   SDL_mutexP( tex_lock );

   /* Make sure doesn't already exist. */
   if ((name != NULL) && !(flags & OPENGL_TEX_SKIPCACHE)) {
      texture = gl_texExists( name, sx, sy );
      if (texture != NULL) {
         SDL_mutexV( tex_lock );
         return texture;
      }
   }

   if (flags & OPENGL_TEX_MAPTRANS) {
      texture = gl_loadImagePadTrans( name, surface, NULL, flags, w, h,
            sx, sy, freesur );
      SDL_mutexV( tex_lock );
      return texture;
   }

   /* set up the texture defaults */
   texture = calloc( 1, sizeof(glTexture) );

   texture->w     = (double) w;
   texture->h     = (double) h;
   texture->sx    = (double) sx;
   texture->sy    = (double) sy;

   texture->texture = gl_loadSurface( surface, flags, freesur, &texture->vmax );

   texture->sw    = texture->w / texture->sx;
   texture->sh    = texture->h / texture->sy;
   texture->srw   = texture->sw / texture->w;
   texture->srh   = texture->sh / texture->h;
   texture->flags = flags;

   if (name != NULL) {
      texture->name = strdup(name);
      gl_texAdd( texture, sx, sy );
   }
   else
      texture->name = NULL;

   SDL_mutexV( tex_lock );
   return texture;
}

/**
 * @brief Loads the SDL_Surface to a glTexture.
 *
 *    @param surface Surface to load.
 *    @param flags Flags to use.
 *    @return The glTexture for surface.
 */
glTexture* gl_loadImage( SDL_Surface* surface, unsigned int flags )
{
   return gl_loadImagePad( NULL, surface, flags, surface->w, surface->h, 1, 1, 1 );
}

/**
 * @brief Check to see if a texture matching a path already exists.
 *
 * Note this increments the used counter if it exists.
 *
 *    @param path Path to the texture.
 *    @param sx X sprites.
 *    @param sy Y sprites.
 *    @return The texture, or NULL if none was found.
 */
static glTexture* gl_texExists( const char* path, int sx, int sy )
{
   /* Null does never exist. */
   if (path==NULL) {
      return NULL;
   }
   SDL_mutexP( tex_lock );

   /* Check to see if it already exists */
   if (texture_list == NULL) {
      SDL_mutexV( tex_lock );
      return NULL;
   }

   /* Do some fancy binary search. */
   const glTexList q = { .path=path, .sx=sx, .sy=sy };
   glTexList *t = bsearch( &q, texture_list, array_size(texture_list), sizeof(glTexList), tex_cmp );
   if (t==NULL) {
      SDL_mutexV( tex_lock );
      return NULL;
   }

   /* Use new texture. */
   t->used++;
   SDL_mutexV( tex_lock );
   return t->tex;
}

/**
 * @brief Adds a texture to the list under the name of path.
 */
static int gl_texAdd( glTexture *tex, int sx, int sy )
{
   glTexList *new;

   /* Get the new list element. */
   if (texture_list == NULL)
      texture_list = array_create( glTexList );

   /* Create the new node */
   new = &array_grow( &texture_list );
   new->used = 1;
   new->tex  = tex;
   new->sx   = sx;
   new->sy   = sy;
   new->path = tex->name;

   /* Sort the list. */
   qsort( texture_list, array_size(texture_list), sizeof(glTexList), tex_cmp );
   return 0;
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
   /* Check if it already exists. */
   if (!(flags & OPENGL_TEX_SKIPCACHE)) {
      glTexture *t = gl_texExists( path, 1, 1 );
      if (t != NULL)
         return t;
   }

   /* Load the image */
   return gl_loadNewImage( path, flags );
}

/**
 * @brief Loads an image as a texture.
 *
 * May not necessarily load the image but use one if it's already open.
 *
 * @note Does not close the SDL_RWops file.
 *
 *    @param path Path name used for checking cache and error reporting.
 *    @param rw SDL_RWops structure to load from.
 *    @param flags Flags to control image parameters.
 *    @return Texture loaded from image.
 */
glTexture* gl_newImageRWops( const char* path, SDL_RWops *rw, const unsigned int flags )
{
   /* Check if it already exists. */
   if (!(flags & OPENGL_TEX_SKIPCACHE)) {
      glTexture *t = gl_texExists( path, 1, 1 );
      if (t != NULL)
         return t;
   }

   /* Load the image */
   return gl_loadNewImageRWops( path, rw, flags );
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
   glTexture *texture;
   SDL_RWops *rw;

   if (path==NULL) {
      WARN(_("Trying to load image from NULL path."));
      return NULL;
   }

   /* Load from packfile */
   rw = PHYSFSRWOPS_openRead( path );
   if (rw == NULL) {
      WARN(_("Failed to load surface '%s' from ndata."), path);
      return NULL;
   }

   texture = gl_loadNewImageRWops( path, rw, flags );

   SDL_RWclose( rw );
   return texture;
}

/**
 * @brief Only loads the image, does not add to stack unlike gl_newImage.
 *
 *    @param path Only used for debugging. Can be set to NULL.
 *    @param rw SDL_Rwops structure to use to load.
 *    @param flags Flags to control image parameters.
 *    @return Texture loaded from image.
 */
static glTexture* gl_loadNewImageRWops( const char *path, SDL_RWops *rw, unsigned int flags )
{
   SDL_Surface *surface;
   glTexture *tex;

   /* Placeholder for warnings. */
   if (path==NULL)
      path = _("unknown");

   surface = IMG_Load_RW( rw, 0 );

   flags  |= OPENGL_TEX_VFLIP;
   if (surface == NULL) {
      WARN(_("Unable to load image '%s'."), path );
      return NULL;
   }

   if (surface == NULL) {
      WARN(_("'%s' could not be opened"), path );
      return NULL;
   }

   SDL_mutexP( tex_lock );
   if (flags & OPENGL_TEX_MAPTRANS)
      tex = gl_loadImagePadTrans( path, surface, rw, flags, surface->w, surface->h, 1, 1, 1 );
   else
      tex = gl_loadImagePad( path, surface, flags, surface->w, surface->h, 1, 1, 1 );
   SDL_mutexV( tex_lock );
   return tex;
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

   /* Check if it already exists. */
   if (!(flags & OPENGL_TEX_SKIPCACHE)) {
      texture = gl_texExists( path, sx, sy );
      if (texture != NULL)
         return texture;
   }

   /* Create new image. */
   texture = gl_newImage( path, flags | OPENGL_TEX_SKIPCACHE );
   if (texture == NULL)
      return NULL;

   /* will possibly overwrite an existing texture properties
    * so we have to load same texture always the same sprites */
   texture->sx    = (double) sx;
   texture->sy    = (double) sy;
   texture->sw    = texture->w / texture->sx;
   texture->sh    = texture->h / texture->sy;
   texture->srw   = texture->sw / texture->w;
   texture->srh   = texture->sh / texture->h;
   return texture;
}

/**
 * @brief Loads the texture immediately, but also sets it as a sprite.
 *
 *    @param path Image name for deduplication.
 *    @param rw SDL_RWops structure to load for.
 *    @param sx Number of X sprites in image.
 *    @param sy Number of Y sprites in image.
 *    @param flags Flags to control image parameters.
 *    @return Texture loaded.
 */
glTexture* gl_newSpriteRWops( const char* path, SDL_RWops *rw,
   const int sx, const int sy, const unsigned int flags )
{
   glTexture* texture;

   /* Check if it already exists. */
   if (!(flags & OPENGL_TEX_SKIPCACHE)) {
      texture = gl_texExists( path, sx, sy );
      if (texture != NULL)
         return texture;
   }

   /* Create new image. */
   texture = gl_newImageRWops( path, rw, flags | OPENGL_TEX_SKIPCACHE );
   if (texture == NULL)
      return NULL;

   /* will possibly overwrite an existing texture properties
    * so we have to load same texture always the same sprites */
   texture->sx    = (double) sx;
   texture->sy    = (double) sy;
   texture->sw    = texture->w / texture->sx;
   texture->sh    = texture->h / texture->sy;
   texture->srw   = texture->sw / texture->w;
   texture->srh   = texture->sh / texture->h;
   return texture;
}

/**
 * @brief Frees a texture.
 *
 *    @param texture Texture to free. (If NULL, function does nothing.)
 */
void gl_freeTexture( glTexture *texture )
{
   if (texture == NULL)
      return;

   SDL_mutexP( tex_lock );

   /* see if we can find it in stack */
   for (int i=0; i<array_size(texture_list); i++) {
      glTexList *cur = &texture_list[i];

      if (cur->tex!=texture)
         continue;

      /* found it */
      cur->used--;
      if (cur->used <= 0) { /* not used anymore */
         /* free the texture */
         glDeleteTextures( 1, &texture->texture );
         free(texture->trans);
         free(texture->name);
         free(texture);

         /* free the list node */
         array_erase( &texture_list, &texture_list[i], &texture_list[i+1] );
      }
      SDL_mutexV( tex_lock );
      return; /* we already found it so we can exit */
   }

   /* Not found */
   if (texture->name != NULL) /* Surfaces will have NULL names */
      WARN(_("Attempting to free texture '%s' not found in stack!"), texture->name);

   /* Have to set context. */
   tex_ctxSet();

   /* Free anyways */
   glDeleteTextures( 1, &texture->texture );
   free(texture->trans);
   free(texture->name);
   free(texture);

   gl_checkErr();

   tex_ctxUnset();
   SDL_mutexV( tex_lock );
}

/**
 * @brief Duplicates a texture.
 *
 *    @param texture Texture to duplicate.
 *    @return Duplicate of texture.
 */
glTexture* gl_dupTexture( const glTexture *texture )
{
   /* No segfaults kthxbye. */
   if (texture == NULL)
      return NULL;

   /* check to see if it already exists */
   SDL_mutexP( tex_lock );
   for (int i=0; i<array_size(texture_list); i++) {
      glTexList *cur = &texture_list[i];
      if (texture == cur->tex) {
         cur->used++;
         SDL_mutexV( tex_lock );
         return cur->tex;
      }
   }
   SDL_mutexV( tex_lock );

   /* Invalid texture. */
   WARN(_("Unable to duplicate texture '%s'."), texture->name);
   return NULL;
}

/**
 * @brief Creates a texture from a raw opengl index.
 */
USE_RESULT glTexture* gl_rawTexture( const char *name, GLuint texid, double w, double h )
{
   glTexture *texture;
   SDL_mutexP( tex_lock );

   /* set up the texture defaults */
   texture = calloc( 1, sizeof(glTexture) );

   texture->w     = (double) w;
   texture->h     = (double) h;
   texture->sx    = (double) 1.;
   texture->sy    = (double) 1.;

   texture->texture = texid;

   texture->sw    = texture->w / texture->sx;
   texture->sh    = texture->h / texture->sy;
   texture->srw   = texture->sw / texture->w;
   texture->srh   = texture->sh / texture->h;
   texture->flags = 0;

   if (name != NULL) {
      texture->name = strdup(name);
      gl_texAdd( texture, 1, 1 );
   }
   else
      texture->name = NULL;

   SDL_mutexV( tex_lock );
   return texture;
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
   /* Get the position in the sheet. */
   int i = y*(int)(t->w) + x ;
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
 *    @param sx Number of sprites in X direction.
 *    @param sy Number of sprites in Y direction.
 *    @param dir Direction to get sprite from.
 */
void gl_getSpriteFromDir( int* x, int* y, int sx, int sy, double dir )
{
   int s;
   double shard, rdir;

#ifdef DEBUGGING
   if ((dir > 2.*M_PI) || (dir < 0.)) {
      WARN(_("Angle not between 0 and 2.*M_PI [%f]."), dir);
      *x = *y = 0;
      return;
   }
#endif /* DEBUGGING */

   /* what each image represents in angle */
   shard = 2.*M_PI / (sy*sx);

   /* real dir is slightly moved downwards */
   rdir = dir + shard/2.;

   /* now calculate the sprite we need */
   s = (int)(rdir / shard);

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
   tex_lock = SDL_CreateMutex();
   tex_mainthread = SDL_ThreadID();
   return 0;
}

/**
 * @brief Cleans up the opengl texture subsystem.
 */
void gl_exitTextures (void)
{
   if (array_size(texture_list) <= 0) {
      array_free(texture_list);
      return;
   }

   /* Make sure there's no texture leak */
#if DEBUGGING
   DEBUG(_("Texture leak detected!"));
   for (int i=0; i<array_size(texture_list); i++) {
      const glTexList *cur = &texture_list[i];
      DEBUG( n_( "   '%s' opened %d time", "   '%s' opened %d times", cur->used ), cur->tex->name, cur->used );
   }
#endif /* DEBUGGING */

   array_free(texture_list);

   SDL_DestroyMutex( tex_lock );
}

/**
 * @brief Copy a texture array.
 */
glTexture** gl_copyTexArray( glTexture **tex )
{
   glTexture **t;
   int n = array_size(tex);

   if (n <= 0)
      return NULL;

   t = array_create_size( glTexture*, n );
   for (int i=0; i<array_size(tex); i++)
      array_push_back( &t, gl_dupTexture( tex[i] ) );
   return t;
}

/**
 * @brief Adds an element to a texture array.
 */
glTexture** gl_addTexArray( glTexture **tex, glTexture *t )
{
   if (tex==NULL)
      tex = array_create_size( glTexture*, 1 );
   array_push_back( &tex, t );
   return tex;
}
