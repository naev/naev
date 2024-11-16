#include "gltf.h"

#include "SDL_image.h"
#include "glad.h"
#include <libgen.h>
#include <math.h>

#include "physfsrwops.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#ifdef HAVE_NAEV
#include "conf.h"
#include "naev.h"
#include "nfile.h"
#include "opengl_shader.h"
#else /* HAVE_NAEV */
#include "common.h"
#include "shader_min.h"
#define gl_contextSet()
#define gl_contextUnset()
#endif /* HAVE_NAEV */
#include "array.h"
#include "mat3.h"
#include "vec3.h"

#define SHADOWMAP_SIZE_LOW 128  /**< Size of the shadow map. */
#define SHADOWMAP_SIZE_HIGH 512 /**< High resolution shadowmap size. */

/* Horrible hack that turns a variable name into a string. */
#define STR_HELPER( x ) #x
#define STR( x ) STR_HELPER( x )

/**
 * @brief Simple caching for GLTF objects to avoid double loads.
 */
typedef struct ObjectCache {
   char       *name;     /**< Original file name for loading. */
   GltfObject *obj;      /**< Associated object. */
   int         refcount; /**< Reference counting. */
} ObjectCache;
static ObjectCache *obj_cache  = NULL;
static SDL_mutex   *cache_lock = NULL;

static Material material_default;

/**
 * @brief Simple point light model for shaders.
 */
typedef struct ShaderLight_ {
   GLuint Hshadow;       /* mat4 */
   GLuint sun;           /* bool */
   GLuint position;      /* vec3 */
   GLuint colour;        /* vec3 */
   GLuint intensity;     /* float */
   GLuint shadowmap_tex; /* sampler2D */
} ShaderLight;

/*
 * EV:Nova
 * -> key light is top left, slight elevation, pointed at ship. directional
 * light
 * -> 1 to 3 fill lights. Depends on ship. Point lights.
 */
const Lighting L_default_const = {
   .ambient_r = 0.,
   .ambient_g = 0.,
   .ambient_b = 0.,
   .intensity = 1.,
   .nlights   = 2,
   .lights =
      {
         {
            /* Key Light. */
            .colour = { .v = { 1., 1., 1. } },
            // Endless Sky (point) power: 150, pos: -12.339, 10.559, -11.787
            /*
             */
            .sun       = 0,
            .pos       = { .v = { -3., 2.75, -3. } },
            .intensity = 80.,
            // Sharky (directional) power: 5, direction: 10.75, -12.272, 7.463
            /*
            .sun = 1,
            .pos = { .v = { 12., 10.5, -12. } },
            .intensity = 2.5,
             */
         },
         {
            /* Fill light. */
            .colour = { .v = { 1., 1., 1. } },
            // Endless Sky (directional) power: 1.5,
            // direction: 9.772, 11.602, 6.988
            /*
             */
            .sun       = 1,
            .pos       = { .v = { 10., 11.5, 7. } },
            .intensity = 1.,
            // Sharky (point) power: 2000., position: -12.339, 10.559, 11.787
            /*
            .sun = 0,
            .pos = { .v = { -12.5, 10.5, 12. } },
            .intensity = 2000.,
             */
         },
         { 0 },
      },
};
const Lighting L_store_const = {
   .ambient_r = 0.1,
   .ambient_g = 0.1,
   .ambient_b = 0.1,
   .nlights   = 2,
   .intensity = 1.5,
   .lights =
      {
         {
            /* Key Light. */
            .colour    = { .v = { 1., 1., 1. } },
            .sun       = 0,
            .pos       = { .v = { -3., 2.75, -3. } },
            .intensity = 100.,
         },
         {
            /* Fill light. */
            .colour    = { .v = { 1., 1., 1. } },
            .sun       = 1,
            .pos       = { .v = { 10., 11.5, 7. } },
            .intensity = 1.,
         },
         { 0 },
      },
};
Lighting L_default;

static GLuint light_fbo_low[MAX_LIGHTS]; /**< FBO correpsonding to the light. */
static GLuint
   light_tex_low[MAX_LIGHTS]; /**< Texture corresponding to the light. */
static GLuint light_fbo_high[MAX_LIGHTS]; /**< FBO correpsonding to the light,
                                             with high resolution. */
static GLuint light_tex_high[MAX_LIGHTS]; /**< Texture corresponding to the
                                             light with high resolution. */
static GLuint  shadowmap_size = SHADOWMAP_SIZE_LOW;
static GLuint *light_fbo      = light_fbo_low;
static GLuint *light_tex      = light_tex_low;
static mat4    light_mat_def[MAX_LIGHTS]; /**< Shadow matrices. */
static mat4    light_mat_alt[MAX_LIGHTS];
static mat4   *light_mat = light_mat_def;

/**
 * @brief Shader to use witha material.
 */
typedef struct Shader_ {
   GLuint program;
   /* Attriutes. */
   GLuint vertex;
   GLuint vertex_normal;
   GLuint vertex_tex0;
   GLuint vertex_tex1;
   /* Vertex Uniforms. */
   GLuint Hmodel;
   GLuint Hnormal;
   GLuint Hshadow;
   /* Fragment uniforms. */
   GLuint      baseColour_tex;
   GLuint      baseColour_texcoord;
   GLuint      metallic_tex;
   GLuint      metallic_texcoord;
   GLuint      u_has_normal;
   GLuint      normal_tex;
   GLuint      normal_texcoord;
   GLuint      normal_scale;
   GLuint      metallicFactor;
   GLuint      roughnessFactor;
   GLuint      baseColour;
   GLuint      sheenTint;
   GLuint      sheen;
   GLuint      clearcoat;
   GLuint      clearcoat_roughness;
   GLuint      emissive;
   GLuint      emissive_tex;
   GLuint      emissive_texcoord;
   GLuint      occlusion_tex;
   GLuint      occlusion_texcoord;
   ShaderLight lights[MAX_LIGHTS];
   GLuint      nlights;
   GLuint      blend;
   GLuint      u_ambient;
   /* Custom Naev. */
   // GLuint waxiness;
} Shader;
static Shader  gltf_shader;
static Shader  shadow_shader;
static Texture tex_zero = {
   .tex      = 0,
   .texcoord = 0,
   .strength = 1. }; /* Used to detect initialization for now. */
static Texture tex_ones = { .tex = 0, .texcoord = 0, .strength = 1. };

/* Below here are for blurring purposes. */
static GLuint  shadow_vbo;
static GLuint  shadow_fbo_low;
static GLuint  shadow_fbo_high;
static GLuint  shadow_tex_low;
static GLuint  shadow_tex_high;
static GLuint *shadow_fbo = &shadow_fbo_low;
static GLuint *shadow_tex = &shadow_tex_low;
static Shader  shadow_shader_blurX;
static Shader  shadow_shader_blurY;

/* Options. */
static int use_normal_mapping    = 1;
static int use_ambient_occlusion = 1;
static int max_tex_size          = 0;

/* Prototypes. */
static int         cache_cmp( const void *p1, const void *p2 );
static GltfObject *cache_get( const char *filename, int *new );
static int         cache_dec( GltfObject *obj );
static void        gltf_applyAnim( GltfObject *obj, GLfloat time );

/**
 * @brief Loads a texture if applicable, uses default value otherwise.
 *
 *    @param otex Texture to output to.
 *    @param ctex Texture to load.
 *    @param def Default texture to use if not defined.
 *    @param notsrgb Whether or not the texture should use SRGB.
 *    @return OpenGL ID of the new texture.
 */
