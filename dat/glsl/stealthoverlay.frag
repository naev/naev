uniform vec4 colour;
uniform sampler2D sampler;

in vec2 tex_coord;
out vec4 colour_out;

const float ALPHA = 0.3;

void main(void) {
   vec4 tex = texture(sampler, tex_coord);
   float pilotaura = tex.r;
   float astaura   = tex.b;
   tex.rgb = vec3( pilotaura, 0.0, astaura ) * tex.a;

   float s = pilotaura + astaura;
   float a = mix( ALPHA, ALPHA + ALPHA*(1.0-ALPHA), s-1.0 );

   colour_out = colour * vec4( 0.8*tex.rgb, tex.a*a );
}
