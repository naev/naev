#include "gltf.h"

#include "common.h"

#include "glad.h"
#include "SDL_image.h"
#include <assert.h>
#include <math.h>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "vec3.h"
#include "shader_min.h"

//const vec3 primary_light = { .v = {4., 2., -20.} };
//const vec3 primary_light = { .v = {20., 0., 0.} };
const vec3 primary_light = { .v = {0., 0., -3.} };

/* Horrible hack that turns a variable name into a string. */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define MAX_LIGHTS 4    /**< Maximum amount of lights. TODO deferred rendering. */

#define SHADOWMAP_SIZE  512   /**< Size of the shadow map. */

/**
 * @brief Simple point light model.
 */
typedef struct ShaderLight_ {
   GLuint position;  /* vec3 */
   GLuint range;     /* float */
   GLuint colour;    /* vec3 */
   GLuint intensity; /* float */
} ShaderLight;

/**
 * @brief Shader to use witha material.
 */
typedef struct Shader_ {
   GLuint program;
   /* Attriutes. */
   GLuint vertex;
   GLuint vertex_normal;
   GLuint vertex_tex0;
   /* Vertex Uniforms. */
   GLuint Hmodel;
   GLuint Hprojection;
   GLuint Hshadow_projection;
   /* Fragment uniforms. */
   GLuint baseColour_tex;
   GLuint metallic_tex;
   GLuint normal_tex;
   GLuint metallicFactor;
   GLuint roughnessFactor;
   GLuint baseColour;
   GLuint clearcoat;
   GLuint clearcoat_roughness;
   GLuint emissive;
   GLuint emissive_tex;
   GLuint occlusion_tex;
   ShaderLight lights[MAX_LIGHTS];
   GLuint nlights;
   GLuint blend;
   GLuint shadowmap_tex;
} Shader;
static Shader object_shader;
static Shader shadow_shader;
static GLuint fbo_shadow;
static GLuint tex_shadow;
static GLuint tex_zero;
static GLuint tex_ones;

/**
 * @brief PBR Material of an object.
 */
typedef struct Material_ {
   char *name; /**< Name of the material if applicable. */
   int blend;  /**< Whether or not to blend it. */
   /* pbr_metallic_roughness */
   GLuint baseColour_tex;  /**< Base colour of the material. */
   GLuint metallic_tex;    /**< Metallic/roughness map of the material. Metallic is stored in G channel, hile roughness is in the B channel. */
   GLfloat metallicFactor; /**< Metallic factor (single value). Multplies the map if available. */
   GLfloat roughnessFactor;/**< Roughness factor (single value). Multiplies the map if available. */
   GLfloat baseColour[4];  /**< Base colour of the material. Multiplies the texture if available. */
   /* pbr_specular_glossiness */
   /* TODO */
   /* clearcoat */
   /*GLuint clearcoat_tex;
   GLuint clearcoat_roughness_tex;
   GLuint clearcoat_normal_tex; */
   GLfloat clearcoat;
   GLfloat clearcoat_roughness;
   /* misc. */
   GLuint normal_tex;
   GLuint occlusion_tex;
   GLuint emissive_tex;
   GLfloat emissiveFactor[3];
   //GLuint tex0;
} Material;
static Material material_default;

/**
 * @brief Represents an underlying 3D mesh.
 */
typedef struct Mesh_ {
   size_t nidx;      /**< Number of indices. */
   GLuint vbo_idx;   /**< Index VBO. */
   GLuint vbo_pos;   /**< Position VBO. */
   GLuint vbo_nor;   /**< Normal VBO. */
   GLuint vbo_tex;   /**< Texture coordinate VBO. */
   int material;     /**< ID of material to use. */

   GLfloat radius;   /**< Sphere fit on the model centered at 0,0. */
   vec3 aabb_min;    /**< Minimum value of AABB wrapping around it. */
   vec3 aabb_max;    /**< Maximum value of AABB wrapping around it. */
} Mesh;

/**
 * @brief Represents a node of an object. Each node can have multiple meshes and children nodes with an associated transformation.
 */
