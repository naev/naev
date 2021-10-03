uniform vec4 color;
uniform sampler2D sampler;

in vec2 tex_coord;
out vec4 color_out;

const float ALPHA = 0.2;

void main(void) {
   vec4 tex = texture(sampler, tex_coord);
   tex.rgb = vec3(
      tex.r,
      0.0,
      tex.b - tex.g
   ) * tex.a;

   float s = tex.r + tex.b;
   float a = mix( ALPHA, ALPHA + ALPHA*(1.0-ALPHA), s-1.0 );

   color_out = color * vec4( 0.8*tex.rgb, a );
}

