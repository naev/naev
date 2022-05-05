--[[
   Active hypergate
--]]
local fmt = require "format"
local lg = require "love.graphics"
local lf = require "love.filesystem"
local love_shaders = require "love_shaders"
local luatk = require "luatk"
local luatk_map = require "luatk.map"

local pos, tex, mask, cvs, shader
local tw, th

local pixelcode = lf.read( "spob/lua/glsl/hypergate.frag" )

local hypergate = {}
local hypergate_spob

local function update_canvas ()
   local oldcanvas = lg.getCanvas()
   lg.setCanvas( cvs )
   lg.clear( 0, 0, 0, 0 )
   lg.setColor( 1, 1, 1, 1 )
   --lg.setBlendMode( "alpha", "premultiplied" )

   -- Draw base hypergate
   tex:draw( 0, 0 )

   -- Draw active overlay shader
   local oldshader = lg.getShader()
   lg.setShader( shader )
   mask:draw( 0, 0 )
   lg.setShader( oldshader )

   --lg.setBlendMode( "alpha" )
   lg.setCanvas( oldcanvas )
end

function hypergate.load( p, opts )
   opts = opts or {}
   hypergate_spob = p

   if tex==nil then
      -- Handle some options
      local basecol = opts.basecol or { 0.2, 0.8, 0.8 }

      -- Set up texture stuff
      local prefix = "gfx/spob/space/"
      tex  = lg.newImage( prefix.."hypergate_neutral_activated.webp" )
      mask = lg.newImage( prefix.."hypergate_mask.webp" )

      -- Position stuff
      pos = p:pos()
      tw, th = tex:getDimensions()
      pos = pos + vec2.new( -tw/2, th/2 )

      -- The canvas
      cvs  = lg.newCanvas( tw, th, {dpiscale=1} )

      -- Set up shader
      local fragcode = string.format( pixelcode, basecol[1], basecol[2], basecol[3] )
      shader = lg.newShader( fragcode, love_shaders.vertexcode )
      shader._dt = -1000 * rnd.rnd()
      shader.update = function( self, dt )
         self._dt = self._dt + dt
         self:send( "u_time", self._dt )
      end

      update_canvas()
   end

   return cvs.t.tex, tw/2
end

function hypergate.unload ()
   shader= nil
   tex   = nil
   mask  = nil
   cvs   = nil
   --sfx   = nil
end

function hypergate.render ()
   update_canvas() -- We want to do this here or it gets slow in autonav
   local z = camera.getZoom()
   local x, y = gfx.screencoords( pos, true ):get()
   z = 1/z
   cvs:draw( x, y, 0, z, z )
end

function hypergate.update( dt )
   shader:update( dt )
end

function hypergate.can_land ()
   return true, "The hypergate is active."
end

local hypergate_window
function hypergate.land( _s, p )
   -- Avoid double landing
   if p:shipvarPeek( "hypergate" ) then return end

   if player.pilot() == p then
      local target = hypergate_window()
      -- TODO animation and stuff, probably similar to wormholes
      if target then
         player.teleport( target )
         p:effectClear()
         p:effectAdd("Hypergate Exit")
      end
   else
      p:shipvarPush( "hypergate", true )
      p:effectAdd("Hypergate Enter")
   end
end

function hypergate_window ()
   local w = 900
   local h = 600
   local wdw = luatk.newWindow( nil, nil, w, h )
   luatk.newText( wdw, 0, 10, w, 20, fmt.f(_("Hypergate ({sysname})"), {sysname=hypergate_spob:system()}), nil, "center" )

   -- Load shaders
   local path = "spob/lua/glsl/"
   local function load_shader( filename )
      local src = lf.read( path..filename )
      return lg.newShader( src )
   end
   local shd_jumpgoto = load_shader( "jumpgoto.frag" )
   shd_jumpgoto.dt = 0

   -- Get potential destinations from tags
   local csys = system.cur()
   local cpos = csys:pos()
   local destinations = {}
   for i,s in ipairs(system.getAll()) do
      if s ~= csys then
         for j,sp in ipairs(s:spobs()) do
            local t = sp:tags()
            if t.hypergate and t.active then
               table.insert( destinations, sp )
            end
         end
      end
   end
   table.sort( destinations, function( a, b ) return a:nameRaw() < b:nameRaw() end )
   local destnames = {}
   for i,d in ipairs(destinations) do
      if d:known() then
         table.insert( destnames, d:system():nameRaw() )
      else
         table.insert( destnames, _("Unknown Signature") ) -- TODO convert name into symbol or hash
      end
   end

   local inv = vec2.new(1,-1)
   local targetknown = false
   local mapw, maph = w-330, h-60
   --local mapfont = lg.newFont(32)
   local jumpx, jumpy, jumpl, jumpa = 0, 0, 0, 0
   local jumpw = 10
   local map = luatk_map.newMap( wdw, 20, 40, mapw, maph, {
      render = function ( m )
         if not targetknown then
            lg.setColor( {0, 0, 0, 0.3} )
            lg.rectangle("fill", 0, 0, mapw, maph )
            -- Show big question mark or something
         else
            local mx, my = m.pos:get()
            local s = luatk_map.scale
            lg.setColor( {0, 0.5, 1, 0.7} )
            lg.push()
            lg.translate( (jumpx-mx)*s + mapw*0.5, (jumpy-my)*s + maph*0.5 )
            lg.rotate( jumpa )
            --lg.rectangle("fill", -jumpl*0.5*s, -5*s, jumpl*s, 10*s )
            lg.setShader( shd_jumpgoto )
            love_shaders.img:draw( -jumpl*0.5*s, -jumpw*0.5, 0, jumpl*s, jumpw )
            lg.setShader()
            lg.pop()
         end
      end,
   } )
   local function map_center( _sys, idx, hardset )
      local s = destinations[ idx ]:system()
      targetknown = s:known()
      if targetknown then
         local p = (cpos + s:pos())*0.5
         jumpx, jumpy = (p*inv):get()
         jumpl, jumpa = ((s:pos()-cpos)*inv):polar()
         shd_jumpgoto:send( "dimensions", {jumpl*luatk_map.scale,jumpw} )
         map:center( p, hardset )
      else
         jumpx, jumpy = 0, 0
         jumpl, jumpa = 0, 0
         map.center( cpos, hardset )
      end
   end
   map_center( nil, 1, true ) -- Center on first item in the list

   local lst = luatk.newList( wdw, w-260-20, 40, 260, h-40-20-40-20, destnames, map_center )

   local target_gate
   local function btn_jump ()
      local _sel, idx = lst:get()
      target_gate = destinations[ idx ]
      luatk.close()
   end
   luatk.newButton( wdw, w-(120+20)*2, h-40-20, 120, 40, _("Jump!"), btn_jump )
   luatk.newButton( wdw, w-120-20, h-40-20, 120, 40, _("Close"), luatk.close )

   wdw:setUpdate( function ( dt )
      shd_jumpgoto.dt = shd_jumpgoto.dt + dt
      shd_jumpgoto:send( "dt", shd_jumpgoto.dt )
   end )
   wdw:setAccept( btn_jump )
   wdw:setCancel( luatk.close )
   wdw:setKeypress( function ( key )
      if key=="down" then
         local _sel, idx = lst:get()
         lst:set( idx+1 )
         return true
      elseif key=="up"then
         local _sel, idx = lst:get()
         lst:set( idx-1 )
         return true
      end
      return false
   end )

   luatk.run()

   return target_gate
end

return hypergate
