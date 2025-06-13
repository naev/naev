uniform sampler2D u_tex;

uniform float u_timer   = 0.0;
uniform float u_elapsed = 0.0;

const float LENGTH   = 3.0;

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   vec2 uv = tex_coord;
   colour_out = texture( u_tex, uv );
   float progress = u_elapsed / LENGTH;
   colour_out.rgb = mix( vec3(0.0), colour_out.rgb, progress );
}
