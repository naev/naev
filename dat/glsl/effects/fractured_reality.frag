uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_timer;
uniform float u_elapsed;

in vec2 tex_coord;
out vec4 colour_out;

void main(void) {
   colour_out = texture( u_tex, tex_coord );
   float f = smoothstep( 0.0, 1.0, min(u_timer,u_elapsed)*10.0 );
   colour_out.rgb = mix( colour_out.rgb, 1.0-colour_out.rgb, f );
}
