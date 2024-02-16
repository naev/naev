#include "gltf.h"

/* We use this file in utils/model-view-c to debug things. */
#ifdef PACKAGE
#define HAVE_NAEV
#endif

#include "glad.h"
#include "SDL_image.h"
#include <assert.h>
#include <math.h>
#include <libgen.h>

#include "physfsrwops.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#ifdef HAVE_NAEV
#include "naev.h"
#include "opengl_shader.h"
#include "conf.h"
#else /* HAVE_NAEV */
#include "common.h"
#include "shader_min.h"
#define gl_contextSet()
#define gl_contextUnset()
#endif /* HAVE_NAEV */
#include "vec3.h"

/* Horrible hack that turns a variable name into a string. */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

static Material material_default;

/**
 * @brief Simple point light model for shaders.
 */
typedef struct ShaderLight_ {
   GLuint Hshadow;   /* mat4 */
   GLuint sun;       /* bool */
   GLuint position;  /* vec3 */
   GLuint colour;    /* vec3 */
   GLuint intensity; /* float */
   GLuint shadowmap_tex; /* sampler2D */
} ShaderLight;

/**
 * @brief Simple point light model.
 */
typedef struct Light_ {
   int sun;          /**< Whether or not it's a sun-type light source. */
   vec3 pos;         /**< Position of the light in normalized coordinates. */
   double intensity; /**< Radiosity of the lights. */
   vec3 colour;      /**< Light colour. */
   /* Below are added automatically. */
   GLuint fbo;       /**< FBO correpsonding to the light. */
   GLuint tex;       /**< Texture corresponding to the light. */
} Light;

/* left(-)/right(+), down(-)/up(+), forward(-)/back(+) */
#if 0
static Light lights[MAX_LIGHTS] = {
   {
      .sun = 1,
      .pos = { .v = {1., 0.5, 1.0} },
      .colour = { .v = {1., 1., 1.} },
      .intensity = 1.5,
   },
   {
      .sun = 0,
      .pos = { .v = {-6., 5.5, -5.} },
      .colour = { .v = {0.7, 0.85, 1.} },
      .intensity = 600.,
   },
};
#else
static Light lights[MAX_LIGHTS] = {
   {
      .pos = { .v = {5., 2., 0.} },
      .colour = { .v = {1., 1., 1.} },
      .intensity = 150.,
   },
   {
      .pos = { .v = {0.5, 0.15, -2.0} },
      .colour = { .v = {1., 1., 1.} },
      .intensity = 10.,
   },
   {
      .pos = { .v = {-0.5, 0.15, -2.0} },
      .colour = { .v = {1., 1., 1.} },
      .intensity = 10.,
   },
};
#endif
static double light_intensity = 1.;
static vec3 ambient = { .v = {0., 0., 0.} };

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
   GLuint Hshadow;
   GLuint u_time;
   /* Fragment uniforms. */
   GLuint baseColour_tex;
   GLuint baseColour_texcoord;
   GLuint metallic_tex;
   GLuint metallic_texcoord;
   GLuint u_has_normal;
   GLuint normal_tex;
   GLuint normal_texcoord;
   GLuint normal_scale;
   GLuint metallicFactor;
   GLuint roughnessFactor;
   GLuint baseColour;
   GLuint sheenTint;
   GLuint sheen;
   GLuint clearcoat;
   GLuint clearcoat_roughness;
   GLuint emissive;
   GLuint emissive_tex;
   GLuint emissive_texcoord;
   GLuint occlusion_tex;
   GLuint occlusion_texcoord;
   ShaderLight lights[MAX_LIGHTS];
   GLuint nlights;
   GLuint blend;
   GLuint u_ambient;
   /* Custom Naev. */
   //GLuint waxiness;
} Shader;
static Shader object_shader;
static Shader shadow_shader;
static Texture tex_zero = {.tex=0, .texcoord=0, .strength=1.}; /* Used to detect initialization for now. */
static Texture tex_ones = {.tex=0, .texcoord=0, .strength=1.};

/* Below here are for blurring purposes. */
static GLuint shadow_vbo;
static GLuint shadow_fbo;
static GLuint shadow_tex;
static Shader shadow_shader_blurX;
static Shader shadow_shader_blurY;

/* Options. */
static int use_normal_mapping = 1;
static int use_ambient_occlusion = 1;

/*
 * Prototypes.
 */
static void object_lightSetup( int doambient, int dogeneral );

/**
 * @brief Loads a texture if applicable, uses default value otherwise.
 *
 *    @param otex Texture to output to.
 *    @param ctex Texture to load.
 *    @param def Default texture to use if not defined.
 *    @param notsrgb Whether or not the texture should use SRGB.
 *    @return OpenGL ID of the new texture.
 */
