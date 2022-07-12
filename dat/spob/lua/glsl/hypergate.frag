#include "lib/simplex.glsl"
#include "lib/perlin.glsl"

uniform float u_time = 0.0;
const vec3 basecol = vec3( %f, %f, %f );

float fbm3( vec2 x )
{
   float v = 0.0;
   float a = 0.5;
   const vec2 shift = vec2(100.0);
   for (int i=0; i<3; i++) {
      v += a * snoise(x);
      x  = x * 2.0 + shift;
      a *= 0.5;
   }
   return v;
}

float fbm3p( vec2 x )
{
   float v = 0.0;
   float a = 0.5;
   const vec2 shift = vec2(100.0);
   for (int i=0; i<3; i++) {
      v += a * pnoise(x, vec2(0.5));
      x  = x * 2.0 + shift;
      a *= 0.5;
   }
   return v;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 centered = texture_coords*2.0-1.0;

   // Random small rotation
	float r = 0.0025*u_time + 0.5;
	float c = cos(r);
	float s = sin(r);
   mat2 R = mat2( c, s, -1.3*s, 1.3*c ); // Do some cheap perspective correction
   centered = R * centered;

   /* Periodic polar coordinates. */
   vec2 polar = vec2( length(centered), atan( centered.y, centered.x ) );
   polar.y = pnoise( polar * vec2(1.0, 1.0/(M_PI*2.0)), vec2(1.0) );
   polar.x -= u_time * 0.025 - 100.0;

   /* Set up basic stuff. */
   vec4 mask = texture( tex, texture_coords );
   vec4 col = vec4( basecol, 1.0 );

   /* Some simple noise. */
   vec2 noisexy = vec2( fbm3( centered ), fbm3( polar ) );
   float noise = fbm3( noisexy );
   col.rgb += 0.2 * vec3( snoise( noisexy + vec2(100.0) ),
                          snoise( noisexy - vec2(100.0) ),
                          snoise( noisexy + vec2(100.0,-100.0) ) );

   /* Putting it together. */
   col.a = (0.7 + 0.3 * noise);
   col.a *= mask.r;
   return col;
}
