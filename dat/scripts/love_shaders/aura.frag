#include "lib/math.glsl"
#include "lib/simplex.glsl"
#include "lib/blend.glsl"

uniform float u_time;
uniform Image blurtex;

const vec3 basecolour = vec3( %f, %f, %f );
const float strength = %f;
const float speed = %f;
const float u_r = %f;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 blurcolour = texture( blurtex, uv );

   // Hack to hopefully speed up
   if (blurcolour.a <= 0.0)
      return vec4(0.0);

   vec4 texcolour = texture( tex, uv );
   vec2 offset = vec2( 50.0*sin( M_PI*u_time * 0.001 * speed ), -3.0*u_time*speed );

   float n = 0.0;
   for (float i=1.0; i<4.0; i=i+1.0) {
      float m = pow( 2.0, i );
      n += snoise( offset +  px * strength * 0.009 * m + 1000.0 * u_r ) * (1.0 / m);
   }
   n = 0.5*n + 0.5;

   blurcolour.a = 1.0-2.0*distance( 0.5, blurcolour.a );
   blurcolour.a *= n;

   texcolour.rgb = blendScreen( texcolour.rgb, basecolour, blurcolour.a );
   texcolour.a = max( texcolour.a, blurcolour.a );
   return colour * texcolour;
}
