// \file starfield.pix
// \author Morgan McGuire
//
// \cite Based on Star Nest by Kali
// https://www.shadertoy.com/view/4dfGDM
// That shader and this one are open source under the MIT license
//
// Assumes an sRGB target (i.e., the output is already encoded to gamma 2.1)
// viewport resolution (in pixels)
uniform vec2 u_resolution;

// In the noise-function space. xy corresponds to screen-space XY
uniform vec3 u_origin = vec3(1.0);

uniform float zoom = 0.1;

//uniform mat2   rotate;
const float theta = 1.5;
const mat2 rotate = mat2( cos(theta), -sin(theta), sin(theta), cos(theta) );

//uniform sampler2D oldImage;

#define iterations 17

#define volsteps 8

#define sparsity 0.5  // 0.4 to 0.5 (sparse)
#define stepsize 0.2

#define frequencyVariation   1.3 // 0.5 to 2.0

#define brightness 0.0018
#define distfading 0.6800

vec4 effect( vec4 colour_in, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = gl_FragCoord.xy / u_resolution - 0.5;
   uv.y *= u_resolution.y / u_resolution.x;

   vec3 dir = vec3(uv * zoom, 1.0);
   dir.xz *= rotate;

   float s = 0.1, fade = 0.01;
   vec4 colour = vec4( vec3(0.0), 1.0 );

   for (int r = 0; r < volsteps; ++r) {
      vec3 p = u_origin + dir * (s * 0.5);
      p = abs(vec3(frequencyVariation) - mod(p, vec3(frequencyVariation * 2.0)));

      float prevlen = 0.0, a = 0.0;
      for (int i = 0; i < iterations; ++i) {
         p = abs(p);
         p = p * (1.0 / dot(p, p)) + (-sparsity); // the magic formula
         float len = length(p);
         a += abs(len - prevlen); // absolute sum of average change
         prevlen = len;
      }

      a *= a * a; // add contrast

      // coloring based on distance
      colour.rgb += (vec3(s, s*s, s*s*s) * a * brightness + 1.0) * fade;
      fade *= distfading; // distance fading
      s += stepsize;
   }

   colour.rgb = min(colour.rgb, vec3(1.2));

   // Detect and suppress flickering single pixels (ignoring the huge gradients that we encounter inside bright areas)
   float intensity = min(colour.r + colour.g + colour.b, 0.7);

   ivec2 sgn = (ivec2(gl_FragCoord.xy) & 1) * 2 - 1;
   vec2 gradient = vec2(dFdx(intensity) * sgn.x, dFdy(intensity) * sgn.y);
   float cutoff = max(max(gradient.x, gradient.y) - 0.1, 0.0);
   colour.rgb *= max(1.0 - cutoff * 6.0, 0.3);

   // Motion blur; increases temporal coherence of undersampled flickering stars
   // and provides temporal filtering under true motion.
   //vec3 oldValue = texelFetch(oldImage, int2(gl_FragCoord.xy), 0).rgb;
   //colour.rgb = lerp(oldValue - vec3(0.004), colour.rgb, 0.5);
   colour.a = 1.0;

   return colour;
}
