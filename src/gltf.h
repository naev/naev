/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/* We use this file in utils/model-view-c to debug things. */
#ifdef PACKAGE
#define HAVE_NAEV
#endif

#include <stddef.h>

#include "glad.h"

#include "mat4.h"
#include "vec3.h"
#ifdef HAVE_NAEV
#include "opengl_tex.h"
#endif /* HAVE_NAEV */

#define MAX_LIGHTS                                                             \
   7 /**< Maximum amount of lights. TODO deferred rendering.                   \
      */

typedef struct GltfObject GltfObject;

typedef struct GltfTrail {
   char *generator; /**< Type of the trail to use. */
   vec3  pos;       /**< Location of the trail. */
} GltfTrail;

typedef struct GltfMount {
   int id;   /**< ID of the mount, should match with the position of the XML
                value. */
   vec3 pos; /**< Position of the mount. */
} GltfMount;

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
GltfObject *gltf_loadFromFile_( const char *filename );
void        gltf_free( GltfObject *obj );

/* Rendering and updating. */
void gltf_render( GLuint fb, GltfObject *obj, const mat4 *H, GLfloat time,
                  double size );
void gltf_renderScene( GLuint fb, GltfObject *obj, int scene, const mat4 *H,
                       GLfloat time, double size, const Lighting *L );

/* Lighting. */
void   gltf_lightReset( void );
int    gltf_lightSet( int idx, const Light *L );
void   gltf_lightAmbient( double r, double g, double b );
void   gltf_lightAmbientGet( double *r, double *g, double *b );
void   gltf_lightIntensity( double strength );
double gltf_lightIntensityGet( void );
void   gltf_lightTransform( Lighting *L, const mat4 *H );
int    gltf_numLights( void );

int   gltf_sceneBody( const GltfObject *obj );
int   gltf_sceneEngine( const GltfObject *obj );
int   gltf_numAnimations( const GltfObject *obj );
int   gltf_numTrails( const GltfObject *obj );
char *gltf_trailName( const GltfObject *obj, int id );
vec3  gltf_trailPosition( const GltfObject *obj, int id );
int   gltf_numMounts( const GltfObject *obj );
int   gltf_mountIndex( const GltfObject *obj, int id );
vec3  gltf_mountPosition( const GltfObject *obj, int id );

/* Misc functions. */
GLuint gltf_shadowmap( int light );
