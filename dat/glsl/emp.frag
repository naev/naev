#include "lib/math.glsl"
#include "lib/simplex.glsl"

/* Common uniforms for special effects. */
uniform float u_time    = 0.0; /**< Elapsed time. */
uniform float u_r       = 0.0; /**< Random seed. */
/* Custom stuff. */
uniform float u_speed   = 1.3; /**< How fast it playes. */

const float UP          = 0.7; /**< Time spent on creating the explosion vs not doing it. */
const float MAX         = 0.65;/**< How big the effect can get. */

/* Entry point. */
vec4 effect( vec4 colour, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float progress = u_time * u_speed;
   vec2 uv = texture_coords*2.0-1.0;

   colour = vec4( 0.1, 0.7, 0.9, 1.0 );

   float angle = 2.0*M_PI*random(u_r);
   float shear = 0.5*random(10.0*u_r+10.0);
   float c = cos(angle);
   float s = sin(angle);

   mat2 R = mat2(c,-s,s,c);
   mat2 S = mat2(1.0,shear,0.0,1.0);
   uv = S * R * uv;

   float r = length(uv);

   vec3 nuv = vec3( 3.0*uv, u_time );
   float n = abs(snoise( nuv ));
   n += 0.5*abs(snoise(2.0*nuv));
   n += 0.25*abs(snoise(4.0*nuv));
   n = pow(1.0-n,2.0);
   colour.rgb += 0.3*n;

   float t = MAX*smoothstep( 0.0, 1.0, progress / UP );
   float mixr = smoothstep( 0.0, 1.0, (progress-UP) / (1.0-UP) );
   float w = 0.3;
   float d, d2;
   if (mixr <= 0.0)
      d = abs(r-t)-w;
   else
      d = abs(r-(MAX-2.0*mixr))-w-mixr;
   colour.a *= smoothstep( 0.0, w-0.1, -d ) * n;

   w = 0.1;
   if (mixr <= 0.0)
      d = abs(r-t)-w;
   else
      d = abs(r-(MAX-2.0*mixr))-w-mixr;
   colour.a += smoothstep( 0.0, w, -d );

   colour.a = pow( colour.a, 2.0 );

   return colour;
}

#ifndef _LOVE
in vec2 pos;
out vec4 color_out;
uniform sampler2D dummy;
void main (void)
{
   color_out = effect( vec4(0.0), dummy, pos, vec2(0.0) );
}
#endif
