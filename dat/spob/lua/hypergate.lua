local lg = require "love.graphics"

local pos, tex
function load( p )
   if tex==nil then
      tex = lg.newImage( "gfx/spob/space/hypergate_destroyed.webp" )
      pos = p:pos()
      local tw, th = tex:getDimensions()
      pos = pos + vec2.new( -tw/2, th/2 )
   end

   return tex.tex
end

function unload ()
   tex = nil -- Should get GC'd
end

--[[
function render ()
   local z = camera.getZoom()
   local x, y = gfx.screencoords( pos, true ):get()
   tex:draw( x, y, 0, 1/z )
end
--]]

function can_land ()
   return false
end