static int object_loadTexture( Texture *otex, const cgltf_texture_view *ctex, const Texture *def, int notsrgb )
{
   const SDL_PixelFormatEnum fmt = SDL_PIXELFORMAT_ABGR8888;
   GLuint tex;
   SDL_Surface *surface = NULL;

   /* Must haev texture to load it. */
   if ((ctex==NULL) || (ctex->texture==NULL)) {
      *otex = *def;
      return 0;
   }

   /* Load from path. */
   if (ctex->texture->image->uri != NULL) {
      SDL_RWops *rw = PHYSFSRWOPS_openRead( ctex->texture->image->uri );
      if (rw==NULL) {
         WARN(_("Unable to open '%s': %s"), ctex->texture->image->uri, SDL_GetError() );
         *otex = *def;
         return 0;
      }
      surface = IMG_Load_RW( rw, 1 );
      if (surface==NULL) {
         WARN(_("Unable to load surface '%s': %s"), ctex->texture->image->uri, SDL_GetError());
         *otex = *def;
         return 0;
      }
   }

   glGenTextures( 1, &tex );
   glBindTexture( GL_TEXTURE_2D, tex );

   /* Set stuff. */
   if (ctex->texture->sampler != NULL) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ctex->texture->sampler->mag_filter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ctex->texture->sampler->min_filter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ctex->texture->sampler->wrap_s);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ctex->texture->sampler->wrap_t);
   }
   else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   }

   if (surface != NULL) {
      int has_alpha = surface->format->Amask;
      if (surface->format->format != fmt) {
         SDL_Surface *temp = surface;
         surface = SDL_ConvertSurfaceFormat( temp, fmt, 0 );
         SDL_FreeSurface( temp );
      }

      SDL_LockSurface( surface );
      glPixelStorei( GL_UNPACK_ALIGNMENT, MIN( surface->pitch & -surface->pitch, 8 ) );
      if (notsrgb)
         glTexImage2D( GL_TEXTURE_2D, 0, has_alpha ? GL_RGBA : GL_RGB,
               surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
      else
         glTexImage2D( GL_TEXTURE_2D, 0, has_alpha ? GL_SRGB_ALPHA : GL_SRGB,
               surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
      glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
      SDL_UnlockSurface( surface );
   }
   else {
      DEBUG("Buffer textures not supported yet!");
      /*
      glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB_ALPHA,
            surface->w, surface->h, 0,
            surface->format->Amask ? GL_RGBA : GL_RGB,
            GL_UNSIGNED_BYTE, surface->pixels );
      */
   }

   /* Set up mipmaps. */
   /* TODO only generate if necessary. */
   glGenerateMipmap(GL_TEXTURE_2D);

   /* Free the surface. */
   SDL_FreeSurface( surface );

   glBindTexture( GL_TEXTURE_2D, 0 );

   gl_checkErr();

   otex->tex = tex;
   otex->texcoord = ctex->texcoord;
   otex->strength = ctex->transform.scale[0];

   return 0;
}

/**
 * @brief Loads a material for the object.
 */
static int object_loadMaterial( Material *mat, const cgltf_material *cmat, const cgltf_data *data )
{
   const GLfloat white[4] = { 1., 1., 1., 1. };
   /* TODO complete this. */
   if (cmat && cmat->has_pbr_metallic_roughness) {
      mat->metallicFactor  = cmat->pbr_metallic_roughness.metallic_factor;
      mat->roughnessFactor = cmat->pbr_metallic_roughness.roughness_factor;
      object_loadTexture( &mat->baseColour_tex, &cmat->pbr_metallic_roughness.base_color_texture, &tex_ones, 0 );
      if (mat->baseColour_tex.tex == tex_ones.tex)
         memcpy( mat->baseColour, cmat->pbr_metallic_roughness.base_color_factor, sizeof(mat->baseColour) );
      else
         memcpy( mat->baseColour, white, sizeof(mat->baseColour) );
      object_loadTexture( &mat->metallic_tex, &cmat->pbr_metallic_roughness.metallic_roughness_texture, &tex_ones, 1 );
   }
   else {
      memcpy( mat->baseColour, white, sizeof(mat->baseColour) );
      mat->metallicFactor  = 0.;
      mat->roughnessFactor = 1.;
      mat->baseColour_tex  = tex_ones;
      mat->metallic_tex    = tex_ones;
      mat->normal_tex      = tex_zero;
   }

   /* Sheen. */
   if (cmat && cmat->has_sheen) {
      memcpy( mat->sheen, cmat->sheen.sheen_color_factor, sizeof(mat->sheen) );
      mat->sheen_roughness = cmat->sheen.sheen_roughness_factor;
   }
   else {
      memset( mat->sheen, 0, sizeof(mat->sheen) );
      mat->sheen_roughness = 0.;
   }

   /* Handle clearcoat. */
   if (cmat && cmat->has_clearcoat) {
      mat->clearcoat = cmat->clearcoat.clearcoat_factor;
      mat->clearcoat_roughness = cmat->clearcoat.clearcoat_roughness_factor;
   }
   else {
      mat->clearcoat = 0.;
      mat->clearcoat_roughness = 0.;
   }

   /* Handle emissiveness and such. */
   if (cmat) {
      memcpy( mat->emissiveFactor, cmat->emissive_factor, sizeof(GLfloat)*3 );
      object_loadTexture( &mat->emissive_tex, &cmat->emissive_texture, &tex_ones, 0 );
      if (use_ambient_occlusion)
         object_loadTexture( &mat->occlusion_tex, &cmat->occlusion_texture, &tex_ones, 1 );
      else
         mat->occlusion_tex = tex_ones;
      if (use_normal_mapping)
         object_loadTexture( &mat->normal_tex, &cmat->normal_texture, &tex_zero, 1 );
      else
         mat->normal_tex = tex_ones;
      mat->blend        = (cmat->alpha_mode == cgltf_alpha_mode_blend);
   }
   else {
      memset( mat->emissiveFactor, 0, sizeof(GLfloat)*3 );
      mat->emissive_tex    = tex_ones;
      mat->occlusion_tex   = tex_ones;
      mat->normal_tex      = tex_ones;
      mat->blend           = 0;
   }
   /* Emissive strength extension just multiplies the emissiveness. */
   if (cmat && cmat->has_emissive_strength) {
      for (int i=0; i<3; i++)
         mat->emissiveFactor[i] *= cmat->emissive_strength.emissive_strength;
   }

   mat->noshadows = 0;
   if (cmat && data) {
      char buf[STRMAX_SHORT];
      cgltf_size len = sizeof(buf);
      cgltf_copy_extras_json( data, &cmat->extras, buf, &len );
      jsmn_parser p;
      jsmntok_t t[16]; /* Max number of expected tokens. */
      jsmn_init(&p);
      int r = jsmn_parse( &p, buf, len, t, sizeof(t)/sizeof(jsmntok_t) );
      for (int j=0; j<r; j++) {
         jsmntok_t *tj = &t[j];
         const char *str = "NAEV_noShadows";
         if (strncmp( str, &buf[tj->start], MIN(strlen(str),(size_t)(tj->end-tj->start)) )==0) {
            if (j+1 >= r)
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
static GLuint object_loadVBO( const cgltf_accessor *acc, GLfloat *radius, vec3 *aabb_min, vec3 *aabb_max, const cgltf_node *cnode )
{
   GLuint vid;
   cgltf_size num = cgltf_accessor_unpack_floats( acc, NULL, 0 );
   cgltf_float *dat = malloc( sizeof(cgltf_float) * num );
   cgltf_accessor_unpack_floats( acc, dat, num );

   /* OpenGL magic. */
   glGenBuffers( 1, &vid );
   glBindBuffer( GL_ARRAY_BUFFER, vid );
   glBufferData( GL_ARRAY_BUFFER, sizeof(cgltf_float) * num, dat, GL_STATIC_DRAW );
   glBindBuffer( GL_ARRAY_BUFFER, 0 );

   /* If applicable, store some useful stuff. */
   if (radius != NULL) {
      mat4 H;
      cgltf_node_transform_local( cnode, H.ptr );
      *radius = 0.;
      memset( aabb_min, 0, sizeof(vec3) );
      memset( aabb_max, 0, sizeof(vec3) );
      for (unsigned int i=0; i<num; i+=3) {
         vec3 v, d;
         for (unsigned int j=0; j<3; j++)
            d.v[j] = dat[i+j];
         mat4_mul_vec( &v, &H, &d );
         for (unsigned int j=0; j<3; j++) {
            aabb_min->v[j] = MIN( v.v[j], aabb_min->v[j] );
            aabb_max->v[j] = MAX( v.v[j], aabb_max->v[j] );
         }
         *radius = MAX( *radius, vec3_length( &v ) );
      }
   }

   gl_checkErr();
   free( dat );
   return vid;
}

/**
 * @brief Loads a mesh for the object.
 */
static int object_loadNodeRecursive( cgltf_data *data, Node *node, const cgltf_node *cnode )
{
   cgltf_mesh *cmesh = cnode->mesh;
   /* Get transform for node. */
   cgltf_node_transform_local( cnode, node->H.ptr );
   //cgltf_node_transform_world( cnode, node->H.ptr );

   if (cmesh == NULL) {
      node->nmesh = 0;
   }
   else {
      /* Load meshes. */
      node->mesh = calloc( cmesh->primitives_count, sizeof(Mesh) );
      node->nmesh = cmesh->primitives_count;
      for (size_t i=0; i<cmesh->primitives_count; i++) {
         Mesh *mesh = &node->mesh[i];
         cgltf_primitive *prim = &cmesh->primitives[i];
         cgltf_accessor *acc = prim->indices;
         if (acc==NULL) {
            mesh->material = -1;
            continue;
         }

         cgltf_size num = cgltf_num_components(acc->type) * acc->count;
         GLuint *idx = malloc( sizeof(cgltf_uint) * num );
         for (size_t j=0; j<num; j++)
            cgltf_accessor_read_uint( acc, j, &idx[j], 1 );

         /* Check material. */
         if (prim->material != NULL)
            mesh->material = prim->material - data->materials;
         else
            mesh->material = -1;

         /* Store indices. */
         glGenBuffers( 1, &mesh->vbo_idx );
         glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->vbo_idx );
         glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(cgltf_uint) * num, idx, GL_STATIC_DRAW );
         mesh->nidx = acc->count;
         glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
         free( idx );
         gl_checkErr();

         for (size_t j=0; j<prim->attributes_count; j++) {
            cgltf_attribute *attr = &prim->attributes[j];
            switch (attr->type) {
               case cgltf_attribute_type_position:
                  mesh->vbo_pos = object_loadVBO( attr->data, &mesh->radius, &mesh->aabb_min, &mesh->aabb_max, cnode );
                  node->radius = MAX( node->radius, mesh->radius );
                  vec3_max( &node->aabb_max, &node->aabb_max, &mesh->aabb_max );
                  vec3_min( &node->aabb_min, &node->aabb_min, &mesh->aabb_min );
                  break;

               case cgltf_attribute_type_normal:
                  mesh->vbo_nor = object_loadVBO( attr->data, NULL, NULL, NULL, NULL );
                  break;

               case cgltf_attribute_type_texcoord:
                  if (attr->index==0)
                     mesh->vbo_tex0 = object_loadVBO( attr->data, NULL, NULL, NULL, NULL );
                  else
                     mesh->vbo_tex1 = object_loadVBO( attr->data, NULL, NULL, NULL, NULL );
                  /* TODO handle other cases? */
                  break;

               case cgltf_attribute_type_color:
               case cgltf_attribute_type_tangent:
               default:
                  break;
            }
         }
      }
   }

   /* Iterate over children. */
   node->children = calloc( cnode->children_count, sizeof(Node) );
   node->nchildren = cnode->children_count;
   for (size_t i=0; i<cnode->children_count; i++) {
      Node *child = &node->children[i];
      object_loadNodeRecursive( data, child, cnode->children[i] );
      node->radius = MAX( node->radius, child->radius );
      vec3_max( &node->aabb_max, &node->aabb_max, &child->aabb_max );
      vec3_min( &node->aabb_min, &node->aabb_min, &child->aabb_min );
   }
   return 0;
}

static void shadow_matrix( const Object *obj, mat4 *m, const Light *light )
{
   (void) obj;
   const vec3 up        = { .v = {0., 1., 0.} };
   const vec3 center    = { .v = {0., 0., 0.} };
   const vec3 light_pos = light->pos;
   const mat4 L = mat4_lookat( &light_pos, &center, &up );
   const float norm = vec3_length( &light_pos );
   const float r = 1.0;
   const mat4 O = mat4_ortho( -r, r, -r, r, norm-1.0, norm+1.0 );
   mat4_mul( m, &L, &O );
}

/**
 * @brief Renders a mesh shadow with a transform.
 */
static void renderMeshShadow( const Object *obj, const Mesh *mesh, const mat4 *H )
{
   (void) obj;
   const Shader *shd = &shadow_shader;

   /* Skip with no shadows. */
   if ((mesh->material>=0) && (obj->materials[ mesh->material ].noshadows))
      return;

   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->vbo_idx );

   /* TODO put everything in a single VBO */
   glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo_pos );
   glVertexAttribPointer( shd->vertex, 3, GL_FLOAT, GL_FALSE, 0, NULL );
   glEnableVertexAttribArray( shd->vertex );

   glUniformMatrix4fv( shd->Hmodel,      1, GL_FALSE, H->ptr );
   glDrawElements( GL_TRIANGLES, mesh->nidx, GL_UNSIGNED_INT, 0 );
}

/**
 * @brief Renders a mesh with a transform.
 */
static void renderMesh( const Object *obj, const Mesh *mesh, const mat4 *H )
{
   const Material *mat;
   const Shader *shd = &object_shader;

   if (mesh->material < 0)
      mat = &material_default;
   else
      mat = &obj->materials[ mesh->material ];

   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->vbo_idx );

   /* TODO put everything in a single VBO */
   glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo_pos );
   glVertexAttribPointer( shd->vertex, 3, GL_FLOAT, GL_FALSE, 0, NULL );
   glEnableVertexAttribArray( shd->vertex );
   if (mesh->vbo_nor) {
      glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo_nor );
      glVertexAttribPointer( shd->vertex_normal, 3, GL_FLOAT, GL_FALSE, 0, NULL );
      glEnableVertexAttribArray( shd->vertex_normal );
   }
   gl_checkErr();
   if (mesh->vbo_tex0) {
      glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo_tex0 );
      glVertexAttribPointer( shd->vertex_tex0, 2, GL_FLOAT, GL_FALSE, 0, NULL );
      glEnableVertexAttribArray( shd->vertex_tex0 );
   }
   gl_checkErr();
   if (mesh->vbo_tex1) {
      glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo_tex1 );
      glVertexAttribPointer( shd->vertex_tex1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
      glEnableVertexAttribArray( shd->vertex_tex1 );
   }
   gl_checkErr();

   /* Set up shader. */
   glUseProgram( shd->program );

   glUniformMatrix4fv( shd->Hmodel,    1, GL_FALSE, H->ptr );
   glUniform1f( shd->metallicFactor,   mat->metallicFactor );
   glUniform1f( shd->roughnessFactor,  mat->roughnessFactor );
   glUniform4f( shd->baseColour,       mat->baseColour[0], mat->baseColour[1], mat->baseColour[2], mat->baseColour[3] );
   glUniform3f( shd->sheenTint,        mat->sheen[0], mat->sheen[1], mat->sheen[2] );
   glUniform1f( shd->sheen,            mat->sheen_roughness );
   glUniform1f( shd->clearcoat,        mat->clearcoat );
   glUniform1f( shd->clearcoat_roughness, mat->clearcoat_roughness );
   glUniform3f( shd->emissive,         mat->emissiveFactor[0], mat->emissiveFactor[1], mat->emissiveFactor[2] );
   glUniform1i( shd->blend,            mat->blend );
   if (use_normal_mapping) {
      glUniform1i( shd->u_has_normal,     (mat->normal_tex.tex!=tex_zero.tex) );
      glUniform1f( shd->normal_scale,     mat->normal_tex.strength );
      glUniform1i( shd->normal_texcoord,     mat->normal_tex.texcoord);
   }
   //glUniform1f( shd->waxiness, mat->waxiness );
   /* Texture coordinates. */
   glUniform1i( shd->baseColour_texcoord, mat->baseColour_tex.texcoord);
   glUniform1i( shd->metallic_texcoord,   mat->metallic_tex.texcoord);
   glUniform1i( shd->emissive_texcoord,   mat->emissive_tex.texcoord);
   if (use_ambient_occlusion)
      glUniform1i( shd->occlusion_texcoord,  mat->occlusion_tex.texcoord);
   gl_checkErr();

   /* Texture. */
   glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, mat->metallic_tex.tex );
      glUniform1i( shd->metallic_tex, 1 );
   glActiveTexture( GL_TEXTURE2 );
      glBindTexture( GL_TEXTURE_2D, mat->emissive_tex.tex );
      glUniform1i( shd->emissive_tex, 2 );
   if (use_normal_mapping) {
      glActiveTexture( GL_TEXTURE3 );
         glBindTexture( GL_TEXTURE_2D, mat->normal_tex.tex );
         glUniform1i( shd->normal_tex, 3 );
   }
   if (use_ambient_occlusion) {
      glActiveTexture( GL_TEXTURE4 );
         glBindTexture( GL_TEXTURE_2D, mat->occlusion_tex.tex );
         glUniform1i( shd->occlusion_tex, 4 );
   }
   /* Have to have GL_TEXTURE0 be last. */
   glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, mat->baseColour_tex.tex );
      glUniform1i( shd->baseColour_tex, 0 );
   gl_checkErr();

   glDrawElements( GL_TRIANGLES, mesh->nidx, GL_UNSIGNED_INT, 0 );
}

