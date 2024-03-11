local luatk = require 'luatk'
local lg = require 'love.graphics'

local luatk_bezier = {}

local function lerp(a, b, t)
	return a + (b - a) * t
end

local function cubicBezier( t, p0, p1, p2, p3 )
	local l1 = lerp(p0, p1, t)
	local l2 = lerp(p1, p2, t)
	local l3 = lerp(p2, p3, t)
	local l4 = lerp(l1, l2, t)
	local l5 = lerp(l2, l3, t)
   return lerp(l4, l5, t)
end

local Bezier = {}
setmetatable( Bezier, { __index = luatk.Widget } )
local Bezier_mt = { __index = Bezier }
function luatk_bezier.newBezier( parent, x, y, w, h, curves )
   local wgt = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, Bezier_mt )

   wgt:set( curves )

   return wgt
end
function Bezier:draw( bx, by )
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h

   lg.push()
   lg.translate( x, y )

   lg.setColour( {0, 0, 0} )
   lg.rectangle( "fill", 0, 0, w, h )

   lg.setColour( {1, 1, 1} )
   if self.lines then
      lg.line( table.unpack(self.lines) )
   end

   lg.pop()
end
function Bezier:set( curves )
   curves = curves or {}

   -- Generate the points from the curves
   self.lines = {}
   for k,v in ipairs(curves) do
      for t = 0,1,0.1 do -- only 10 samples
         local p = cubicBezier( t, v[1], v[1]+v[2], v[4]+v[3], v[4] )
         table.insert( self.lines, p )
      end
   end

   -- Scale and make fit into selection
   local minx, maxx, miny, maxy = math.huge, -math.huge, math.huge, -math.huge
   for k,v in ipairs(self.lines) do
      local x,y = v:get()
      minx = math.min( minx, x )
      maxx = math.max( maxx, x )
      miny = math.min( miny, y )
      maxy = math.max( maxy, y )
   end
   local minv = vec2.new( minx, miny )
   local scale = math.min( (self.w-20)/(maxx-minx), (self.h-20)/(maxy-miny) )
   local off = (vec2.new(self.w,self.h)-vec2.new(maxx-minx,maxy-miny)*scale)*0.5
   for k,v in ipairs(self.lines) do
      self.lines[k] = (v-minv) * scale + off
   end
end

return luatk_bezier
