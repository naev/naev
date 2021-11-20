#include "gltf.h"

#include "common.h"

#include "glad.h"
#include "SDL_image.h"
#include <assert.h>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "shader_min.h"

#define MAX_LIGHTS 4

typedef struct ShaderLight_ {
   GLuint position;  /* vec3 */
   GLuint range;     /* float */
   GLuint colour;    /* vec3 */
   GLuint intensity; /* float */
} ShaderLight;

typedef struct Shader_ {
   GLuint program;
   /* Attriutes. */
   GLuint vertex;
   GLuint vertex_normal;
   GLuint vertex_tex0;
   /* Vertex Uniforms. */
   GLuint Hmodel;
   GLuint Hprojection;
   /* Fragment uniforms. */
   GLuint baseColour_tex;
   GLuint metallic_tex;
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
} Shader;
static Shader object_shader;
static GLuint tex_zero = -1;
static GLuint tex_ones = -1;

typedef struct Material_ {
   char *name;       /**< Name of the material if applicable. */
   /* pbr_metallic_roughness */
   GLuint baseColour_tex;
   GLuint metallic_tex;
   GLfloat metallicFactor;
   GLfloat roughnessFactor;
   GLfloat baseColour[4];
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

typedef struct Mesh_ {
   size_t nidx;      /**< Number of indices. */
   GLuint vbo_idx;   /**< Index VBO. */
   GLuint vbo_pos;   /**< Position VBO. */
   GLuint vbo_nor;   /**< Normal VBO. */
   GLuint vbo_tex;   /**< Texture coordinate VBO. */
   int material;     /**< ID of material to use. */
} Mesh;

struct Node_;
typedef struct Node_ {
   char *name;       /**< Name information. */
   GLfloat H[16];    /**< Homogeneous transform. */
   Mesh *mesh;       /**< Meshes. */
   size_t nmesh;     /**< Number of meshes. */
   struct Node_ *children; /**< Children mesh. */
   size_t nchildren; /**< Number of children mesh. */
} Node;

struct Object_ {
   Node *nodes;         /**< Nodes the object has. */
   size_t nnodes;       /**< Number of nodes. */
   Material *materials; /**< Available materials. */
   size_t nmaterials;   /**< Number of materials. */
   GLfloat radius;      /**< Sphere fit on the model centered at 0,0. */
};

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
      mat->metallic_tex    = tex_ones;
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
      mat->occlusion_tex = object_loadTexture( &cmat->occlusion_texture, tex_ones );
      mat->emissive_tex = object_loadTexture( &cmat->emissive_texture, tex_ones );
   }
   else {
      memset( mat->emissiveFactor, 0, sizeof(GLfloat)*3 );
      mat->emissive_tex = tex_ones;
      mat->occlusion_tex = tex_ones;
   }

   return 0;
}

/**
 * @brief Loads a VBO from an accessor.
 */
static GLuint object_loadVBO( cgltf_accessor *acc )
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
   cgltf_node_transform_local( cnode, node->H );
   //cgltf_node_transform_world( cnode, node->H );

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
                  mesh->vbo_pos = object_loadVBO( attr->data );
                  break;

               case cgltf_attribute_type_normal:
                  mesh->vbo_nor = object_loadVBO( attr->data );
                  break;

               case cgltf_attribute_type_texcoord:
                  mesh->vbo_tex = object_loadVBO( attr->data );
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
   for (size_t i=0; i<cnode->children_count; i++)
      object_loadNodeRecursive( data, &node->children[i], cnode->children[i] );
   return 0;
}

static void object_renderMesh( const Object *obj, const Mesh *mesh, const GLfloat H[16] )
{
   const Material *mat;
   const Shader *shd = &object_shader;

   if (mesh->material < 0)
      mat = &material_default;
   else
      mat = &obj->materials[ mesh->material ];

   /* Depth testing. */
   glEnable( GL_DEPTH_TEST );
   glDepthFunc( GL_LESS );

   glEnable(GL_BLEND);
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

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
   const GLfloat sca = 0.2;
   const GLfloat Hprojection[16] = {
      sca, 0.0, 0.0, 0.0,
      0.0, sca, 0.0, 0.0,
      0.0, 0.0, sca, 0.0,
      0.0, 0.0, 0.0, 1.0 };
   glUniformMatrix4fv( shd->Hprojection, 1, GL_FALSE, Hprojection );
   glUniformMatrix4fv( shd->Hmodel,      1, GL_FALSE, H );
   glUniform1f( shd->metallicFactor, mat->metallicFactor );
   glUniform1f( shd->roughnessFactor, mat->roughnessFactor );
   glUniform4f( shd->baseColour, mat->baseColour[0], mat->baseColour[1], mat->baseColour[2], mat->baseColour[3] );
   glUniform1f( shd->clearcoat, mat->clearcoat );
   glUniform1f( shd->clearcoat_roughness, mat->clearcoat_roughness );
   glUniform3f( shd->emissive, mat->emissiveFactor[0], mat->emissiveFactor[1], mat->emissiveFactor[2] );
   gl_checkErr();

   /* Lighting. */
   glUniform1i( shd->nlights, 1 );
   const ShaderLight *sl = &shd->lights[0];
   glUniform3f( sl->position, 4., 2., -20. );
   glUniform1f( sl->range, -1. );
   glUniform3f( sl->colour, 1.0, 1.0, 1.0 );
   glUniform1f( sl->intensity, 500. );

   /* Texture. */
   glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, mat->baseColour_tex );
      glUniform1i( shd->baseColour_tex, 0 );
   glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, mat->emissive_tex );
      glUniform1i( shd->emissive_tex, 1 );
   glActiveTexture( GL_TEXTURE2 );
      glBindTexture( GL_TEXTURE_2D, mat->occlusion_tex );
      glUniform1i( shd->occlusion_tex, 2 );
   gl_checkErr();

   glDrawElements( GL_TRIANGLES, mesh->nidx, GL_UNSIGNED_INT, 0 );

   glUseProgram( 0 );

   glBindTexture( GL_TEXTURE_2D, 0 );

   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

   glDisable(GL_DEPTH_TEST);

   gl_checkErr();
}

