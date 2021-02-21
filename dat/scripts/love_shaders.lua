local love_shaders = {}

local graphics = require "love.graphics"

--[[
-- Default vertex code (doesn't do anything fancy.
--]]
local _vertexcode = [[
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]

function love_shaders.hologram( noise )
   noise = noise or 1.0
   -- TODO make different strengths for hologram noise
   local pixelcode = [[
#include "lib/math.glsl"
#include "lib/blur.glsl"
#include "lib/simplex2D.glsl"

uniform float u_time;

float onOff(float a, float b, float c)
{
   return step(c, sin(u_time + a*cos(u_time*b)));
}

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 screen_coords )
{
   const vec3 bluetint  = vec3(0.3, 0.5, 0.8);  
   const float brightness = 0.4;
   const float contrast = 2.0;
#ifdef HOLOGRAM_STRONG
   const float strength = 64.0; // 64.0
   const float shadowspeed = 0.3;
   const float shadowrange = 0.35;
   const float shadowcount = 5.0;
   const float highlightspeed = 0.3;
   const float highlightrange = 0.35;
   const float highlightcount = 5.0;
   const float wobbleamplitude = 0.2;
   const float wobblespeed = 0.3;
   const float bluramplitude = 12.0;
   const float blurspeed = 0.31;
   const float scanlinemean = 0.85;
   const float scanlineamplitude = 0.3;
   const float scanlinespeed = 5.0;
#else /* HOLOGRAM_STRONG */
   const float strength = 32.0; // 64.0
   const float shadowspeed = 0.2;
   const float shadowrange = 0.2; // 0.35
   const float shadowcount = 5.0;
   const float highlightspeed = 0.2;
   const float highlightrange = 0.2; // 0.35
   const float highlightcount = 5.0;
   const float wobbleamplitude = 0.2; // 0.4
   const float wobblespeed = 0.9; // 0.3
   const float bluramplitude = 9.0;
   const float blurspeed = 0.13;
   const float scanlinemean = 0.9;
   const float scanlineamplitude = 0.2;
   const float scanlinespeed = 3.0;
#endif /* HOLOGRAM_STRONG */

   /* Get the texture. */
   vec2 look = uv;
   float window = 1./(1.+20.*(look.y-mod(u_time/4.,1.))*(look.y-mod(u_time/4.,1.)));
   look.x = look.x + 0.01 * sin(look.y*10. + u_time)*onOff(4.0,4.0,wobblespeed)*(1.+cos(u_time*80.))*window;
   float vShift = wobbleamplitude * onOff(2.0,3.0,M_PI*wobblespeed)*(sin(u_time)*sin(u_time*20.)+(0.5 + 0.1*sin(u_time*200.0)*cos(u_time)));
   look.y = mod( look.y + vShift, 1.0 );

   /* Blur a bit. */
   vec4 texcolor = Texel( tex, look );
   float blurdir = snoise( vec2( blurspeed*u_time, 0 ) );
   vec2 blurvec = bluramplitude * vec2( cos(blurdir), sin(blurdir) );
   vec4 blurbg = blur9( tex, look, love_ScreenSize.xy, blurvec );
   texcolor = mix( texcolor, blurbg, step( blurbg.a, texcolor.a ) );

   /* Drop to greyscale while increasing brightness and contrast */
   //float greyscale = dot( texcolor.xyz, vec3( 0.2126, 0.7152, 0.0722 ) ); // standard
   float greyscale = dot( texcolor.xyz, vec3( 0.2989, 0.5870, 0.1140 ) ); // percieved
   texcolor.xyz = contrast*vec3(greyscale) + brightness;
  
   /* Shadows. */
   float shadow = 1.0 - shadowrange + (shadowrange * sin((uv.y + (u_time * shadowspeed)) * shadowcount));
   texcolor.xyz *= shadow;

   /* Highlights */
   float highlight = 1.0 - highlightrange + (highlightrange * sin((uv.y + (u_time * -highlightspeed)) * highlightcount));
	texcolor.xyz += highlight;

   // Other effects.
   float x = (uv.x + 4.0) * (uv.y + 4.0) * u_time * 10.0;
   float grain = 1.0 - (mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01) - 0.005) * strength;
   float flicker = max(1.0, random(u_time * uv) * 1.5);
   //float scanlines = 0.85 * clamp(sin(uv.y * 400.0), 0.25, 1.0) * random(uv * vec2(0,sin(u_time * 0.2)) * 0.1) * 2.0;
   float scanlines = scanlinemean + scanlineamplitude*step( 0.5, sin(0.5*screen_coords.y + scanlinespeed*u_time)-0.1 );

   texcolor.xyz *= grain * flicker * scanlines * bluetint;
   return texcolor * color;
}
]]

   if noise >= 2.0 then
      pixelcode = "#define HOLOGRAM_STRONG 1\n"..pixelcode
   end

   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 0
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end

return love_shaders
