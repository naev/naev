--[[
   Default background
--]]
local starfield = require "bkg.lib.starfield"
local lg = require "love.graphics"
local lf = require 'love.filesystem'
local love_shaders = require 'love_shaders'
local prng = require("prng").new()

local pixelcode = lf.read( "bkg/shaders/blackhole.frag" )

local canvas, shader, time, bx, by
function background ()
   prng:setSeed( system.cur():nameRaw() )

   --sf = naev.conf().nebu_scale
   local nw, nh = naev.gfx.dim()
   local cw, ch = nw, nh
   canvas = lg.newCanvas( cw, ch )
   bx, by = 3, 3

   -- Set up the shader
   local ax, ay, az = 0.1*prng:random(), 0.1*prng:random(), 2*prng:random()-1
   shader = lg.newShader( string.format( pixelcode, ax, ay, az ), love_shaders.vertexcode )
   time = -1000 * rnd.rnd()

   starfield.init{ nolocalstars=true }
end

function renderbg( dt )
   lg.setCanvas( canvas )
   lg.clear( 0, 0, 0, 0 )
   starfield.render( dt )
   lg.setCanvas()

   local x, y = camera.get():get()
   local z = camera.getZoom()
   time = time + dt
   shader:send( "u_time", time )
   shader:send( "u_camera", bx+x*0.0003, by+y*0.0003, z )

   lg.setShader( shader )
   canvas:draw( 0, 0, 0, 1, 1 )
   lg.setShader()
end
