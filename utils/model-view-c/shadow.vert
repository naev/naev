const mat4 I = mat4(
   1.0, 0.0, 0.0, 0.0,
   0.0, 1.0, 0.0, 0.0,
   0.0, 0.0, 1.0, 0.0,
   0.0, 0.0, 0.0, 1.0 );

uniform mat4 shadow_projection = I;
uniform mat4 model = I;

in vec3 vertex;

void main (void)
{
   gl_Position = shadow_projection * model * vec4( vertex, 1.0 );
}
