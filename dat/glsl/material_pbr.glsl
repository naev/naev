// Common stuff for the material_pbr.glsl shaders
#ifndef MAX_LIGHTS
#  define MAX_LIGHTS 7
#endif

/* Lighting information. Set once per scene. */
struct Light {
   vec3 position;    /**< Position or orientation if sun. */
   vec3 colour;      /**< Colour to use. */
   int sun;          /**< Whether or not a sun. */
};
layout(std140) uniform Lighting {
   vec3 ambient; /**< Ambient lighting. */
   int nlights;
   Light lights[ MAX_LIGHTS ];
} lighting;

/* PBR material. Set once per draw call. */
layout(std140) uniform Material {
   vec4 baseColour;
   vec3 emissive;
   float metallicFactor;
   float roughnessFactor;
   int blend;
   int baseColour_texcoord;
   int metallic_texcoord;
   int emissive_texcoord;
   int normal_texcoord;
   int occlusion_texcoord;
   int has_normal; /**< Whether or not has a normal map. */
   float normal_scale;
} material ;

/* Primitive information. Set once per draw call. */
layout(std140) uniform Primitive {
   mat4 model;
   mat3 normal;
   mat4 shadow[MAX_LIGHTS];
} primitive;
