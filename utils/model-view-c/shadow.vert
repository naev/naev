uniform mat4 u_shadow;
uniform mat4 u_model;

in vec3 vertex;

void main (void)
{
   gl_Position = u_shadow * u_model * vec4( vertex, 1.0 );
}
