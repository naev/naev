local luatk = require 'luatk'
local lg = require 'love.graphics'

local luatk_map = {}

local Map = {}
setmetatable( Map, { __index = luatk.Widget } )
local Map_mt = { __index = Map }
function luatk_map.newMap( parent, x, y, w, h )
   local wgt = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, Map_mt )

   local sysname = {} -- To do quick look ups
   wgt.sys = {}
   local function addsys( s )
      table.insert( wgt.sys, s )
      sysname[ s:nameRaw() ] = #wgt.sys
   end
   local sys = system.getAll()
   for i,s in ipairs(sys) do
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
   for ids,s in ipairs(sys) do
      local ps = s:pos()
      for j,a in ipairs(s:adjacentSystems()) do
         local ida = sysname[ a:nameRaw() ]
         if ida and ida < ids then
            local pa = wgt.sys[ ida ]:pos()
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

   lg.setColor( {0, 0, 0} )
   lg.rectangle( "fill", x, y, w, h )

   -- Set scissors
   local scs = lg.getScissor()
   lg.setScissor( x, y, w, h )

   local scale = 0.5

   -- Display edges
   lg.setColor( {0.5, 0.5, 0.5} )
   for i,e in ipairs(self.edges) do
      local px, py = ((e.c-self.pos)*scale):get()
      local l2 = e.l*0.5*scale
      if not (px < -l2 or px > w+l2 or py < -l2 or py > h+l2) then
         lg.push()
         lg.translate( x+px, y+py )
         lg.rotate( e.a )
         lg.rectangle("fill", -l2, -2.5*scale, e.l*scale, 5*scale )
         lg.pop()
      end
   end

   -- Display systems
   local r = 5
   lg.setColor( {0.8, 0.8, 0.8} )
   for i,s in ipairs(self.sys) do
      local p = (s:pos()-self.pos)*scale
      local px, py = p:get()
      if not (px < -r or px > w+r or py < -r or py > h+r) then
         lg.circle( "line", x+px, y+py, r )
      end
   end

   -- Restore scissors
   lg.setScissor( scs )
end
function Map:center( pos )
   self.pos = pos or vec2.new()
end

return luatk_map
