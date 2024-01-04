/* Not including math.glsl here because a python script reads this also and
	can't handle include preprocessor. */
const float M_PI        = 3.14159265358979323846;  /* pi */
const float view_angle = -M_PI / 4.0;
const mat4 view = mat4(
      1.0,             0.0,              0.0, 0.0,
      0.0,  cos(view_angle), sin(view_angle), 0.0,
      0.0, -sin(view_angle), cos(view_angle), 0.0,
      0.0,             0.0,              0.0, 1.0 );
uniform mat4 shadow_projection;
uniform mat4 model;

in vec3 vertex;
in vec3 vertex_normal;
in vec2 vertex_tex0;
out vec3 position;
out vec3 shadow;
out vec3 normal;
out vec2 tex_coord0;

void main (void)
{
   /* Coordinates and position. */
   vec4 pos    = model * vec4( vertex, 1.0 );
   tex_coord0  = vertex_tex0;

   /* Compute normal vector. */
   normal      = transpose(inverse(mat3(model))) * vertex_normal;

   /* Position for fragment shader. */
   position    = pos.xyz;
   gl_Position = view * pos;

   /* Shadows. */
   vec4 shadow_pos = shadow_projection * pos;
   shadow = shadow_pos.rgb / shadow_pos.w;
   shadow = shadow * 0.5 + 0.5;
}
