// novalidation
precision highp float;
#include "lib/blur.glsl"
uniform vec2 blurvec;
const vec2 wh = vec2( %f, %f );
const float strength = %f;
vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolour = blur%s( tex, uv, wh, blurvec, strength );
   return texcolour;
}