static int gltf_loadTexture( const GltfObject *obj, Texture *otex,
                             const cgltf_texture_view *ctex, const Texture *def,
                             int notsrgb )
{
   const SDL_PixelFormatEnum fmt = SDL_PIXELFORMAT_ABGR8888;
   GLuint                    tex;
   SDL_Surface              *surface = NULL;
   int                       has_alpha;
   const char               *path;
   GLint                     internalformat;
   SDL_RWops                *rw;
#ifdef HAVE_NAEV
   has_alpha = 0;
#endif /* HAVE_NAEV */

   /* Must haev texture to load it. */
   if ( ( ctex == NULL ) || ( ctex->texture == NULL ) ) {
      *otex = *def;
      return 0;
   }

   /* Set up some data. */
   otex->texcoord = ctex->texcoord;
   otex->strength = ctex->transform.scale[0];

   /* Load from path. */
   path = ( ctex->texture->has_webp ) ? ctex->texture->webp_image->uri
                                      : ctex->texture->image->uri;
   if ( path == NULL ) {
      DEBUG( "Buffer textures not supported yet!" );
      return -1;
   }

   char filepath[PATH_MAX];
#ifdef HAVE_NAEV
   snprintf( filepath, sizeof( filepath ), "%s/%s", obj->path, path );
#else  /* HAVE_NAEV */
   snprintf( filepath, sizeof( filepath ), "%s", path );
#endif /* HAVE_NAEV */
   nfile_simplifyPath( filepath );

   /* Check to see if it already exists. */
#ifdef HAVE_NAEV
   int created;
   int flags = OPENGL_TEX_MIPMAPS;
   if ( notsrgb )
      flags |= OPENGL_TEX_NOTSRGB;
   otex->gtex = gl_texExistsOrCreate( filepath, flags, 1, 1, &created );
   if ( !created ) {
      /* Already exists, so texture should be valid. */
      otex->tex = otex->gtex->texture;
      return 0;
   }
#endif /* HAVE_NAEV */

   /* Load the image data as a surface. */
   rw = PHYSFSRWOPS_openRead( filepath );
   if ( rw == NULL ) {
      WARN( _( "Unable to open '%s': %s" ), filepath, SDL_GetError() );
      *otex = *def;
      return 0;
   }
   surface = IMG_Load_RW( rw, 1 );
   if ( surface == NULL ) {
      WARN( _( "Unable to load surface '%s': %s" ), filepath, SDL_GetError() );
      *otex = *def;
      return 0;
   }

   gl_contextSet();

   glGenTextures( 1, &tex );
   glBindTexture( GL_TEXTURE_2D, tex );

   /* Set stuff. */
   if ( ctex->texture->sampler != NULL ) {
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                       ctex->texture->sampler->mag_filter );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                       ctex->texture->sampler->min_filter );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                       ctex->texture->sampler->wrap_s );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                       ctex->texture->sampler->wrap_t );
   } else {
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
   }

   if ( surface != NULL ) {
      has_alpha = surface->format->Amask;
      if ( notsrgb )
         internalformat = has_alpha ? GL_RGBA : GL_RGB;
      else
         internalformat = has_alpha ? GL_SRGB_ALPHA : GL_SRGB;
      if ( surface->format->format != fmt ) {
         SDL_Surface *temp = surface;
         surface           = SDL_ConvertSurfaceFormat( temp, fmt, 0 );
         SDL_FreeSurface( temp );
      }

      SDL_LockSurface( surface );
      glPixelStorei( GL_UNPACK_ALIGNMENT,
                     MIN( surface->pitch & -surface->pitch, 8 ) );
      if ( notsrgb )
         glTexImage2D( GL_TEXTURE_2D, 0, internalformat, surface->w, surface->h,
                       0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
      else
         glTexImage2D( GL_TEXTURE_2D, 0, internalformat, surface->w, surface->h,
                       0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
      glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
      SDL_UnlockSurface( surface );
   } else {
      /*
      glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB_ALPHA,
            surface->w, surface->h, 0,
            surface->format->Amask ? GL_RGBA : GL_RGB,
            GL_UNSIGNED_BYTE, surface->pixels );
      */
   }

#ifdef HAVE_NAEV
   /* Downsample as necessary. */
   if ( ( max_tex_size > 0 ) && ( surface != NULL ) &&
        ( MAX( surface->w, surface->h ) > max_tex_size ) ) {
      GLuint fbo, downfbo, downtex;
      GLint  status;

      /* Create the downsampling framebuffers. */
      gl_fboCreate( &downfbo, &downtex, max_tex_size, max_tex_size );

      /* Create the render buffer, keeping RGB status. */
      glGenTextures( 1, &downtex );
      glBindTexture( GL_TEXTURE_2D, downtex );
      glTexImage2D( GL_TEXTURE_2D, 0, internalformat, max_tex_size,
                    max_tex_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );

      /* Create the frame buffer. */
      glGenFramebuffers( 1, &downfbo );
      glBindFramebuffer( GL_FRAMEBUFFER, downfbo );

      /* Attach the colour buffer. */
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_TEXTURE_2D, downtex, 0 );

      /* Check status. */
      status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
      if ( status != GL_FRAMEBUFFER_COMPLETE )
         WARN( _( "Error setting up framebuffer!" ) );

      /* Attach a framebuffer to the current texture. */
      glGenFramebuffers( 1, &fbo );
      glBindFramebuffer( GL_FRAMEBUFFER, fbo );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_TEXTURE_2D, tex, 0 );

      /* Blit to framebuffer. */
      glBindFramebuffer( GL_READ_FRAMEBUFFER, fbo );
      glBindFramebuffer( GL_DRAW_FRAMEBUFFER, downfbo );
      glBlitFramebuffer( 0, 0, surface->w, surface->h, 0, 0, max_tex_size,
                         max_tex_size, GL_COLOR_BUFFER_BIT, GL_LINEAR );
      glBindFramebuffer( GL_FRAMEBUFFER, 0 );

      /* Clean up. */
      glDeleteTextures( 1, &tex );
      glDeleteFramebuffers( 1, &downfbo );
      glDeleteFramebuffers( 1, &fbo );
      tex = downtex;
      glBindTexture( GL_TEXTURE_2D, tex );

      /* Reapply sampling. */
      if ( ctex->texture->sampler != NULL ) {
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                          ctex->texture->sampler->mag_filter );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                          ctex->texture->sampler->min_filter );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                          ctex->texture->sampler->wrap_s );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                          ctex->texture->sampler->wrap_t );
      } else {
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      }
   }
#endif /* HAVE_NAEV */

   /* Set up mipmaps. */
   /* TODO only generate if necessary. */
   glGenerateMipmap( GL_TEXTURE_2D );

   /* Free the surface. */
   SDL_FreeSurface( surface );

   glBindTexture( GL_TEXTURE_2D, 0 );

   gl_checkErr();
   gl_contextUnset();

   otex->tex = tex;
#ifdef HAVE_NAEV
   otex->gtex->texture = tex; /* Update the texture. */
#endif                        /* HAVE_NAEV */
   return 0;
}

/**
 * @brief Loads a material for the object.
 */
static int gltf_loadMaterial( const GltfObject *obj, Material *mat,
                              const cgltf_material *cmat,
                              const cgltf_data     *data )
{
   const GLfloat white[4] = { 1., 1., 1., 1. };
   /* TODO complete this. */
   if ( cmat && cmat->has_pbr_metallic_roughness ) {
      mat->metallicFactor  = cmat->pbr_metallic_roughness.metallic_factor;
      mat->roughnessFactor = cmat->pbr_metallic_roughness.roughness_factor;
      gltf_loadTexture( obj, &mat->baseColour_tex,
                        &cmat->pbr_metallic_roughness.base_color_texture,
                        &tex_ones, 0 );
      if ( mat->baseColour_tex.tex == tex_ones.tex )
         memcpy( mat->baseColour,
                 cmat->pbr_metallic_roughness.base_color_factor,
                 sizeof( mat->baseColour ) );
      else
         memcpy( mat->baseColour, white, sizeof( mat->baseColour ) );
      gltf_loadTexture(
         obj, &mat->metallic_tex,
         &cmat->pbr_metallic_roughness.metallic_roughness_texture, &tex_ones,
         1 );
   } else {
      memcpy( mat->baseColour, white, sizeof( mat->baseColour ) );
      mat->metallicFactor  = 0.;
      mat->roughnessFactor = 1.;
      mat->baseColour_tex  = tex_ones;
      mat->metallic_tex    = tex_ones;
      mat->normal_tex      = tex_zero;
   }

   /* Sheen. */
   if ( cmat && cmat->has_sheen ) {
      memcpy( mat->sheen, cmat->sheen.sheen_color_factor,
              sizeof( mat->sheen ) );
      mat->sheen_roughness = cmat->sheen.sheen_roughness_factor;
   } else {
      memset( mat->sheen, 0, sizeof( mat->sheen ) );
      mat->sheen_roughness = 0.;
   }

   /* Handle clearcoat. */
   if ( cmat && cmat->has_clearcoat ) {
      mat->clearcoat           = cmat->clearcoat.clearcoat_factor;
      mat->clearcoat_roughness = cmat->clearcoat.clearcoat_roughness_factor;
   } else {
      mat->clearcoat           = 0.;
      mat->clearcoat_roughness = 0.;
   }

   /* Handle emissiveness and such. */
   if ( cmat ) {
      memcpy( mat->emissiveFactor, cmat->emissive_factor,
              sizeof( GLfloat ) * 3 );
      gltf_loadTexture( obj, &mat->emissive_tex, &cmat->emissive_texture,
                        &tex_ones, 0 );
      if ( use_ambient_occlusion )
         gltf_loadTexture( obj, &mat->occlusion_tex, &cmat->occlusion_texture,
                           &tex_ones, 1 );
      else
         mat->occlusion_tex = tex_ones;
      if ( use_normal_mapping )
         gltf_loadTexture( obj, &mat->normal_tex, &cmat->normal_texture,
                           &tex_zero, 1 );
      else
         mat->normal_tex = tex_ones;
      mat->blend        = ( cmat->alpha_mode == cgltf_alpha_mode_blend );
      mat->double_sided = cmat->double_sided;
      mat->unlit        = cmat->unlit;
   } else {
      memset( mat->emissiveFactor, 0, sizeof( GLfloat ) * 3 );
      mat->emissive_tex  = tex_ones;
      mat->occlusion_tex = tex_ones;
      mat->normal_tex    = tex_ones;
      mat->blend         = 0;
   }
   /* Emissive strength extension just multiplies the emissiveness. */
   if ( cmat && cmat->has_emissive_strength ) {
      for ( int i = 0; i < 3; i++ )
         mat->emissiveFactor[i] *= cmat->emissive_strength.emissive_strength;
   }

   mat->noshadows = mat->blend; /* Transparent things don't cast shadows. */
   if ( cmat && data ) {
      char       buf[STRMAX_SHORT];
      cgltf_size len = sizeof( buf );
      cgltf_copy_extras_json( data, &cmat->extras, buf, &len );
      jsmn_parser p;
      jsmntok_t   t[16]; /* Max number of expected tokens. */
      jsmn_init( &p );
      int r = jsmn_parse( &p, buf, len, t, sizeof( t ) / sizeof( jsmntok_t ) );
      for ( int j = 0; j < r; j++ ) {
         const jsmntok_t *tj  = &t[j];
         const char      *str = "NAEV_noShadows";
         if ( strncmp( str, &buf[tj->start],
                       MIN( strlen( str ),
                            (size_t)( tj->end - tj->start ) ) ) == 0 ) {
            if ( j + 1 >= r )
               break;
            /* Disables shadows. */
            mat->noshadows = 1;
            break;
         }
      }
   }

#if 0
   mat->waxiness = 0.;
   if (cmat) {
      for (size_t i=0; i<cmat->extensions_count; i++) {
         cgltf_extension *ext = &cmat->extensions[i];
         if (strcmp(ext->name,"NAEV_ext")!=0)
            continue;
         jsmn_parser p;
         jsmntok_t t[16]; /* Max number of expected tokens. */
         jsmn_init(&p);
         int r = jsmn_parse( &p, ext->data, strlen(ext->data), t, sizeof(t)/sizeof(jsmntok_t) );
         for (int j=0; j<r; j++) {
            jsmntok_t *tj = &t[j];
            const char *str = "waxFactor";
            if (strncmp( str, &ext->data[tj->start], MIN(strlen(str),(size_t)(tj->end-tj->start)) )==0) {
               if (j+1 >= r)
                  break;
               mat->waxiness = atof(&ext->data[t[j+1].start]);
               break;
            }
         }
      }
   }
#endif

   return 0;
}

/**
 * @brief Loads a VBO from an accessor.
 *
 *    @param acc Accessor to load from.
 *    @return OpenGL ID of the new VBO.
 */