struct Node_;
typedef struct Node_ {
   char *name;       /**< Name information. */
   mat4 H;           /**< Homogeneous transform. */
   Mesh *mesh;       /**< Meshes. */
   size_t nmesh;     /**< Number of meshes. */
   struct Node_ *children; /**< Children mesh. */
   size_t nchildren; /**< Number of children mesh. */

   GLfloat radius;   /**< Sphere fit on the model centered at 0,0. */
   vec3 aabb_min;    /**< Minimum value of AABB wrapping around it. */
   vec3 aabb_max;    /**< Maximum value of AABB wrapping around it. */
} Node;

/**
 * @brief Defines a complete object.
 */
struct Object_ {
   Node *nodes;         /**< Nodes the object has. */
   size_t nnodes;       /**< Number of nodes. */
   Material *materials; /**< Available materials. */
   size_t nmaterials;   /**< Number of materials. */
   GLfloat radius;      /**< Sphere fit on the model centered at 0,0. */
   vec3 aabb_min;       /**< Minimum value of AABB wrapping around it. */
   vec3 aabb_max;       /**< Maximum value of AABB wrapping around it. */
};

/**
 * @brief Loads a texture if applicable, uses default value otherwise.
 *
 *    @param ctex Texture to load.
 *    @param def Default texture to use if not defined.
 *    @return OpenGL ID of the new texture.
 */
