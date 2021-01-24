uniform vec4 c1;
uniform vec4 c2;
uniform float t1;
uniform float t2;
uniform float now;
in vec2 pos;
out vec4 color_out;

void main(void) {
   float t;

   color_out = pos.x*(c2-c1) + c1;

   /* Distance to the segment. */
//   if (pos.x > 1.)
//      color_out.a = 1. - clamp( ( (pos.x-1.)*(pos.x-1.) + pos.y*pos.y ), 0., 1.);
//   else if (pos.x < 0.)
//      color_out.a = 1. - clamp( pos.x*pos.x + pos.y*pos.y , 0., 1.);
//   else
      color_out.a *= 1. - clamp( pos.y*pos.y, 0., 1.);

   /* No sinusoidal effect. */
   t = pos.x*(t2-t1) + t1;
   //color_out.a *= .5;
   //color_out.a *= .8 - .5*sin( omega*t );

   /* Fading effect, 50000 is the hardcoded duration of the track. */
   //color_out.a *= 4. * (now-t)/50000 * (1. - (now-t)/50000);
   //color_out.a *= min(1.,(now-t)/1000) * (1. - (now-t)/50000) * (1. - (now-t)/50000);
   color_out.a *= (1. - (now-t)/50000) * (1. - (now-t)/50000);

#include "colorblind.glsl"
}
