uniform vec4 color;
uniform sampler2D sampler;

in vec2 tex_coord;
out vec4 color_out;

const float ALPHA = 0.3;

void main(void) {
   vec4 tex = texture(sampler, tex_coord);
   float pilotaura = tex.r;
   float astaura   = tex.b;
   tex.rgb = vec3( pilotaura, 0.0, astaura ) * tex.a;

   float s = pilotaura + astaura;
   float a = mix( ALPHA, ALPHA + ALPHA*(1.0-ALPHA), s-1.0 );

   color_out = color * vec4( 0.8*tex.rgb, tex.a*a );
}
