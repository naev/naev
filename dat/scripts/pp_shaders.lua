--[[

   Post-processing shader library for Lua.

   This basically wraps around the shader framework and allows to easily create
   some post-processing shaders with minimal code overhead.

--]]

local pp_shaders = {}

local f = file.new( 'glsl/postprocess.vert' )
f:open('r')
pp_shaders.vertexcode = "#version 140\n"..f:read()

function pp_shaders.newShader( fragcode )
   return shader.new([[
#version 140

uniform sampler2D MainTex;
uniform vec4 love_ScreenSize;
in vec4 VaryingTexCoord;
out vec4 color_out;

vec4 effect( sampler2D tex, vec2 texcoord, vec2 pixcoord );

void main (void)
{
   color_out = effect( MainTex, VaryingTexCoord.st, love_ScreenSize.xy );
}
]] .. fragcode, pp_shaders.vertexcode )
end

return pp_shaders