static GLuint gltf_loadVBO( const cgltf_accessor *acc, cgltf_float **data,
                            cgltf_size *datasize )
{
   GLuint       vid;
   cgltf_size   num = cgltf_accessor_unpack_floats( acc, NULL, 0 );
   cgltf_float *dat = calloc( num, sizeof( cgltf_float ) );
   cgltf_accessor_unpack_floats( acc, dat, num );
   if ( data != NULL ) {
      *data     = dat;
      *datasize = num;
   }

   /* OpenGL magic. */
   gl_contextSet();
   glGenBuffers( 1, &vid );
   glBindBuffer( GL_ARRAY_BUFFER, vid );
   glBufferData( GL_ARRAY_BUFFER, sizeof( cgltf_float ) * num, dat,
                 GL_STATIC_DRAW );
   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   gl_checkErr();
   gl_contextUnset();

   if ( data == NULL )
      free( dat );
   return vid;
}

/**
 * @brief Loads a mesh for the object.
 */
static int gltf_loadMesh( GltfObject *obj, const cgltf_data *data, Mesh *mesh,
                          const cgltf_mesh *cmesh )
{
   mesh->primitives =
      calloc( cmesh->primitives_count, sizeof( MeshPrimitive ) );
   mesh->nprimitives = cmesh->primitives_count;
   for ( size_t i = 0; i < cmesh->primitives_count; i++ ) {
      MeshPrimitive         *prim    = &mesh->primitives[i];
      const cgltf_primitive *cprim   = &cmesh->primitives[i];
      const cgltf_accessor  *acc     = cprim->indices;
      cgltf_float           *rawdata = NULL;
      cgltf_size             datasize;
      if ( acc == NULL ) {
         prim->material = -1;
         continue;
      }

      cgltf_size num = cgltf_num_components( acc->type ) * acc->count;
      GLuint    *idx = calloc( num, sizeof( cgltf_uint ) );
      for ( size_t j = 0; j < num; j++ )
         cgltf_accessor_read_uint( acc, j, &idx[j], 1 );

      /* Check material. */
      if ( cprim->material != NULL )
         prim->material = cgltf_material_index( data, cprim->material );
      else
         prim->material = -1;

      /* Store indices. */
      gl_contextSet();
      glGenBuffers( 1, &prim->vbo_idx );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, prim->vbo_idx );
      glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( cgltf_uint ) * num, idx,
                    GL_STATIC_DRAW );
      prim->nidx = acc->count;
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
      gl_checkErr();
      gl_contextUnset();
      free( idx );

      for ( size_t j = 0; j < cprim->attributes_count; j++ ) {
         const cgltf_attribute *attr = &cprim->attributes[j];
         switch ( attr->type ) {
         case cgltf_attribute_type_position:
            prim->vbo_pos = gltf_loadVBO( attr->data, &rawdata, &datasize );
            break;

         case cgltf_attribute_type_normal:
            prim->vbo_nor = gltf_loadVBO( attr->data, NULL, NULL );
            break;

         case cgltf_attribute_type_texcoord:
            if ( attr->index == 0 )
               prim->vbo_tex0 = gltf_loadVBO( attr->data, NULL, NULL );
            else
               prim->vbo_tex1 = gltf_loadVBO( attr->data, NULL, NULL );
            /* TODO handle other cases? */
            break;

         case cgltf_attribute_type_color:
         case cgltf_attribute_type_tangent:
         default:
            break;
         }
      }

      /* Try to figure out dimensions. */
      if ( rawdata != NULL ) {
         /* Try to find associated node. */
         for ( size_t n = 0; n < obj->nnodes; n++ ) {
            Node *node = &obj->nodes[n];
            if ( cmesh != data->nodes[n].mesh )
               continue;
            mat4 H;
            cgltf_node_transform_world( &data->nodes[n], H.ptr );
            for ( unsigned int di = 0; di < datasize; di += 3 ) {
               vec3 v, d;
               for ( unsigned int dj = 0; dj < 3; dj++ )
                  d.v[dj] = rawdata[di + dj];
               // mat4_mul_vec( &v, &node->H, &d );
               mat4_mul_vec( &v, &H, &d );
               vec3_min( &node->aabb_min, &v, &node->aabb_min );
               vec3_max( &node->aabb_max, &v, &node->aabb_max );
               node->radius = MAX( node->radius, vec3_length( &v ) );
            }
         }
         free( rawdata );
      }
   }
   return 0;
}

/**
 * @brief Loads a node for the object.
 */
static int gltf_loadNode( GltfObject *obj, const cgltf_data *data, Node *node,
                          const cgltf_node *cnode )
{
   /* Get transform for node. */
   cgltf_node_transform_local( cnode, node->Horig.ptr );
   node->H = node->Horig; /* Copy over. */
   // node->parent = cgltf_node_index( data, cnode->parent );

   if ( cnode->has_rotation )
      memcpy( node->nt.r.q, cnode->rotation, sizeof( cnode->rotation ) );
   else {
      node->nt.r.q[0] = 0.;
      node->nt.r.q[1] = 0.;
      node->nt.r.q[3] = 0.;
      node->nt.r.q[3] = 1.;
   }
   if ( cnode->has_translation )
      memcpy( node->nt.t.v, cnode->translation, sizeof( cnode->translation ) );
   else {
      node->nt.t.v[0] = 0.;
      node->nt.t.v[1] = 0.;
      node->nt.t.v[2] = 0.;
   }
   if ( cnode->has_scale )
      memcpy( node->nt.s.v, cnode->scale, sizeof( cnode->scale ) );
   else {
      node->nt.s.v[0] = 1.;
      node->nt.s.v[1] = 1.;
      node->nt.s.v[2] = 1.;
   }
   node->ntorig = node->nt;

   /* Get the mesh. */
   if ( cnode->mesh != NULL )
      node->mesh = cgltf_mesh_index( data, cnode->mesh );
   else
      node->mesh = -1;

   /* Handle extras. */
   if ( cnode->extras.data != NULL ) {
      char       buf[STRMAX_SHORT];
      cgltf_size len = sizeof( buf );
      cgltf_copy_extras_json( data, &cnode->extras, buf, &len );
      jsmn_parser p;
      jsmntok_t   t[32]; /* Max number of expected tokens. */
      jsmn_init( &p );
      int r = jsmn_parse( &p, buf, len, t, sizeof( t ) / sizeof( jsmntok_t ) );
      for ( int j = 0; j < r; j++ ) {
         const jsmntok_t *tj = &t[j];
         /* Handle trail generators, which are stored as properties on nodes. */
         const char *strtrail = "NAEV_trail_generator";
         if ( strncmp( strtrail, &buf[tj->start],
                       MIN( strlen( strtrail ),
                            (size_t)( tj->end - tj->start ) ) ) == 0 ) {
            GltfTrail trail;
            mat4      H;
            vec3      v = { .v = {
                          0.,
                          0.,
                          0.,
                       } };
            if ( j + 1 >= r )
               break;
            trail.generator = calloc( 1, t[j + 1].end - t[j + 1].start + 1 );
            strncpy( trail.generator, &cnode->extras.data[t[j + 1].start],
                     t[j + 1].end - t[j + 1].start );
            trail.generator[t[j + 1].end - t[j + 1].start] = '\0';
            cgltf_node_transform_world( cnode, H.ptr );
            mat4_mul_vec( &trail.pos, &H, &v );
            if ( obj->trails == NULL )
               obj->trails = array_create( GltfTrail );
            array_push_back( &obj->trails, trail );
            break;
         }
         /* Handle mount points, also stored as extras. */
         const char *strmount = "NAEV_weapon_mount";
         if ( strncmp( strmount, &buf[tj->start],
                       MIN( strlen( strmount ),
                            (size_t)( tj->end - tj->start ) ) ) == 0 ) {
            GltfMount mount;
            mat4      H;
            vec3      v = { .v = {
                          0.,
                          0.,
                          0.,
                       } };
            if ( j + 1 >= r )
               break;
            mount.id = atoi( &cnode->extras.data[t[j + 1].start] );
            cgltf_node_transform_world( cnode, H.ptr );
            mat4_mul_vec( &mount.pos, &H, &v );
            if ( obj->mounts == NULL )
               obj->mounts = array_create( GltfMount );
            array_push_back( &obj->mounts, mount );
            break;
         }
      }
   }

   /* Iterate over children. */
   node->nchildren = cnode->children_count;
   if ( node->nchildren > 0 ) {
      node->children = calloc( cnode->children_count, sizeof( size_t ) );
      for ( cgltf_size i = 0; i < cnode->children_count; i++ )
         node->children[i] = cgltf_node_index( data, cnode->children[i] );
   }
   return 0;
}

static int gltf_loadAnimation( GltfObject *obj, const cgltf_data *data,
                               Animation *anim, const cgltf_animation *canim )
{
   anim->nsamplers = canim->samplers_count;
   anim->samplers  = calloc( anim->nsamplers, sizeof( AnimationSampler ) );
   for ( cgltf_size i = 0; i < canim->samplers_count; i++ ) {
      const cgltf_animation_sampler *csamp = &canim->samplers[i];
      AnimationSampler              *samp  = &anim->samplers[i];
      cgltf_size                     n;

      switch ( csamp->interpolation ) {
      case cgltf_interpolation_type_linear:
         samp->interp = ANIM_INTER_LINEAR;
         break;
      case cgltf_interpolation_type_step:
         samp->interp = ANIM_INTER_STEP;
         break;
      default:
         WARN( _( "Unsupported interpolation type %d!" ),
               csamp->interpolation );
         break;
      }

      samp->n    = cgltf_accessor_unpack_floats( csamp->input, NULL, 0 );
      samp->time = calloc( samp->n, sizeof( cgltf_float ) );
      cgltf_accessor_unpack_floats( csamp->input, samp->time, samp->n );
      samp->max = samp->time[samp->n - 1]; /**< Assume last keyframe. */

      samp->l = cgltf_num_components( csamp->output->type );
      n       = cgltf_accessor_unpack_floats( csamp->output, NULL, 0 );
      if ( cgltf_num_components( csamp->output->type ) * samp->n != n )
         WARN( _( "Wrong number of elements. Got %lu, but expected %lu!" ),
               samp->n * cgltf_num_components( csamp->output->type ), n );
      samp->data = calloc( n, sizeof( cgltf_float ) );
      cgltf_accessor_unpack_floats( csamp->output, samp->data, n );
   }

   anim->nchannels = canim->channels_count;
   anim->channels  = calloc( anim->nchannels, sizeof( AnimationChannel ) );
   for ( cgltf_size i = 0; i < canim->channels_count; i++ ) {
      const cgltf_animation_channel *cchan = &canim->channels[i];
      AnimationChannel              *chan  = &anim->channels[i];
      chan->sampler =
         &anim
             ->samplers[cgltf_animation_sampler_index( canim, cchan->sampler )];
      chan->target = &obj->nodes[cgltf_node_index( data, cchan->target_node )];
      switch ( cchan->target_path ) {
      case cgltf_animation_path_type_translation:
         chan->type = ANIM_TYPE_TRANSLATION;
         break;
      case cgltf_animation_path_type_rotation:
         chan->type = ANIM_TYPE_ROTATION;
         break;
      case cgltf_animation_path_type_scale:
         chan->type = ANIM_TYPE_SCALE;
         break;
      default:
         WARN( _( "Uknown animation type %d!" ), cchan->target_path );
         break;
      }
   }
   return 0;
}

