--[[

   Map Widget for Luatk. Similar to in-game star map.

--]]
local luatk = require 'luatk'
local lg = require 'love.graphics'
local lf = require "love.filesystem"
local lk = require "love.keyboard"
local love_shaders = require "love_shaders"

local luatk_map = {}

local scale      = 0.33
local sys_radius = 15
local edge_width = 6

-- Defaults, for access from the outside
luatk_map.sys_radius = sys_radius
luatk_map.edge_width = edge_width

local cInert      = colour.new("Inert")
local cGreen      = colour.new("Green")
local cAquaBlue   = colour.new("AquaBlue")
local cRed        = colour.new("Red")
local cFontGreen  = colour.new("FontGreen")
local cYellow     = colour.new("Yellow")
local cFriend     = colour.new("Friend")
local cHostile    = colour.new("Hostile")
local cNeutral    = colour.new("Neutral")
local cRestricted = colour.new("Restricted")
local cGrey80     = colour.new("Grey80")
local inv = vec2.new(1,-1)

local Map = {}
setmetatable( Map, { __index = luatk.Widget } )
local Map_mt = { __index = Map }
function luatk_map.newMap( parent, x, y, w, h, options )
   options = options or {}
   local wgt = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, Map_mt )
   wgt.type    = "map"
   wgt.canfocus = true
   wgt.scale   = scale
   wgt.deffont = options.font or luatk._deffont or lg.getFont()
   -- TODO load same font family
   wgt.smallfont = options.fontsmall or lg.newFont( math.floor(wgt.deffont:getHeight()*0.9+0.5) )
   wgt.tinyfont = options.fonttiny or lg.newFont( math.floor(wgt.deffont:getHeight()*0.8+0.5) )
   wgt.hidenames = options.hidenames
   wgt.binaryhighlight = options.binaryhighlight
   wgt.notinteractive = options.notinteractive

   local sysname = {} -- To do quick look ups
   wgt.sys = {}
   local fplayer = faction.player()
   local function addsys( s, known )
      if s:hidden() then return end

      local sys = { s=s, p=s:pos()*inv, n=s:name(), coutter=cInert, r=luatk_map.sys_radius }
      local f = s:faction()
      if not f or not known then
         sys.c = cInert
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
         if wgt.binaryhighlight then
            if wgt.binaryhighlight( s ) then
               sys.c = cFontGreen
               sys.coutter = sys.c
               sys.r = sys.r*1.2
               sys.highlight = true
            else
               sys.c = cInert
               sys.coutter = sys.c
               sys.r = sys.r*0.8
            end
         elseif f:areEnemies( fplayer ) then
            sys.c = cHostile
         elseif not haslandable then
            sys.c = cRestricted
         elseif f:areAllies( fplayer ) then
            sys.c = cFriend
         else
            sys.c = cNeutral
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
         return cGrey80
      elseif j:hidden() then
         return cRed
      elseif j:hide() <= 0 then
         return cGreen
      else
         return cAquaBlue
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
   wgt.pos = options.pos or system.cur():pos()
   wgt.pos = wgt.pos * inv
   wgt.target = wgt.pos
   wgt.custrender = options.render

   -- Load shaders
   local path = "scripts/luatk/shaders/"
   local function load_shader( filename )
      local src = lf.read( path..filename )
      local shd = lg.newShader( src )
      shd.dt = 0
      return shd
   end
   wgt.shd_selectsys = load_shader( "selectsys.frag" )
   wgt.shd_jumplane = load_shader( "jumplane.frag" )
   wgt.shd_jumpgoto = load_shader( "jumplanegoto.frag" )

   -- Internals
   wgt._canvas = lg.newCanvas( wgt.w, wgt.h )
   if not wgt.hidenames then
      wgt._canvasnames = lg.newCanvas( wgt.w, wgt.h )
   end
   wgt._dirty = true

   return wgt
