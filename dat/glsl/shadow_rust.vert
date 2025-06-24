#if GLSL_VERSION >= 420
layout(std140, binding=0) uniform Shadow {
#else
layout(std140) uniform Shadow {
#endif
   mat4 view;
} shadow;

layout(location = 0) in vec3 vertex;

void main (void)
{
   gl_Position = shadow.view * vec4( vertex, 1.0 );
}
