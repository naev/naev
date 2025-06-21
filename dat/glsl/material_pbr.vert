#include "material_pbr.glsl"
#include "lib/math.glsl"

const float view_angle = -M_PI / 4.0;
const mat4 view = mat4(
      1.0,            0.0,              0.0, 0.0,
      0.0, cos(view_angle), sin(view_angle), 0.0,
      0.0,-sin(view_angle), cos(view_angle), 0.0,
      0.0,            0.0,              0.0, 1.0 );

layout(std140) uniform Primitive {
   mat4 u_model;
   mat3 u_normal;
   mat4 u_shadow[MAX_LIGHTS];
};

/* Vertex inputs. */
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_tex0;
layout(location = 3) in vec2 vertex_tex1;
/* Vertex outputs. */
out vec2 tex_coord0;
out vec2 tex_coord1;
out vec3 position;
out vec3 shadow[MAX_LIGHTS];
out vec3 normal;

void main (void)
{
   /* Coordinates and position. */
   vec4 pos    = u_model * vec4( vertex, 1.0 );
   position    = pos.xyz / pos.w;
   tex_coord0  = vertex_tex0;
   tex_coord1  = vertex_tex1;

   /* Compute normal vector. */
   normal      = -normalize(u_normal * vertex_normal); /* TODO why is it inverted?? */

   /* Position for fragment shader. */
   gl_Position = view * pos;

   /* Shadows. */
   for (int i=0; i<u_nlights; i++) {
      vec4 shadow_pos = u_shadow[i] * pos;
      shadow[i] = shadow_pos.rgb / shadow_pos.w;
      shadow[i] = shadow[i] * 0.5 + 0.5;
   }
}
