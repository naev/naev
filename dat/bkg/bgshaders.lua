--[[
   Small library to handle rendering a fullscreen background shader.
   Automatically handles scaling as necesary.
--]]
local love = require "love"
local lg = require "love.graphics"
local love_shaders = require 'love_shaders'

local bgshaders = {}

-- Since we don't actually activate the Love framework we have to fake the
-- the dimensions and width, and set up the origins.
local nw, nh = naev.gfx.dim()
love.x = 0
love.y = 0
love.w = nw
love.h = nh
lg.origin()

local bgshader, bgscale, bgcanvas
local cw, ch

function bgshaders.init( shader, scale )
   bgshader = shader
   bgscale  = scale or 1

   if bgscale ~= 1 then
      cw, ch = nw/bgscale, nh/bgscale
      bgcanvas = lg.newCanvas( cw, ch )
   end
end

function bgshaders.render( dt, col )
   dt = dt or 0
   col = col or {1, 1, 1, 1}

   -- Update shader if necessary
   if bgshader.update then
      bgshader:update( dt )
   end

   -- We have to draw to a canvas
   if bgcanvas then
      -- Save state
      local oldcanvas = lg.getCanvas()
      local oldshader = lg.getShader()

      -- Render to canvas
      lg.setCanvas( bgcanvas )
      lg.clear( 0, 0, 0, 0 )
      lg.setShader( bgshader )
      lg.setColor( col )
      lg.setBlendMode( "alpha", "premultiplied" )
      love_shaders.img:draw( 0, 0, 0, cw, ch )
      lg.setBlendMode( "alpha" )
      lg.setShader( oldshader )
      lg.setCanvas( oldcanvas )

      -- Render to screen
      lg.setColor( 1, 1, 1, 1 )
      bgcanvas:draw( 0, 0, 0, sf, sf )
      return
   end

   -- Native resolution
   lg.setColor( col )
   local oldshader = lg.getShader()
   lg.setShader( bgshader )
   love_shaders.img:draw( 0, 0, 0, nw, nh )
   lg.setShader( oldshader )
end

return bgshaders