static void shadow_matrix( mat4 *m, const Light *light )
{
   const vec3  up     = { .v = { 0., 1., 0. } };
   const vec3  center = { .v = { 0., 0., 0. } };
   const float r      = 1.0;
   if ( light->sun ) {
      vec3 light_pos = light->pos;
      vec3_normalize( &light_pos );
      const mat4 L = mat4_lookat( &light_pos, &center, &up );
      const mat4 O = mat4_ortho( -r, r, -r, r, 0.0, 2.0 );
      mat4_mul( m, &L, &O );
   } else {
      /* TODO fix this. Point lights should use perspective matrix... */
      const vec3  light_pos = light->pos;
      const mat4  L         = mat4_lookat( &light_pos, &center, &up );
      const float norm      = vec3_length( &light_pos );
      const mat4  O = mat4_ortho( -r, r, -r, r, norm - 1.0, norm + 1.0 );
      mat4_mul( m, &L, &O );
   }
}

/**
 * @brief Renders a mesh shadow with a transform.
 */
static void renderMeshPrimitiveShadow( const GltfObject    *obj,
                                       const MeshPrimitive *mesh,
                                       const mat4          *H )
{
   (void)obj;
   const Shader *shd = &shadow_shader;

   /* Skip with no shadows. */
   if ( ( mesh->material >= 0 ) &&
        ( obj->materials[mesh->material].noshadows ) )
      return;

   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->vbo_idx );

   /* TODO put everything in a single VBO */
   glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo_pos );
   glVertexAttribPointer( shd->vertex, 3, GL_FLOAT, GL_FALSE, 0, NULL );
   glEnableVertexAttribArray( shd->vertex );

   glUniformMatrix4fv( shd->Hmodel, 1, GL_FALSE, H->ptr );
   glDrawElements( GL_TRIANGLES, mesh->nidx, GL_UNSIGNED_INT, 0 );
}
static void renderMeshShadow( const GltfObject *obj, const Mesh *mesh,
                              const mat4 *H )
{
   for ( int i = 0; i < mesh->nprimitives; i++ )
      renderMeshPrimitiveShadow( obj, &mesh->primitives[i], H );
}

/**
 * @brief Renders a mesh with a transform.
 */
static void renderMeshPrimitive( const GltfObject    *obj,
                                 const MeshPrimitive *mesh, const mat4 *H )
{
   const Material *mat;
   const Shader   *shd = &gltf_shader;

   if ( mesh->material < 0 )
      mat = &material_default;
   else
      mat = &obj->materials[mesh->material];

   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->vbo_idx );

   /* TODO put everything in a single VBO */
   glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo_pos );
   glVertexAttribPointer( shd->vertex, 3, GL_FLOAT, GL_FALSE, 0, NULL );
   glEnableVertexAttribArray( shd->vertex );
   if ( mesh->vbo_nor ) {
      glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo_nor );
      glVertexAttribPointer( shd->vertex_normal, 3, GL_FLOAT, GL_FALSE, 0,
                             NULL );
      glEnableVertexAttribArray( shd->vertex_normal );
   }
   gl_checkErr();
   if ( mesh->vbo_tex0 ) {
      glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo_tex0 );
      glVertexAttribPointer( shd->vertex_tex0, 2, GL_FLOAT, GL_FALSE, 0, NULL );
      glEnableVertexAttribArray( shd->vertex_tex0 );
   }
   gl_checkErr();
   if ( mesh->vbo_tex1 ) {
      glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo_tex1 );
      glVertexAttribPointer( shd->vertex_tex1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
      glEnableVertexAttribArray( shd->vertex_tex1 );
   }
   gl_checkErr();

   /* Set up shader. */
   glUseProgram( shd->program );

   /* Compute normal matrix. */
   mat3 Hnormal;
   mat3_from_mat4( &Hnormal, H );
   mat3_invert( &Hnormal );
   mat3_transpose( &Hnormal );

   /* Pass the uniforms. */
   glUniformMatrix4fv( shd->Hmodel, 1, GL_FALSE, H->ptr );
   glUniformMatrix3fv( shd->Hnormal, 1, GL_FALSE, Hnormal.ptr );
   glUniform1f( shd->metallicFactor, mat->metallicFactor );
   glUniform1f( shd->roughnessFactor, mat->roughnessFactor );
   glUniform4f( shd->baseColour, mat->baseColour[0], mat->baseColour[1],
                mat->baseColour[2], mat->baseColour[3] );
   glUniform3f( shd->sheenTint, mat->sheen[0], mat->sheen[1], mat->sheen[2] );
   glUniform1f( shd->sheen, mat->sheen_roughness );
   glUniform1f( shd->clearcoat, mat->clearcoat );
   glUniform1f( shd->clearcoat_roughness, mat->clearcoat_roughness );
   glUniform3f( shd->emissive, mat->emissiveFactor[0], mat->emissiveFactor[1],
                mat->emissiveFactor[2] );
   glUniform1i( shd->blend, mat->blend );
   if ( use_normal_mapping ) {
      glUniform1i( shd->u_has_normal, ( mat->normal_tex.tex != tex_zero.tex ) );
      glUniform1f( shd->normal_scale, mat->normal_tex.strength );
      glUniform1i( shd->normal_texcoord, mat->normal_tex.texcoord );
   }
   // glUniform1f( shd->waxiness, mat->waxiness );
   /* Texture coordinates. */
   glUniform1i( shd->baseColour_texcoord, mat->baseColour_tex.texcoord );
   glUniform1i( shd->metallic_texcoord, mat->metallic_tex.texcoord );
   glUniform1i( shd->emissive_texcoord, mat->emissive_tex.texcoord );
   if ( use_ambient_occlusion )
      glUniform1i( shd->occlusion_texcoord, mat->occlusion_tex.texcoord );
   gl_checkErr();

   /* Texture. */
   glActiveTexture( GL_TEXTURE1 );
   glBindTexture( GL_TEXTURE_2D, mat->metallic_tex.tex );
   glUniform1i( shd->metallic_tex, 1 );
   glActiveTexture( GL_TEXTURE2 );
   glBindTexture( GL_TEXTURE_2D, mat->emissive_tex.tex );
   glUniform1i( shd->emissive_tex, 2 );
   if ( use_normal_mapping ) {
      glActiveTexture( GL_TEXTURE3 );
      glBindTexture( GL_TEXTURE_2D, mat->normal_tex.tex );
      glUniform1i( shd->normal_tex, 3 );
   }
   if ( use_ambient_occlusion ) {
      glActiveTexture( GL_TEXTURE4 );
      glBindTexture( GL_TEXTURE_2D, mat->occlusion_tex.tex );
      glUniform1i( shd->occlusion_tex, 4 );
   }
   /* Have to have GL_TEXTURE0 be last. */
   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, mat->baseColour_tex.tex );
   glUniform1i( shd->baseColour_tex, 0 );
   gl_checkErr();

   if ( mat->double_sided )
      glDisable( GL_CULL_FACE );
   if ( mat->blend ) /* Don't write depth for transparent objects. */
      glDepthMask( GL_FALSE );
   glDrawElements( GL_TRIANGLES, mesh->nidx, GL_UNSIGNED_INT, 0 );
   if ( mat->double_sided )
      glEnable( GL_CULL_FACE );
   if ( mat->blend )
      glDepthMask( GL_TRUE );
}
static void renderMesh( const GltfObject *obj, const Mesh *mesh, const mat4 *H )
{
   for ( int i = 0; i < mesh->nprimitives; i++ )
      renderMeshPrimitive( obj, &mesh->primitives[i], H );
}

/**
 * @brief Recursive rendering to the shadow buffer.
 */
static void gltf_renderNodeShadow( const GltfObject *obj, const Node *node,
                                   const mat4 *H )
{
   /* Multiply matrices, can be animated so not caching. */
   /* TODO cache when not animated. */
   mat4 HH = node->H;
   mat4_apply( &HH, H );

   /* Draw mesh. */
   if ( node->mesh >= 0 )
      renderMeshShadow( obj, &obj->meshes[node->mesh], &HH );

   /* Draw children. */
   for ( size_t i = 0; i < node->nchildren; i++ )
      gltf_renderNodeShadow( obj, &obj->nodes[node->children[i]], &HH );

   gl_checkErr();
}

/**
 * @brief Recursive rendering of a mesh.
 */
static void gltf_renderNodeMesh( const GltfObject *obj, const Node *node,
                                 const mat4 *H )
{
   /* Multiply matrices, can be animated so not caching. */
   /* TODO cache when not animated. */
   mat4 HH = node->H;
   mat4_apply( &HH, H );

   /* If determinant is negative, we have to invert winding. */
   mat3 m;
   mat3_from_mat4( &m, &HH );
   glFrontFace( ( mat3_det( &m ) < 0. ) ? GL_CW : GL_CCW );

   /* Draw mesh. */
   if ( node->mesh >= 0 )
      renderMesh( obj, &obj->meshes[node->mesh], &HH );

   /* Draw children. */
   for ( size_t i = 0; i < node->nchildren; i++ )
      gltf_renderNodeMesh( obj, &obj->nodes[node->children[i]], &HH );

   gl_checkErr();
}

