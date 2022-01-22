uniform mat4 projection;
uniform mat4 model;

in vec4 vertex;
in vec3 vertex_normal;
in vec2 vertex_tex;
out vec2 tex_coord;
out vec3 normal;

/* Not including math.glsl here because a python script reads this also and
can't handle include preprocessor. */
const float M_PI        = 3.14159265358979323846;  /* pi */

const float view_angle = M_PI / 4.0;
const mat4 view = mat4(
      1.0,             0.0,              0.0, 0.0,
      0.0,  cos(view_angle), sin(view_angle), 0.0,
      0.0, -sin(view_angle), cos(view_angle), 0.0,
      0.0,             0.0,              0.0, 1.0 );

void main(void) {
   tex_coord   = vec2(vertex_tex.x, -vertex_tex.y);
   normal      = mat3(model) * vertex_normal;
   gl_Position = projection * view * model * vertex;
}