/**
 * @brief Recursive rendering to the shadow buffer.
 */
static void object_renderNodeShadow( const Object *obj, const Node *node, const mat4 *H )
{
   /* Multiply matrices, can be animated so not caching. */
   /* TODO cache when not animated. */
   mat4 HH = node->H;
   mat4_apply( &HH, H );

   /* Draw meshes. */
   for (size_t i=0; i<node->nmesh; i++)
      renderMeshShadow( obj, &node->mesh[i], &HH );

   /* Draw children. */
   for (size_t i=0; i<node->nchildren; i++)
      object_renderNodeShadow( obj, &node->children[i], &HH );

   gl_checkErr();
}

/**
 * @brief Recursive rendering of a mesh.
 */
static void object_renderNodeMesh( const Object *obj, const Node *node, const mat4 *H )
{
   /* Multiply matrices, can be animated so not caching. */
   /* TODO cache when not animated. */
   mat4 HH = node->H;
   mat4_apply( &HH, H );

   /* Draw meshes. */
   for (size_t i=0; i<node->nmesh; i++)
      renderMesh( obj, &node->mesh[i], &HH );

   /* Draw children. */
   for (size_t i=0; i<node->nchildren; i++)
      object_renderNodeMesh( obj, &node->children[i], &HH );

   gl_checkErr();
}

