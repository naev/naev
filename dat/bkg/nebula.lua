--[[
   One jump away from the nebula, so we get a bit nebula in the background.
--]]
local lg = require "love.graphics"
local lf = require 'love.filesystem'
local love_shaders = require 'love_shaders'
local prng_lib = require "prng"
local prng = prng_lib.new()

local nebulafrag = lf.read('bkg/shaders/nebula.frag')

local function R()
   return (2*prng:random()-1)*1000
end

local nebula = {}

function nebula.init( params )
   params = params or {}
  
   -- Initialize seed
   prng:setSeed( system.cur():nameRaw() )

   -- Initialize shader
   local w, h = 1024, 1024
   local shader = lg.newShader(
      string.format(nebulafrag, w, h, R(), R(), R()),
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

   local move = 0.03
   local scale = 5
   local angle = 1
   naev.bkg.image( cvs.t.tex, 0, 0, move, scale, angle )
end

return nebula