static void gltf_renderShadow( const GltfObject *obj, int scene, const mat4 *H,
                               const Light *light, int i )
{
   (void)light;
   const Shader *shd = &shadow_shader;

   /* Set up the shadow map and render. */
   glBindFramebuffer( GL_FRAMEBUFFER, light_fbo[i] );
   glClear( GL_DEPTH_BUFFER_BIT );
   glViewport( 0, 0, shadowmap_size, shadowmap_size );

   /* Set up shader. */
   glUseProgram( shd->program );
   glUniformMatrix4fv( shd->Hshadow, 1, GL_FALSE, light_mat[i].ptr );

   for ( size_t j = 0; j < obj->scenes[scene].nnodes; j++ )
      gltf_renderNodeShadow( obj, &obj->nodes[obj->scenes[scene].nodes[j]], H );

   glDisable( GL_CULL_FACE );
   gl_checkErr();

   /* Now we have to blur. We'll do a separable filter approach and do two
    * passes. */
   /* First pass for X. */
   shd = &shadow_shader_blurX;
   glBindFramebuffer( GL_FRAMEBUFFER, *shadow_fbo );
   glClear( GL_DEPTH_BUFFER_BIT );
   glUseProgram( shd->program );

   glBindBuffer( GL_ARRAY_BUFFER, shadow_vbo );
   glVertexAttribPointer( shd->vertex, 2, GL_FLOAT, GL_FALSE, 0, NULL );
   glEnableVertexAttribArray( shd->vertex );

   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, light_tex[i] );

   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   gl_checkErr();
   /* Second pass for Y and back into the proper framebuffer. */
   shd = &shadow_shader_blurY;
   glBindFramebuffer( GL_FRAMEBUFFER, light_fbo[i] );
   glClear( GL_DEPTH_BUFFER_BIT );

   glUseProgram( shd->program );

   glBindBuffer( GL_ARRAY_BUFFER, shadow_vbo );
   glVertexAttribPointer( shd->vertex, 2, GL_FLOAT, GL_FALSE, 0, NULL );
   glEnableVertexAttribArray( shd->vertex );

   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, *shadow_tex );

   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   /* Clean up. */
   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   glDisableVertexAttribArray( shd->vertex );
   glBindFramebuffer( GL_FRAMEBUFFER, 0 );
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

   gl_checkErr();
}

static void gltf_renderMesh( const GltfObject *obj, int scene, const mat4 *H,
                             const Lighting *L )
{
   /* Load constant stuff. */
   const Shader *shd = &gltf_shader;
   glUseProgram( shd->program );
   glUniform3f( shd->u_ambient, L->ambient_r, L->ambient_g, L->ambient_b );
   glUniform1i( shd->nlights, L->nlights );
   for ( int i = 0; i < L->nlights; i++ ) {
      const Light       *l  = &L->lights[i];
      const ShaderLight *sl = &shd->lights[i];

      /* Other parameters. */
      glUniform1i( sl->sun, l->sun );
      if ( l->sun ) {
         /* Normalize here istead of frag shader. */
         vec3 light_pos = l->pos;
         vec3_normalize( &light_pos );
         glUniform3f( sl->position, light_pos.v[0], light_pos.v[1],
                      light_pos.v[2] );
      } else
         glUniform3f( sl->position, l->pos.v[0], l->pos.v[1], l->pos.v[2] );
      glUniform3f( sl->colour, l->colour.v[0], l->colour.v[1], l->colour.v[2] );
      glUniform1f( sl->intensity, l->intensity * L->intensity );

      /* Set up matrix. */
      glUniformMatrix4fv( sl->Hshadow, 1, GL_FALSE, light_mat[i].ptr );

      /* Set up textures. */
      glActiveTexture( GL_TEXTURE5 + i );
      glBindTexture( GL_TEXTURE_2D, light_tex[i] );
      glUniform1i( shd->lights[i].shadowmap_tex, 5 + i );
   }
   gl_checkErr();

   /* Cull faces. */
   glEnable( GL_CULL_FACE );
   for ( size_t i = 0; i < obj->scenes[scene].nnodes; i++ )
      gltf_renderNodeMesh( obj, &obj->nodes[obj->scenes[scene].nodes[i]], H );

   glBindTexture( GL_TEXTURE_2D, 0 );
   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

   gl_checkErr();
}

static void gltf_applyAnimNode( GltfObject *obj, Animation *anim, GLfloat time )
{
   (void)obj;
   for ( size_t j = 0; j < anim->nchannels; j++ ) {
      const AnimationChannel *chan = &anim->channels[j];
      const AnimationSampler *samp = chan->sampler;
      int                     pi, ni;
      GLfloat                 p, n, mix;
      GLfloat                 t    = fmod( time, samp->max );
      Node                   *node = chan->target;

      /* See if target has animation, and initialize. */
      if ( !node->has_anim ) {
         node->nt       = node->ntorig;
         node->has_anim = 1;
      }

      /* TODO something better than linear search. */
      pi = 0;
      for ( size_t i = 0; i < samp->n; i++ ) {
         if ( samp->time[i] < t ) {
            pi = i;
            break;
         }
      }
      ni = ( pi + 1 ) % samp->n;
      p  = samp->time[pi];
      n  = samp->time[ni];

      /* Interpolate. */
      switch ( samp->interp ) {
      case ANIM_INTER_LINEAR:
         mix = ( t - p ) / ( n - p );
         break;
      case ANIM_INTER_STEP:
      default:
         mix = 0.;
         break;
      }
      /* Apply. */
      switch ( chan->type ) {
      case ANIM_TYPE_ROTATION:
         quat_slerp( node->nt.r.q, &samp->data[pi * samp->l],
                     &samp->data[ni * samp->l], mix );
         break;
      case ANIM_TYPE_TRANSLATION:
         for ( size_t i = 0; i < samp->l; i++ )
            node->nt.t.v[i] = samp->data[pi * samp->l + i] * ( 1. - mix ) +
                              samp->data[ni * samp->l + i] * mix;
         break;
      case ANIM_TYPE_SCALE:
         for ( size_t i = 0; i < samp->l; i++ )
            node->nt.s.v[i] = samp->data[pi * samp->l + i] * ( 1. - mix ) +
                              samp->data[ni * samp->l + i] * mix;
         break;
      }
   }
}

static void gltf_applyAnim( GltfObject *obj, GLfloat time )
{
   if ( obj->nanimations <= 0 )
      return;

   for ( size_t i = 0; i < obj->nnodes; i++ )
      obj->nodes[i].has_anim = 0;
   for ( size_t i = 0; i < obj->nanimations; i++ )
      gltf_applyAnimNode( obj, &obj->animations[i], time );
   for ( size_t i = 0; i < obj->nnodes; i++ ) {
      Node *node = &obj->nodes[i];
      if ( !node->has_anim )
         continue;
      mat4_trs( &node->H, &node->nt.t, &node->nt.r, &node->nt.s );
   }
}

/**
 * @brief Renders an object (with a transformation).
 *
 *    @param obj GltfObject to render.
 *    @param H Transformation to apply (or NULL to use identity).
 */
void gltf_render( GLuint fb, GltfObject *obj, const mat4 *H, GLfloat time,
                  double size )
{
   return gltf_renderScene( fb, obj, obj->scene_body, H, time, size, 0 );
}

void gltf_renderScene( GLuint fb, GltfObject *obj, int scene, const mat4 *H,
                       GLfloat time, double size, const Lighting *L )
{
   if ( scene < 0 )
      return;
   const GLfloat sca    = 1.0 / obj->radius;
   const mat4    Hscale = { .m = { { sca, 0.0, 0.0, 0.0 },
                                   { 0.0, sca, 0.0, 0.0 },
                                   { 0.0, 0.0, -sca, 0.0 },
                                   { 0.0, 0.0, 0.0, 1.0 } } };
   mat4          Hptr;

   /* Choose lighting stuff based on size. */
   if ( size > 255. ) {
      shadowmap_size = SHADOWMAP_SIZE_HIGH;
      light_fbo      = light_fbo_high;
      light_tex      = light_tex_high;
      shadow_fbo     = &shadow_fbo_high;
      shadow_tex     = &shadow_tex_high;
   } else {
      shadowmap_size = SHADOWMAP_SIZE_LOW;
      light_fbo      = light_fbo_low;
      light_tex      = light_tex_low;
      shadow_fbo     = &shadow_fbo_low;
      shadow_tex     = &shadow_tex_low;
   }

   /* Apply scaling. */
   if ( H == NULL )
      Hptr = Hscale;
   else {
      Hptr = *H;
      mat4_apply( &Hptr, &Hscale );
   }

   /* Do animations if applicable. */
   gltf_applyAnim( obj, time );

   if ( L == NULL ) {
      /* Use default light and shadow matrices. */
      L         = &L_default;
      light_mat = light_mat_def;
   } else {
      /* Compute shadow matrices. */
      for ( int i = 0; i < L->nlights; i++ )
         shadow_matrix( &light_mat_alt[i], &L->lights[i] );
      light_mat = light_mat_alt;
   }

   /* Set up blend mode. */
   glEnable( GL_BLEND );
   glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                        GL_ONE_MINUS_SRC_ALPHA );

   /* Depth testing. */
   glEnable( GL_DEPTH_TEST );
   glDepthFunc( GL_LESS );

   /* Render shadows for each light. */
   glCullFace( GL_FRONT );
   for ( int i = 0; i < L->nlights; i++ )
      gltf_renderShadow( obj, scene, &Hptr, &L->lights[i], i );

   /* Finally render the scene. */
   glViewport( 0, 0, size, size );
   glBindFramebuffer( GL_FRAMEBUFFER, fb );
   gltf_renderMesh( obj, scene, &Hptr, L );

   /* Some clean up. */
   glDisable( GL_CULL_FACE );
   glDisable( GL_DEPTH_TEST );
   glUseProgram( 0 );
#ifdef HAVE_NAEV
   glViewport( 0, 0, gl_screen.rw, gl_screen.rh );
#endif /* HAVE_NAEV */
}

