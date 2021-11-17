--[[
   One jump away from the nebula, so we get a bit nebula in the background.
--]]
local lg = require "love.graphics"
local lf = require 'love.filesystem'
local love_shaders = require 'love_shaders'
local prng = require("prng").new()

local nebulafrag = lf.read('bkg/shaders/nebula.frag')

local function R()
   return (2*prng:random()-1)*1000
end

local nebula = {}

function nebula.init( params )
   params = params or {}
   local steps    = params.stops or 48
   local hue_inner= params.hue_inner or 1.0
   local hue_outter= params.hue_outter or 240/360
   local opacity  = params.opacity or 60
   local absorption = params.absorption  or 45
   local scale    = params.scale or 1
   local move     = params.move or (0.005 * scale )
   local offset   = params.offset or vec2.new()
   local angle    = params.angle or 0

   -- Initialize seed
   prng:setSeed( system.cur():nameRaw() )

   -- Initialize shader
   local w, h = 1024, 1024
   local shader = lg.newShader(
      string.format(nebulafrag, steps, hue_inner, hue_outter, absorption, opacity, w, h, R(), R(), R()),
      love_shaders.vertexcode )
   local cvs = lg.newCanvas( w, h, {dpiscale=1} )

   local oldcanvas = lg.getCanvas()
   lg.setCanvas( cvs )
   lg.clear( 0, 0, 0, 0 )
   lg.setShader( shader )
   lg.setColor( {1,1,1,1} )
   love_shaders.img:draw( 0, 0, 0, w, h )
   lg.setShader()
   lg.setCanvas( oldcanvas )

   local x, y = offset:get()
   naev.bkg.image( cvs.t.tex, x / move, y / move, move, scale, angle )
end

return nebula
