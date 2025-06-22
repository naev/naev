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
   vec3 u_ambient; /**< Ambient lighting. */
   int u_nlights;
   Light u_lights[ MAX_LIGHTS ];
};

/* PBR material. Set once per draw call. */
layout(std140) uniform Material {
   vec4 baseColour;
   vec3 emissive;
   float metallicFactor;
   float roughnessFactor;
   int u_blend;
   int baseColour_texcoord;
   int metallic_texcoord;
   int emissive_texcoord;
   int normal_texcoord;
   int occlusion_texcoord;
   int u_has_normal; /**< Whether or not has a normal map. */
   float normal_scale;
};

/* Primitive information. Set once per draw call. */
layout(std140) uniform Primitive {
   mat4 u_model;
   mat3 u_normal;
   mat4 u_shadow[MAX_LIGHTS];
};