static cgltf_result
gltf_read( const struct cgltf_memory_options *memory_options,
           const struct cgltf_file_options *file_options, const char *path,
           cgltf_size *size, void **data )
{
   (void)file_options;
   PHYSFS_Stat path_stat;

   void *( *memory_alloc )( void *, cgltf_size ) =
      memory_options->alloc_func ? memory_options->alloc_func
                                 : &cgltf_default_alloc;
   void ( *memory_free )( void *, void * ) = memory_options->free_func
                                                ? memory_options->free_func
                                                : &cgltf_default_free;

   if ( !PHYSFS_stat( path, &path_stat ) ) {
      WARN( _( "File '%s' not found!" ), path );
      return cgltf_result_file_not_found;
   }

   cgltf_size file_size = size ? *size : 0;
   if ( file_size == 0 )
      file_size = path_stat.filesize;

   char *file_data =
      (char *)memory_alloc( memory_options->user_data, file_size );
   if ( !file_data )
      return cgltf_result_out_of_memory;

   PHYSFS_file *pfile = PHYSFS_openRead( path );
   if ( pfile == NULL ) {
      WARN( _( "Unable to open '%s' for reading: %s" ), path,
            PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
      return cgltf_result_file_not_found;
   }
   cgltf_size read_size = PHYSFS_readBytes( pfile, file_data, file_size );
   PHYSFS_close( pfile );

   if ( read_size != file_size ) {
      memory_free( memory_options->user_data, file_data );
      return cgltf_result_io_error;
   }

   if ( size )
      *size = file_size;
   if ( data )
      *data = file_data;

   return cgltf_result_success;
}

/* TODO replace _Thread_local and friends with SDL_qsort_r when we switch to
 * SDL3. */
static _Thread_local const GltfObject *cmp_obj;
static int cmp_node( const void *p1, const void *p2 )
{
   const Node *n1 = &cmp_obj->nodes[*(size_t *)p1];
   const Node *n2 = &cmp_obj->nodes[*(size_t *)p2];
   int         b1 = 0;
   int         b2 = 0;
   int         b;
   if ( n1->mesh >= 0 ) {
      const Mesh *m = &cmp_obj->meshes[n1->mesh];
      for ( int i = 0; i < m->nprimitives; i++ ) {
         int mat = m->primitives[i].material;
         if ( mat >= 0 )
            b1 |= cmp_obj->materials[mat].blend;
      }
   }
   if ( n2->mesh >= 0 ) {
      const Mesh *m = &cmp_obj->meshes[n2->mesh];
      for ( int i = 0; i < m->nprimitives; i++ ) {
         int mat = m->primitives[i].material;
         if ( mat >= 0 )
            b2 |= cmp_obj->materials[mat].blend;
      }
   }
   b = b1 - b2;
   if ( b )
      return b;
   return 0;
}
static int cmp_mesh( const void *p1, const void *p2 )
{
   const MeshPrimitive *m1 = p1;
   const MeshPrimitive *m2 = p2;
   int b1 = ( m1->material >= 0 ) ? cmp_obj->materials[m1->material].blend : -1;
   int b2 = ( m2->material >= 0 ) ? cmp_obj->materials[m2->material].blend : -1;
   int b  = b1 - b2;
   if ( b )
      return b;
   return 0;
}
static int cmp_mount( const void *p1, const void *p2 )
{
   const GltfMount *m1 = p1;
   const GltfMount *m2 = p2;
   return m1->id - m2->id;
}
static int cmp_trail( const void *p1, const void *p2 )
{
   const GltfTrail *t1 = p1;
   const GltfTrail *t2 = p2;
   double           d  = t1->pos.v[1] - t2->pos.v[1];
   if ( d < 0. )
      return -1;
   else if ( d > 0. )
      return +1;
   return 0;
}

static const char *gltf_error_str( cgltf_result result )
{
   switch ( result ) {
   case cgltf_result_success:
      return p_( "cgltf", "success" );
   case cgltf_result_data_too_short:
      return p_( "cgltf", "data too short" );
   case cgltf_result_unknown_format:
      return p_( "cgltf", "unknown format" );
   case cgltf_result_invalid_json:
      return p_( "cgltf", "invalid json" );
   case cgltf_result_invalid_gltf:
      return p_( "cgltf", "invalid gltf" );
   case cgltf_result_invalid_options:
      return p_( "cgltf", "invalid options" );
   case cgltf_result_file_not_found:
      return p_( "cgltf", "file not found" );
   case cgltf_result_io_error:
      return p_( "cgltf", "io error" );
   case cgltf_result_out_of_memory:
      return p_( "cgltf", "out of memory" );
   case cgltf_result_legacy_gltf:
      return p_( "cgltf", "legacy gltf" );
   default:
      return NULL;
   }
}

/**
 * @brief Loads an object from a file.
 *
 *    @param filename Name of the file to load from.
 *    @return Newly loaded object file.
 */
GltfObject *gltf_loadFromFile( const char *filename )
{
   GltfObject   *obj;
   cgltf_result  res;
   cgltf_data   *data;
   cgltf_options opts;
   char         *dirpath;
   int new;
   memset( &opts, 0, sizeof( opts ) );

   /* Initialize object. */
   obj = cache_get( filename, &new );
   if ( !new )
      return obj;

   /* Set up the gltf path. */
   dirpath = strdup( filename );
#ifdef HAVE_NAEV
   SDL_asprintf( &obj->path, "%s", dirname( dirpath ) );
#else  /* HAVE_NAEV */
   char mountpath[PATH_MAX];
   snprintf( mountpath, sizeof( mountpath ), "%s/%s",
             PHYSFS_getRealDir( filename ), dirname( dirpath ) );
   PHYSFS_mount( mountpath, "/", 0 ); /* Prefix so more priority. */
   obj->path = strdup( mountpath );
#endif /* HAVE_NAEV */
   free( dirpath );

   /* Start loading the file. */
   opts.file.read = gltf_read;
   res            = cgltf_parse_file( &opts, filename, &data );
   if ( res != cgltf_result_success ) {
      WARN( _( "Error loading GLTF file '%s': %s" ), filename,
            gltf_error_str( res ) );
      return NULL;
   }

#if DEBUGGING
   /* Validate just in case. */
   res = cgltf_validate( data );
   if ( res != cgltf_result_success ) {
      WARN( _( "Error loading GLTF file '%s': %s" ), filename,
            gltf_error_str( res ) );
      return NULL;
   }
#endif /* DEBUGGING */

   /* Will load from PHYSFS. */
   res = cgltf_load_buffers( &opts, data, filename );
   if ( res != cgltf_result_success ) {
      WARN( _( "Error loading GLTF file '%s': %s" ), filename,
            gltf_error_str( res ) );
      return NULL;
   }

   /* Load materials. */
   obj->materials  = calloc( data->materials_count, sizeof( Material ) );
   obj->nmaterials = data->materials_count;
   for ( size_t i = 0; i < data->materials_count; i++ )
      gltf_loadMaterial( obj, &obj->materials[i], &data->materials[i], data );

   /* Load nodes. */
   obj->nodes  = calloc( data->nodes_count, sizeof( Node ) );
   obj->nnodes = data->nodes_count;
   for ( size_t n = 0; n < obj->nnodes; n++ ) {
      const cgltf_node *cnode = &data->nodes[n];
      Node             *node  = &obj->nodes[n];
      gltf_loadNode( obj, data, node, cnode );
   }

   /* Load animations, has to be before meshes so we can update rotation
    * information. */
   obj->animations  = calloc( data->animations_count, sizeof( Animation ) );
   obj->nanimations = data->animations_count;
   for ( size_t i = 0; i < obj->nanimations; i++ )
      gltf_loadAnimation( obj, data, &obj->animations[i],
                          &data->animations[i] );
   gltf_applyAnim(
      obj,
      0. ); /* Initial propagation of transformations for determining size. */

   /* Load meshes, has to be after nodes so we can fill information backwards.
    */
   obj->meshes  = calloc( data->meshes_count, sizeof( Mesh ) );
   obj->nmeshes = data->meshes_count;
   for ( size_t i = 0; i < data->meshes_count; i++ )
      gltf_loadMesh( obj, data, &obj->meshes[i], &data->meshes[i] );

   /* Load scenes. */
   obj->scenes       = calloc( data->scenes_count, sizeof( Scene ) );
   obj->nscenes      = data->scenes_count;
   obj->scene_body   = 0; /**< Always the default scene. */
   obj->scene_engine = -1;
   for ( size_t s = 0; s < obj->nscenes; s++ ) {
      /* Load nodes. */
      const cgltf_scene *cscene =
         &data->scenes[s]; /* data->scene may be NULL */
      Scene *scene = &obj->scenes[s];
      if ( cscene->name != NULL ) {
         scene->name = strdup( cscene->name );
         if ( strcmp( scene->name, "body" ) == 0 )
            obj->scene_body = s;
         else if ( strcmp( scene->name, "engine" ) == 0 )
            obj->scene_engine = s;
      }

      /* Set up and allocate scene. */
      scene->nodes  = calloc( cscene->nodes_count, sizeof( size_t ) );
      scene->nnodes = cscene->nodes_count;
      for ( size_t i = 0; i < scene->nnodes; i++ )
         scene->nodes[i] = cgltf_node_index( data, cscene->nodes[i] );
   }

   /* Get true radius. */
   for ( size_t i = 0; i < obj->nnodes; i++ )
      obj->radius = MAX( obj->radius, obj->nodes[i].radius );

   /* Sort stuff afterwards so we can lock it. */
   cmp_obj = obj; /* For comparisons. */
   for ( size_t i = 0; i < obj->nmeshes; i++ ) {
      Mesh *m = &obj->meshes[i];
      qsort( m->primitives, m->nprimitives, sizeof( MeshPrimitive ), cmp_mesh );
   }
   for ( size_t s = 0; s < obj->nscenes; s++ ) {
      Scene *scene = &obj->scenes[s];
      qsort( scene->nodes, scene->nnodes, sizeof( size_t ), cmp_node );
   }
   cmp_obj = NULL; /* No more comparisons. */

   /* Some post-processing. */
   /* Sort trails from lowest to highest so they get properly ordered in-game.
    */
   qsort( obj->trails, array_size( obj->trails ), sizeof( GltfTrail ),
          cmp_trail );
   for ( int i = 0; i < array_size( obj->trails ); i++ ) {
      GltfTrail *t = &obj->trails[i];
      vec3_scale( &t->pos, 1. / obj->radius );
   }
   /* Sort mounts to match ids. */
   qsort( obj->mounts, array_size( obj->mounts ), sizeof( GltfMount ),
          cmp_mount );
   for ( int i = 0; i < array_size( obj->mounts ); i++ ) {
      GltfMount *m = &obj->mounts[i];
      vec3_scale( &m->pos, 1. / obj->radius );
      if ( m->id != i )
         WARN( _( "gltf warning '%s': expected mount with id=%d, but got %d!" ),
               filename, i, m->id );
   }

#ifndef HAVE_NAEV
   LOG( "Loaded %s", filename );
   LOG( "   has %d trail generators", array_size( obj->trails ) );
   LOG( "   has %d weapon mounts", array_size( obj->mounts ) );

   PHYSFS_unmount( obj->path );
#endif /* HAVE_NAEV */

   cgltf_free( data );

   return obj;
}

static void gltf_freeMesh( Mesh *mesh )
{
   for ( int i = 0; i < mesh->nprimitives; i++ ) {
      MeshPrimitive *mp = &mesh->primitives[i];
      if ( mp->vbo_idx )
         glDeleteBuffers( 1, &mp->vbo_idx );
      if ( mp->vbo_pos )
         glDeleteBuffers( 1, &mp->vbo_pos );
      if ( mp->vbo_nor )
         glDeleteBuffers( 1, &mp->vbo_nor );
      if ( mp->vbo_tex0 )
         glDeleteBuffers( 1, &mp->vbo_tex0 );
      if ( mp->vbo_tex1 )
         glDeleteBuffers( 1, &mp->vbo_tex1 );
   }
   free( mesh->primitives );
   gl_checkErr();
}

static void gltf_freeNode( Node *node )
{
   free( node->name );
   free( node->children );
}

static void gltf_freeAnimation( Animation *anim )
{
   free( anim->name );
   for ( size_t i = 0; i < anim->nsamplers; i++ ) {
      AnimationSampler *samp = &anim->samplers[i];
      free( samp->time );
      free( samp->data );
   }
   free( anim->samplers );
   free( anim->channels );
}

static void gltf_freeTex( Texture *tex )
{
   /* Don't have to free default textures. */
   if ( ( tex == NULL ) || ( tex->tex == tex_zero.tex ) ||
        ( tex->tex == tex_ones.tex ) )
      return;

#ifdef HAVE_NAEV
   gl_freeTexture( tex->gtex ); /* Frees texture too. */
#else                           /* HAVE_NAEV */
   glDeleteTextures( 1, &tex->tex );
#endif                          /* HAVE_NAEV */
   gl_checkErr();
}

void gltf_free( GltfObject *obj )
{
   int c;
   if ( obj == NULL )
      return;

   /* Try to get from cache, if refcount > 0 then it'll be 0. */
   c = cache_dec( obj );
   if ( c == 0 )
      return;

   free( obj->path );

   for ( size_t i = 0; i < obj->nmeshes; i++ ) {
      Mesh *mesh = &obj->meshes[i];
      gltf_freeMesh( mesh );
   }
   free( obj->meshes );

   for ( size_t i = 0; i < obj->nnodes; i++ ) {
      Node *node = &obj->nodes[i];
      gltf_freeNode( node );
   }
   free( obj->nodes );

   for ( size_t i = 0; i < obj->nanimations; i++ ) {
      Animation *anim = &obj->animations[i];
      gltf_freeAnimation( anim );
   }
   free( obj->animations );

   for ( size_t s = 0; s < obj->nscenes; s++ ) {
      Scene *scene = &obj->scenes[s];
      free( scene->name );
      free( scene->nodes );
   }
   free( obj->scenes );

   for ( size_t i = 0; i < obj->nmaterials; i++ ) {
      Material *m = &obj->materials[i];
      gltf_freeTex( &m->baseColour_tex );
      gltf_freeTex( &m->metallic_tex );
      gltf_freeTex( &m->normal_tex );
      gltf_freeTex( &m->occlusion_tex );
      gltf_freeTex( &m->emissive_tex );
   }
   free( obj->materials );

   /* Clean up extras. */
   for ( int i = 0; i < array_size( obj->trails ); i++ ) {
      GltfTrail *t = &obj->trails[i];
      free( t->generator );
   }
   array_free( obj->trails );
   array_free( obj->mounts );

   free( obj );
}

int gltf_init( void )
{
   const GLubyte data_zero[4] = { 0, 0, 0, 0 };
   const GLubyte data_ones[4] = { 255, 255, 255, 255 };
   const GLfloat b[4]         = { 1., 1., 1., 1. };
   GLenum        status;
   Shader       *shd;
   const char   *prepend_fix = "#define MAX_LIGHTS " STR( MAX_LIGHTS ) "\n";
   char          prepend[STRMAX];

   cache_lock = SDL_CreateMutex();

   /* Set up default lighting. */
   L_default = L_default_const;
   for ( int i = 0; i < L_default.nlights; i++ )
      shadow_matrix( &light_mat_def[i], &L_default.lights[i] );

   /* Set global options. */
   use_normal_mapping    = !conf.low_memory;
   use_ambient_occlusion = !conf.low_memory;
   max_tex_size          = ( conf.low_memory ? conf.max_3d_tex_size : 0 );

   /* Set prefix stuff. */
   snprintf( prepend, sizeof( prepend ),
             "%s\n#define HAS_NORMAL %d\n#define HAS_AO %d\n", prepend_fix,
             use_normal_mapping, use_ambient_occlusion );

   /* Load textures. */
   glGenTextures( 1, &tex_zero.tex );
   glBindTexture( GL_TEXTURE_2D, tex_zero.tex );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 data_zero );
   glGenTextures( 1, &tex_ones.tex );
   glBindTexture( GL_TEXTURE_2D, tex_ones.tex );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 data_ones );
   glBindTexture( GL_TEXTURE_2D, 0 );
   gl_checkErr();

   /* Set up default material. */
   gltf_loadMaterial( NULL, &material_default, NULL, NULL );

   /* We'll have to set up some rendering stuff for blurring purposes. */
   const GLfloat vbo_data[8] = { 0., 0., 1., 0., 0., 1., 1., 1. };
   glGenBuffers( 1, &shadow_vbo );
   glBindBuffer( GL_ARRAY_BUFFER, shadow_vbo );
   glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * 8, vbo_data,
                 GL_STATIC_DRAW );
   glBindBuffer( GL_ARRAY_BUFFER, 0 );

   for ( int l = 0; l < 2; l++ ) {
      if ( l == 0 ) {
         shadowmap_size = SHADOWMAP_SIZE_LOW;
         light_fbo      = light_fbo_low;
         light_tex      = light_tex_low;
         shadow_fbo     = &shadow_fbo_low;
         shadow_tex     = &shadow_tex_low;
      } else {
         shadowmap_size = SHADOWMAP_SIZE_HIGH;
         light_fbo      = light_fbo_high;
         light_tex      = light_tex_high;
         shadow_fbo     = &shadow_fbo_high;
         shadow_tex     = &shadow_tex_high;
      }

      /* Gen the texture. */
      glGenTextures( 1, shadow_tex );
      glBindTexture( GL_TEXTURE_2D, *shadow_tex );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, shadowmap_size,
                    shadowmap_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
      glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, b );
      glBindTexture( GL_TEXTURE_2D, 0 );
      /* Set up shadow buffer FBO. */
      glGenFramebuffers( 1, shadow_fbo );
      glBindFramebuffer( GL_FRAMEBUFFER, *shadow_fbo );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_TEXTURE_2D, *shadow_tex, 0 );
      glDrawBuffer( GL_NONE );
      glReadBuffer( GL_NONE );
      glBindFramebuffer( GL_FRAMEBUFFER, 0 );
      status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
      if ( status != GL_FRAMEBUFFER_COMPLETE )
         WARN( _( "Error setting up shadowmap framebuffer!" ) );

      /* Now generate the light maps .*/
      glGenTextures( MAX_LIGHTS, light_tex );
      glGenFramebuffers( MAX_LIGHTS, light_fbo );
      for ( int i = 0; i < MAX_LIGHTS; i++ ) {
         /* Set up shadow buffer depth tex. */
         glBindTexture( GL_TEXTURE_2D, light_tex[i] );
         glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, shadowmap_size,
                       shadowmap_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                          GL_CLAMP_TO_BORDER );
         glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                          GL_CLAMP_TO_BORDER );
         glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, b );
         glBindTexture( GL_TEXTURE_2D, 0 );
         /* Set up shadow buffer FBO. */
         glBindFramebuffer( GL_FRAMEBUFFER, light_fbo[i] );
         glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                 GL_TEXTURE_2D, light_tex[i], 0 );
         glDrawBuffer( GL_NONE );
         glReadBuffer( GL_NONE );
         glBindFramebuffer( GL_FRAMEBUFFER, 0 );
         status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
         if ( status != GL_FRAMEBUFFER_COMPLETE )
            WARN( _( "Error setting up shadowmap framebuffer!" ) );
      }
   }

   /* Compile the shadow shader. */
   shd = &shadow_shader;
   shd->program =
      gl_program_backend( "shadow.vert", "shadow.frag", NULL, prepend );
   if ( shd->program == 0 )
      return -1;
   glUseProgram( shd->program );
   /* Attributes. */
   shd->vertex = glGetAttribLocation( shd->program, "vertex" );
   /* Vertex uniforms. */
   shd->Hshadow = glGetUniformLocation( shd->program, "u_shadow" );
   shd->Hmodel  = glGetUniformLocation( shd->program, "u_model" );

   /* Compile the X blur shader. */
   shd = &shadow_shader_blurX;
   shd->program =
      gl_program_backend( "blur.vert", "blurX.frag", NULL, prepend );
   if ( shd->program == 0 )
      return -1;
   glUseProgram( shd->program );
   /* Attributes. */
   shd->vertex = glGetAttribLocation( shd->program, "vertex" );
   /* Uniforms. */
   shd->baseColour_tex = glGetUniformLocation( shd->program, "sampler" );
   glUniform1i( shd->baseColour_tex, 0 );

   /* Compile the Y blur shader. */
   shd = &shadow_shader_blurY;
   shd->program =
      gl_program_backend( "blur.vert", "blurY.frag", NULL, prepend );
   if ( shd->program == 0 )
      return -1;
   glUseProgram( shd->program );
   /* Attributes. */
   shd->vertex = glGetAttribLocation( shd->program, "vertex" );
   /* Uniforms. */
   shd->baseColour_tex = glGetUniformLocation( shd->program, "sampler" );

   /* Compile the shader. */
   shd = &gltf_shader;
   shd->program =
      gl_program_backend( "gltf.vert", "gltf_pbr.frag", NULL, prepend );
   if ( shd->program == 0 )
      return -1;
   glUseProgram( shd->program );
   /* Attributes. */
   shd->vertex        = glGetAttribLocation( shd->program, "vertex" );
   shd->vertex_normal = glGetAttribLocation( shd->program, "vertex_normal" );
   shd->vertex_tex0   = glGetAttribLocation( shd->program, "vertex_tex0" );
   shd->vertex_tex1   = glGetAttribLocation( shd->program, "vertex_tex1" );
   /* Vertex uniforms. */
   shd->Hmodel  = glGetUniformLocation( shd->program, "u_model" );
   shd->Hnormal = glGetUniformLocation( shd->program, "u_normal" );
   /* Fragment uniforms. */
   shd->blend          = glGetUniformLocation( shd->program, "u_blend" );
   shd->u_ambient      = glGetUniformLocation( shd->program, "u_ambient" );
   shd->baseColour_tex = glGetUniformLocation( shd->program, "baseColour_tex" );
   shd->baseColour_texcoord =
      glGetUniformLocation( shd->program, "baseColour_texcoord" );
   shd->metallic_tex = glGetUniformLocation( shd->program, "metallic_tex" );
   shd->metallic_texcoord =
      glGetUniformLocation( shd->program, "metallic_texcoord" );
   if ( use_normal_mapping ) {
      shd->u_has_normal = glGetUniformLocation( shd->program, "u_has_normal" );
      shd->normal_tex   = glGetUniformLocation( shd->program, "normal_tex" );
      shd->normal_texcoord =
         glGetUniformLocation( shd->program, "normal_texcoord" );
      shd->normal_scale = glGetUniformLocation( shd->program, "normal_scale" );
   }
   shd->metallicFactor = glGetUniformLocation( shd->program, "metallicFactor" );
   shd->roughnessFactor =
      glGetUniformLocation( shd->program, "roughnessFactor" );
   shd->baseColour = glGetUniformLocation( shd->program, "baseColour" );
   shd->sheenTint  = glGetUniformLocation( shd->program, "sheenTint" );
   shd->sheen      = glGetUniformLocation( shd->program, "sheen" );
   shd->clearcoat  = glGetUniformLocation( shd->program, "clearcoat" );
   shd->clearcoat_roughness =
      glGetUniformLocation( shd->program, "clearcoat_roughness" );
   shd->emissive = glGetUniformLocation( shd->program, "emissive" );
   if ( use_ambient_occlusion ) {
      shd->occlusion_tex =
         glGetUniformLocation( shd->program, "occlusion_tex" );
      shd->occlusion_texcoord =
         glGetUniformLocation( shd->program, "occlusion_texcoord" );
   }
   shd->emissive_tex = glGetUniformLocation( shd->program, "emissive_tex" );
   shd->emissive_texcoord =
      glGetUniformLocation( shd->program, "emissive_texcoord" );
   /* Special. */
   // shd->waxiness        = glGetUniformLocation( shd->program, "u_waxiness" );
   /* Lights. */
   for ( int i = 0; i < MAX_LIGHTS; i++ ) {
      ShaderLight *sl = &shd->lights[i];
      char         buf[128];
      snprintf( buf, sizeof( buf ), "u_lights[%d].position", i );
      sl->position = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof( buf ), "u_lights[%d].sun", i );
      sl->sun = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof( buf ), "u_lights[%d].colour", i );
      sl->colour = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof( buf ), "u_lights[%d].intensity", i );
      sl->intensity = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof( buf ), "shadowmap_tex[%d]", i );
      sl->shadowmap_tex = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof( buf ), "u_shadow[%d]", i );
      sl->Hshadow = glGetUniformLocation( shd->program, buf );
   }
   shd->nlights = glGetUniformLocation( shd->program, "u_nlights" );

   glUseProgram( 0 );
   /* Light default values set on init. */
   gl_checkErr();

   return 0;
}

