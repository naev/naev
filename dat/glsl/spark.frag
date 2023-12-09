#include "lib/math.glsl"
#include "lib/simplex.glsl"

/* Common uniforms for special effects. */
uniform float u_time    = 0.0; /**< Elapsed time. */
uniform float u_r       = 0.0; /**< Random seed. */
/* Custom stuff. */
uniform float u_speed   = 1.3; /**< How fast it playes. */

/* Entry point. */
vec4 effect( vec4 unused, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   float progress = u_time * u_speed;
   vec2 uv = texture_coords*2.0-1.0;
   vec4 colour = vec4( 0.1, 0.7, 0.9, 1.0 );

   float angle = 2.0*M_PI*random(u_r) + snoise( vec2(0.9*progress,u_r) );
   float c = cos(angle);
   float s = sin(angle);

   mat2 R = mat2(c,-s,s,c);

   vec3 nuv = vec3( 4.0*uv, 0.01*u_time );
   //vec2 nuv = 3.0*uv;
   float n = abs(snoise( nuv ));
   n += 0.5*abs(snoise(2.0*nuv));
   n += 0.25*abs(snoise(4.0*nuv));
   n = pow(1.0-n,2.0);
   colour.rgb += 0.3*n;

   float t = mix( -1.0, 1.0, progress );
   float d;

   float tp = progress*progress;
   float scale = tp*(1.0-tp)*4.0;

   uv = R*uv+vec2(t,0.0);

   float r = length(uv*vec2(1.0,0.7));

   float w = 0.3;
   d = r-w*scale;

   colour.a *= smoothstep( 0.0, 0.2, -d ) * n;

   w = 0.2*scale;
   d = r-w;
   colour.a += 0.8*smoothstep( 0.0, w, -d );

   colour.a = pow( colour.a, 3.0 );

   return colour;
}

#ifndef _LOVE
in vec2 pos;
out vec4 colour_out;
uniform sampler2D dummy;
void main (void)
{
   colour_out = effect( vec4(0.0), dummy, pos, vec2(0.0) );
}
#endif /* _LOVE */
