#ifndef MAX_LIGHTS
#  define MAX_LIGHTS 5
#endif

/* Not including math.glsl here because a python script reads this also and
 * can't handle include preprocessor.
 */
const float M_PI        = 3.14159265358979323846;  /* pi */
const float view_angle = -M_PI / 4.0;
const mat4 view = mat4(
      1.0,            0.0,              0.0, 0.0,
      0.0, cos(view_angle), sin(view_angle), 0.0,
      0.0,-sin(view_angle), cos(view_angle), 0.0,
      0.0,            0.0,              0.0, 1.0 );
uniform mat4 u_shadow[MAX_LIGHTS];
uniform mat4 u_model;
uniform mat3 u_normal;
uniform int u_nlights;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_tex0;
layout(location = 3) in vec2 vertex_tex1;
out vec3 position;
out vec3 shadow[MAX_LIGHTS];
out vec3 normal;
out vec2 tex_coord0;
out vec2 tex_coord1;

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
