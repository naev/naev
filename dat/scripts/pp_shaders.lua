--[[--
   Post-processing shader library for Lua.

   This basically wraps around the shader framework and allows to easily create
   some post-processing shaders with minimal code overhead.
   @module pp_shaders
--]]
local pp_shaders = {}

-- We load the C-side shader for the vertex shader
local f = file.new( 'glsl/postprocess.vert' )
f:open('r')
pp_shaders.vertexcode = "#version 140\n"..f:read()

--[[--
   Creates a new post-processing shader.

   @tparam string fragcode Fragment shader code.
   @return The newly created shader.
--]]
function pp_shaders.newShader( fragcode )
   local s = shader.new([[
#version 140

uniform sampler2D MainTex;
uniform vec4 love_ScreenSize;
in vec4 VaryingTexCoord;
out vec4 colour_out;

vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord );

void main (void)
{
   colour_out = effect( MainTex, VaryingTexCoord.st, vec2(VaryingTexCoord.s,1.0-VaryingTexCoord.t) * love_ScreenSize.xy );
}
]] .. fragcode, pp_shaders.vertexcode )
   return s
end

--[[
-- A post-processing version of the corruption shader.
--]]
function pp_shaders.corruption( strength )
   strength = strength or 1.0
   local pixelcode = string.format([[
#include "lib/math.glsl"

uniform float u_time;

const int    fps     = 15;
const float strength = %f;

vec4 effect( sampler2D tex, vec2 uv, vec2 px ) {
   float time = u_time - mod( u_time, 1.0 / float(fps) );
   float glitchStep = mix(4.0, 32.0, random(vec2(time)));
   vec4 screenColour = texture( tex, uv );
   uv.x = round(uv.x * glitchStep ) / glitchStep;
   vec4 glitchColour = texture( tex, uv );
   return mix(screenColour, glitchColour, vec4(0.03*strength));
}
   ]], strength )
   return pp_shaders.newShader( pixelcode )
end

--[[
-- A shader to highlight some area
--]]
function pp_shaders.highlightBox( x, y, w, h )
   w = w * 0.5
   h = h * 0.5
   local pixelcode = string.format([[
#include "lib/sdf.glsl"

vec4 effect( sampler2D tex, vec2 uv, vec2 px ) {
   float d = sdBox( px - vec2(%f,%f), vec2(%f,%f) ) - 3.0;

   vec4 screenColour = texture( tex, uv );
   if (d > 0.0) {
      const vec3 highcol = vec3(0.0, 0.0, 1.0);
      float dd = 1.0 - clamp( d/500.0, 0.0, 1.0 );
      screenColour.rgb *= 0.75 - 0.5 * dd;
      float hd = 1.0 - clamp( d/25.0, 0.0, 1.0 );
      screenColour.rgb = mix( screenColour.rgb, highcol, hd);
   }

   return screenColour;
}
   ]], x+w, y+h, w, h )
   return pp_shaders.newShader( pixelcode )
end

return pp_shaders
