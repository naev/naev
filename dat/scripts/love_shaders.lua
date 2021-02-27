local love_shaders = {}

local graphics = require "love.graphics"
local love_math = require "love.math"
local love_image = require "love.image"

-- Tiny image for activating shaders
local idata = love_image.newImageData( 1, 1 )
idata:setPixel( 0, 0, 1, 1, 1, 1 )
love_shaders.img = graphics.newImage( idata )

--[[
-- Default fragment code that doesn't do anything fancy.
--]]
local _pixelcode = [[
vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec4 texcolor = Texel(tex, texture_coords);
   return texcolor * color;
}
]]
--[[
-- Default vertex code that doesn't do anything fancy.
--]]
local _vertexcode = [[
vec4 position( mat4 transform_projection, vec4 vertex_position )
{
   return transform_projection * vertex_position;
}
]]

function love_shaders.paper( width, height, sharpness )
   if width==nil then
      width, height = naev.gfx.dim()
   end
   sharpness = sharpness or 1
   local pixelcode = [[
precision highp float;

#include "lib/simplex.glsl"

uniform float u_r;
uniform float u_sharp;

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 px )
{
   vec4 texcolor = color * texture2D( tex, uv );

   float n = 0.0;
   for (float i=1.0; i<8.0; i=i+1.0) {
      float m = pow( 2.0, i );
      n += snoise( px * u_sharp * 0.003 * m + 1000.0 * u_r ) * (1.0 / m);
   }

   texcolor.rgb *= 0.68 + 0.3 * n;

   return texcolor;
}
]]

   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader:send( "u_r", love_math.random() )
   shader:send( "u_sharp", sharpness );

   -- Render to image
   local paperbg = graphics.newCanvas( width, height )
   local oldcanvas = graphics.getCanvas()
   local oldshader = graphics.getShader()
   graphics.setCanvas( paperbg )
   graphics.clear( 1, 1, 1, 1 )
   graphics.setShader( shader )
   love_shaders.img:draw( 0, 0, 0, width, height )
   graphics.setShader( oldshader )
   graphics.setCanvas( oldcanvas )

   return paperbg
end

function love_shaders.oldify( noise )
   noise = noise or 1.0
   local pixelcode = [[
#include "lib/simplex.glsl"
#include "lib/perlin.glsl"
#include "lib/math.glsl"
#include "lib/blur.glsl"
#include "lib/blend.glsl"
#include "lib/colour.glsl"

uniform float u_time;

float grain(vec2 uv, vec2 mult, float frame, float multiplier) {
   float offset = snoise(vec3(mult / multiplier, frame));
   float n1 = pnoise(vec3(mult, offset), vec3(1.0/uv * love_ScreenSize.xy, 1.0));
   return n1 / 2.0 + 0.5;
}
vec4 graineffect( vec4 bgcolor, vec2 uv, vec2 px ) {
   const float fps = 15.0;
   const float zoom = 0.2;
   float frame = floor(fps*u_time) / fps;
   const float tearing = 3.0; /* Tears "1/tearing" of the frames. */

   vec3 g = vec3( grain( uv, px * zoom, frame, 2.5 ) );

   // get the luminance of the image
   float luminance = rgb2lum( bgcolor.rgb );
   vec3 desaturated = vec3(luminance);

   // now blend the noise over top the backround
   // in our case soft-light looks pretty good
   vec4 color;
   color = vec4( vec3(1.2,1.0,0.4)*luminance, bgcolor.a );
   color = vec4( blendSoftLight(color.rgb, g), bgcolor.a );

   // and further reduce the noise strength based on some
   // threshold of the background luminance
   float response = smoothstep(0.05, 0.5, luminance);
   color.rgb = mix(desaturated, color.rgb, pow(response,2.0));

   // Vertical tears
   if (distance( love_ScreenSize.x * random(vec2(frame, 0.0)), px.x) < tearing*random(vec2(frame, 1000.0))-(tearing-1.0))
      color.rgb *= vec3( random( vec2(frame, 5000.0) ));

   // Flickering
   color.rgb *= 1.0 + 0.05*snoise( vec2(3.0*frame, M_PI) );

   return color;
}
vec4 vignette( vec2 uv )
{
   uv *= 1.0 - uv.yx;
   float vig = uv.x*uv.y * 15.0; // multiply with sth for intensity
   vig = pow(vig, 0.3); // change pow for modifying the extend of the  vignette
   return vec4(vig);
}
vec4 effect( vec4 color, Image tex, vec2 uv, vec2 screen_coords )
{
   vec4 texcolor = color * texture2D( tex, uv );

   texcolor = graineffect( texcolor, uv, screen_coords );
   vec4 v = vignette( uv );
   texcolor.rgb *= v.rgb;
   //texcolor = mix( texcolor, v, v.a );

   return texcolor;
}
]]

   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000. * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end

function love_shaders.hologram( noise )
   noise = noise or 1.0
   local pixelcode = [[
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
   vec4 texcolor = color * texture2D( tex, look );
   float blurdir = snoise( vec2( blurspeed*u_time, 0 ) );
   vec2 blurvec = bluramplitude * vec2( cos(blurdir), sin(blurdir) );
   vec4 blurbg = blur9( tex, look, love_ScreenSize.xy, blurvec );
   texcolor.rgb = blendSoftLight( texcolor.rgb, blurbg.rgb );
   texcolor.a = max( texcolor.a, blurbg.a );

   /* Drop to greyscale while increasing brightness and contrast */
   //float greyscale = dot( texcolor.xyz, vec3( 0.2126, 0.7152, 0.0722 ) ); // standard
   float greyscale = rgb2lum( texcolor.rgb ); // percieved
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
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end


function love_shaders.corruption( noise )
   noise = noise or 1.0
   local pixelcode = string.format([[
#include "lib/math.glsl"

uniform float u_time;

const int    fps     = 15;
const float strength = %f;

vec4 effect( vec4 color, Image tex, vec2 uv, vec2 px ) {
   float time = u_time - mod( u_time, 1.0 / float(fps) );

   float glitchStep = mix(4.0, 32.0, random(vec2(time)));

   vec4 screenColor = texture2D( tex, uv );
   uv.x = round(uv.x * glitchStep ) / glitchStep;
   vec4 glitchColor = texture2D( tex, uv );
   return color * mix(screenColor, glitchColor, vec4(0.1*strength));
}
]], noise )

   local shader = graphics.newShader( pixelcode, _vertexcode )
   shader._dt = 1000 * love_math.random()
   shader.update = function (self, dt)
      self._dt = self._dt + dt
      self:send( "u_time", self._dt )
   end
   return shader
end

return love_shaders
