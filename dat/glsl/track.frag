#include "math.h"

uniform vec4 c1;
uniform vec4 c2;
uniform float t1;
uniform float t2;
in vec2 pos;
out vec4 color_out;

/* Has a peak at 1/k */
float impulse( float x, float k )
{
   float h = x*k;
   return h * exp( 1.0 - h );
}

float fastdropoff( float x )
{
  return x < 0.92 ? (0. - .63348*log(1.0 - x)) : (-20.0 * x) + 20.0 ;
}

/* k is the sharpness, more k == sharper.
 * Good default is 3.0 */
float smoothbeam( float x, float k )
{
   return 1. - pow( abs( sin( M_PI * x / 2. ) ), k );
}

float random (vec2 st) {
  return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void main(void) {
   float t, d, m;

   // Interpolating base color
   color_out = pos.x*(c2-c1) + c1;

   // Modulate alpha base on length
   t = pos.x*(t2-t1) + t1; // interpolate between both points
   color_out.a *= fastdropoff( t );

   // Modulate alpha based on dispersion
   m = impulse( 1.-t, 5. );

   // Modulate width
   d = smoothbeam( pos.y, 3.*m );

   color_out.a *= d * (1. - (random(pos) * .3));

#include "colorblind.glsl"
}
