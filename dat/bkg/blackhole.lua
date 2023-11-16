--[[
   Default background
--]]
local bgshaders = require "bkg.lib.bgshaders"
local starfield = require "bkg.lib.starfield"
local lg = require "love.graphics"
local lf = require 'love.filesystem'
local love_shaders = require 'love_shaders'
local prng = require("prng").new()

local pixelcode = lf.read( "bkg/shaders/blackhole.frag" )

local blackhole, shader, time, bx, by, sf, move
local bgstars
function background ()
   local nconf = naev.conf()
   sf = math.max( 1.0, nconf.nebu_scale * 0.5 )

   local sysname = system.cur():nameRaw()
   prng:setSeed( sysname )

   local off = system.cur():pos() - vec2.new( -765, -490 )
   local _m, a = off:polar()
   local scale

   if sysname == "Anubis Black Hole" then
      scale = 8
      off = vec2.new()
      move = 0.01
   else
      scale = 1
      off = vec2.newP( 4, a )
      move = 0.00025
   end
   bx, by = off:get()

   -- Set up the shader
   local ax, ay, az = 0.2*prng:random(), 0.2*prng:random(), 2*prng:random()-1
   shader = lg.newShader( string.format( pixelcode, scale, ax, ay, az ), love_shaders.vertexcode )
   time = -1000 * rnd.rnd()

   bgstars = lg.newCanvas()
   shader:send( "u_bgtex", bgstars )

   starfield.init{ nolocalstars=true }
   blackhole = bgshaders.init( shader, sf )
end

function renderbg( dt )
   local x, y = camera.values()
   time = time + dt
   shader:send( "u_time", time )

   shader:send( "u_camera", bx+x*move, -by-y*move, sf )

   lg.setCanvas( bgstars )
   starfield.render( dt )
   lg.setCanvas()

   blackhole:render( dt )
end