static GLuint object_loadTexture( const cgltf_texture_view *ctex, GLint def )
{
   GLuint tex;
   SDL_Surface *surface = NULL;

   /* Must haev texture to load it. */
   if ((ctex==NULL) || (ctex->texture==NULL))
      return def;

   /* Load from path. */
   if (ctex->texture->image->uri != NULL) {
      surface = IMG_Load( ctex->texture->image->uri );
      if (surface==NULL) {
         WARN("Unable to load surface '%s'!", ctex->texture->image->uri);
         return def;
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
      SDL_Surface *temp = surface;
      surface = SDL_ConvertSurfaceFormat( temp, SDL_PIXELFORMAT_RGBA32, 0 );
      SDL_FreeSurface( temp );

      SDL_LockSurface( surface );
      glPixelStorei( GL_UNPACK_ALIGNMENT, MIN( surface->pitch & -surface->pitch, 8 ) );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB_ALPHA,
            surface->w, surface->h, 0,
            surface->format->Amask ? GL_RGBA : GL_RGB,
            GL_UNSIGNED_BYTE, surface->pixels );
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

   return tex;
}

/**
 * @brief Loads a material for the object.
 */
static int object_loadMaterial( Material *mat, const cgltf_material *cmat )
{
   const GLfloat white[4] = { 1., 1., 1., 1. };
   /* TODO complete this. */
   if (cmat && cmat->has_pbr_metallic_roughness) {
      mat->metallicFactor  = cmat->pbr_metallic_roughness.metallic_factor;
      mat->roughnessFactor = cmat->pbr_metallic_roughness.roughness_factor;
      mat->baseColour_tex  = object_loadTexture( &cmat->pbr_metallic_roughness.base_color_texture, tex_ones );
      if (mat->baseColour_tex == tex_ones)
         memcpy( mat->baseColour, cmat->pbr_metallic_roughness.base_color_factor, sizeof(mat->baseColour) );
      else
         memcpy( mat->baseColour, white, sizeof(mat->baseColour) );
      mat->metallic_tex    = object_loadTexture( &cmat->pbr_metallic_roughness.metallic_roughness_texture, tex_zero );
   }
   else {
      memcpy( mat->baseColour, white, sizeof(mat->baseColour) );
      mat->metallicFactor  = 1.;
      mat->roughnessFactor = 1.;
      mat->baseColour_tex  = tex_ones;
      mat->metallic_tex    = tex_zero;
      mat->normal_tex      = tex_zero;
   }

   if (cmat && cmat->has_clearcoat) {
      mat->clearcoat = cmat->clearcoat.clearcoat_factor;
      mat->clearcoat_roughness = cmat->clearcoat.clearcoat_roughness_factor;
   }
   else {
      mat->clearcoat = 0.;
      mat->clearcoat_roughness = 0.;
   }

   if (cmat) {
      memcpy( mat->emissiveFactor, cmat->emissive_factor, sizeof(GLfloat)*3 );
      mat->occlusion_tex= object_loadTexture( &cmat->occlusion_texture, tex_ones );
      mat->emissive_tex = object_loadTexture( &cmat->emissive_texture, tex_ones );
      mat->normal_tex   = object_loadTexture( &cmat->pbr_metallic_roughness.metallic_roughness_texture, tex_zero );
      mat->blend        = (cmat->alpha_mode == cgltf_alpha_mode_blend);
   }
   else {
      memset( mat->emissiveFactor, 0, sizeof(GLfloat)*3 );
      mat->emissive_tex    = tex_ones;
      mat->occlusion_tex   = tex_ones;
      mat->normal_tex      = tex_ones;
      mat->blend           = 0;
   }

   return 0;
}

/**
 * @brief Loads a VBO from an accessor.
 *
 *    @param acc Accessor to load from.
 *    @return OpenGL ID of the new VBO.
 */
static GLuint object_loadVBO( const cgltf_accessor *acc, GLfloat *radius, vec3 *aabb_min, vec3 *aabb_max )
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
      *radius = 0.;
      memset( aabb_min, 0, sizeof(vec3) );
      memset( aabb_max, 0, sizeof(vec3) );
      for (unsigned int i=0; i<num; i+=3) {
         vec3 v;
         for (unsigned int j=0; j<3; j++) {
            aabb_min->v[j] = MIN( dat[i+j], aabb_min->v[j] );
            aabb_max->v[j] = MAX( dat[i+j], aabb_max->v[j] );
            v.v[j] = dat[i+j];
         }
         *radius = vec3_length( &v );
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
         gl_checkErr();

         for (size_t j=0; j<prim->attributes_count; j++) {
            cgltf_attribute *attr = &prim->attributes[j];
            switch (attr->type) {
               case cgltf_attribute_type_position:
                  mesh->vbo_pos = object_loadVBO( attr->data, &mesh->radius, &mesh->aabb_min, &mesh->aabb_max );
                  node->radius = MAX( node->radius, mesh->radius );
                  vec3_max( &node->aabb_max, &node->aabb_max, &mesh->aabb_max );
                  vec3_min( &node->aabb_min, &node->aabb_min, &mesh->aabb_min );
                  break;

               case cgltf_attribute_type_normal:
                  mesh->vbo_nor = object_loadVBO( attr->data, NULL, NULL, NULL );
                  break;

               case cgltf_attribute_type_texcoord:
                  mesh->vbo_tex = object_loadVBO( attr->data, NULL, NULL, NULL );
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

static void shadow_matrix( mat4 *m )
{
   const vec3 up        = { .v = {0., 1., 0.} };
   const vec3 light_pos = primary_light; //{ .v = {4., 2., -20.} };
   const vec3 center    = { .v = {0., 0., 0.} };
   *m = mat4_lookat( &center, &light_pos, &up );
}

/**
 * @brief Renders a mesh shadow with a transform.
 */
static void renderMeshShadow( const Object *obj, const Mesh *mesh, const mat4 *H )
{
   (void) obj;
   const Shader *shd = &shadow_shader;

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
   if (mesh->vbo_tex) {
      glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo_tex );
      glVertexAttribPointer( shd->vertex_tex0, 2, GL_FLOAT, GL_FALSE, 0, NULL );
      glEnableVertexAttribArray( shd->vertex_tex0 );
   }
   gl_checkErr();

   /* Set up shader. */
   glUseProgram( shd->program );
   mat4 Hshadow;
   shadow_matrix( &Hshadow );

   //glUniformMatrix4fv( shd->Hprojection, 1, GL_FALSE, HPROJECTION.ptr );
   glUniformMatrix4fv( shd->Hshadow_projection, 1, GL_FALSE, Hshadow.ptr );
   glUniformMatrix4fv( shd->Hmodel,      1, GL_FALSE, H->ptr );
   glUniform1f( shd->metallicFactor, mat->metallicFactor );
   glUniform1f( shd->roughnessFactor, mat->roughnessFactor );
   glUniform4f( shd->baseColour, mat->baseColour[0], mat->baseColour[1], mat->baseColour[2], mat->baseColour[3] );
   glUniform1f( shd->clearcoat, mat->clearcoat );
   glUniform1f( shd->clearcoat_roughness, mat->clearcoat_roughness );
   glUniform3f( shd->emissive, mat->emissiveFactor[0], mat->emissiveFactor[1], mat->emissiveFactor[2] );
   glUniform1i( shd->blend, mat->blend );
   gl_checkErr();

   /* Lighting. */
   glUniform1i( shd->nlights, 1 );
   const ShaderLight *sl = &shd->lights[0];
   //glUniform3f( sl->position, 4., 2., -20. );
   glUniform3f( sl->position, primary_light.v[0], primary_light.v[1], primary_light.v[2] );
   glUniform1f( sl->range, -1. );
   glUniform3f( sl->colour, 1.0, 1.0, 1.0 );
   glUniform1f( sl->intensity, 500. );

   /* Texture. */
   glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, mat->baseColour_tex );
      glUniform1i( shd->baseColour_tex, 0 );
   glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, mat->metallic_tex );
      glUniform1i( shd->metallic_tex, 1 );
   glActiveTexture( GL_TEXTURE2 );
      glBindTexture( GL_TEXTURE_2D, mat->normal_tex );
      glUniform1i( shd->normal_tex, 2 );
   glActiveTexture( GL_TEXTURE3 );
      glBindTexture( GL_TEXTURE_2D, mat->emissive_tex );
      glUniform1i( shd->emissive_tex, 3 );
   glActiveTexture( GL_TEXTURE4 );
      glBindTexture( GL_TEXTURE_2D, mat->occlusion_tex );
      glUniform1i( shd->occlusion_tex, 4 );
   glActiveTexture( GL_TEXTURE5 );
      glBindTexture( GL_TEXTURE_2D, tex_shadow );
      glUniform1i( shd->shadowmap_tex, 5 );
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

static void object_renderShadow( const Object *obj, const mat4 *H )
{
   const Shader *shd = &shadow_shader;

   /* Set up the shadow map and render. */
   glBindFramebuffer(GL_FRAMEBUFFER, fbo_shadow);
   glClear(GL_DEPTH_BUFFER_BIT);
   glViewport(0, 0, SHADOWMAP_SIZE, SHADOWMAP_SIZE);

   /* Cull faces. */
   //glEnable(GL_CULL_FACE);
   //glCullFace(GL_FRONT);

   /* Set up shader. */
   glUseProgram( shd->program );
   mat4 Hshadow;
   shadow_matrix( &Hshadow );
   glUniformMatrix4fv( shd->Hshadow_projection, 1, GL_FALSE, Hshadow.ptr );

   for (size_t i=0; i<obj->nnodes; i++)
      object_renderNodeShadow( obj, &obj->nodes[i], H );

   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   glUseProgram( 0 );

   glBindTexture( GL_TEXTURE_2D, 0 );

   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

   //glDisable(GL_CULL_FACE);

   gl_checkErr();
}

static void object_renderMesh( const Object *obj, const mat4 *H )
{
   glEnable(GL_BLEND);
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glViewport(0, 0, SCREEN_W, SCREEN_H );

   /* Cull faces. */
   //glEnable(GL_CULL_FACE);
   //glCullFace(GL_BACK);

   for (size_t i=0; i<obj->nnodes; i++)
      object_renderNodeMesh( obj, &obj->nodes[i], H );

   glUseProgram( 0 );

   glBindTexture( GL_TEXTURE_2D, 0 );

   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

   //glDisable(GL_CULL_FACE);

   gl_checkErr();
}

/**
 * @brief Renders an object (with a transformation).
 *
 *    @param obj Object to render.
 *    @param H Transformation to apply (or NULL to use identity).
 */
void object_render( const Object *obj, const mat4 *H )
{
   const mat4 I = mat4_identity();
   const mat4 *Hptr = (H!=NULL) ? H : &I;

   /* Depth testing. */
   glEnable( GL_DEPTH_TEST );
   glDepthFunc( GL_LESS );

   object_renderShadow( obj, Hptr );
   object_renderMesh( obj, Hptr );

   glDisable( GL_DEPTH_TEST );
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
   memset( &opts, 0, sizeof(opts) );

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

   /* TODO load buffers properly from physfs. */
   res = cgltf_load_buffers( &opts, data, "./" );
   assert( res == cgltf_result_success );

   /* Load materials. */
   obj->materials = calloc( data->materials_count, sizeof(Material) );
   obj->nmaterials = data->materials_count;
   for (size_t i=0; i<data->materials_count; i++)
      object_loadMaterial( &obj->materials[i], &data->materials[i] );

   /* Load nodes. */
   cgltf_scene *scene = &data->scenes[0]; /* data->scene may be NULL */
   obj->nodes = calloc( scene->nodes_count, sizeof(Node) );
   obj->nnodes = scene->nodes_count;
   for (size_t i=0; i<scene->nodes_count; i++) {
      Node *n = &obj->nodes[i];
      object_loadNodeRecursive( data, n, scene->nodes[i] );
      obj->radius = MAX( obj->radius, n->radius );
      vec3_max( &obj->aabb_max, &obj->aabb_max, &n->aabb_max );
      vec3_min( &obj->aabb_min, &obj->aabb_min, &n->aabb_min );
   }

   cgltf_free(data);

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
      if (m->vbo_tex)
         glDeleteBuffers( 1, &m->vbo_tex );
   }
   free( node->mesh );
   gl_checkErr();

   for (size_t i=0; i<node->nchildren; i++)
      object_freeNode( &node->children[i] );
   free( node->children );
}

static void object_freeTex( GLuint tex )
{
   /* Don't have to free default textures. */
   if (tex==tex_zero || tex==tex_ones)
      return;

   if (tex)
      glDeleteTextures( 1, &tex );
   gl_checkErr();
}

void object_free( Object *obj )
{
   for (size_t i=0; i<obj->nnodes; i++)
      object_freeNode( &obj->nodes[i] );
   free( obj->nodes );

   for (size_t i=0; i<obj->nmaterials; i++) {
      Material *m = &obj->materials[i];
      object_freeTex( m->baseColour_tex );
      object_freeTex( m->metallic_tex );
      object_freeTex( m->normal_tex );
      object_freeTex( m->occlusion_tex );
      object_freeTex( m->emissive_tex );
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

   /* Load textures. */
   glGenTextures( 1, &tex_zero );
   glBindTexture( GL_TEXTURE_2D, tex_zero );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_zero );
   glGenTextures( 1, &tex_ones );
   glBindTexture( GL_TEXTURE_2D, tex_ones );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_ones );
   glBindTexture( GL_TEXTURE_2D, 0 );
   gl_checkErr();

   /* Set up default material. */
   object_loadMaterial( &material_default, NULL );

   /* Set up shadow buffer depth tex. */
   glGenTextures(1, &tex_shadow);
   glBindTexture(GL_TEXTURE_2D, tex_shadow);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP_SIZE, SHADOWMAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, b);
   glBindTexture(GL_TEXTURE_2D, 0);
   /* Set up shadow buffer FBO. */
   glGenFramebuffers( 1, &fbo_shadow );
   glBindFramebuffer(GL_FRAMEBUFFER, fbo_shadow);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_shadow, 0);
   glDrawBuffer(GL_NONE);
   glReadBuffer(GL_NONE);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (status != GL_FRAMEBUFFER_COMPLETE)
      WARN(_("Error setting up shadowmap framebuffer!"));

   /* Compile the shadow shader. */
   shd = &shadow_shader;
   shd->program = gl_program_vert_frag( "shadow.vert", "shadow.frag", "" );
   if (shd->program==0)
      return -1;
   glUseProgram( shd->program );
   /** Attributes. */
   shd->vertex          = glGetAttribLocation( shd->program, "vertex" );
   /* Vertex uniforms. */
   shd->Hprojection     = glGetUniformLocation( shd->program, "projection");
   shd->Hshadow_projection = glGetUniformLocation( shd->program, "shadow_projection");
   shd->Hmodel          = glGetUniformLocation( shd->program, "model");

   /* Compile the shader. */
   shd = &object_shader;
   shd->program = gl_program_vert_frag( "gltf.vert", "gltf_pbr.frag", "#define MAX_LIGHTS "STR(MAX_LIGHTS)"\n" );
   if (shd->program==0)
      return -1;
   glUseProgram( shd->program );
   /** Attributes. */
   shd->vertex          = glGetAttribLocation( shd->program, "vertex" );
   shd->vertex_normal   = glGetAttribLocation( shd->program, "vertex_normal" );
   shd->vertex_tex0     = glGetAttribLocation( shd->program, "vertex_tex0" );
   /* Vertex uniforms. */
   shd->Hprojection     = glGetUniformLocation( shd->program, "projection");
   shd->Hshadow_projection = glGetUniformLocation( shd->program, "shadow_projection");
   shd->Hmodel          = glGetUniformLocation( shd->program, "model");
   /* Fragment uniforms. */
   shd->blend           = glGetUniformLocation( shd->program, "u_blend" );
   shd->baseColour_tex  = glGetUniformLocation( shd->program, "baseColour_tex" );
   shd->metallic_tex    = glGetUniformLocation( shd->program, "metallic_tex" );
   shd->normal_tex      = glGetUniformLocation( shd->program, "normal_tex" );
   shd->metallicFactor  = glGetUniformLocation( shd->program, "metallicFactor" );
   shd->roughnessFactor = glGetUniformLocation( shd->program, "roughnessFactor" );
   shd->baseColour      = glGetUniformLocation( shd->program, "baseColour" );
   shd->clearcoat       = glGetUniformLocation( shd->program, "clearcoat" );
   shd->clearcoat_roughness = glGetUniformLocation( shd->program, "clearcoat_roughness" );
   shd->emissive        = glGetUniformLocation( shd->program, "emissive" );
   shd->occlusion_tex   = glGetUniformLocation( shd->program, "occlusion_tex" );
   shd->emissive_tex    = glGetUniformLocation( shd->program, "emissive_tex" );
   for (int i=0; i<MAX_LIGHTS; i++) {
      ShaderLight *sl = &shd->lights[i];
      char buf[128];
      snprintf( buf, sizeof(buf), "u_lights[%d].position", i );
      sl->position      = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof(buf), "u_lights[%d].range", i );
      sl->range         = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof(buf), "u_lights[%d].colour", i );
      sl->colour        = glGetUniformLocation( shd->program, buf );
      snprintf( buf, sizeof(buf), "u_lights[%d].intensity", i );
      sl->intensity     = glGetUniformLocation( shd->program, buf );
   }
   shd->nlights         = glGetUniformLocation( shd->program, "u_nlights" );
   //shd->shadowmap       = glGetUniformLocation( shd->program, "shadowmap" );
   shd->shadowmap_tex   = glGetUniformLocation( shd->program, "shadowmap_tex" );
   glUseProgram(0);
   gl_checkErr();

   return 0;
}

void object_exit (void)
{
   glDeleteTextures( 1, &tex_shadow );
   glDeleteFramebuffers( 1, &fbo_shadow );
   glDeleteTextures( 1, &tex_zero );
   glDeleteTextures( 1, &tex_ones );
   glDeleteProgram( object_shader.program );
   glDeleteProgram( shadow_shader.program );
}

GLuint object_shadowmap (void)
{
   return tex_shadow;
}
