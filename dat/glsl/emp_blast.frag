#include "lib/sdf.glsl"
#include "lib/simplex.glsl"

uniform float u_time;
uniform float u_speed;
uniform float u_grain;
uniform float u_r;

vec4 effect( vec4 color, sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   float progress = u_time * u_speed;
   vec2 uv = texture_coords*2.0-1.0;
   vec4 colour = vec4( 1.0 );

   uv.y *= 1.4; /// approximate affine

   /*
    * Back part.
    */
   {
      /* Base shape. */
      float d = sdCircle( uv, 1.0*progress );
      d = max( d, -sdCircle( uv, 0.6*progress-0.3 )*0.5 );
      colour.rgb -= vec3( 1.0, 0.5, 0.2 )*3.0*progress - 0.5;
      colour.a *= smoothstep( -0.0, 0.2, -d ) * max(0.0, 2.0-progress);
      colour.a *= min(1.0, 6.0-6.0*progress);
      /* Noise texture. */
      float r = sqrt(dot(uv, uv)) - 0.5*progress;
      float a = atan(uv.y, uv.x);
      vec2 nuv = vec2(2.0*r, 8.0*a) * 0.5;
      float n = snoise( nuv );
      float n2 = snoise( uv*13.0 );
      colour.a *= min(1.0, 0.8+0.3*n*progress*(0.6+0.4*n2));
   }

   /* On top part. */
   {
      float c, s, r;
      r = M_PI*(progress+u_r);
      s = sin(r);
      c = cos(r);
      mat2 R = mat2( c, s, -s, c );
      vec2 nuv = R*uv;
      nuv *= 13.0/min(10.0,20.0*progress) * pow(length(nuv),-0.5);

      float d = sdRoundedCross( nuv, 1.0 );
      float a = smoothstep( 0.0, 0.2, -d ) * clamp( (2.0*progress) * (4.0-4.0*progress), 0.0, 1.0 );
      colour.a = max(colour.a,a);
      colour.rgb = a*vec3(1.0)+(1.0-a)*colour.rgb;
   }

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
