--[[
   Active hypergate
--]]
local lg = require "love.graphics"
local lf = require "love.filesystem"
local love_shaders = require "love_shaders"
local luatk = require "luatk"
local luatk_map = require "luatk.map"

local pos, tex, mask, cvs, shader
local tw, th

local pixelcode = lf.read( "spob/lua/glsl/hypergate.frag" )

local hypergate = {}

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
   p:shipvarPush( "hypergate", true )

   p:effectAdd("Hypergate Enter")

   if player.pilot() == p then
      hypergate_window()
   end
end

function hypergate_window ()
   local w = 800
   local h = 600
   local wdw = luatk.newWindow( nil, nil, w, h )

   local csys = system.cur()
   local cpos = csys:pos()

   local destinations = {}
   for i,s in ipairs(system.getAll()) do
      if s ~= csys then
         for j,sp in ipairs(s:spobs()) do
            local t = sp:tags()
            if t.hypergate and not t.ruined then
               table.insert( destinations, sp )
            end
         end
      end
   end
   local destnames = {}
   for i,d in ipairs(destinations) do
      table.insert( destnames, d:system():nameRaw() )
   end

   local inv = vec2.new(1,-1)
   local targetknown = false
   local mapw, maph = w-260, h-60
   --local mapfont = lg.newFont(32)
   local jumpx, jumpy, jumpl, jumpa = 0, 0, 0, 0
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
            lg.rectangle("fill", -jumpl*0.5*s, -5*s, jumpl*s, 10*s )
            lg.pop()
         end
      end,
   } )
   local function map_center( sys )
      local s = system.get( sys )
      targetknown = s:known()
      if targetknown then
         local p = (cpos + s:pos())*0.5
         jumpx, jumpy = (p*inv):get()
         jumpl, jumpa = ((s:pos()-cpos)*inv):polar()
         map:center( p )
      else
         jumpx, jumpy = 0, 0
         map.center( cpos )
      end
   end

   local lst = luatk.newList( wdw, w-200-20, 40, 200, h-180, destnames, map_center )

   luatk.newButton( wdw, w-120-20, h-40-20, 120, 40, _("Close"), luatk.close )

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
end

return hypergate
