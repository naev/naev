--[[
   Some sort of stellar wind type background.
--]]

-- We use the default background too!
local bgshaders = require "bkg.bgshaders"
local love_shaders = require 'love_shaders'
local graphics = require "love.graphics"

local starfield = [[
#include "lib/gamma.glsl"
// \file starfield.pix
// \author Morgan McGuire
//
// \cite Based on Star Nest by Kali
// https://www.shadertoy.com/view/4dfGDM
// That shader and this one are open source under the MIT license
//
// Assumes an sRGB target (i.e., the output is already encoded to gamma 2.1)
// viewport resolution (in pixels)

uniform vec2 u_resolution;

// In the noise-function space. xy corresponds to screen-space XY
uniform vec4 u_camera = vec4(1.0);

uniform sampler2D u_prevtex;

const float theta = 1.0;
const mat2 rotate = mat2( cos(theta), -sin(theta), sin(theta), cos(theta) );

uniform vec2 u_r  = vec2( 400.0, 700.0 );

#define iterations 17

#define volsteps 8

#define sparsity 0.7  // 0.4 to 0.5 (sparse)
#define stepsize 0.2

#define frequencyVariation   1.8 // 0.5 to 2.0

#define brightness 0.0010
#define distfading 0.6800

vec4 effect( vec4 colour_in, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv = (texture_coords - 0.5) * love_ScreenSize.xy * u_camera.w + u_camera.xy + u_r;

   vec3 dir = vec3(uv, 1.0);
   dir.xy *= rotate;

   float s = 0.1, fade = 0.01;
   vec4 colour = vec4( vec3(0.0), 1.0 );

   for (int r=0; r < volsteps; r++) {
      vec3 p = vec3(1.0) + u_camera.xyz + dir * (s * 0.5);
      p = abs(vec3(frequencyVariation) - mod(p, vec3(frequencyVariation * 2.0)));

      float prevlen = 0.0, a = 0.0;
      for (int i=0; i < iterations; i++) {
         p = abs(p);
         p = p * (1.0 / dot(p, p)) + (-sparsity); // the magic formula
         float len = length(p);
         a += abs(len - prevlen); // absolute sum of average change
         prevlen = len;
      }

      a *= a * a; // add contrast

      /* Colouring based on distance. */
      colour.rgb += (vec3(s, s*s, s*s*s) * a * brightness + 1.0) * fade;
      fade *= distfading; /* Distance fading. */
      s += stepsize;
   }
   colour.rgb = min(colour.rgb, vec3(1.2));

   /* Some cheap antialiasing filtering. */
   float intensity = min(colour.r + colour.g + colour.b, 0.7);
   float w = fwidth(intensity);
   colour.rgb = mix( colour.rgb, vec3(0.0), smoothstep(0.0,1.0,w) );

   /* Colour conversion. */
   colour.rgb = clamp( pow( colour.rgb, vec3(2.0) ), 0.0, 1.0 );

   /* Motion blur to increase temporal coherence and provide motion blur. */
   vec3 oldValue = texture(u_prevtex, texture_coords).rgb;
   colour.rgb = mix(oldValue - vec3(0.004), colour.rgb, 0.5);

   /* Darken it all a bit. */
   colour.rgb *= 0.8;
   return colour;
}
]]

local shader, sf

function background ()
   -- Scale factor that controls computation cost. As this shader is really
   -- really expensive, we can't compute it at full resolution
   sf = naev.conf().nebu_scale * 0.5

   -- Initialize shader
   shader = graphics.newShader( starfield, love_shaders.vertexcode )
   bgshaders.init( shader, sf, {usetex=true} )
end

function renderbg( dt )
   -- Get camera properties
   local x, y = camera.get():get()
   local z = camera.getZoom()
   x = x / 1e6
   y = y / 1e6
   shader:send( "u_camera", x*0.5/sf, -y*0.5/sf, 0.0, z*0.0008*sf )

   bgshaders.render()
end

