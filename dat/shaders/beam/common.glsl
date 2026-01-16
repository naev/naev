uniform vec4 colour;
uniform float dt;
uniform float r;

const float ANIM_SPEED   = 1.0/1.5; /**< Controls the global speed of the animations. */

#include "lib/math.glsl"

in vec2 pos_tex;
in vec2 pos_px;
out vec4 colour_out;

/* k is the sharpness, more k == smoother
 * Good default is 3.0 */
float smoothbeam( float x, float k )
{
   return 1.0 - pow( abs( sin( M_PI * x / 2.0 ) ), k );
}

/* Similar to smoothbeam, but more k == sharper. */
float sharpbeam( float x, float k )
{
   return pow( min( cos( M_PI * x / 2.0 ), 1.0 - abs(x) ), k );
}

float beamfade( float p, float x )
{
   if (p < 7.0)
      return p / 7.0;
   return 1.0 - smoothstep( 0.0, 0.2, x-0.8 );
}
