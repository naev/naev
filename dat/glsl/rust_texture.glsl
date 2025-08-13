layout(std140) uniform TextureData {
   mat3 tex_mat;
   mat3 transform;
   vec4 colour;
};

#if defined(VERT)
layout(location = 0) in vec2 vertex;
out vec2 tex_coord;

void main(void) {
   vec3 pos = vec3( vertex, 1.0 );
   tex_coord = (tex_mat * pos).st;
   gl_Position = vec4( (transform * pos).xy, 0.0, 1.0 );
}
#elif defined(FRAG)
uniform sampler2D sampler;

in vec2 tex_coord;
layout(location = 0) out vec4 colour_out;

void main(void) {
   colour_out = colour * texture(sampler, tex_coord);
}

#endif
