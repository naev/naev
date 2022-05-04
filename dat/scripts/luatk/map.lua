local luatk = require 'luatk'
local lg = require 'love.graphics'

local luatk_map = {}

local scale      = 1/3
local sys_radius = 5
--local sys_inner  = 3
local edge_width = 3

local Map = {}
setmetatable( Map, { __index = luatk.Widget } )
local Map_mt = { __index = Map }
function luatk_map.newMap( parent, x, y, w, h )
   local wgt = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, Map_mt )

   local sysname = {} -- To do quick look ups
   wgt.sys = {}
   --local inv = vec2.new(1,-1)
   local function addsys( s )
      local sys = { s=s, p=s:pos() }
      local f = s:faction()
      if f then
         sys.c = { f:colour():rgb(true) }
      else
         sys.c = { 221/255, 221/255, 221/255 }
      end
      table.insert( wgt.sys, sys )
      sysname[ s:nameRaw() ] = #wgt.sys
   end
   local allsys = system.getAll()
   for i,s in ipairs(allsys) do
      if s:known() then
         addsys( s )
      else
         -- Could still be near a known system
         for j,a in ipairs(s:adjacentSystems()) do
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

   wgt.pos = vec2.new()

   return wgt
end
function Map:draw( bx, by )
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h

   lg.push()
   lg.translate( x, y )

   lg.setColor( {0, 0, 0} )
   lg.rectangle( "fill", 0, 0, w, h )

   -- Set scissors
   local scs = lg.getScissor()
   lg.setScissor( x, y, w, h )
   local c = vec2.new( w, h )*0.5

   -- Display edges
   lg.setColor( {0.5, 0.5, 0.5} )
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
   for i,sys in ipairs(self.sys) do
      local s = sys.s
      local p = (s:pos()-self.pos)*scale + c
      local px, py = p:get()
      if not (px < -r or px > w+r or py < -r or py > h+r) then
         lg.setColor( sys.c )
         lg.circle( "line", px, py, r )
      end
   end

   -- Restore scissors
   lg.setScissor( scs )
   lg.pop()
end
function Map:center( pos )
   self.pos = pos or vec2.new()
end

return luatk_map