static void object_renderShadow( const Object *obj, int scene, const mat4 *H, const Light *light, double time )
{
   const Shader *shd = &shadow_shader;

   /* Set up the shadow map and render. */
   glBindFramebuffer(GL_FRAMEBUFFER, light->fbo);
   glClear(GL_DEPTH_BUFFER_BIT);
   glViewport(0, 0, SHADOWMAP_SIZE, SHADOWMAP_SIZE);

   /* Cull faces. */
   glEnable(GL_CULL_FACE);
   glCullFace(GL_FRONT);

   /* Set up shader. */
   glUseProgram( shd->program );
   mat4 Hshadow;
   shadow_matrix( obj, &Hshadow, light );
   glUniformMatrix4fv( shd->Hshadow, 1, GL_FALSE, Hshadow.ptr );
   glUniform1f( shd->u_time, time );

   for (size_t i=0; i<obj->scenes[scene].nnodes; i++)
      object_renderNodeShadow( obj, &obj->scenes[scene].nodes[i], H );

   glDisable(GL_CULL_FACE);
   gl_checkErr();

   /* Now we have to blur. We'll do a separable filter approach and do two passes. */
   /* First pass for X. */
   shd = &shadow_shader_blurX;
   glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
   glClear(GL_DEPTH_BUFFER_BIT);
   glUseProgram( shd->program );

   glBindBuffer( GL_ARRAY_BUFFER, shadow_vbo );
   glVertexAttribPointer( shd->vertex, 2, GL_FLOAT, GL_FALSE, 0, NULL );
   glEnableVertexAttribArray( shd->vertex );

   glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, light->tex );

   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   gl_checkErr();
   /* Second pass for Y and back into the proper framebuffer. */
   shd = &shadow_shader_blurY;
   glBindFramebuffer(GL_FRAMEBUFFER, light->fbo);
   glClear(GL_DEPTH_BUFFER_BIT);

   glUseProgram( shd->program );

   glBindBuffer( GL_ARRAY_BUFFER, shadow_vbo );
   glVertexAttribPointer( shd->vertex, 2, GL_FLOAT, GL_FALSE, 0, NULL );
   glEnableVertexAttribArray( shd->vertex );

   glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, shadow_tex );

   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   /* Clean up. */
   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   glDisableVertexAttribArray( shd->vertex );
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

   glUseProgram( 0 );

   glBindTexture( GL_TEXTURE_2D, 0 );

   gl_checkErr();
}

