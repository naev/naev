#include "material_pbr.glsl"
#include "lib/math.glsl"

const float view_angle = -M_PI / 4.0;
const mat4 view = mat4(
      1.0,            0.0,              0.0, 0.0,
      0.0, cos(view_angle), sin(view_angle), 0.0,
      0.0,-sin(view_angle), cos(view_angle), 0.0,
      0.0,            0.0,              0.0, 1.0 );

/* Vertex inputs. */
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_tex0;
layout(location = 3) in vec2 v_tex1;
/* Vertex outputs. */
#if GLSL_VERSION >= 440
layout(location=0) out InterfBlock {
#else
out InterfBlock {
#endif
   vec3 position;
   vec3 normal;
   vec2 tex0;
   vec2 tex1;
   vec3 shadow[MAX_LIGHTS];
} OUT;

void main (void)
{
   /* Coordinates and position. */
   vec4 pos    = primitive.model * vec4( vertex, 1.0 );
   OUT.position= pos.xyz / pos.w;
   OUT.tex0    = v_tex0;
   OUT.tex1    = v_tex1;

   /* Compute normal vector. */
   OUT.normal  = -normalize(primitive.normal * v_normal); /* TODO why is it inverted?? */

   /* Position for fragment shader. */
   gl_Position = view * pos;

   /* Shadows. */
   for (int i=0; i<lighting.nlights; i++) {
      vec4 shadow_pos = primitive.shadow[i] * pos;
      OUT.shadow[i] = shadow_pos.rgb / shadow_pos.w;
      OUT.shadow[i] = OUT.shadow[i] * 0.5 + 0.5;
   }
}
