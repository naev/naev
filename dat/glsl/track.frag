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
   return 1. - pow( max(0.0, abs(1.-x) * 2.0 - 1.0 ), 1.0 );
}

/* k is the sharpness, more k == sharper.
 * Good default is 3.0 */
float smoothbeam( float x, float k )
{
   return 1. - pow( abs( sin( M_PI * x / 2. ) ), k );
}

void main(void) {
   float t, d, m;

   // Interpolating base color
   color_out = pos.x*(c2-c1) + c1;

   // Modulate alpha base on length
   t = pos.x*(t2-t1) + t1; // interpolate between both points
   color_out.a *= fastdropoff( t );

   // Modulate alpha based on dispersion
   // Modulate width
   m = impulse( 1.-t, 5. );
   d = smoothbeam( pos.y, 3.*m );
   color_out.a *= d;

#include "colorblind.glsl"
}
