local lg = require "love.graphics"
local lmisn = require "lmisn"
local love_shaders = require "love_shaders"

local lib = {}

function lib.create_map( sys, w, h, n )
   local jumps = {}
   local systems = lmisn.getSysAtDistance( sys, 0, n )
   local maxdist = 0
   local center = sys:pos()
   for k,s in ipairs(systems) do
      maxdist = math.max( maxdist, s:pos():dist( center ) )
      for i,j in ipairs(s:jumps()) do
         local r = j:reverse()
         --if not inlist( jumps, j ) and not inlist( jumps, r ) then
         if inlist( systems, j:dest() ) and not inlist( jumps, j ) and not inlist( jumps, r ) then
            table.insert( jumps, j )
         end
      end
   end

   -- Flip Y
   lg.push()
      lg.translate( 0, h )
      lg.scale( 1, -1 )

   local c = love_shaders.paper( w, h )
   local cx, cy = center:get()
   lg.setCanvas( c )
   lg.setColour( 0, 0, 0, 1 )
   local r = 10
   for k,s in ipairs(systems) do
      local x, y = s:pos():get()
      lg.circle( "fill",
         w*(0.5+0.5*(x-cx)/maxdist),
         h*(0.5+0.5*(y-cy)/maxdist),
         r)
   end
   for k,j in ipairs(jumps) do
      local p1 = j:system():pos()
      local x1, y1 = p1:get()
      local p2 = j:dest():pos()
      local x2, y2 = p2:get()
      local len, ang = (p2-p1):polar()
      local x = (0.5+0.5*((x1+x2)*0.5-cx)/maxdist)*w
      local y = (0.5+0.5*((y1+y2)*0.5-cy)/maxdist)*h
      len = len/maxdist*w*0.5

      lg.push()
         lg.translate( x, y )
         lg.rotate( ang )
      lg.rectangle( "fill", -len*0.5, -2, len, 4 )
      lg.pop()
   end
   lg.setCanvas()

   lg.pop()

   return c
end

return lib
