#include "lib/math.glsl"

uniform float u_time;

const int fps        = 15;
const float strength = %f;

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px ) {
   float time = u_time - mod( u_time, 1.0 / float(fps) );

   float glitchStep = mix(4.0, 32.0, random(vec2(time)));

   vec4 screenColour = texture( tex, uv );
   uv.x = round(uv.x * glitchStep ) / glitchStep;
   vec4 glitchColour = texture( tex, uv );
   return colour * mix(screenColour, glitchColour, vec4(0.03*strength));
}
