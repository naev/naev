uniform sampler2D u_tex;

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   vec2 uv = tex_coord;
   colour_out.rgb = vec3(0.0);
   colour_out.a = texture( u_tex, uv ).a;
}
