#include "lib/math.glsl"
#include "lib/blur.glsl"
#include "lib/blend.glsl"
#include "lib/simplex.glsl"
#include "lib/colour.glsl"

uniform float u_time;

float onOff(float a, float b, float c)
{
   return step(c, sin(u_time + a*cos(u_time*b)));
}

vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 screen_coords )
{
   const float strength = %f; /* Simple parameter to control everything. */

   /* Strength-based constants. */
   const float BLURAMPLITUDE     = 6.0 * strength;
   const float BLURSPEED         = 0.15 * strength;
   const float HIGHLIGHTSPEED    = 0.05 + 0.05 * strength;
   const float HIGHLIGHTRANGE    = 0.1 + 0.05 * strength;
   const float HIGHLIGHTCOUNT    = 4.0 + 2.5 * strength;
   const float SHADOWSPEED       = 0.17 + 0.14 * strength;
   const float SHADOWRANGE       = 0.07 + 0.05 * strength;
   const float SHADOWCOUNT       = 4.7 + 2.3 * strength;
   const float GRAINSTRENGTH     = 32.0 + 48.0 * strength;
   const float WOBBLEAMPLITUDEX  = 0.0025 * strength;
   const float WOBBLEAMPLITUDEY  = 0.003 * strength;
   const float WOBBLESPEED       = 0.1 * strength;

   /* Truly constant values. */
   //const vec3 bluetint           = vec3( 0.075, 0.215, 0.604 );/* Gamma: vec3(0.3, 0.5, 0.8); */
   const vec3 bluetint           = vec3( 0.1, 0.3, 0.7 );/* Gamma: vec3(0.3, 0.5, 0.8); */
   const float BRIGHTNESS        = 0.1;
   const float CONTRAST          = 1.0/0.2;
   const float SCANLINEMEAN      = 0.9;
   const float SCANLINEAMPLITUDE = 0.2;
   const float SCANLINESPEED     = 1.0;

   /* Get the texture. */
   vec2 look      = uv;
   float tmod     = mod(u_time/4.0,1.0);
   float window   = 1.0 / (1.0+20.0*(look.y-tmod)*(look.y-tmod));
   look.x += WOBBLEAMPLITUDEX * sin(look.y*10.0 + u_time)*onOff(1.0,1.0,WOBBLESPEED)*(1.0+cos(u_time*80.0))*window;
   float vShift   = WOBBLEAMPLITUDEY * onOff(2.0,3.0,M_PI*WOBBLESPEED)*(sin(u_time)*sin(u_time*20.0)+(0.5 + 0.1*sin(u_time*200.0)*cos(u_time)));
   look.y = mod( look.y + vShift, 1.0 );

   /* Blur a bit. */
   vec4 texcolour  = colour * texture( tex, look );
   float blurdir  = snoise( vec2( BLURSPEED*u_time, 0.0 ) );
   vec2 blurvec   = BLURAMPLITUDE * vec2( cos(blurdir), sin(blurdir) );
   vec4 blurbg    = blur9( tex, look, love_ScreenSize.xy, blurvec );
   texcolour.rgb   = blendSoftLight( texcolour.rgb, blurbg.rgb );
   texcolour.a     = max( texcolour.a, blurbg.a );

   /* Drop to greyscale while increasing brightness and contrast */
   float greyscale= rgb2lum( texcolour.rgb ); // percieved
   texcolour.xyz   = CONTRAST * vec3(greyscale) + BRIGHTNESS;

   /* Shadows. */
   float shadow   = 1.0 + SHADOWRANGE * sin((uv.y + (u_time * SHADOWSPEED)) * SHADOWCOUNT);
   texcolour.xyz  *= shadow;

   /* Highlights */
   float highlight= HIGHLIGHTRANGE * (0.5 + 0.5 * sin((uv.y + (u_time * -HIGHLIGHTSPEED)) * HIGHLIGHTCOUNT) );
   texcolour.xyz  += highlight;

   // Other effects.
   float x = (uv.x + 4.0) * (uv.y + 4.0) * u_time * 10.0;
   float grain = 1.0 - (mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01) - 0.005) * GRAINSTRENGTH;
   float flicker = max(1.0, random(u_time * uv) * 1.5);
   float scanlines = SCANLINEMEAN + SCANLINEAMPLITUDE*step( 0.5, sin(0.5*screen_coords.y + SCANLINESPEED*u_time)-0.1 );

   texcolour.xyz *= grain * flicker * scanlines * bluetint;
   return texcolour * colour;
}
