local lg = require 'love.graphics'

local onion = require "common.onion"

local onion_gfx

local function update( s, dt )
   local d = s:data()
   if not d.p:exists() then
      return s:rm()
   end
   d.t = d.t + dt*0.7
   s:setPos( d.p:pos() )
end

local function render( sp, x, y, z )
   local d = sp:data()
   lg.setColour( 1, 1, 1, 0.5+0.1*math.cos(d.t*4.5) )
   onion_gfx:draw( x-z*d.r, y-z*d.r, d.t, d.s*z, d.s*z )
end

local function onionize( p )
   if not onion_gfx then
      onion_gfx = onion.img_onion()
   end

   local size = p:ship():screenSize()*1.25
   local s = spfx.new( math.huge, update, nil, render, nil, p:pos(), nil, nil, size*0.5 )
   local d = s:data()
   d.t = 0
   d.p = p
   d.r = size * 0.5
   d.s = size / onion_gfx:getWidth()
   return s
end

return onionize
