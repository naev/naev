#include "lib/gamma.glsl"
/*
 * Based on http://casual-effects.blogspot.com/2013/08/starfield-shader.html by Morgan McGuire
 * which is based on Star Nest by Kali https://www.shadertoy.com/view/XlfGRj
 * Both under MIT license.
 * Adapted to the Naev engine by bobbens
 */
uniform vec2 u_resolution;
uniform vec4 u_camera = vec4(1.0); /* xy corresponds to screen space */
uniform sampler2D u_prevtex;
const vec2 R      = vec2( %f, %f );
const float THETA = %f;
const mat2 ROT    = mat2( cos(THETA), -sin(THETA), sin(THETA), cos(THETA) );
const mat2 IROT   = inverse(ROT);

#define ITERATIONS   17
#define VOLSTEPS     8
#define SPARSITY     0.7  // 0.4 to 0.5 (sparse)
#define STEPSIZE     0.2
#define FREQVAR      1.8 // 0.5 to 2.0
#define BRIGHTNESS   0.0010
#define DISTFADING   0.6800

vec4 effect( vec4 colour_in, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = ROT*((texture_coords - 0.5) * love_ScreenSize.xy * u_camera.w) + R;
   vec3 dir = vec3(uv, 1.0);
   vec3 cam = vec3(1.0) + vec3(ROT*u_camera.xy, u_camera.z);

   float s = 0.1, fade = 0.01;
   vec4 colour = vec4( vec3(0.0), 1.0 );

   for (int r=0; r < VOLSTEPS; r++) {
      vec3 p = cam + dir * (s * 0.5);
      p = abs(vec3(FREQVAR) - mod(p, vec3(FREQVAR * 2.0)));

      float prevlen = 0.0, a = 0.0;
      for (int i=0; i < ITERATIONS; i++) {
         p = abs(p);
         p = p * (1.0 / dot(p, p)) + (-SPARSITY); // the magic formula
         float len = length(p);
         a += abs(len - prevlen); // absolute sum of average change
         prevlen = len;
      }

      a *= a * a; // add contrast

      /* Colouring based on distance. */
      colour.rgb += (vec3(s, s*s, s*s*s) * a * BRIGHTNESS + 1.0) * fade;
      fade *= DISTFADING; /* Distance fading. */
      s += STEPSIZE;
   }
   colour.rgb = min(colour.rgb, vec3(1.2));

   /* Some cheap antialiasing filtering. */
   float intensity = min(colour.r + colour.g + colour.b, 0.7);
   float w = fwidth(intensity);
   colour.rgb = mix( colour.rgb, vec3(0.0), smoothstep(0.0,1.0,w) );

   /* Colour conversion. */
   colour.rgb = clamp( pow( colour.rgb, vec3(2.0) ), 0.0, 1.0 );

   /* Motion blur to increase temporal coherence and provide motion blur. */
   vec3 oldValue = texture(u_prevtex, texture_coords).rgb;
   colour.rgb = mix(oldValue - vec3(0.004), colour.rgb, 0.5);

   /* Darken it all a bit. */
   colour.rgb *= 0.8;
   return colour * colour_in;
}