end
function Map:draw( bx, by )
   local x, y, w, h = bx+self.x, by+self.y, self.w, self.h

   if self._dirty then
      self._dirty = false

      local sx, sy, sw, sh = lg.getScissor()
      local cvs = lg.getCanvas()
      lg.setCanvas( self._canvas )
      lg.setScissor()
      lg.clear( 0, 0, 0, 1 )

      -- Get dimensions
      local c = vec2.new( w, h )*0.5

      -- Display edges
      local r = math.max( self.scale * luatk_map.sys_radius, 3 )
      local ew = math.max( self.scale * luatk_map.edge_width, 1 )
      local shd = lg.getShader()
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
            love_shaders.img:draw( px-l2, py-ew*0.5, e.a, l, ew )
         end
      end
      lg.setShader(shd)

      local cs = system.cur()

      -- Display systems
      for i,sys in ipairs(self.sys) do
         local s = sys.s
         local p = (s:pos()*inv-self.pos)*self.scale + c
         local px, py = p:get()
         local sr = sys.r * self.scale
         if not (px < -sr or px > w+sr or py < -sr or py > h+sr) then
            lg.setColour( sys.coutter )
            lg.circle( "line", px, py, sr )
            if sys.spob then
               lg.setColour( sys.c )
               lg.circle( "fill", px, py, 0.65*sr )
            end
         end
         if sys.s==cs then
            lg.setColour{ 1, 1, 1, 1 }
            lg.circle( "line", px, py, 1.5*r )
         end
      end

      -- Render names
      if not self.hidenames then
         lg.setCanvas( self._canvasnames )
         lg.clear( 0, 0, 0, 0 )
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
            if (sys.highlight and self.scale >= 0.33) or self.scale >= 0.5 then
               local n = sys.n
               local p = (sys.s:pos()*inv-self.pos)*self.scale + c
               local px, py = p:get()
               local fw = f:getWidth( n )
               px = px + r + 2
               if sys.s==cs then
                  px = px + 0.5*r
               end
               py = py - fh * 0.5
               if not (px < -fw or px > w or py < -fh or py > h) then
                  lg.print( n, f, px, py )
               end
            end
         end
      end

      -- Restore canvas
      lg.setCanvas( cvs )
      lg.setScissor( sx, sy, sw, sh )
   end

   -- Draw canvas
   lg.setColor(1, 1, 1, 1)
   self._canvas:draw( x, y )

   -- Render jump route
   if not self.hidetarget then
      luatk.rerender() -- Animated, so we have to draw every frame
      local cpos = system.cur():pos()
      local mx, my = self.pos:get()
      local jmax = player:jumps()
      local jcur = jmax
      local s = self.scale
      local r = luatk_map.sys_radius * s
      local jumpw = math.max( 10, r )
      local shd = lg.getShader()
      lg.setShader( self.shd_jumpgoto )
      self.shd_jumpgoto:send( "paramf", r )
      local route = player.autonavRoute()
      for k,sys in ipairs(route) do
         local spos = sys:pos()
         local p = (cpos + spos)*0.5
         local jumpx, jumpy = (p*inv):get()
         local jumpl, jumpa = ((spos-cpos)*inv):polar()
         jumpl = jumpl + luatk_map.sys_radius * 0.5

         local col, parami
         if jcur==jmax and jmax > 0 then
            col = cGreen
            parami = 1
         elseif jcur < 1 then
            col = cRed
            parami = 0
         else
            col = cYellow
            parami = 1
         end
         jcur = jcur-1
         lg.setColour( col )

         local jx = x + (jumpx-mx)*s + self.w*0.5
         local jy = y + (jumpy-my)*s + self.h*0.5
         self.shd_jumpgoto:send( "dimensions", {jumpl*s,jumpw} )
         self.shd_jumpgoto:send( "parami", parami )
         love_shaders.img:draw( jx-jumpl*0.5*s, jy-jumpw*0.5, jumpa, jumpl*s, jumpw )

         cpos = spos
      end

      -- Render target
      if #route > 0 then
         local tx, ty = (route[#route]:pos()*inv):get()
         lg.setColour( {1, 1, 1, 1} )
         self.shd_selectsys:send( "dimensions", {2*r, 2*r} )
         lg.setShader( self.shd_selectsys )
         love_shaders.img:draw( x+(tx-mx)*s + self.w*0.5 - 2*r, y+(ty-my)*s + self.h*0.5 - 2*r, 0, 4*r, 4*r )
      end

      lg.setShader(shd)
   end

   -- Allow for custom rendering
   if self.custrender then
      self.custrender( self, x, y )
   end

   -- Draw canvas
   if not self.hidenames then
      lg.setColor(1, 1, 1, 1)
      self._canvasnames:draw( x, y )
   end
end
function Map:rerender()
   self._dirty = true
   luatk.rerender()
end
function Map:center( pos, hardset )
   self.target = pos or vec2.new()
   self.target = self.target * inv
   if hardset then
      self.pos = self.target
   else
      self.speed = (self.pos-self.target):dist() * 3
   end
   self:rerender()
end
function Map:update( dt )
   if (self.pos - self.target):dist2() > 1e-3 then
      self:rerender() -- Fully animated, so draw every frame
      local mod, dir = (self.target - self.pos):polar()
      self.pos = self.pos + vec2.newP( math.min(mod,self.speed*dt), dir )
   end

   if player.autonavDest() then
      self.shd_jumpgoto.dt = self.shd_jumpgoto.dt + dt
      self.shd_jumpgoto:send( "dt", self.shd_jumpgoto.dt )
      self.shd_selectsys.dt = self.shd_selectsys.dt + dt
      self.shd_selectsys:send( "dt", self.shd_selectsys.dt )
   end
end
function Map:clicked( mx, my )
   local m = vec2.new(mx,my)
   if not self.notinteractive and self._mouse:dist(m) < 10 then
      local c = vec2.new( self.w, -self.h )*0.5
      local p = (m*inv-c) / self.scale + self.pos*inv
      local s, d = nil, math.huge
      for k,sys in ipairs(self.sys) do
         local sd = (sys.s:pos()-p):dist2()
         if sd < d then
            s = sys.s
            d = sd
         end
      end
      local r = math.max( self.scale * luatk_map.sys_radius, 3 )
      if d < (r*4)^2 then
         player.autonavSetTarget( s, lk.isDown("lshift","rshift") )
      end
   end
end
function Map:pressed( mx, my )
   self._mouse = vec2.new( mx, my )
end
function Map:mmoved( _mx, _my, dx, dy )
   if self._pressed then
      self.pos = self.pos - vec2.new( dx, dy ) / self.scale
      self.target = self.pos
      self:rerender()
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
      self:rerender()
   end
end

return luatk_map
