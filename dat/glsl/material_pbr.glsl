// Common stuff for the material_pbr.glsl shaders
#ifndef MAX_LIGHTS
#  define MAX_LIGHTS 7
#endif

/**
 * Lighting information.
 */
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
