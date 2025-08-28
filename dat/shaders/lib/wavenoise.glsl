#ifndef _WAVE_NOISE_H
#define _WAVE_NOISE_H
#include "lib/math.glsl"

/*
 * Noise function modified from https://www.shadertoy.com/view/lscyD7
 * afl_ext 2018
 * public domain
 */

float wave( vec2 uv, vec2 emitter, float speed, float phase, float timeshift )
{
   float dst = distance(uv, emitter);
   return pow(M_E, sin(dst * phase - timeshift * speed)) / M_E;
}
vec2 wavedrag( vec2 uv, vec2 emitter )
{
   return normalize(uv - emitter);
}
float seedWaves = 0.0;
vec2 randWaves2(void)
{
   float x = random(seedWaves);
   seedWaves += 1.0;
   float y = random(seedWaves);
   seedWaves += 1.0;
   return vec2(x,y) * 2.0 - 1.0;
}
float wavenoise( vec2 position, float dragmult, float timeshift )
{
   float iter  = 0.0;
   float phase = 6.0;
   float speed = 2.0;
   float weight= 1.0;
   float w     = 0.0;
   float ws    = 0.0;
   for(int i=0;i<32;i++) {
      vec2 p = randWaves2() * 30.0;
      float res  = wave(position, p, speed, phase, 0.0 + timeshift);
      float res2 = wave(position, p, speed, phase, 0.006 + timeshift);
      position -= wavedrag(position, p) * (res - res2) * weight * dragmult;
      w    += res * weight;
      iter += 12.0;
      ws   += weight;
      weight= mix(weight, 0.0, 0.2);
      phase*= 1.2;
      speed*= 1.02;
   }
   return w / ws;
}

#endif /* _WAVE_NOISE_H */
