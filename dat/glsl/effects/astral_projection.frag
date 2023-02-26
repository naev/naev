#include "lib/simplex.glsl"
#include "lib/blur.glsl"

uniform vec3 dimensions;
uniform sampler2D u_tex;

uniform float u_r       = 0.0;
//uniform float u_timer   = 0.0;
uniform float u_elapsed = 0.0;
uniform float u_dir     = 0.0;

const vec4 COLOUR_OUTLINE  = vec4( 0.9, 0.9, 0.4, 3.0/3.0 );
const vec4 COLOUR_BRIGHT   = vec4( 0.6, 0.6, 0.3, 2.0/3.0 );
const vec4 COLOUR_DARK     = vec4( 0.5, 0.7, 0.4, 1.0/3.0 );

in vec2 tex_coord;
out vec4 colour_out;

vec4 calc_colour( float w, float strength )
{
    vec2 nuv = 0.001 * tex_coord * dimensions.xy / dimensions.z;
    float n = 0.5*snoise( vec3( nuv, 0.1*u_elapsed ) )+0.5; // 0-1 range

    nuv += strength*(n-0.25)*vec2(cos(u_dir+0.3*(n-0.5)*(n-0.5)),-sin(u_dir-0.3*(n-0.5)*(n-0.5)));

    float m = abs(1.0-2.0*fract(u_elapsed+10.0*((tex_coord.x+0.02*cos(64.0*nuv.y)))));
    float f = 0.5+0.75*abs(1.0-2.0*fract((1.7+0.03*tex_coord.x)*u_elapsed+abs(1.0-2.0*fract((1.9+0.03*tex_coord.y)*u_elapsed))));
    vec4 color = clamp(mix(f*COLOUR_OUTLINE, mix(COLOUR_DARK, COLOUR_BRIGHT, m*(m+0.5)), w), 0.0, 1.0);
    return color;
}

void main(void)
{
   vec4 base_colour = texture( u_tex, tex_coord );
   if (base_colour.a <= 0.0)
      discard;

   float w = length( base_colour.rgb );

   colour_out =  0.75*calc_colour( w, 0.02 );
   colour_out += 0.5 *calc_colour( w, 0.04 );
   colour_out += 0.25*calc_colour( w, 0.06 );
   colour_out *= base_colour.a;
   colour_out = clamp( colour_out, 0.0, 1.0 );
}
