#include "lib/simplex.glsl"

uniform sampler2D MainTex;
in vec4 VaryingTexCoord;
out vec4 colour_out;

uniform vec4 love_ScreenSize;
uniform float damage_strength = 0.0; /**< 0 to 1 value where 1 indicates a ton of damage. */
uniform float u_time = 0.0; /**< Constantly increasing timer value. */

const float OFFSET_MOD  = 13.0; /**< How many pixels to multiply by. */
const float OFFSET_MAX  = 5.0; /**< Maximum offset in pixels. */

void main (void)
{
   vec2 uv = VaryingTexCoord.st;
   vec2 t = vec2( u_time, u_time );

   /* Create two noise values to create a flickering. */
   float noise1 = snoise( fract( uv + vec2(random(t), 0.25) * 10.0 ) * 0.75 );
   float noise2 = snoise( fract( uv + vec2(random(t), 0.78) * 10.0 ) * 0.5 );

   /* Here we compute the final offset that we will use. */
   float offset = step( 0.5 * (noise1 + noise2), 0.5 ); // this is a 0 or 1 value
   offset = min( OFFSET_MAX, OFFSET_MOD * offset * damage_strength ) / love_ScreenSize.x;

   colour_out.r = texture( MainTex, uv + vec2(offset,0.0) ).r;
   colour_out.g = texture( MainTex, uv ).g;
   colour_out.b = texture( MainTex, uv - vec2(offset,0.0) ).b;
   colour_out.a = 1.0;
}
