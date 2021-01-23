uniform vec4 color;
uniform float t1;
uniform float t2;
uniform float omega;
in vec2 pos;
out vec4 color_out;

void main(void) {
   color_out = color;

   /* Distance from the segment (squared for performance reason). */
   if (pos.x > 1.)
      color_out.a = 1. - clamp( ( (pos.x-1.)**2 + pos.y**2 ), 0., 1.);
   else if (pos.x < 0.)
      color_out.a = 1. - clamp( ( (pos.x)**2 + pos.y**2 ), 0., 1.);
   else
      color_out.a = 1. - clamp( pos.y**2, 0., 1.);

   /* Sinusoidal effect. */
   color_out.a = color_out.a * ( 1 - sin( omega*(pos.x*(t1-t2) + t1) ) );

#include "colorblind.glsl"
}
