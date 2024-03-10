local luatk = require 'luatk'
local lg = require 'love.graphics'

local luatk_map = {}

local scale      = 1/3
local sys_radius = 5
--local sys_inner  = 3
local edge_width = 3

-- For access from the outside
luatk_map.scale = scale
luatk_map.sys_radius = sys_radius
luatk_map.edge_width = edge_width

local Map = {}
setmetatable( Map, { __index = luatk.Widget } )
local Map_mt = { __index = Map }
function luatk_map.newMap( parent, x, y, w, h, options )
   options = options or {}
   local wgt = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, Map_mt )

   local sysname = {} -- To do quick look ups
   wgt.sys = {}
   local inv = vec2.new(1,-1)
   local function addsys( s )
      local sys = { s=s, p=s:pos()*inv }
      local f = s:faction()
      if f then
         sys.c = { f:colour():rgb(true) }
      else
         sys.c = { 221/255, 221/255, 221/255 } -- cInert
      end
      table.insert( wgt.sys, sys )
      sysname[ s:nameRaw() ] = #wgt.sys
   end
   for i,s in ipairs(system.getAll()) do
      if s:known() then
         addsys( s )
      else
         -- Could still be near a known system
         for j,a in ipairs(s:jumps()) do
            if a:known() then
               addsys( s )
               break
            end
         end
      end
   end

   wgt.edges = {}
   for ids,sys in ipairs(wgt.sys) do
      local s = sys.s
      local ps = sys.p
      for j,a in ipairs(s:adjacentSystems()) do
         local ida = sysname[ a:nameRaw() ]
         if ida and ida < ids then
            local pa = wgt.sys[ ida ].p
            local len, ang = (pa-ps):polar()
            local e = { v0=ps, v1=pa, c=(ps+pa)*0.5, a=ang, l=len }
            table.insert( wgt.edges, e )
         end
      end
   end

   -- Set up custom options and the likes
   wgt.pos = options.pos or vec2.new()
   wgt.custrender = options.render

   return wgt
end
function Map:draw( bx, by )
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h

   lg.push()
   lg.translate( x, y )

   lg.setColour( {0, 0, 0} )
   lg.rectangle( "fill", 0, 0, w, h )

   -- Set scissors
   local scs = lg.getScissor()
   lg.setScissor( x, y, w, h )
   local c = vec2.new( w, h )*0.5

   -- Display edges
   lg.setColour( {0.5, 0.5, 0.5} )
   for i,e in ipairs(self.edges) do
      local px, py = ((e.c-self.pos)*scale + c):get()
      local l = (e.l-sys_radius*2)*scale
      local l2 = l*0.5
      if not (px < -l2 or px > w+l2 or py < -l2 or py > h+l2) then
         lg.push()
         lg.translate( px, py )
         lg.rotate( e.a )
         lg.rectangle("fill", -l2, -edge_width*0.5*scale, l, edge_width*scale )
         lg.pop()
      end
   end

   -- Display systems
   local r = sys_radius
   local inv = vec2.new(1,-1)
   for i,sys in ipairs(self.sys) do
      local s = sys.s
      local p = (s:pos()*inv-self.pos)*scale + c
      local px, py = p:get()
      if not (px < -r or px > w+r or py < -r or py > h+r) then
         lg.setColour( sys.c )
         lg.circle( "line", px, py, r )
      end
   end

   -- Allow for custom rendering
   if self.custrender then
      self.custrender( self )
   end

   -- Restore scissors
   lg.setScissor( scs )
   lg.pop()
end
function Map:center( pos, hardset )
   self.target = pos or vec2.new()
   self.target = self.target * vec2.new(1,-1)

   if hardset then
      self.pos = self.target
   else
      self.speed = (self.pos-self.target):dist() * 3
   end
end
function Map:update( dt )
   if (self.pos - self.target):dist2() > 1e-3 then
      local mod, dir = (self.target - self.pos):polar()
      self.pos = self.pos + vec2.newP( math.min(mod,self.speed*dt), dir )
   end
end
function Map:pressed( mx, my )
   self._mouse = vec2.new( mx, my )
end
function Map:mmoved( mx, my )
   if self._pressed then
      self.pos = self.pos + (self._mouse - vec2.new( mx, my )) / scale
      self.target = self.pos
      self._mouse = vec2.new( mx, my )
   end
end

return luatk_map
