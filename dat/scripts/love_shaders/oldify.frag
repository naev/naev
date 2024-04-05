#include "lib/simplex.glsl"
#include "lib/perlin.glsl"
#include "lib/math.glsl"
#include "lib/blur.glsl"
#include "lib/blend.glsl"
#include "lib/colour.glsl"

uniform float u_time;

const float strength = %f;

float grain(vec2 uv, vec2 mult, float frame, float multiplier) {
   float offset = snoise(vec3(mult / multiplier, frame));
   float n1 = pnoise(vec3(mult, offset), vec3(1.0/uv * love_ScreenSize.xy, 1.0));
   return n1 / 2.0 + 0.5;
}
vec4 graineffect( vec4 bgcolour, vec2 uv, vec2 px ) {
   const float fps = 15.0;
   const float zoom = 0.2;
   float frame = floor(fps*u_time) / fps;
   const float tearing = 3.0; /* Tears "1/tearing" of the frames. */

   vec3 g = vec3( grain( uv, px * zoom, frame, 2.5 ) );

   // get the luminance of the image
   float luminance = rgb2lum( bgcolour.rgb );
   vec3 desaturated = vec3(luminance);

   // now blend the noise over top the backround
   // in our case soft-light looks pretty good
   vec4 colour;
   colour = vec4( vec3(1.2,1.0,0.4)*luminance, bgcolour.a );
   colour = vec4( blendSoftLight(colour.rgb, g), bgcolour.a );

   // and further reduce the noise strength based on some
   // threshold of the background luminance
   float response = smoothstep(0.05, 0.5, luminance);
   colour.rgb = mix(desaturated, colour.rgb, pow(response,2.0));

   // Vertical tears
   if (distance( love_ScreenSize.x * random(vec2(frame, 0.0)), px.x) < tearing*random(vec2(frame, 1000.0))-(tearing-1.0))
      colour.rgb *= vec3( random( vec2(frame, 5000.0) ));

   // Flickering
   colour.rgb *= 1.0 + 0.05*snoise( vec2(3.0*frame, M_PI) );

   return colour;
}
vec4 vignette( vec2 uv )
{
   uv *= 1.0 - uv.yx;
   float vig = uv.x*uv.y * 15.0; // multiply with sth for intensity
   vig = pow(vig, 0.3); // change pow for modifying the extend of the  vignette
   return vec4(vig);
}
vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 screen_coords )
{
   vec4 texcolour = colour * texture( tex, uv );

   texcolour = graineffect( texcolour, uv, screen_coords );
   vec4 v = vignette( uv );
   texcolour.rgb *= v.rgb;
   //texcolour = mix( texcolour, v, v.a );

   return texcolour;
}