static void object_renderMesh( const Object *obj, int scene, const mat4 *H, double time )
{
   /* Load constant stuff. */
   const Shader *shd = &object_shader;
   glUseProgram( shd->program );
   glUniform1f( shd->u_time, time );
   mat4 Hshadow;
   for (int i=0; i<MAX_LIGHTS; i++) {
      const Light *l = &lights[i];

      shadow_matrix( obj, &Hshadow, l );
      glUniformMatrix4fv( shd->lights[i].Hshadow, 1, GL_FALSE, Hshadow.ptr );

      glActiveTexture( GL_TEXTURE5+i );
         glBindTexture( GL_TEXTURE_2D, l->tex );
         glUniform1i( shd->lights[i].shadowmap_tex, 5+i );
   }
   gl_checkErr();

   glEnable(GL_BLEND);
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

   /* Cull faces. */
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   for (size_t i=0; i<obj->scenes[scene].nnodes; i++)
      object_renderNodeMesh( obj, &obj->scenes[scene].nodes[i], H );

   glUseProgram( 0 );

   glBindTexture( GL_TEXTURE_2D, 0 );

   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

   glDisable(GL_CULL_FACE);

   gl_checkErr();
}

/**
 * @brief Renders an object (with a transformation).
 *
 *    @param obj Object to render.
 *    @param H Transformation to apply (or NULL to use identity).
 */
void object_render( GLuint fb, const Object *obj, const mat4 *H, double time, double size )
{
   return object_renderScene( fb, obj, obj->scene_body, H, time, size, 0 );
}

void object_renderScene( GLuint fb, const Object *obj, int scene, const mat4 *H, double time, double size, unsigned int flags )
{
   (void) time; /* TODO implement animations. */
   (void) flags;
   if (scene < 0)
      return;
   const GLfloat sca = 1.0/obj->radius;
   const mat4 Hscale = { .m = {
      { sca, 0.0, 0.0, 0.0 },
      { 0.0, sca, 0.0, 0.0 },
      { 0.0, 0.0,-sca, 0.0 },
      { 0.0, 0.0, 0.0, 1.0 } } };
   mat4 Hptr;

   if (H==NULL)
      Hptr = Hscale;
   else {
      Hptr = *H;
      mat4_apply( &Hptr, &Hscale );
   }

   /* Some constant stuff. */
   const Shader *shd = &object_shader;
   glUseProgram( shd->program );

   /* Depth testing. */
   glEnable( GL_DEPTH_TEST );
   glDepthFunc( GL_LESS );

   /* TODO only render shadows if applicable. */
   for (int i=0; i<MAX_LIGHTS; i++)
      object_renderShadow( obj, scene, &Hptr, &lights[i], time );

   glViewport( 0, 0, size, size );
   glBindFramebuffer(GL_FRAMEBUFFER, fb);
   object_renderMesh( obj, scene, &Hptr, time );

   glDisable( GL_DEPTH_TEST );
   glUseProgram( 0 );
#ifdef HAVE_NAEV
   glViewport( 0, 0, gl_screen.rw, gl_screen.rh );
#endif /* HAVE_NAEV */
}