void gltf_exit( void )
{
   /* Not initialized. */
   if ( tex_zero.tex == 0 )
      return;

   SDL_DestroyMutex( cache_lock );
   for ( int i = 0; i < array_size( obj_cache ); i++ ) {
      WARN( _( "Object Cache '%s' not properly freed (refcount=%d)!" ),
            obj_cache[i].name, obj_cache[i].refcount );
      free( obj_cache->name );
   }
   array_free( obj_cache );

   glDeleteBuffers( 1, &shadow_vbo );
   glDeleteTextures( 1, &shadow_tex_high );
   glDeleteTextures( 1, &shadow_tex_low );
   glDeleteFramebuffers( 1, &shadow_fbo_high );
   glDeleteFramebuffers( 1, &shadow_fbo_low );
   glDeleteTextures( MAX_LIGHTS, light_tex_low );
   glDeleteFramebuffers( MAX_LIGHTS, light_fbo_low );
   glDeleteTextures( MAX_LIGHTS, light_tex_high );
   glDeleteFramebuffers( MAX_LIGHTS, light_fbo_high );
   glDeleteTextures( 1, &tex_zero.tex );
   glDeleteTextures( 1, &tex_ones.tex );
   glDeleteProgram( gltf_shader.program );
   glDeleteProgram( shadow_shader.program );
}

