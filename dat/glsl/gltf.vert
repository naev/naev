#ifndef MAX_LIGHTS
#  define MAX_LIGHTS 3
#endif

/* Not including math.glsl here because a python script reads this also and
	can't handle include preprocessor. */
const float M_PI        = 3.14159265358979323846;  /* pi */
const float view_angle = M_PI / 4.0;
const mat4 view = mat4(
      1.0,            0.0,              0.0, 0.0,
      0.0, cos(view_angle),-sin(view_angle), 0.0,
      0.0, sin(view_angle), cos(view_angle), 0.0,
      0.0,            0.0,              0.0, 1.0 );
uniform mat4 u_shadow[MAX_LIGHTS];
uniform mat4 u_model;
uniform float u_time;

in vec3 vertex;
in vec3 vertex_normal;
in vec2 vertex_tex0;
in vec2 vertex_tex1;
out vec3 position;
out vec3 shadow[MAX_LIGHTS];
out vec3 normal;
out vec2 tex_coord0;
out vec2 tex_coord1;

void main (void)
{
   /* Coordinates and position. */
   vec4 pos    = u_model * vec4( vertex, 1.0 );
   position    = pos.xyz;
   tex_coord0  = vertex_tex0;
   tex_coord1  = vertex_tex1;

   /* Compute normal vector. */
   normal      = transpose(inverse(mat3(u_model))) * vertex_normal;

   /* Position for fragment shader. */
   gl_Position = view * pos;

   /* Shadows. */
   for (int i=0; i<MAX_LIGHTS; i++) {
      vec4 shadow_pos = u_shadow[i] * pos;
      shadow[i] = shadow_pos.rgb / shadow_pos.w;
      shadow[i] = shadow[i] * 0.5 + 0.5;
   }
}
