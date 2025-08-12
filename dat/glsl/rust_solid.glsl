layout(std140) uniform SolidData {
   mat3 transform;
   vec4 colour;
};

#if defined(VERT)
layout(location = 0) in vec2 vertex;

void main(void) {
   vec3 pos = vec3( vertex, 1.0 );
   gl_Position = vec4( (transform * pos).xy, 0.0, 1.0 );
}
#elif defined(FRAG)
layout(location = 0) out vec4 colour_out;

void main(void) {
   colour_out = colour;
}
#endif