/**
 * @brief Resets the lighting settings.
 */
void gltf_lightReset( void )
{
   L_default = L_default_const;
}

/**
 * @brief Sets a light.
 *
 *    @param idx Index to use, 0 refers to first light after the default.
 *    @param L Light to set.
 */
int gltf_lightSet( int idx, const Light *L )
{
   int n = L_default_const.nlights +
           idx; /* We start counting after the default lights. */
   if ( n >= MAX_LIGHTS ) {
      WARN( _( "Trying to set more lights than MAX_LIGHTS allows!" ) );
      return -1;
   }
   L_default.nlights   = MAX( L_default.nlights, n + 1 );
   L_default.lights[n] = *L;
   shadow_matrix( &light_mat_def[n], &L_default.lights[n] );
   return 0;
}

/**
 * @brief Sets the ambient colour. Should be multiplied by intensity.
 */
void gltf_lightAmbient( double r, double g, double b )
{
   const double factor = 1.0 / M_PI;
   L_default.ambient_r = r * factor;
   L_default.ambient_g = g * factor;
   L_default.ambient_b = b * factor;
}

/**
 * @brief Gets the ambient light values.
 */
void gltf_lightAmbientGet( double *r, double *g, double *b )
{
   *r = L_default.ambient_r * M_PI;
   *g = L_default.ambient_g * M_PI;
   *b = L_default.ambient_b * M_PI;
}

/**
 * @brief Sets the general light intensity.
 */
void gltf_lightIntensity( double strength )
{
   L_default.intensity = strength;
}

/**
 * @brief Gets the general light intensity.
 */
double gltf_lightIntensityGet( void )
{
   return L_default.intensity;
}

/**
 * @brief Transforms the lighting positions based on a trasnnform matrix.
 */
void gltf_lightTransform( Lighting *L, const mat4 *H )
{
   for ( int i = 0; i < L->nlights; i++ ) {
      Light *l = &L->lights[i];
      if ( l->sun ) {
         mat3 h;
         vec3 v = l->pos;
         mat3_from_mat4( &h, H );
         mat3_mul_vec( &l->pos, &h, &v );
      } else {
         vec3 v = l->pos;
         mat4_mul_vec( &l->pos, H, &v );
      }
   }
}

/**
 * @brief Gets the shadow map texture.
 */
GLuint gltf_shadowmap( int light )
{
   return light_tex[light];
}

/**
 * @brief Checks to see if two caches are the same.
 */
static int cache_cmp( const void *p1, const void *p2 )
{
   const ObjectCache *o1 = p1;
   const ObjectCache *o2 = p2;
   return strcmp( o1->name, o2->name );
}

/**
 * @brief Gets a new object from cache, allocating as necessary.
 *
 *    @param filename Name of the file to load.
 *    @param[out] new Set to 1 when a new object is created, and 0 when reused.
 *    @return Object associated to the cache.
 */
static GltfObject *cache_get( const char *filename, int *new )
{
   SDL_mutexP( cache_lock );
   const ObjectCache cache = {
      .name = (char *)filename,
   };
   ObjectCache *hit = bsearch( &cache, obj_cache, array_size( obj_cache ),
                               sizeof( ObjectCache ), cache_cmp );
   if ( hit == NULL ) {
      ObjectCache newcache = {
         .name     = strdup( filename ),
         .obj      = calloc( 1, sizeof( GltfObject ) ),
         .refcount = 1,
      };
      if ( obj_cache == NULL )
         obj_cache = array_create( ObjectCache );
      array_push_back( &obj_cache, newcache );
      qsort( obj_cache, array_size( obj_cache ), sizeof( ObjectCache ),
             cache_cmp );
      *new = 1;
      SDL_mutexV( cache_lock );
      return newcache.obj;
   }
   hit->refcount++;
   *new = 0;
   SDL_mutexV( cache_lock );
   return hit->obj;
}

/**
 * @brief Decerments the refcount for a cached object and sees if it should be
 * freed.
 *
 *    @param obj Object to check to see if it should be freed.
 *    @return Whether or not it should be freed.
 */
static int cache_dec( GltfObject *obj )
{
   SDL_mutexP( cache_lock );
   for ( int i = 0; i < array_size( obj_cache ); i++ ) {
      ObjectCache *c = &obj_cache[i];
      if ( c->obj != obj )
         continue;

      c->refcount--;
      if ( c->refcount <= 0 ) {
         free( c->name );
         array_erase( &obj_cache, &c[0], &c[1] );
         SDL_mutexV( cache_lock );
         return 1;
      }
      SDL_mutexV( cache_lock );
      return 0;
   }
   WARN( _( "GltfObject '%s' not found in cache!" ), obj->path );
   SDL_mutexV( cache_lock );
   return 0;
}
