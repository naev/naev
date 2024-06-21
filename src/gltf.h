/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include <stddef.h>

#include "glad.h"

#include "mat4.h"
#include "vec3.h"

#define MAX_LIGHTS                                                             \
   5 /**< Maximum amount of lights. TODO deferred rendering.                   \
      */

typedef struct Texture {
   GLuint  tex;
   GLuint  texcoord;
   GLfloat strength;
} Texture;

/**
 * @brief PBR Material of an object.
 */
typedef struct Material {
   char *name;         /**< Name of the material if applicable. */
   int   blend;        /**< Whether or not to blend it. */
   int   noshadows;    /**< Whether or not it ignores shadows. */
   int   double_sided; /**< Whether or not it's double sided. */
   int   unlit;        /**< Whether or not the texture is unlit. */
   /* pbr_metallic_roughness */
   Texture baseColour_tex; /**< Base colour of the material. */
   Texture metallic_tex;   /**< Metallic/roughness map of the material. Metallic
                              is stored in G channel, hile roughness is in the B
                              channel. */
   GLfloat metallicFactor; /**< Metallic factor (single value). Multplies the
                              map if available. */
   GLfloat roughnessFactor; /**< Roughness factor (single value). Multiplies the
                               map if available. */
   GLfloat baseColour[4];   /**< Base colour of the material. Multiplies the
                               texture if available. */
   /* pbr_specular_glossiness */
   /* Sheen. */
   GLfloat sheen[3];
   GLfloat sheen_roughness;
   /* Clearcoat */
   /*GLuint clearcoat_tex;
   GLuint clearcoat_roughness_tex;
   GLuint clearcoat_normal_tex; */
   GLfloat clearcoat;
   GLfloat clearcoat_roughness;
   /* misc. */
   Texture normal_tex;
   Texture occlusion_tex;
   Texture emissive_tex;
   GLfloat emissiveFactor[3];
   /* Custom Naev. */
   // GLfloat waxiness;
} Material;

/**
 * @brief Represents the underlyig 3D data and associated material.
 */
typedef struct MeshPrimitive {
   size_t nidx;     /**< Number of indices. */
   GLuint vbo_idx;  /**< Index VBO. */
   GLuint vbo_pos;  /**< Position VBO. */
   GLuint vbo_nor;  /**< Normal VBO. */
   GLuint vbo_tex0; /**< Texture 0 coordinate VBO. */
   GLuint vbo_tex1; /**< Texture 1 coordinate VBO. */
   int    material; /**< ID of material to use. */
} MeshPrimitive;

/**
 * @brief Represents a mesh that can be made of multiple primitives.
 */
typedef struct Mesh {
   MeshPrimitive *primitives;  /**< Primitives in the mesh. */
   int            nprimitives; /**< Number of primitives. */
} Mesh;

typedef struct NodeTransform {
   vec3 t; /**< Translation from animation. */
   quat r; /**< Rotation from animation. */
   vec3 s; /**< Scale from animation. */
} NodeTransform;

/**
 * @brief Represents a node of an object. Each node can have multiple meshes and
 * children nodes with an associated transformation.
 */
typedef struct Node {
   char *name;  /**< Name information. */
   mat4  H;     /**< Homogeneous transform. */
   mat4  Horig; /**< Base homogeneous transform. */
   int   mesh;  /**< Associated Mesh. */
   // int parent;     /**< Parent node. */
   size_t *children;  /**< Children nodes. */
   size_t  nchildren; /**< Number of children mesh. */

   GLfloat radius;   /**< Sphere fit on the model centered at 0,0. */
   vec3    aabb_min; /**< Minimum value of AABB wrapping around it. */
   vec3    aabb_max; /**< Maximum value of AABB wrapping around it. */

   /* Animation data. */
   int           has_anim; /**< Has an animation. */
   NodeTransform nt;       /**< Animated transform. */
   NodeTransform ntorig;   /**< Original values. */
} Node;

typedef enum AnimationInterpolation {
   ANIM_INTER_LINEAR,
   ANIM_INTER_STEP,
} AnimationInterpolation;

typedef enum AnimationType {
   ANIM_TYPE_ROTATION,
   ANIM_TYPE_TRANSLATION,
   ANIM_TYPE_SCALE,
} AnimationType;

