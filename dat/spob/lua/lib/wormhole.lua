local lg = require "love.graphics"
local lf = require "love.filesystem"
local audio = require "love.audio"
local love_shaders = require "love_shaders"
local starfield = require "bkg.lib.starfield"
local luaspfx = require "luaspfx"

local pixelcode = lf.read( "spob/lua/glsl/wormhole.frag" )
local jumpsfx = audio.newSource( 'snd/sounds/wormhole.ogg' )

local s = 256

local wormhole = {}

local function update_canvas ()
   local oldcanvas = lg.getCanvas()
   local oldshader = lg.getShader()
   lg.setShader( mem.shader )
   lg.setCanvas( mem.cvs )
   lg.clear( 0, 0, 0, 0 )
   lg.setColor( 1, 1, 1, 1 )
   lg.setBlendMode( "alpha", "premultiplied" )
   love_shaders.img:draw( 0, 0, 0, s, s )
   lg.setBlendMode( "alpha" )
   lg.setShader( oldshader )
   lg.setCanvas( oldcanvas )
end

function wormhole.init( spb, target, params )
   params = params or {}
   mem.spob = spb
   mem.target = target
   mem.params = params
end

function wormhole.load ()
   local _spob, sys = spob.getS( mem.target )
   if mem.shader==nil then
      -- Load shader
      local col_inner = mem.params.col_inner or {0.2, 0.8, 1.0}
      local col_outter = mem.params.col_outter or {0.0, 0.8, 1.0}
      local pcode = string.format( pixelcode,
         col_inner[1], col_inner[2], col_inner[3],
         col_outter[1], col_outter[2], col_outter[3] )
      mem.shader = lg.newShader( pcode, love_shaders.vertexcode )
      mem.shader._dt = -1000 * rnd.rnd()
      mem.shader.update = function( self, dt )
         self._dt = self._dt + dt
         self:send( "u_time", self._dt )
      end
      mem.pos = mem.spob:pos()
      mem.pos = mem.pos + vec2.new( -s/2, s/2 )
      mem.cvs = lg.newCanvas( s, s, {dpiscale=1} )

      -- Set up background texture
      local _nw, _nh, ns = gfx.dim()
      -- TODO have this actually render the real background, not just starfield
      -- so it works properly with Nebulas and other fancy backgrounds
      starfield.init{ seed=sys:nameRaw(), static=true, nolocalstars=true, size=s*ns }
      mem.shader:send( "u_bgtex", starfield.canvas() )

      mem.sfx = audio.newSource( 'snd/sounds/loops/wormhole.ogg' )
      mem.sfx:setRelative(false)
      local px, py = mem.pos:get()
      mem.sfx:setPosition( px, py, 0 )
      mem.sfx:setAttenuationDistances( 500, 25e3 )
      mem.sfx:setLooping(true)
      mem.sfx:play()
      update_canvas()
   end
   return mem.cvs.t.tex, s/2
end

function wormhole.unload ()
   mem.shader= nil
   mem.cvs   = nil
   mem.sfx   = nil
end

function wormhole.update( dt )
   mem.shader:update( dt )
end

function wormhole.render ()
   update_canvas() -- We want to do this here or it gets slow in autonav
   local z = camera.getZoom()
   local x, y = gfx.screencoords( mem.pos, true ):get()
   z = 1/z
   mem.cvs:draw( x, y, 0, z, z )
end

function wormhole.can_land ()
   return true, _("The wormhole seems to be active.")
end

function wormhole.land( _s, p )
   -- Avoid double landing
   if p:shipvarPeek( "wormhole" ) then return end
   p:shipvarPush( "wormhole", true )

   -- Not player, just play animation and remove
   if p ~= player.pilot() then
      p:effectAdd("Wormhole Enter")
      luaspfx.sfx( p:pos(), p:vel(), jumpsfx )
      return
   end

   var.push( "wormhole_target", mem.target )
   naev.eventStart("Wormhole")
end

return wormhole
