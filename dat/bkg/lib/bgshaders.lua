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

local bgshader = {}
local bgshader_mt = { __index = bgshader }
local bgbright

function bgshaders.init( shader, scale, params )
   local shd = {
      bgshader = shader,
      bgscale  = scale or 1,
      params   = params or {},
   }
   setmetatable( shd, bgshader_mt )

   if shd.bgscale ~= 1 or shd.params.usetex then
      shd.cw, shd.ch = nw/shd.bgscale, nh/shd.bgscale
      shd.bgcanvas = lg.newCanvas( shd.cw, shd.ch )
   end

   if shd.params.usetex then
      shd.prevcanvas = lg.newCanvas( shd.cw, shd.ch )
   end

   bgbright =  naev.conf().bg_brightness

   return shd
end

function bgshader:render( dt, col )
   dt = dt or 0
   col = col or {1, 1, 1, 1}
   col[1] = col[1] * bgbright
   col[2] = col[2] * bgbright
   col[3] = col[3] * bgbright

   -- Update shader if necessary
   if self.bgshader.update then
      self.bgshader:update( dt )
   end

   -- We have to draw to a canvas
   if self.bgcanvas then
      -- Save state
      local oldcanvas = lg.getCanvas()
      local oldshader = lg.getShader()

      if self.prevcanvas then
         self.bgshader:send( "u_prevtex", self.prevcanvas )
      end

      -- Render to canvas
      lg.setCanvas( self.bgcanvas )
      lg.clear( 0, 0, 0, 0 )
      lg.setShader( self.bgshader )
      lg.setColor( col )
      lg.setBlendMode( "alpha", "premultiplied" )
      love_shaders.img:draw( 0, 0, 0, self.cw, self.ch )
      lg.setBlendMode( "alpha" )
      lg.setShader( oldshader )
      lg.setCanvas( oldcanvas )

      -- Render to screen
      lg.setColor( 1, 1, 1, 1 )
      self.bgcanvas:draw( 0, 0, 0, self.bgscale, self.bgscale )

      -- Swap buffers
      if self.prevcanvas then
         self.prevcanvas, self.bgcanvas = self.bgcanvas, self.prevcanvas
      end
      return
   end

   -- Native resolution
   lg.setColor( col )
   local oldshader = lg.getShader()
   lg.setShader( self.bgshader )
   love_shaders.img:draw( 0, 0, 0, nw, nh )
   lg.setShader( oldshader )
end

return bgshaders
