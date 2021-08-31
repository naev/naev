uniform vec4 color;
uniform sampler2D sampler;

in vec2 tex_coord;
out vec4 color_out;

void main(void) {
   vec4 tex = texture(sampler, tex_coord);
   tex.rgb = vec3( min(1.0, tex.r*100.0), 0.0,
         min(1.0, tex.b*100.0) * (1.0-step(1e-3,tex.g)) );
   color_out = color * vec4( 0.6*tex.rgb, 1.0 );
}

