local lg = require "love.graphics"
local lf = require "love.filesystem"
local audio = require "love.audio"
local love_shaders = require "love_shaders"
local starfield = require "bkg.lib.starfield"
local luaspfx = require "luaspfx"
local fmt = require "format"

local pixelcode = lf.read( "spob/lua/shaders/wormhole.frag" )
local jumpsfx = audio.newSoundData( 'snd/sounds/wormhole' )

-- Default parameters that can be overwritten
local SIZE = 256
local COL_INNER   = {0.2, 0.8, 1.0}
local COL_OUTTER  = {0.0, 0.8, 1.0}

local wormhole = {}

local function update_canvas ()
   local oldcanvas = lg.getCanvas()
   local oldshader = lg.getShader()
   lg.setShader( mem.shader )
   lg.setCanvas( mem.cvs )
   lg.clear( 0, 0, 0, 0 )
   lg.setColour( 1, 1, 1, 1 )
   love_shaders.img:draw( 0, 0, 0, mem.size, mem.size )
   lg.setShader( oldshader )
   lg.setCanvas( oldcanvas )
end

function wormhole.setup( target, params )
   params = params or {}
   if type(target)=='function' then
      mem.target = target
   else
      mem.target = spob.get(target)
   end
   mem.params = params
   mem.size = params.size or SIZE

   -- Hook up the API
   init     = wormhole.init
   load     = wormhole.load
   unload   = wormhole.unload
   update   = wormhole.update
   render   = wormhole.render
   can_land = wormhole.can_land
   land     = wormhole.land
end

function wormhole.init( spb )
   mem.spob = spb
end

function wormhole.load ()
   if type(mem.target)=='function' then
      mem._target = spob.get( mem.target() )
      if mem._target == nil then
         return warn(fmt.f(_("Wormhole '{spb}' target function failed to return a spob!"),
            {spb=mem.spob}))
      end
   else
      mem._target = mem.target
   end

   local sys = mem._target:system()
   if mem.shader==nil then
      -- Load shader
      local col_inner = mem.params.col_inner or COL_INNER
      local col_outter = mem.params.col_outter or COL_OUTTER
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
      mem.cvs = lg.newCanvas( mem.size, mem.size, {dpiscale=1} )
      mem.radius = mem.size*0.5

      -- Set up background texture
      local _nw, _nh, ns = gfx.dim()
      -- TODO have this actually render the real background, not just starfield
      -- so it works properly with Nebulas and other fancy backgrounds
      starfield.init{ seed=sys:nameRaw(), static=true, nolocalstars=true, size=mem.size*ns }
      mem.shader:send( "u_bgtex", starfield.canvas() )

      -- Only play sound if player exists (avoid on menu, etc...)
      if not player.pilot():exists() then
         mem.sfx = audio.newSource( 'snd/sounds/loops/wormhole' )
         local px, py = mem.pos:get()
         mem.sfx:setPosition( px, py, 0 )
         mem.sfx:setRelative(false)
         mem.sfx:setAttenuationDistances( 500, 25e3 )
         mem.sfx:setLooping(true)
         mem.sfx:play()
      end
      update_canvas()
   end
   return mem.cvs.t.tex, mem.radius
end

function wormhole.unload ()
   mem.shader= nil
   mem.cvs   = nil
   if mem.sfx then
      mem.sfx:stop()
   end
   mem.sfx   = nil
end

function wormhole.update( dt )
   mem.shader:update( dt )
end

function wormhole.render ()
   update_canvas() -- We want to do this here or it gets slow in autonav
   local z = camera.getZoom()
   local x, y = gfx.screencoords( mem.pos ):get()
   z = 1/z
   local r = mem.radius*z
   mem.cvs:draw( x-r, y-r, 0, z, z )
end

function wormhole.can_land ()
   if not mem._target then
      return false, _("Wormhole has no target!")
   end
   return true, _("The wormhole seems to be active.")
end

function wormhole.land( _s, p )
   if not mem._target then return end

   -- Avoid double landing
   if p:shipvarPeek( "wormhole" ) then return end
   p:shipvarPush( "wormhole", true )

   -- Not player, just play animation and remove
   if p ~= player.pilot() then
      p:effectAdd("Wormhole Enter")
      luaspfx.sfx( false, nil, jumpsfx )
      return
   end

   local nc = naev.cache()
   nc.wormhole_target = mem._target
   nc.wormhole_colour = mem.params.col_travel or mem.params.col_outter or COL_OUTTER
   naev.eventStart("Wormhole")
end

return wormhole
