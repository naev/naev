--[[
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

function background ()
   -- Scale factor that controls computation cost. As this shader is really
   -- really expensive, we can't compute it at full resolution
   sf = 4

   nw, nh = naev.gfx.dim()
   shader = love_shaders.windy{ strength=sf }
   cw, ch = nw/sf, nh/sf
   canvas = lg.newCanvas( cw, ch )

   -- Default nebula background
   cur_sys = system.cur()
   prng:setSeed( cur_sys:name() )
   background_nebula()
   background_stars()
end

function renderfg( dt )
   -- Get camera properties
   local x, y = camera.get():get()
   local z = camera.getZoom()
   shader:send( "u_camera", x*0.5/sf, -y*0.5/sf, z )
   shader:update( dt )
  
   -- Save state
   local oldcanvas = lg.getCanvas()
   local oldshader = lg.getShader()

   -- Render to canvas
   lg.setCanvas( canvas )
   lg.clear( 0, 0, 0, 0 )
   lg.setShader( shader )
   lg.setColor( 0.2, 0.6, 0.9, 0.8 )
   lg.setBlendMode( "alpha", "premultiplied" )
   love_shaders.img:draw( 0, 0, 0, cw, ch )
   lg.setBlendMode( "alpha" )
   lg.setShader( oldshader )
   lg.setCanvas( oldcanvas )

   -- Render to screen
   lg.setColor( 1, 1, 1, 1 )
   canvas:draw( 0, 0, 0, sf, sf )
end
