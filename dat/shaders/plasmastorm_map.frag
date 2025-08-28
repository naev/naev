#include "lib/simplex.glsl"
#include "lib/wavenoise.glsl"

uniform float time;
uniform vec2 globalpos;
uniform float alpha;

in vec2 localpos;
out vec4 colour_out;

const float DRAGMULT    = 10.0;
const float SCALE       = 1.0;
const vec3 COL_BACK     = vec3( 150.0, 50.0, 150.0 ) / 255.;
const vec3 COL_FRONT    = vec3( 220.0, 80.0, 220.0 ) / 255.;
const float SMOOTHNESS  = 0.9;

void main (void)
{
   vec3 uv;

   /* Fallout */
   float dist = length(localpos);
   dist = (dist < 1.0-SMOOTHNESS) ? 1.0 : (1.0 - dist) / SMOOTHNESS;
   float a = smoothstep( 0.0, 1.0, dist );
   if (a <= 0.0) {
      discard;
      return;
   }

   /* Coordinates. */
   uv.xy = (localpos + globalpos) / SCALE;
   uv.z  = time;

   /* Generate the base noise. */
   float s = wavenoise( uv.xy, DRAGMULT, snoise(vec3(uv.xy*1.7,uv.z/10.0))+time*0.3);
   s = s*s;
   colour_out.rgb = mix( COL_BACK, COL_FRONT, s );
   colour_out.a = s;

   /* Flashing parts. */
   float flash_0 = sin(time);
   float flash_1 = sin(15.0 * time);
   float flash_2 = sin(2.85 * time);
   float flash_3 = sin(5.18 * time);
   float flash = max( 0.0,  snoise( uv*0.5 ) * flash_0 * flash_1 * flash_2 * flash_3 );
   colour_out += vec4( 1.5*(s+0.5*flash)*flash*(0.5+0.5*flash_2) );
   colour_out = clamp( colour_out, vec4(0.0), vec4(1.0) );
   colour_out.a *= alpha * a * 0.7;
}
