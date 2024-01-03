uniform mat4 shadow_projection;
uniform mat4 model;

in vec3 vertex;

void main (void)
{
   gl_Position = shadow_projection * model * vec4( vertex, 1.0 );
}