typedef struct AnimationSampler {
   float                 *time;   /**< Time data for keyframes. */
   GLfloat               *data;   /**< Associated data for keyframes. */
   AnimationInterpolation interp; /**< Type of interpolation. */
   size_t                 n;      /**< Number of keyframes. */
   size_t                 l;      /**< Length of each data element. */
   size_t                 cur;    /**< Current activate keyframe. */
   GLfloat                max;    /**< Last time of keyframe. */
} AnimationSampler;

typedef struct AnimationChannel {
   AnimationType     type;    /**< Type of animation. */
   Node             *target;  /**< Target node to modify. */
   AnimationSampler *sampler; /**< Keyframe sampling data. */
} AnimationChannel;

typedef struct Animation {
   char             *name;      /**< Name of the animation. */
   AnimationSampler *samplers;  /**< Samplers of the animation. */
   size_t            nsamplers; /**< Number of sampler.s */
   AnimationChannel *channels;  /**< Channels of the animation. */
   size_t            nchannels; /**< Number of channels. */
} Animation;

/**
 * @brief Represents a scene that can have multiple nodes.
 */
typedef struct Scene {
   char   *name;   /**< Name of the scene. */
   size_t *nodes;  /**< Nodes the scene has. */
   size_t  nnodes; /**< Number of nodes. */
} Scene;

typedef struct GltfTrail {
   char *generator;
   vec3  pos;
} GltfTrail;

typedef struct GltfMount {
   int  id;
   vec3 pos;
} GltfMount;

/**
 * @brief Defines a complete object.
 */
typedef struct GltfObject {
   char      *path; /**< Path containing the gltf, used for finding elements. */
   Mesh      *meshes;      /**< The meshes. */
   size_t     nmeshes;     /**< Number of meshes. */
   Node      *nodes;       /**< The nodes. */
   size_t     nnodes;      /**< Number of nodes. */
   Scene     *scenes;      /**< The scenes. */
   size_t     nscenes;     /**< Number of scenes. */
   Material  *materials;   /**< Available materials. */
   size_t     nmaterials;  /**< Number of materials. */
   Animation *animations;  /**< The animations. */
   size_t     nanimations; /**< Number of animations. */
   GLfloat    radius;      /**< Sphere fit on the model centered at 0,0. */
   /* Some useful default scenes. */
   int scene_body;   /**< Body of the object. */
   int scene_engine; /**< Engine of the object (if applicable or -1) */
   /* Useful things used for special cases. */
   GltfTrail *trails; /**< Trails for trail generation. */
   GltfMount *mounts; /**< Mount points fo weapons. */
} GltfObject;

/**
 * @brief Simple point/sun light model.
 */
typedef struct Light {
   int sun; /**< Whether or not it's a sun-type light source. */
   /* left(-)/right(+), down(-)/up(+), forward(-)/back(+) */
   vec3
      pos; /**< Position of the light in normalized coordinates, or orientation
              for pos (defined as vector from origin to opposite direction). */
   double intensity; /**< Radiosity of the lights. */
   vec3   colour;    /**< Light colour. */
} Light;

typedef struct Lighting {
   double ambient_r, ambient_g, ambient_b; /**< Ambient lighting. */
   Light  lights[MAX_LIGHTS];              /**< Standard lights. */
   int    nlights;   /**< Number of lights being used. Has to be less than
                        MAX_LIGHTS. */
   double intensity; /**< Scales the intensity of the lights globally. */
} Lighting;
extern const Lighting
   L_default_const; /**< Default constant lighting for resetting. */
extern const Lighting L_store_const; /**< Default store lighting setting. */
extern Lighting       L_default;     /**< Default space lighting. */

/* Framework itself. */
int  gltf_init( void );
void gltf_exit( void );

/* Loading and freeing. */
GltfObject *gltf_loadFromFile( const char *filename );
void        gltf_free( GltfObject *obj );

/* Rendering and updating. */
void gltf_render( GLuint fb, GltfObject *obj, const mat4 *H, GLfloat time,
                  double size );
void gltf_renderScene( GLuint fb, GltfObject *obj, int scene, const mat4 *H,
                       GLfloat time, double size, const Lighting *L );

/* Lighting. */
void   gltf_lightReset( void );
void   gltf_lightSet( int idx, const Light *L );
void   gltf_lightAmbient( double r, double g, double b );
void   gltf_lightAmbientGet( double *r, double *g, double *b );
void   gltf_lightIntensity( double strength );
double gltf_lightIntensityGet( void );
void   gltf_lightTransform( Lighting *L, const mat4 *H );

/* Misc functions. */
GLuint gltf_shadowmap( int light );