static cgltf_result object_read( const struct cgltf_memory_options* memory_options, const struct cgltf_file_options* file_options, const char* path, cgltf_size* size, void** data)
{
	(void)file_options;
   PHYSFS_Stat path_stat;

	void* (*memory_alloc)(void*, cgltf_size) = memory_options->alloc_func ? memory_options->alloc_func : &cgltf_default_alloc;
	void (*memory_free)(void*, void*) = memory_options->free_func ? memory_options->free_func : &cgltf_default_free;

   if (!PHYSFS_stat( path, &path_stat )) {
      WARN(_("File '%s' not found!"), path);
		return cgltf_result_file_not_found;
   }

	cgltf_size file_size = size ? *size : 0;
   if (file_size==0)
      file_size = path_stat.filesize;

	char* file_data = (char*)memory_alloc(memory_options->user_data, file_size);
	if (!file_data)
		return cgltf_result_out_of_memory;

   PHYSFS_file *pfile = PHYSFS_openRead( path );
   if (pfile == NULL) {
      WARN(_("Unable to open '%s' for reading: %s"), path, PHYSFS_getErrorByCode( PHYSFS_getLastErrorCode() ) );
      return cgltf_result_file_not_found;
   }
   cgltf_size read_size = PHYSFS_readBytes( pfile, file_data, file_size );
   PHYSFS_close( pfile );

	if (read_size != file_size) {
		memory_free(memory_options->user_data, file_data);
		return cgltf_result_io_error;
	}

	if (size)
		*size = file_size;
	if (data)
		*data = file_data;

	return cgltf_result_success;
}

/**
 * @brief Loads an object from a file.
 *
 *    @param filename Name of the file to load from.
 *    @return Newly loaded object file.
 */
Object *object_loadFromFile( const char *filename )
{
   Object *obj;
   cgltf_result res;
   cgltf_data *data;
   cgltf_options opts;
   char *dirpath;
   char mountpath[PATH_MAX];
   memset( &opts, 0, sizeof(opts) );
   opts.file.read = object_read;

   /* First see if we have to initialize subsystem. */
   gl_contextSet();
   if (tex_zero.tex==0)
      object_init();

   /* Initialize object. */
   obj = calloc( sizeof(Object), 1 );

   /* Start loading the file. */
   res = cgltf_parse_file( &opts, filename, &data );
   assert( res == cgltf_result_success );

#if DEBUGGING
   /* Validate just in case. */
   res = cgltf_validate( data );
   assert( res == cgltf_result_success );
#endif /* DEBUGGING */

   /* Will load from PHYSFS. */
   res = cgltf_load_buffers( &opts, data, filename );
   assert( res == cgltf_result_success );

   /* Now mount the directory and try to do things. */
   dirpath = strdup(filename);
   snprintf( mountpath, sizeof(mountpath), "%s/%s", PHYSFS_getRealDir(filename), dirname(dirpath) );
   PHYSFS_mount( mountpath, "/", 0 ); /* Prefix so more priority. */
   free( dirpath );

   /* Load materials. */
   obj->materials = calloc( data->materials_count, sizeof(Material) );
   obj->nmaterials = data->materials_count;
   for (size_t i=0; i<data->materials_count; i++)
      object_loadMaterial( &obj->materials[i], &data->materials[i], data );

   /* Load scenes. */
   obj->scenes = calloc( data->scenes_count, sizeof(Scene) );
   obj->nscenes = data->scenes_count;
   obj->scene_body = 0; /**< Always the default scene. */
   obj->scene_engine = -1;
   for (size_t s=0; s<obj->nscenes; s++) {
      /* Load nodes. */
      cgltf_scene *cscene = &data->scenes[s]; /* data->scene may be NULL */
      Scene *scene = &obj->scenes[s];
      scene->name = strdup( cscene->name );
      if (strcmp(scene->name,"body")==0)
         obj->scene_body = s;
      else if (strcmp(scene->name,"engine")==0)
         obj->scene_engine = s;

      /* Set up and allocate scene. */
      scene->nodes = calloc( cscene->nodes_count, sizeof(Node) );
      scene->nnodes = cscene->nodes_count;
      for (size_t i=0; i<cscene->nodes_count; i++) {
         Node *n = &scene->nodes[i];
         object_loadNodeRecursive( data, n, cscene->nodes[i] );
         obj->radius = MAX( obj->radius, n->radius );
         vec3_max( &obj->aabb_max, &obj->aabb_max, &n->aabb_max );
         vec3_min( &obj->aabb_min, &obj->aabb_min, &n->aabb_min );
      }
   }

   /* Unmount directory. */
   PHYSFS_unmount( mountpath );

   cgltf_free(data);
   gl_contextUnset();

   return obj;
}

static void object_freeNode( Node *node )
{
   for (size_t i=0; i<node->nmesh; i++) {
      Mesh *m = &node->mesh[i];
      if (m->vbo_idx)
         glDeleteBuffers( 1, &m->vbo_idx );
      if (m->vbo_pos)
         glDeleteBuffers( 1, &m->vbo_pos );
      if (m->vbo_nor)
         glDeleteBuffers( 1, &m->vbo_nor );
      if (m->vbo_tex0)
         glDeleteBuffers( 1, &m->vbo_tex0 );
      if (m->vbo_tex1)
         glDeleteBuffers( 1, &m->vbo_tex1 );
   }
   free( node->mesh );
   gl_checkErr();

   for (size_t i=0; i<node->nchildren; i++)
      object_freeNode( &node->children[i] );
   free( node->children );
}

static void object_freeTex( Texture *tex )
{
   /* Don't have to free default textures. */
   if (tex->tex==tex_zero.tex || tex->tex==tex_ones.tex)
      return;

   if (tex)
      glDeleteTextures( 1, &tex->tex );
   gl_checkErr();
}

void object_free( Object *obj )
{
   if (obj==NULL)
      return;

   for (size_t s=0; s<obj->nscenes; s++) {
      Scene *scene = &obj->scenes[s];
      for (size_t i=0; i<scene->nnodes; i++)
         object_freeNode( &scene->nodes[i] );
      free( scene->name );
      free( scene->nodes );
   }
   free( obj->scenes );

   for (size_t i=0; i<obj->nmaterials; i++) {
      Material *m = &obj->materials[i];
      object_freeTex( &m->baseColour_tex );
      object_freeTex( &m->metallic_tex );
      object_freeTex( &m->normal_tex );
      object_freeTex( &m->occlusion_tex );
      object_freeTex( &m->emissive_tex );
   }
   free( obj->materials );

   free( obj );
}

