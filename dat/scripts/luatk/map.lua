local luatk = require 'luatk'
local lg = require 'love.graphics'
local lf = require "love.filesystem"
local love_shaders = require "love_shaders"

local luatk_map = {}

local scale      = 0.5
local sys_radius = 15
local edge_width = 6

-- Defaults, for access from the outside
luatk_map.scale = scale
luatk_map.sys_radius = sys_radius
luatk_map.edge_width = edge_width

local cInert = colour.new("Inert")

local Map = {}
setmetatable( Map, { __index = luatk.Widget } )
local Map_mt = { __index = Map }
function luatk_map.newMap( parent, x, y, w, h, options )
   options = options or {}
   local wgt = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, Map_mt )
   wgt.type    = "map"
   wgt.canfocus = true
   wgt.scale   = luatk_map.scale
   wgt.deffont = options.font or luatk._deffont or lg.getFont()
   -- TODO load same font family
   wgt.smallfont = options.fontsmall or lg.newFont( math.floor(wgt.deffont:getHeight()*0.9+0.5) )
   wgt.tinyfont = options.fonttiny or lg.newFont( math.floor(wgt.deffont:getHeight()*0.8+0.5) )

   local sysname = {} -- To do quick look ups
   wgt.sys = {}
   local inv = vec2.new(1,-1)
   local fplayer = faction.player()
   local function addsys( s, known )
      local sys = { s=s, p=s:pos()*inv, n=s:name() }
      local f = s:faction()
      if not f or not known then
         sys.c = colour.new("Inert")
      else
         local haslandable = false
         for k,spb in ipairs(s:spobs()) do
            if spb:known() then
               sys.spob = true
               if spb:canLand() then
                  haslandable = true
                  break
               end
            end
         end
         if f:areEnemies( fplayer ) then
            sys.c = colour.new("Hostile")
         elseif not haslandable then
            sys.c = colour.new("Restricted")
         elseif f:areAllies( fplayer ) then
            sys.c = colour.new("Friend")
         else
            sys.c = colour.new("Neutral")
         end
      end
      table.insert( wgt.sys, sys )
      sysname[ s:nameRaw() ] = #wgt.sys
   end
   local sysall = system.getAll()
   for i,s in ipairs(sysall) do
      if s:known() then
         addsys( s, true )
      else
         -- Could still be near a known system
         for j,a in ipairs(s:jumps()) do
            if a:known() then
               addsys( s, false )
               break
            end
         end
      end
   end

   local function edge_col( j )
      if j:exitonly() then
         return colour.new("Grey80")
      elseif j:hidden() then
         return colour.new("Red")
      elseif j:hide() <= 0 then
         return colour.new("Green")
      else
         return colour.new("AquaBlue")
      end
   end

   wgt.edges = {}
   for ids,s in ipairs(sysall) do
      local ps = s:pos()*inv
      for i,j in ipairs(s:jumps(true)) do
         local a = j:dest()
         local ida = sysname[ a:nameRaw() ]
         if ida and ida < ids and j:known() then
            local pa = wgt.sys[ ida ].p
            local len, ang = (pa-ps):polar()
            local cs, ce = edge_col(j), edge_col(j:reverse())
            local e = { v0=ps, v1=pa, c=(ps+pa)*0.5, a=ang, l=len, cs=cs, ce=ce }
            table.insert( wgt.edges, e )
         end
      end
   end

   -- Set up custom options and the likes
   wgt.pos = options.pos or vec2.new()
   wgt.target = wgt.pos
   wgt.custrender = options.render

   -- Load shaders
   local path = "scripts/luatk/glsl/"
   local function load_shader( filename )
      local src = lf.read( path..filename )
      return lg.newShader( src )
   end
   wgt.shd_jumplane = load_shader( "jumplane.frag" )

   return wgt
end
function Map:draw( bx, by )
   -- TODO use a canvas for the widget so we can do animations for cheap
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h

   lg.push()
   lg.translate( x, y )

   lg.setColour( {0, 0, 0} )
   lg.rectangle( "fill", 0, 0, w, h )

   -- Set scissors
   local sx, sy, sw, sh = luatk.joinScissors( x, y, w, h )
   local c = vec2.new( w, h )*0.5

   -- Display edges
   local r = math.max( self.scale * luatk_map.sys_radius, 3 )
   local ew = math.max( self.scale * luatk_map.edge_width, 1 )
   lg.setShader( self.shd_jumplane )
   self.shd_jumplane:send( "paramf", r*3 )
   for i,e in ipairs(self.edges) do
      local px, py = ((e.c-self.pos)*self.scale + c):get()
      local l = e.l*self.scale
      local l2 = l*0.5
      if not (px < -l2 or px > w+l2 or py < -l2 or py > h+l2) then
         lg.setColour( e.cs )
         self.shd_jumplane:send( "paramv", e.ce:rgba() )
         self.shd_jumplane:send( "dimensions", l, ew )

         lg.push()
         lg.translate( px, py )
         lg.rotate( e.a )
         love_shaders.img:draw( -l2, -ew*0.5, 0, l, ew )
         lg.pop()
      end
   end
   lg.setShader()

   -- Display systems
   local inv = vec2.new(1,-1)
   for i,sys in ipairs(self.sys) do
      local s = sys.s
      local p = (s:pos()*inv-self.pos)*self.scale + c
      local px, py = p:get()
      if not (px < -r or px > w+r or py < -r or py > h+r) then
         lg.setColour( cInert )
         lg.circle( "line", px, py, r )
         if sys.spob then
            lg.setColour( sys.c )
            lg.circle( "fill", px, py, 0.65*r )
         end
      end
   end

   -- Render names
   if self.scale >= 0.5 then
      local f
      if self.scale >= 1.5 then
         f = self.deffont
      elseif self.scale > 1.0 then
         f = self.smallfont
      else
         f = self.tinyfont
      end
      local fh = f:getHeight()
      lg.setColour( 1, 1, 1 )
      for i,sys in ipairs(self.sys) do
         local n = sys.n
         local p = (sys.s:pos()*inv-self.pos)*self.scale + c
         local px, py = p:get()
         local fw = f:getWidth( n )
         px = px + r + 2.
         py = py - fh * 0.5
         if not (px < -fw or px > w or py < -fh or py > h) then
            lg.print( n, f, px, py )
         end
      end
   end

   -- Allow for custom rendering
   if self.custrender then
      self.custrender( self )
   end

   -- Restore scissors
   lg.setScissor( sx, sy, sw, sh )
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
      luatk.rerender() -- Fully animated, so draw every frame
      local mod, dir = (self.target - self.pos):polar()
      self.pos = self.pos + vec2.newP( math.min(mod,self.speed*dt), dir )
   end
end
function Map:pressed( mx, my )
   self._mouse = vec2.new( mx, my )
end
function Map:mmoved( mx, my )
   if self._pressed then
      self.pos = self.pos + (self._mouse - vec2.new( mx, my )) / self.scale
      self.target = self.pos
      self._mouse = vec2.new( mx, my )
      luatk.rerender()
   end
end
function Map:wheelmoved( _mx, my )
   if my > 0 then
      self:setScale( self.scale * 1.1 )
   elseif my < 0 then
      self:setScale( self.scale * 0.9 )
   end
   return true -- Always eat the event
end
function Map:setScale( s )
   local ss =self.scale
   self.scale = s
   self.scale = math.min( 5, self.scale )
   self.scale = math.max( 0.1, self.scale )
   if ss ~= self.scale then
      luatk.rerender()
   end
end

return luatk_map
