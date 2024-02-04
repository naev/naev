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

#define OBJECT_FLAG_NOLIGHTS  (1<<0)   /**< Do not run shadows computations. */

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
typedef struct Object_ {
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
} Object;

/* Framework itself. */
int object_init (void);
void object_exit (void);

/* Loading and freeing. */
Object *object_loadFromFile( const char *filename );
void object_free( Object *obj );

/* Rendering and updating. */
void object_render( GLuint fb, const Object *obj, const mat4 *H, double time, double size );
void object_renderScene( GLuint fb, const Object *obj, int scene, const mat4 *H, double time, double size, unsigned int flags );

/* Lighting. */
void object_lightAmbient( double r, double g, double b );

/* Misc functions. */
GLuint object_shadowmap( int light );