static void matmul( GLfloat H[16], const GLfloat R[16] )
{
   for (int i=0; i<4; i++) {
      float l0 = H[i * 4 + 0];
      float l1 = H[i * 4 + 1];
      float l2 = H[i * 4 + 2];

      float r0 = l0 * R[0] + l1 * R[4] + l2 * R[8];
      float r1 = l0 * R[1] + l1 * R[5] + l2 * R[9];
      float r2 = l0 * R[2] + l1 * R[6] + l2 * R[10];

      H[i * 4 + 0] = r0;
      H[i * 4 + 1] = r1;
      H[i * 4 + 2] = r2;
   }
   H[12] += R[12];
   H[13] += R[13];
   H[14] += R[14];
}

void object_renderNode( const Object *obj, const Node *node, const GLfloat H[16] )
{
   /* Multiply matrices, can be animated so not caching. */
   /* TODO cache when not animated. */
   //matmul( H, node->H );
   GLfloat HH[16];
   memcpy( HH, node->H, sizeof(GLfloat)*16 );
   matmul( HH, H );

   /* Draw meshes. */
   for (size_t i=0; i<node->nmesh; i++)
      object_renderMesh( obj, &node->mesh[i], HH );

   /* Draw children. */
   for (size_t i=0; i<node->nchildren; i++)
      object_renderNode( obj, &node->children[i], HH );

   gl_checkErr();
}

void object_render( const Object *obj, const GLfloat *H )
{
   const GLfloat I[16] = { 1.0, 0.0, 0.0, 0.0,
                           0.0, 1.0, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0,
                           0.0, 0.0, 0.0, 1.0 };

   for (size_t i=0; i<obj->nnodes; i++)
      object_renderNode( obj, &obj->nodes[i], (H!=NULL) ? H : I );
}

Object *object_loadFromFile( const char *filename )
{
   Object *obj;
   cgltf_result res;
   cgltf_data *data;
   cgltf_options opts;
   memset( &opts, 0, sizeof(opts) );

   obj = calloc( sizeof(Object), 1 );

   res = cgltf_parse_file( &opts, filename, &data );
   assert( res == cgltf_result_success );

#if DEBUGGING
   res = cgltf_validate( data );
   assert( res == cgltf_result_success );
#endif /* DEBUGGING */

   res = cgltf_validate( data );
   assert( res == cgltf_result_success );

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
   for (size_t i=0; i<scene->nodes_count; i++)
      object_loadNodeRecursive( data, &obj->nodes[i], scene->nodes[i] );

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
   Shader *shd = &object_shader;

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

   /* Compile the shader. */
   shd->program = gl_program_vert_frag( "gltf.vert", "gltf_pbr.frag" );
   if (shd->program==0)
      return -1;

   glUseProgram( shd->program );
   /** Attributes. */
   shd->vertex          = glGetAttribLocation( shd->program, "vertex" );
   shd->vertex_normal   = glGetAttribLocation( shd->program, "vertex_normal" );
   shd->vertex_tex0     = glGetAttribLocation( shd->program, "vertex_tex0" );
   /* Vertex uniforms. */
   shd->Hprojection     = glGetUniformLocation( shd->program, "projection");
   shd->Hmodel          = glGetUniformLocation( shd->program, "model");
   /* Fragment uniforms. */
   shd->baseColour_tex  = glGetUniformLocation( shd->program, "baseColour_tex" );
   shd->metallic_tex    = glGetUniformLocation( shd->program, "metallic_tex" );
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
   glUseProgram(0);
   gl_checkErr();

#if 0
   GLint count;
   glGetProgramiv( shd->program, GL_ACTIVE_UNIFORMS, &count );
   DEBUG("Active Uniforms: %d", count);
   for (GLint i=0; i < count; i++) {
      GLint size; // size of the variable
      GLenum type; // type of the variable (float, vec3 or mat4, etc)
      const GLsizei bufSize = 16; // maximum name length
      GLchar name[bufSize]; // variable name in GLSL
      GLsizei length; // name length
      glGetActiveUniform(shd->program, i, bufSize, &length, &size, &type, name);
      DEBUG("Uniform #%d Type: %u Name: %s", i, type, name);
   }
#endif

   return 0;
}

void object_exit (void)
{
   glDeleteTextures( 1, &tex_zero );
   glDeleteTextures( 1, &tex_ones );
}
