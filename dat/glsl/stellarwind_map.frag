#include "lib/nebula.glsl"

uniform float brightness;
uniform float time;
uniform vec2 globalpos;
uniform float alpha;
in vec2 localpos;
out vec4 colour_out;

const float strength = 1.0;
const float speed    = 1.0;
const float density  = 0.7;

const float smoothness     = 0.7;
const float noiseScale     = 0.001;
const float noiseTimeScale = 0.03;

float fbm3(vec3 v) {
   float result = snoise(v);
   result += snoise(v * 2.0) / 2.0;
   result += snoise(v * 4.0) / 4.0;
   result /= (1.0 + 1.0/2.0 + 1.0/4.0);
   return result;
}

float getNoise(vec3 v) {
   v.xy += vec2( fbm3(v), fbm3(vec3(v.xy, v.z + 1000.0)));
   return fbm3(v) / 2.0 + 0.5;
}

void main (void)
{
   float f = 0.0;
   vec3 uv;

   /* Fallout */
   float dist = length(localpos);
   dist = (dist < 1.0-smoothness) ? 1.0 : (1.0 - dist) / smoothness;
   float a = smoothstep( 0.0, 1.0, dist );
   if (a <= 0.0) {
      discard;
      return;
   }

   /* Calculate coordinates */
   uv.xy = (localpos + globalpos) * 1000.0;
   uv.xy *= strength;
   uv.z  = time * speed;

   uv *= vec3( noiseScale, noiseScale, noiseTimeScale );

   float noise = getNoise( uv );
   noise = pow( noise, 4.0 / density ) * 2.0;  //more contrast
   colour_out = vec4( 0.2, 0.6, 0.9, noise );
   colour_out *= alpha * a;
}
