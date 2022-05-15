local lg = require "love.graphics"
local lf = require "love.filesystem"
local audio = require "love.audio"
local love_shaders = require "love_shaders"
local starfield = require "bkg.lib.starfield"
local luaspfx = require "luaspfx"

local pixelcode = lf.read( "spob/lua/glsl/wormhole.frag" )
local jumpsfx = audio.newSource( 'snd/sounds/wormhole.ogg' )

local cvs, shader, pos, target, sfx
local s = 256

local wormhole = {}

local function update_canvas ()
   local oldcanvas = lg.getCanvas()
   local oldshader = lg.getShader()
   lg.setShader( shader )
   lg.setCanvas( cvs )
   lg.clear( 0, 0, 0, 0 )
   lg.setColor( 1, 1, 1, 1 )
   lg.setBlendMode( "alpha", "premultiplied" )
   love_shaders.img:draw( 0, 0, 0, s, s )
   lg.setBlendMode( "alpha" )
   lg.setShader( oldshader )
   lg.setCanvas( oldcanvas )
end

function wormhole.load( p, wormhole_target )
   local _spob, sys = spob.getS( wormhole_target )
   target = wormhole_target
   if shader==nil then
      -- Load shader
      shader = lg.newShader( pixelcode, love_shaders.vertexcode )
      shader._dt = -1000 * rnd.rnd()
      shader.update = function( self, dt )
         self._dt = self._dt + dt
         self:send( "u_time", self._dt )
      end
      pos = p:pos()
      pos = pos + vec2.new( -s/2, s/2 )
      cvs = lg.newCanvas( s, s, {dpiscale=1} )

      -- Set up background texture
      local _nw, _nh, ns = gfx.dim()
      starfield.init{ seed=sys:nameRaw(), static=true, nolocalstars=true, size=s*ns }
      shader:send( "u_bgtex", starfield.canvas() )

      sfx = audio.newSource( 'snd/sounds/loops/wormhole.ogg' )
      sfx:setRelative(false)
      local px, py = p:pos():get()
      sfx:setPosition( px, py, 0 )
      sfx:setAttenuationDistances( 500, 25e3 )
      sfx:setLooping(true)
      sfx:play()
      update_canvas()
   end
   return cvs.t.tex, s/2
end

function wormhole.unload ()
   shader= nil
   cvs   = nil
   sfx   = nil
end

function wormhole.update( dt )
   shader:update( dt )
end

function wormhole.render ()
   update_canvas() -- We want to do this here or it gets slow in autonav
   local z = camera.getZoom()
   local x, y = gfx.screencoords( pos, true ):get()
   z = 1/z
   cvs:draw( x, y, 0, z, z )
end

function wormhole.can_land ()
   return true, "The wormhole seems to be active."
end

function wormhole.land( _s, p )
   -- Avoid double landing
   if p:shipvarPeek( "wormhole" ) then return end
   p:shipvarPush( "wormhole", true )

   -- Not player, just play animation and remove
   if p ~= player.pilot() then
      p:effectAdd("Wormhole Enter")
      luaspfx.sfx( 10, p:pos(), jumpsfx )
      return
   end

   var.push( "wormhole_target", target )
   naev.eventStart("Wormhole")
end

return wormhole