int object_init (void)
{
   const GLubyte data_zero[4] = { 0, 0, 0, 0 };
   const GLubyte data_ones[4] = { 255, 255, 255, 255 };
   const GLfloat b[4] = { 1., 1., 1., 1. };
   GLenum status;
   Shader *shd;
   const char *prepend_fix = "#define MAX_LIGHTS "STR(MAX_LIGHTS)"\n#define SHADOWMAP_SIZE "STR(SHADOWMAP_SIZE)"\n";
   char prepend[STRMAX];

   /* Set global options. */
   use_normal_mapping = !conf.low_memory;
   use_ambient_occlusion = !conf.low_memory;

   /* Set prefix stuff. */
   snprintf( prepend, sizeof(prepend), "%s\n#define HAS_NORMAL %d\n#define HAS_AO %d\n",
         prepend_fix, use_normal_mapping, use_ambient_occlusion );

   /* Load textures. */
   glGenTextures( 1, &tex_zero.tex );
   glBindTexture( GL_TEXTURE_2D, tex_zero.tex );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_zero );
   glGenTextures( 1, &tex_ones.tex );
   glBindTexture( GL_TEXTURE_2D, tex_ones.tex );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_ones );
   glBindTexture( GL_TEXTURE_2D, 0 );
   gl_checkErr();

   /* Set up default material. */
   object_loadMaterial( &material_default, NULL, NULL );

   /* We'll have to set up some rendering stuff for blurring purposes. */
   const GLfloat vbo_data[8] = {
      0., 0.,
      1., 0.,
      0., 1.,
      1., 1. };
   glGenBuffers( 1, &shadow_vbo );
   glBindBuffer( GL_ARRAY_BUFFER, shadow_vbo );
   glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, vbo_data, GL_STATIC_DRAW );
   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   /* Gen the texture. */
   glGenTextures(1, &shadow_tex);
   glBindTexture(GL_TEXTURE_2D, shadow_tex);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOWMAP_SIZE, SHADOWMAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
   glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, b);
   glBindTexture(GL_TEXTURE_2D, 0);
   /* Set up shadow buffer FBO. */
   glGenFramebuffers( 1, &shadow_fbo );
   glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_tex, 0);
   glDrawBuffer(GL_NONE);
   glReadBuffer(GL_NONE);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (status != GL_FRAMEBUFFER_COMPLETE)
      WARN(_("Error setting up shadowmap framebuffer!"));

   for (int i=0; i<MAX_LIGHTS; i++) {
      /* Set up shadow buffer depth tex. */
      glGenTextures(1, &lights[i].tex);
      glBindTexture(GL_TEXTURE_2D, lights[i].tex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOWMAP_SIZE, SHADOWMAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, b);
      glBindTexture(GL_TEXTURE_2D, 0);
      /* Set up shadow buffer FBO. */
      glGenFramebuffers( 1, &lights[i].fbo );
      glBindFramebuffer(GL_FRAMEBUFFER, lights[i].fbo);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, lights[i].tex, 0);
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if (status != GL_FRAMEBUFFER_COMPLETE)
         WARN(_("Error setting up shadowmap framebuffer!"));
   }

   /* Compile the shadow shader. */
   shd = &shadow_shader;
   shd->program = gl_program_backend( "shadow.vert", "shadow.frag", NULL, prepend );
   if (shd->program==0)
      return -1;
   glUseProgram( shd->program );
   /* Attributes. */
   shd->vertex          = glGetAttribLocation( shd->program, "vertex" );
   /* Vertex uniforms. */
   shd->Hshadow         = glGetUniformLocation( shd->program, "u_shadow");
   shd->Hmodel          = glGetUniformLocation( shd->program, "u_model");
   shd->u_time          = glGetUniformLocation( shd->program, "u_time" );

   /* Compile the X blur shader. */
   shd = &shadow_shader_blurX;
   shd->program = gl_program_backend( "blur.vert", "blurX.frag", NULL, prepend );
   if (shd->program==0)
      return -1;
   glUseProgram( shd->program );
   /* Attributes. */
   shd->vertex          = glGetAttribLocation( shd->program, "vertex" );
   /* Uniforms. */
   shd->baseColour_tex  = glGetUniformLocation( shd->program, "sampler" );
   glUniform1i( shd->baseColour_tex, 0 );

   /* Compile the Y blur shader. */
   shd = &shadow_shader_blurY;
   shd->program = gl_program_backend( "blur.vert", "blurY.frag", NULL, prepend );
   if (shd->program==0)
      return -1;
   glUseProgram( shd->program );
   /* Attributes. */
   shd->vertex          = glGetAttribLocation( shd->program, "vertex" );
   /* Uniforms. */
   shd->baseColour_tex  = glGetUniformLocation( shd->program, "sampler" );

   /* Compile the shader. */
   shd = &object_shader;
   shd->program = gl_program_backend( "gltf.vert", "gltf_pbr.frag", NULL, prepend );
   if (shd->program==0)
      return -1;
   glUseProgram( shd->program );
   /* Attributes. */
   shd->vertex          = glGetAttribLocation( shd->program, "vertex" );
   shd->vertex_normal   = glGetAttribLocation( shd->program, "vertex_normal" );
   shd->vertex_tex0     = glGetAttribLocation( shd->program, "vertex_tex0" );
   shd->vertex_tex1     = glGetAttribLocation( shd->program, "vertex_tex1" );
   /* Vertex uniforms. */
   shd->Hmodel          = glGetUniformLocation( shd->program, "u_model");
   shd->u_time          = glGetUniformLocation( shd->program, "u_time" );
   /* Fragment uniforms. */
   shd->blend           = glGetUniformLocation( shd->program, "u_blend" );
   shd->u_ambient       = glGetUniformLocation( shd->program, "u_ambient" );
   shd->baseColour_tex  = glGetUniformLocation( shd->program, "baseColour_tex" );
   shd->baseColour_texcoord = glGetUniformLocation( shd->program, "baseColour_texcoord" );
   shd->metallic_tex    = glGetUniformLocation( shd->program, "metallic_tex" );
   shd->metallic_texcoord = glGetUniformLocation( shd->program, "metallic_texcoord" );
   if (use_normal_mapping) {
      shd->u_has_normal    = glGetUniformLocation( shd->program, "u_has_normal" );
      shd->normal_tex      = glGetUniformLocation( shd->program, "normal_tex" );
      shd->normal_texcoord = glGetUniformLocation( shd->program, "normal_texcoord" );
      shd->normal_scale    = glGetUniformLocation( shd->program, "normal_scale" );
   }
   shd->metallicFactor  = glGetUniformLocation( shd->program, "metallicFactor" );
   shd->roughnessFactor = glGetUniformLocation( shd->program, "roughnessFactor" );
   shd->baseColour      = glGetUniformLocation( shd->program, "baseColour" );
   shd->sheenTint       = glGetUniformLocation( shd->program, "sheenTint" );
   shd->sheen           = glGetUniformLocation( shd->program, "sheen" );
   shd->clearcoat       = glGetUniformLocation( shd->program, "clearcoat" );
   shd->clearcoat_roughness = glGetUniformLocation( shd->program, "clearcoat_roughness" );
   shd->emissive        = glGetUniformLocation( shd->program, "emissive" );
   if (use_ambient_occlusion) {
      shd->occlusion_tex   = glGetUniformLocation( shd->program, "occlusion_tex" );
      shd->occlusion_texcoord = glGetUniformLocation( shd->program, "occlusion_texcoord" );
   }
   shd->emissive_tex    = glGetUniformLocation( shd->program, "emissive_tex" );
   shd->emissive_texcoord = glGetUniformLocation( shd->program, "emissive_texcoord" );
   /* Special. */
   //shd->waxiness        = glGetUniformLocation( shd->program, "u_waxiness" );
   /* Lights. */
   for (int i=0; i<MAX_LIGHTS; i++) {
      ShaderLight *sl = &shd->lights[i];
      char buf[128];
      snprintf( buf, sizeof(buf), "u_lights[%d].position", i );
      sl->position      = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof(buf), "u_lights[%d].sun", i );
      sl->sun           = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof(buf), "u_lights[%d].colour", i );
      sl->colour        = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof(buf), "u_lights[%d].intensity", i );
      sl->intensity     = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof(buf), "shadowmap_tex[%d]", i );
      sl->shadowmap_tex = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof(buf), "u_shadow[%d]", i );
      sl->Hshadow       = glGetUniformLocation( shd->program, buf );
   }
   shd->nlights         = glGetUniformLocation( shd->program, "u_nlights" );
   /* Light default values set on init. */
   object_lightSetup( 1, 1 ); /* Unsets the program. */
   gl_checkErr();

   return 0;
}

