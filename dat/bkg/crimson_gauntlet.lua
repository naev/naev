--[[
   Crimson Gauntlet, Digital Battleground

   Some light circuit background.
--]]

-- We use the default background too!
require "bkg.default"

local love = require 'love'
local lg = require 'love.graphics'
local love_shaders = require 'love_shaders'

-- Since we don't actually activate the Love framework we have to fake the
-- the dimensions and width, and set up the origins.
local nw, nh = naev.gfx.dim()
love.x = 0
love.y = 0
love.w = nw
love.h = nh
lg.origin()

local pixelcode = [[
#include "lib/math.glsl"

uniform float u_time = 0.0;
uniform vec3 u_camera = vec3( 0.0, 0.0, 1.0 );

const float NUM_OCTAVES = 3;

/*
   Originall by srtuss, 2013
   https://www.shadertoy.com/view/4sl3Dr
*/

/* 1D noise */
float noise1(float p)
{
   float fl = floor(p);
   float fc = fract(p);
   return mix(random(fl), random(fl + 1.0), fc);
}

/* voronoi distance noise, based on iq's articles */
float voronoi( in vec2 x )
{
   vec2 p = floor(x);
   vec2 f = fract(x);

   vec2 res = vec2(8.0);
   for(int j = -1; j <= 1; j ++) {
      for(int i = -1; i <= 1; i ++) {
         vec2 b = vec2(i, j);
         vec2 r = vec2(b) - f + random(p + b);

         /* chebyshev distance, one of many ways to do this */
         float d = max(abs(r.x), abs(r.y));

         if (d < res.x) {
            res.y = res.x;
            res.x = d;
         }
         else if (d < res.y) {
            res.y = d;
         }
      }
   }
   return res.y - res.x;
}

vec4 effect( vec4 color, Image tex, vec2 texture_coords, vec2 screen_coords )
{
   vec2 uv, wh;

   /* Minor flickering. */
   float flicker = noise1(u_time * 2.0) * 0.2 + 0.9;

   /* Calculate coordinates relative to camera. */
   wh = love_ScreenSize.xy * 0.5;
   uv = (screen_coords - wh) * u_camera.z + wh;
   uv /= 500.;

   float v = 0.0;

   /*
   // that looks highly interesting:
   //v = 1.0 - length(uv) * 1.3;
   */

   /* Add some noise octaves */
   float a = 0.6, f = 1.0;

   /* 4 octaves also look nice, its getting a bit slow though */
   for (int i = 0; i < NUM_OCTAVES; i ++) {
      float v1 = voronoi(uv * f + 5.0);
      float v2 = 0.0;

      /* Make the moving electrons-effect for higher octaves */
      if (i > 0) {
         /* of course everything based on voronoi */
         v2 = voronoi(uv * f * 0.5 + 50.0 + u_time);

         float va = 0.0, vb = 0.0;
         va = 1.0 - smoothstep(0.0, 0.1, v1);
         vb = 1.0 - smoothstep(0.0, 0.08, v2);
         v += a * pow(va * (0.5 + vb), 2.0);
      }

      /* make sharp edges */
      v1 = 1.0 - smoothstep(0.0, 0.3, v1);

      /* noise is used as intensity map */
      v2 = a * (noise1(v1 * 5.5 + 0.1));

      /* octave 0's intensity changes a bit */
      if (i == 0)
         v += v2 * flicker;
      else
         v += v2;

      f *= 3.0;
      a *= 0.7;
   }

   /* blueish color set */
   vec3 cexp = vec3(6.0, 4.0, 2.0);

   vec3 col = vec3(pow(v, cexp.x), pow(v, cexp.y), pow(v, cexp.z)) * 2.0;
   col = clamp( col, 0.0, 1.0 );
   return color * vec4( col, 1.0);
}
]]


function background ()
   w, h = naev.gfx.dim()
   u_time = rnd.rnd()*1000
   shader = lg.newShader( pixelcode, love_shaders.vertexcode )

   -- Default nebula background (no star)
   cur_sys = system.cur()
   prng:setSeed( cur_sys:name() )
   background_nebula()
end

function renderbg( dt )
   -- Get camera properties
   local x, y = camera.get():get()
   local z = camera.getZoom()
   u_time = u_time + dt
   shader:send( "u_camera", x, y, z )
   shader:send( "u_time", u_time )

   local b = 0.1
   lg.setColor( b, b, b, 1 )
   local oldshader = lg.getShader()
   lg.setShader( shader )
   love_shaders.img:draw( 0, 0, 0, w, h )
   lg.setShader( oldshader )
end
