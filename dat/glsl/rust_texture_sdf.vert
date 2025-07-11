layout(std140) uniform TextureData {
   mat3 tex_mat;
   mat3 transform;
   vec4 colour;
};

layout(location = 0) in vec2 vertex;
out vec2 tex_coord;

void main(void) {
   vec3 pos = vec3( vertex, 1.0 );
   tex_coord = (tex_mat * (pos*0.5+0.5)).st;
   gl_Position = vec4( (transform * pos).xy, 0.0, 1.0 );
}
