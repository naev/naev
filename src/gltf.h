/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

#include <stddef.h>

#include "glad.h"

#include "mat4.h"
#include "vec3.h"

#define MAX_LIGHTS 3    /**< Maximum amount of lights. TODO deferred rendering. */
#define SHADOWMAP_SIZE  128   /**< Size of the shadow map. */

typedef struct Texture_ {
   GLuint tex;
   GLuint texcoord;
   GLfloat strength;
} Texture;

/**
 * @brief PBR Material of an object.
 */
typedef struct Material_ {
   char *name; /**< Name of the material if applicable. */
   int blend;  /**< Whether or not to blend it. */
   int noshadows; /**< Whether or not it ignores shadows. */
   /* pbr_metallic_roughness */
   Texture baseColour_tex;  /**< Base colour of the material. */
   Texture metallic_tex;    /**< Metallic/roughness map of the material. Metallic is stored in G channel, hile roughness is in the B channel. */
   GLfloat metallicFactor; /**< Metallic factor (single value). Multplies the map if available. */
   GLfloat roughnessFactor;/**< Roughness factor (single value). Multiplies the map if available. */
   GLfloat baseColour[4];  /**< Base colour of the material. Multiplies the texture if available. */
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
   //GLfloat waxiness;
} Material;

/**
 * @brief Represents an underlying 3D mesh.
 */
typedef struct Mesh_ {
   size_t nidx;      /**< Number of indices. */
   GLuint vbo_idx;   /**< Index VBO. */
   GLuint vbo_pos;   /**< Position VBO. */
   GLuint vbo_nor;   /**< Normal VBO. */
   GLuint vbo_tex0;  /**< Texture 0 coordinate VBO. */
   GLuint vbo_tex1;  /**< Texture 1 coordinate VBO. */
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

typedef struct Scene_ {
   char *name;          /**< Name of the scene. */
   Node *nodes;         /**< Nodes the object has. */
   size_t nnodes;       /**< Number of nodes. */
} Scene;

/**
 * @brief Defines a complete object.
 */
typedef struct GltfObject_ {
   Scene *scenes;       /**< Number of scenes. */
   size_t nscenes;      /**< Number of scenes. */
   Material *materials; /**< Available materials. */
   size_t nmaterials;   /**< Number of materials. */
   GLfloat radius;      /**< Sphere fit on the model centered at 0,0. */
   vec3 aabb_min;       /**< Minimum value of AABB wrapping around it. */
   vec3 aabb_max;       /**< Maximum value of AABB wrapping around it. */
   /* Some useful default scenes. */
   int scene_body;      /**< Body of the object. */
   int scene_engine;    /**< Engine of the object (if applicable or -1) */
} GltfObject;

/**
 * @brief Simple point/sun light model.
 */
typedef struct Light_ {
   int sun;          /**< Whether or not it's a sun-type light source. */
   /* left(-)/right(+), down(-)/up(+), forward(-)/back(+) */
   vec3 pos;         /**< Position of the light in normalized coordinates, or orientation for pos (defined as vector from origin to opposite direction). */
   double intensity; /**< Radiosity of the lights. */
   vec3 colour;      /**< Light colour. */
} Light;

typedef struct Lighting_ {
   double ambient_r, ambient_g, ambient_b; /**< Ambient lighting. */
   Light lights[MAX_LIGHTS];  /**< Standard lights. */
   int nlights; /**< Number of lights being used. Has to be less than MAX_LIGHTS. */
} Lighting;

/* Framework itself. */
int gltf_init (void);
void gltf_exit (void);

/* Loading and freeing. */
GltfObject *gltf_loadFromFile( const char *filename );
void gltf_free( GltfObject *obj );

/* Rendering and updating. */
void gltf_render( GLuint fb, const GltfObject *obj, const mat4 *H, double time, double size );
void gltf_renderScene( GLuint fb, const GltfObject *obj, int scene, const mat4 *H, double time, double size, const Lighting *L );

/* Lighting. */
void gltf_light( double r, double g, double b, double intensity );
void gltf_lightGet( double *r, double *g, double *b, double *intensity );
void gltf_lightAmbient( double r, double g, double b );
void gltf_lightAmbientGet( double *r, double *g, double *b );
void gltf_lightIntensity( double strength );
double gltf_lightIntensityGet (void);

/* Misc functions. */
GLuint gltf_shadowmap( int light );