void object_exit (void)
{
   /* Not initialized. */
   if (tex_zero.tex==0)
      return;

   glDeleteBuffers( 1, &shadow_vbo );
   glDeleteTextures( 1, &shadow_tex );
   glDeleteFramebuffers( 1, &shadow_fbo );
   for (int i=0; i<MAX_LIGHTS; i++) {
      glDeleteTextures( 1, &lights[i].tex );
      glDeleteFramebuffers( 1, &lights[i].fbo );
   }
   glDeleteTextures( 1, &tex_zero.tex );
   glDeleteTextures( 1, &tex_ones.tex );
   glDeleteProgram( object_shader.program );
   glDeleteProgram( shadow_shader.program );
}

static void object_lightSetup( int doambient, int dogeneral )
{
   const Shader *shd = &object_shader;
   /* Skip if not initialized. */
   if (tex_zero.tex==0)
      return;
   glUseProgram( shd->program );
   if (doambient) {
      const double factor = 1.0/M_PI;
      glUniform3f( shd->u_ambient, ambient.v[0]*factor, ambient.v[1]*factor, ambient.v[2]*factor );
   }
   if (dogeneral) {
      glUniform1i( shd->nlights, MAX_LIGHTS );
      for (int i=0; i<MAX_LIGHTS; i++) {
         const Light *l = &lights[i];
         const ShaderLight *sl = &shd->lights[i];
         glUniform3f( sl->position, l->pos.v[0], l->pos.v[1], l->pos.v[2] );
         glUniform3f( sl->colour, l->colour.v[0], l->colour.v[1], l->colour.v[2] );
         glUniform1f( sl->intensity, l->intensity * light_intensity );
      }
   }
   glUseProgram(0);
   gl_checkErr();
}

void object_light( double r, double g, double b, double intensity )
{
   ambient.v[0] = r;
   ambient.v[1] = g;
   ambient.v[2] = b;
   light_intensity = intensity;
   object_lightSetup( 1, 1 );
}

void object_lightGet( double *r, double *g, double *b, double *intensity )
{
   *r = ambient.v[0];
   *g = ambient.v[1];
   *b = ambient.v[2];
   *intensity = light_intensity;
}

/**
 * @brief Sets the ambient colour. Should be multiplied by intensity.
 */
void object_lightAmbient( double r, double g, double b )
{
   ambient.v[0] = r;
   ambient.v[1] = g;
   ambient.v[2] = b;
   object_lightSetup( 1, 0 );
}

/**
 * @brief Gets the ambient light values.
 */
void object_lightAmbientGet( double *r, double *g, double *b )
{
   *r = ambient.v[0];
   *g = ambient.v[1];
   *b = ambient.v[2];
}

/**
 * @brief Sets the general light intensity.
 */
void object_lightIntensity( double strength )
{
   light_intensity = strength;
   object_lightSetup( 0, 1 );
}

/**
 * @brief Gets the general light intensity.
 */
double object_lightIntensityGet (void)
{
   return light_intensity;
}

/**
 * @brief Gets the shadow map texture.
 */
GLuint object_shadowmap( int light )
{
   return lights[light].tex;
}
